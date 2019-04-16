#include "../../include/processes/processes.h"

#define PRESET_ALTITUDE_MIN 18000
#define PRESET_ALTITUDE_MAX 22000
#define PRESET_DURATION_OPEN 5*60*1000 // 5 minutes

#define NUM_OF_CHECKS_BEFORE_OPEN 40 //the number of times the GPS must confirm altitude to open the valve
#define LOG_FREQUENCY 500 //time in milliseconds between logging sensor data
#define ASCENT_CALC_FREQUENCY 10 //this is the number of times we will log data before we will recalculate the ascent velocity that will be sent to the xbee
  //this is used to smooth out the data--if this were updated every loop, the values would not deviate smoothly
  //Add a proper kalman filter to do this in the future

void xbeeCommand(void);

//Control Parameters
int32_t minAltitudeToOpen = PRESET_ALTITUDE_MIN; //meters  //the altitude at which to open - this can be changed only via an Xbee command
int32_t maxAltitudeToOpen = PRESET_ALTITUDE_MAX; //meters   //the max altitude at which to open - this is used to prevent from opening twice
int32_t durationOpen = PRESET_DURATION_OPEN; //milliseconds  //the amount of time to stay open - this can be changed only via an Xbee command

//For ascent velocity calculation
float ascentVelocity = 0.0; //this persistent value is updated every ASCENT_VELOCITY_CALC loops, and is sent via xbee whenever requested
int32_t oldAltitude = 0;  //this is the most recent altitude used for the calculations - i.e. dy = (gps.altitude - oldAltitude)
unsigned long oldTime = 0;  //this is the most recent time used for the calculations - i.e. dx = (millis() - oldTime)
uint8_t logsCounter = 0;  //counts the number of loops since the last time ascent velocity was updated, resets every time it equals ASCENT_VELOCITY_CALC

//For log timing
uint32_t millisLastLog = 0; //keeps track of when data was last logged so that it is logged at nearly every interval specified by LOG_FREQUENCY
boolean checkAltThisLoop = 0; //used to add in a delay for other functions, such that they only occur as frequently as the log operates
  //i.e. this will be false all the time except for during the first loop immediately after data is logged

//For xbee command timing
uint32_t millisLastCommandReceived= 0;  //keeps track of when the last command was received, commands received within myBITS.WAIT_TIME_AFTER_COMMAND will be ignored and assumed duplicates

#define LED_ARMED ledArmed.RED
#define LED_DISARMED ledArmed.GREEN
#define LED_OPEN ledArmed.YELLOW
#define LED_NO_FIX ledStat.YELLOW
#define LED_YES_FIX ledStat.GREEN
boolean statusLEDon = false;
char LEDStatusColor = LED_NO_FIX;

void sFlight() {
  delay(5000); //wait to initialize so we can connect anything we might need to

  Serial.begin(115200); //start communication with computer

  ledStat.initialize();
  ledStat.setStatus(ledStat.YELLOW);  //yellow indicates power on and starting up
  ledArmed.initialize();
  ledArmed.setStatus(LED_ARMED); //green indicates that the system is currently armed

  if(!xbee.initialize()){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }
  else{
    ledStat.setStatus(ledStat.BLUE);  //blue indicates the xbee is functional, likely the most important component
    delay(2000);
  }

  actuator.initialize();
  motor.initialize();

  pinMode(ACT2_READ, INPUT_PULLUP);
  if (digitalRead(ACT2_READ) == LOW){ //Only actuate on startup if the switch is set to do so.
    //open and close valve
    actuator.openValve();
    while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
    actuator.closeValve();
    while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
    actuator.stopValve();

    //Turn fan on and off
    motor.startFan();
    delay(2000);
    motor.stopFan();
  }
  valve.state = armed;

  honeywell.initialize(allData.honeywellBalloonData, allData.honeywellAtmosphereData);

  if(!bme.initialize(allData.bmeBalloonData, allData.bmeAtmosphereData)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }

  //implement something to check that all the pressure sensors are recording the correct data

  if(!datalog.initialize()){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }
  else{
    ledStat.setStatus(ledStat.WHITE);
    delay(2000);
  }

  //Write header string on the SD card
  if (!datalog.write(HEADER_STRING)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }

#if (USING_GPS)
  if(!gps.initialize(allData.gpsData)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }
  //wait for the gps to get a fix before starting up
  //while(!gpsData.fix) gps.read(&gpsData);
  ledStat.setStatus(ledStat.MAGENTA); //magenta is a reminder that the gps is active
  delay(2000);
#else
  allData.gpsData = {};
#endif

  for (int i=0; i<10; i++){
    ledStat.setStatus(ledStat.OFF);
    delay(500);
    ledStat.setStatus(LEDStatusColor);
    delay(500); //led blinks green to confirm it has finished setup successfully; led will turn off once there is a gps fix
  }
  if(HELIOS_DEBUG) Serial.println("Starting Main Loop");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void lFlight() {

#if (USING_GPS)
  gps.read(&allData.gpsData);
#endif

  if (allData.gpsData.fix)  //the led will turn off once the GPS has a fix
    LEDStatusColor = LED_YES_FIX;
  else
    LEDStatusColor = LED_NO_FIX;

  //stop actuator if it is moving and about to hit its endpoints
  if (valve.state==open){
    if (actuator.position() < actuator.START) //if valve has finished opening, turn it off
      actuator.stopValve();
    else
      actuator.openValve();
  } else{
    if (actuator.position() > actuator.END) //if valve has finished closing, turn it off
      actuator.stopValve();
    else
      actuator.closeValve();
  }
  
  if(xbee.checkForMessage() != xbee.NO_PACKET && (millis() - millisLastCommandReceived) > xbee.WAIT_TIME_AFTER_COMMAND){ //if the xbee does receive a command
    xbeeCommand();  //exectue separate function that handles the command
    millisLastCommandReceived = millis(); //set the time at which the last command was received to prevent duplicates
  }

  //log data
  checkAltThisLoop = false;
  if ((millis() - millisLastLog) > LOG_FREQUENCY){ //if it has been LOG_FREQUENCY milliseconds since last log
    honeywell.read(&allData.honeywellBalloonData, honeywell.INDEX_BALLOON);
    honeywell.read(&allData.honeywellAtmosphereData, honeywell.INDEX_ATMOS);
    bme.read(&allData.bmeBalloonData, bme.INDEX_BALLOON);
    bme.read(&allData.bmeAtmosphereData, bme.INDEX_ATMOS);
    millisLastLog = millis();
    logData(datalog, allData, valve, actuator.position());  //execute separate function to log data
    checkAltThisLoop = true;
    if (statusLEDon){
      ledStat.setStatus(ledStat.OFF);
      statusLEDon = false;
    }
    else{
      ledStat.setStatus(LEDStatusColor);
      statusLEDon = true;
    }
  }

  //Opens the plunger and expells helium at ALTIDUDE_TO_OPEN meters for durationOpen seconds
  if(valve.state != disarmed){//if the valve has not already been opened and closed
    if(allData.gpsData.altitude > minAltitudeToOpen && allData.gpsData.altitude < maxAltitudeToOpen && checkAltThisLoop){
      //if we are above the target altitude and we just logged data (I added the checkAltThisLoop variable so that this takes a few seconds of steady measurements to become true)
      valve.numAltitudeChecks++;//add to the altitude verifier
    }
    if(valve.numAltitudeChecks >= NUM_OF_CHECKS_BEFORE_OPEN && valve.state == armed){//if it is time to open the valve
      valve.state = open; //it won't actually open until the start of the next loop
      motor.startFan();
      valve.millisWhenOpened = millis(); //set the time at open so we know when to close the valve
      ledArmed.setStatus(LED_OPEN); //red means that the valve is currently open
    }
    else if(valve.state == open && millis() > (valve.millisWhenOpened + durationOpen)){//if we've been open long enough
      valve.state = disarmed;
      motor.stopFan();  //stop the fan and close the valve
      ledArmed.setStatus(LED_DISARMED); //off means that the system is no longer armed
    }
  }
  //Serial.println("Loop A");
}


void xbeeCommand(){
  if (xbee.getLastCommand() == xbee.COMMAND_ENABLE_VENT){
    valve.state = armed; //reset all control variables to false to allow the valve to reopen
    valve.numAltitudeChecks = 0;
    String str = "Confirm valve enable: " + (String)durationOpen + " ms at " + minAltitudeToOpen + " m altitude.";
    xbee.sendToGround(str);  //send confirmation codes that contain the time and altitude at which the valve will open
    ledArmed.setStatus(LED_ARMED); //indicates that the system is rearmed
    if (HELIOS_DEBUG) Serial.println("Xbee command venting enabled.");
  }else if(xbee.getLastCommand() == xbee.COMMAND_REQUEST_DATA){ //request data command
    String str = "Altitude: " + (String)allData.gpsData.altitude + " m, Ascent Velocity: " + (String)ascentVelocity + " m/s";
    xbee.sendToGround(str);  //send the most recent calculated ascent velocity and altitude to the xbee
    if(HELIOS_DEBUG) Serial.println("Flight data sent via xbee");
  }else if (xbee.getLastCommand() == xbee.COMMAND_ABORT_VALVE){  //an abort command
    valve.state = disarmed;
    motor.stopFan();  //stop the fan
    actuator.closeValve();  //close the valve
    String str = "Confirm Helios has aborted venting";
    xbee.sendToGround(str);
    ledArmed.setStatus(LED_DISARMED);
    if(HELIOS_DEBUG) Serial.println("xbee has commanded venting abort");
  }else if (xbee.getLastCommand() == xbee.COMMAND_VENT_NOW){  //an open valve now command
    valve.state = open;
    actuator.openValve(); //open the valve and start the fan
    motor.startFan();
    //durationOpen = xbee.getCommandedTime(); //reset the commanded time to be open
    valve.millisWhenOpened = millis(); //set the time at open so we know when to close it
    String str = "Valve commanded open for " + (String) durationOpen + " ms.";
    xbee.sendToGround(str);
    ledArmed.setStatus(LED_OPEN);
    if (HELIOS_DEBUG) Serial.println("Helios will open valve for " + (String)durationOpen + " milliseconds");
  }/*else if (xbee.getLastCommand() == xbee.COMMAND_SET_TIME){  //command to reset the amount of time to stay open
    durationOpen = xbee.getCommandedTime();
    xbee.sendConf(xbee.CONFIRM_CODE_SET_VAR, durationOpen); //send confirmation message confirming the time received
  }else if (xbee.getLastCommand() == xbee.COMMAND_SET_ALT){ //command to change the altitude to stay open
    minAltitudeToOpen = xbee.getCommandedTime();
    xbee.sendConf(xbee.CONFIRM_CODE_SET_VAR, minAltitudeToOpen);  //send confirmation message confirming the altitude received
  }else if (xbee.getLastCommand() == xbee.COMMAND_REVERSE_NOW){ //command to open the valve and turn the fan in reverse
    valve.state = open;
    actuator.openValve(); //open valve and start fan in reverse
    motor.reverseFan();
    durationOpen = xbee.getCommandedTime();
    valve.millisWhenOpened = millis();
    xbee.sendConf(xbee.CONFIRM_CODE_REVERSE, durationOpen); //send confirmation code along with the amount of time commanded
    if(HELIOS_DEBUG) Serial.println("Helios will run fan in reverse for " + (String)durationOpen + " seconds");
    ledArmed.setStatus(LED_OPEN);
  }*/
  else if (xbee.getLastCommand() == xbee.COMMAND_TEST_OPEN){ //the next four commands are to control the payload without consideration of other code parameters
    actuator.openValve();
    valve.state = open;
    // Fix this section so it stays open indefinitely
    String str = "Confirmed valve manually opened.";
    xbee.sendToGround(str);
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_CLOSE){
    actuator.closeValve();
    // Fix this section so it can close without becoming armed or disarmed
    xbee.sendToGround("Confirmed valve manually closed.");
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_FWD){
    motor.startFan();
    xbee.sendToGround("Confirmed fan turned on forward.");
  }/*else if (xbee.getLastCommand() == xbee.COMMAND_TEST_REV){
    motor.reverseFan();
    xbee.sendConf(xbee.CONFIRM_CODE_TEST, xbee.CONFIRM_STATE_REV);
  }
  else if (xbee.getLastCommand() == xbee.COMMAND_RESET){
    // figure out the code required to reset a system
    xbee.sendToGround("This is null command. Please try again.");
  }else if (xbee.getLastCommand() == xbee.COMMAND_ALL_DATA){ //send long list of data
    xbee.sendAllData(allData.gpsData.altitude, allData.gpsData.latitude_deg, allData.gpsData.latitude_min, allData.gpsData.longitude_deg, allData.gpsData.longitude_min, ascentVelocity, allData.honeywellBalloonData.pressure,
        allData.honeywellAtmosphereData.pressure, allData.honeywellBalloonData.temperature, allData.honeywellAtmosphereData.temperature, actuator.position());
    if(HELIOS_DEBUG) Serial.println("xbee long data sent");
  }*/
  datalog.write("XBEE COMMAND RECEIVED: " + (String)(xbee.getLastCommand()));  //log whatever command was received to the datalog
}
