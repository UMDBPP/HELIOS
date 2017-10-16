/*
 * Code currently intended for Mega with custom shield
 * Should also work on Balloonduino with a few pin number changes
 */
#define USING_GPS true //turning this false will tell the compiler to ignore anything involving the GPS, which is sometimes annoying during other tests
  //don't forget that this must also be declared in the GPS library header file
#define HELIOS_DEBUG true //this makes every function output what it is doing to serial as well, this can be individually enabled/disabled for every file
#define DEBUG_MODE true //this makes the main code ignore the main setup and loop and instead follow an alternative code sequence

//Import Custom Libraries
#include "myPins.h" //defines pin numbers for use elsewhere
#include "myGPS.h"  //defines functions for interacting with GPS and handles all the Adafruit GPS library nonsense
#include "mySSC.h"  //defines functions for interacting with Honeywell SSC Pressure sensors
#include "myLED.h"  //defines functions for changing LED color
#include "myMotor.h"  //defines functions for handling fan motor
#include "myXbee.h" //defines functions for interacting with xbees - note that this libary is just about communications, the actual command handling is at the end of this file
#include "myAct.h"  //defines functions for extending and reading the actuator
#include "myLog.h"  //defines functions for interacting with SD card logging
//#include "myBME.h"  //defines functions for working with BME280 pressure sensors

#include<Wire.h>  //Required for I2C communication with SSC and BME sensors

//First line printed identifies what each data column is
/*#define HEADER_STRING "Starting:\nYear,Month,Day,Hour,Minute,Second,Millisecond,Latitude_deg,Latitude_min,Latitude_dir,Longitude_deg,Longitude_min,Longitude_dir,
    Velocity,Angle,Altitude,Num_Satellites,In_Pressure,In_Temperature,In_Status,In_Pressure_Raw,In_Temperature_Raw,Out_Pressure,Out_Temperature,Out_Status,Out_Pressure_Raw,
    Out_Temperature_Raw,valveHasOpened,Valve_Closed"
*/

//Control Parameters
int32_t altitudeToOpen = 18000; //meters  //the altitude at which to open - this can be changed only via an Xbee command
int32_t maxAltitudeToOpen = 22000; //meters   //the max altitude at which to open - this is used to prevent from opening twice
int32_t timeOpen = 60000; //milliseconds  //the amount of time to stay open - this can be changed only via an Xbee command
#define NUM_OF_CHECKS_BEFORE_OPEN 40 //the number of times the GPS must confirm altitude to open the valve
#define LOG_FREQUENCY 500 //time in milliseconds between logging sensor data
#define ASCENT_CALC_FREQUENCY 10 //this is the number of times we will log data before we will recalculate the ascent velocity that will be sent to the xbee
  //this is used to smooth out the data--if this were updated every loop, the values would not deviate smoothly

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Global Variables
//For Valve
unsigned long valveTimeAtOpen=400000000; //this variable becomes the time from millis() at which the valve opens (it is initialized to a large value because an older version of the code required as such)
uint8_t valveAltitudeCheckCounter=0;  //the number of times the GPS has read a value above altitudeToOpen
boolean valveHasOpened=0; //becomes true as soon as the valve first opens, becomes false after a re-enable command
boolean valveAlreadyClosed=0; //becomes true as soon as the valve is closed after being opened
boolean valveIsOpen; //true for open, false for closed

//For ascent velocity calculation
float ascentVelocity = 0.0; //this persistent value is updated every ASCENT_VELOCITY_CALC loops, and is sent via xbee whenever requested
int32_t oldAltitude = 0;  //this is the most recent altitude used for the calculations - i.e. dy = (gps.altitude - oldAltitude)
unsigned long oldTime = 0;  //this is the most recent time used for the calculations - i.e. dx = (millis() - oldTime)
uint8_t logsCounter = 0;  //counts the number of loops since the last time ascent velocity was updated, resets every time it equals ASCENT_VELOCITY_CALC

//Data structures
myHoneywellData honeywellData[2]; //structure for storing honeywell data
myGPSData gpsData;  //structure for storing gps data separate of the adafruit gps class

//For log timing
uint32_t log_timer = 0; //keeps track of when data was last logged so that it is logged at nearly every interval specified by LOG_FREQUENCY
boolean logged_now = 0; //used to add in a delay for other functions, such that they only occur as frequently as the log operates
  //i.e. this will be false all the time except for during the first loop immediately after data is logged

//For xbee command timing
uint32_t command_timer= 0;  //keeps track of when the last command was received, commands received within myXbee.WAIT_TIME_AFTER_COMMAND will be ignored and assumed duplicates

//Create objects
myLED led(LED1_PINR, LED1_PING, LED1_PINB); //create an LED object for status purposes
myLED armed(LED2_PINR, LED2_PING, LED2_PINB); //create an LED object to indicate if the system is armed
myXbee xbee;  //create an Xbee object
myActuator actuator;  //create an Actuator object
myDatalog datalog;  //create an SD Card logger object
myHoneywell honeywell;  //create a honeywell SSC sensor and multiplexer module
myMotor motor;  //create a motor module
#if (USING_GPS) //The GPS is often troublesome, so we nest this inside an if statement so it can be readily removed for other testing
  myGPS gps;  //if using gps, create a GPS module
#endif

#define LED_ARMED armed.GREEN
#define LED_DISARMED armed.OFF
#define LED_OPEN armed.RED

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if DEBUG_MODE  //if we are in debug mode, skip turning on everything and just follow this abbreviated code

void setup(){
  delay(5000);
  Serial.begin(115200);
  led.initialize();
  armed.initialize();
  actuator.initialize();
  motor.initialize();
  //if(!xbee.initialize()) Serial.println("XBEE Error");
  //if(!datalog.initialize()) Serial.println("Log error");
  //if(!gps.initialize(&gpsData)) Serial.println("GPS Error");
  //if(!honeywell.initialize(&honeywellData[0], &honeywellData[1])) Serial.println("Honeywell Error");
  delay(1000);
  valveIsOpen = actuator.openValve();
}

void loop(){
  Serial.println(actuator.position());
  if (valveIsOpen && actuator.position() < actuator.START){ //if valve has finished opening, turn it off
    actuator.stopValve();
    valveIsOpen = actuator.closeValve();
    motor.stopFan();
  }
  if (!valveIsOpen && actuator.position() > actuator.END){ //if valve has finished closing, turn it off
    actuator.stopValve();
    valveIsOpen = actuator.openValve();
    motor.startFan();
  }
  //honeywell.read(&honeywellData[1], 1);
  //Serial.print(honeywellData[1].pressure);
  //Serial.print("   ");
  //Serial.print(honeywellData[1].temperature);
  //Serial.print("   ");
  //Serial.println(honeywellData[1].el);
  //delay(250);
  //datalog.write("the logger is working"); delay(2000);
  //gps.read(&gpsData);
  //if (gpsData.fix) led.setStatus(led.GREEN);
  //Serial.println(Serial.println(gpsData.altitude));
  //armed.setStatus(led.RED); delay(1000);
  //armed.setStatus(led.BLUE); delay(1000);
  //armed.setStatus(led.GREEN); delay(1000);
  //armed.setStatus(led.OFF); delay(2000);
  /*honeywell.read(&honeywellData[honeywell.TCA_INSIDE_SENSOR], honeywell.TCA_INSIDE_SENSOR);
  Serial.print("Inside: ");
  Serial.println(honeywellData[honeywell.TCA_INSIDE_SENSOR].pressure, DEC);
  honeywell.read(&honeywellData[honeywell.TCA_OUTSIDE_SENSOR], honeywell.TCA_OUTSIDE_SENSOR);
  Serial.print("Outside: ");
  Serial.println(honeywellData[honeywell.TCA_OUTSIDE_SENSOR].pressure, DEC);
  /*
  if(xbee.receive() && (millis() - command_timer) > xbee.WAIT_TIME_AFTER_COMMAND){
    xbeeCommand();
    command_timer = millis();
  }
  //gps.read(&gpsData, &GPS);
  uint32_t start = millis();
  actuator.openValve();
  while (actuator.position() > 1.0)
    Serial.println(actuator.position());
  actuator.stopValve();
  led.setStatus(led.RED);
  start = millis();
  actuator.closeValve();
  while (actuator.position() < 49.0)
    Serial.println(actuator.position());
  actuator.stopValve();
  motor.startFan();
  delay(5000);
  motor.stopFan();
  led.setStatus(led.GREEN);*/
}

#else //if we are not in debug mode, then run normal sequence

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  delay(5000); //wait to initialize so we can connect anything we might need to
  
  Serial.begin(115200); //start communication with computer

  led.initialize();
  led.setStatus(led.YELLOW);  //yellow indicates power on and starting up
  armed.initialize();
  armed.setStatus(LED_ARMED); //green indicates that the system is currently armed

  if(!xbee.initialize()){
    led.setStatus(led.RED);
    delay(5000);
  }
  else
    led.setStatus(led.BLUE);  //blue indicates the xbee is functional, likely the most important component
  
  actuator.initialize();
  
  motor.initialize();

  pinMode(ACT2_READ, INPUT);
  if (digitalRead(ACT2READ) == HIGH){ //Only actuate on startup if the switch is set to do so.
    //open and close valve
    valveIsOpen = actuator.openValve();
    while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
    valveIsOpen = actuator.closeValve();
    while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
    actuator.stopValve();*/
  
    //Turn fan on and off
    motor.startFan();
    delay(2000);
    motor.stopFan();*/
  }

  honeywell.initialize(&honeywellData[0], &honeywellData[1])){

  /*if(!bme.initialize())
    led.setStatus(led.RED);*/

  //implement something to check that all the pressure sensors are recording the correct data

  if(!datalog.initialize()){
    led.setStatus(led.RED);
    delay(5000);
  }
  else
    led.setStatus(led.WHITE);
  
  //Write header string on the SD card
  if (!datalog.write("Header string goes here")){
    led.setStatus(led.RED);
    delay(5000);
  }

#if (USING_GPS)
  if(!gps.initialize(&gpsData)){
    led.setStatus(led.RED);
    delay(5000);
  }
  //wait for the gps to get a fix before starting up
  //while(!gpsData.fix) gps.read(&gpsData);
  led.setStatus(led.MAGENTA); //magenta is a reminder that the gps is active
  delay(2000);
#else
  gpsData = {};
#endif
  
  for (int i=0; i<10; i++){
    led.setStatus(led.OFF);
    delay(500);
    led.setStatus(led.GREEN);
    delay(500); //led blinks green to confirm it has finished setup successfully; led will turn off once there is a gps fix
  }
  if(HELIOS_DEBUG) Serial.println("Starting Main Loop");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  /*if (millis()/1000 > loopNum){
    loopNum = millis()/1000;
    Serial.println(availableMemory());
  }*/
  
#if (USING_GPS)
  gps.read(&gpsData);
#endif

  if (gpsData.fix)  //the led will turn off once the GPS has a fix
    led.setStatus(led.OFF);
    
  //stop actuator if it is moving and about to hit its endpoints
  if (valveIsOpen && actuator.position() < actuator.START) //if valve has finished opening, turn it off
    actuator.stopValve();
  if (!valveIsOpen && actuator.position() > actuator.END) //if valve has finished closing, turn it off
    actuator.stopValve();
  
  if(xbee.receive() && (millis() - command_timer) > xbee.WAIT_TIME_AFTER_COMMAND){ //if the xbee does receive a command
    xbeeCommand();  //exectue separate function that handles the command
    command_timer = millis(); //set the time at which the last command was received to prevent duplicates
  }
  
  //log data
  logged_now = false;
  if ((millis() - log_timer) > LOG_FREQUENCY){ //if it has been LOG_FREQUENCY milliseconds since last log
    logData();  //execute separate function to log data
    logged_now = true;
  }
  
  //Opens the plunger and expells helium at ALTIDUDE_TO_OPEN meters for timeOpen seconds
  if(!valveAlreadyClosed){//if the valve has not already been opened and closed
    if(gpsData.altitude > altitudeToOpen && gpsData.altitude < maxAltitudeToOpen && logged_now){
      //if we are above the target altitude and we just logged data (I added the logged_now variable so that this takes a few seconds of steady measurements to become true)
      valveAltitudeCheckCounter++;//add to the altitude verifier
    }
    if(valveAltitudeCheckCounter >= NUM_OF_CHECKS_BEFORE_OPEN && !valveHasOpened){//if it is time to open the valve
      actuator.openValve();//this causes a 3 second delay where no data is taken.
      motor.startFan();
      valveTimeAtOpen = millis(); //set the time at open so we know when to close the valve
      valveHasOpened=1; //set that the valve has opened so that it can also be closed
      armed.setStatus(LED_OPEN); //red means that the valve is currently open
    }
    else if(valveHasOpened && millis() > (valveTimeAtOpen + timeOpen)){//if we've been open long enough
      valveAlreadyClosed=1; //set this to true so that the valve can never be reopened
      motor.stopFan();  //stop the fan and close the valve
      valveIsOpen = actuator.closeValve();
      armed.setStatus(LED_DISARMED); //off means that the system is no longer armed
    }
  }
  //Serial.println("Loop A");
}

#endif  //this ends the if/else sequence used while debugging

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Below are assisting functions which are included in this file because they have access to global variables

void xbeeCommand(){
  if (xbee.getLastCommand() == xbee.COMMAND_ENABLE){  //an enable command
    valveAlreadyClosed = 0; //reset all control variables to false to allow the valve to reopen
    valveAltitudeCheckCounter = 0;
    valveHasOpened = 0;
    xbee.sendConf(xbee.CONFIRM_CODE_ENABLE, timeOpen);  //send confirmation codes that contain the time and altitude at which the valve will open
    xbee.sendConf(xbee.CONFIRM_CODE_ENABLE, altitudeToOpen);
    armed.setStatus(LED_ARMED); //indicates that the system is rearmed
  }else if(xbee.getLastCommand() == xbee.COMMAND_REQUEST_DATA){ //request data command
    xbee.sendData(gpsData.altitude, ascentVelocity);  //send the most recent calculated ascent velocity and altitude to the xbee
    if(HELIOS_DEBUG) Serial.println("xbee data sent");
  }
  else if (xbee.getLastCommand() == xbee.COMMAND_ABORT){  //an abort command
    valveAlreadyClosed = 1; //prevent the valve from opening
    motor.stopFan();  //stop the fan
    valveIsOpen = actuator.closeValve();  //close the valve
    xbee.sendConf(xbee.CONFIRM_CODE_ABORT, 0);  //send a code confirming that the abort command was received
    if(HELIOS_DEBUG) Serial.println("xbee has commanded abort");
    armed.setStatus(LED_DISARMED);
  }else if (xbee.getLastCommand() == xbee.COMMAND_VENT_NOW){  //an open valve now command
    valveAlreadyClosed = 0; //reset this parameter so that the code is capable of closing the valve
    valveIsOpen = actuator.openValve(); //open the valve and start the fan
    motor.startFan();
    timeOpen = xbee.getCommandedTime(); //reset the commanded time to be open
    valveTimeAtOpen = millis(); //set the time at open so we know when to close it
    valveHasOpened = 1;
    xbee.sendConf(xbee.CONFIRM_CODE_VENT, timeOpen); //send confirmation code along with the commanded time
    if (HELIOS_DEBUG) Serial.println("Helios will open valve for " + (String)timeOpen + " milliseconds");
    armed.setStatus(LED_OPEN);
  }else if (xbee.getLastCommand() == xbee.COMMAND_SET_TIME){  //command to reset the amount of time to stay open
    timeOpen = xbee.getCommandedTime();
    xbee.sendConf(xbee.CONFIRM_CODE_SET_VAR, timeOpen); //send confirmation message confirming the time received
  }else if (xbee.getLastCommand() == xbee.COMMAND_SET_ALT){ //command to change the altitude to stay open
    altitudeToOpen = xbee.getCommandedTime();
    xbee.sendConf(xbee.CONFIRM_CODE_SET_VAR, altitudeToOpen);  //send confirmation message confirming the altitude received
  }else if (xbee.getLastCommand() == xbee.COMMAND_REVERSE_NOW){ //command to open the valve and turn the fan in reverse
    valveAlreadyClosed = 0; //reset this parameter so the valve is capable of closing
    valveIsOpen = actuator.openValve(); //open valve and start fan in reverse
    motor.reverseFan();
    timeOpen = xbee.getCommandedTime();
    valveTimeAtOpen = millis();
    valveHasOpened = 1;
    xbee.sendConf(xbee.CONFIRM_CODE_REVERSE, timeOpen); //send confirmation code along with the amount of time commanded
    if(HELIOS_DEBUG) Serial.println("Helios will run fan in reverse for " + (String)timeOpen + " seconds");
    armed.setStatus(LED_OPEN);
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_OPEN){ //the next four commands are to control the payload without consideration of other code parameters
    actuator.openValve();
    xbee.sendConf(xbee.CONFIRM_CODE_TEST, xbee.CONFIRM_STATE_OPEN);
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_CLOSE){
    actuator.closeValve();
    xbee.sendConf(xbee.CONFIRM_CODE_TEST, xbee.CONFIRM_STATE_CLOSE);
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_FWD){
    motor.startFan();
    xbee.sendConf(xbee.CONFIRM_CODE_TEST, xbee.CONFIRM_STATE_FWD);
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_REV){
    motor.reverseFan();
    xbee.sendConf(xbee.CONFIRM_CODE_TEST, xbee.CONFIRM_STATE_REV);
  }else if (xbee.getLastCommand() == xbee.COMMAND_RESET){
    // figure out the code required to reset a system
    xbee.sendConf(xbee.CONFIRM_CODE_KILL, 0);
  }else if (xbee.getLastCommand() == xbee.COMMAND_ALL_DATA){ //send long list of data
    xbee.sendAllData(gpsData.altitude, gpsData.latitude_deg, gpsData.latitude_min, gpsData.longitude_deg, gpsData.longitude_min, ascentVelocity, honeywellData[honeywell.TCA_INSIDE_SENSOR].pressure, 
        honeywellData[honeywell.TCA_OUTSIDE_SENSOR].pressure, honeywellData[honeywell.TCA_INSIDE_SENSOR].temperature, honeywellData[honeywell.TCA_OUTSIDE_SENSOR].temperature, actuator.position());
    if(HELIOS_DEBUG) Serial.println("xbee long data sent");
  }
  datalog.write("XBEE COMMAND RECEIVED: " + (String)(xbee.getLastCommand()) + ", " + (String)(xbee.getCommandedTime()));  //log whatever command was received to the datalog
}

void logData(){
  honeywell.read(&honeywellData[honeywell.TCA_INSIDE_SENSOR], honeywell.TCA_INSIDE_SENSOR);
  honeywell.read(&honeywellData[honeywell.TCA_OUTSIDE_SENSOR], honeywell.TCA_OUTSIDE_SENSOR);
  /*getBME(2);
  getBME(3);
  getBME(4);*/
  logsCounter++;
  if (logsCounter == ASCENT_CALC_FREQUENCY){ //send data less frequently to the trinket
    ascentVelocity = 1.0*(gpsData.altitude - oldAltitude)/(millis() - oldTime);
    oldTime = millis();
    oldAltitude = gpsData.altitude;
    logsCounter = 0;
  }
  
  log_timer = millis();
  
  String dataString = "";
  dataString += (String)millis() + ",";

  dataString += (String)gpsData.year + ",";
  dataString += (String)gpsData.month + ","; 
  dataString += (String)gpsData.day + ",";
  dataString += (String)gpsData.hour + ",";
  dataString += (String)gpsData.minute + ",";
  dataString += (String)gpsData.second + ",";
  dataString += (String)gpsData.millisecond + ",";
  dataString += (String)gpsData.fix + ",";
  
  dataString += (String)gpsData.latitude_deg + ",";
  dataString += (String)gpsData.latitude_min + ",";
  dataString += (String)gpsData.latitude_dir + ",";
  dataString += (String)gpsData.longitude_deg + ",";
  dataString += (String)gpsData.longitude_min + ",";
  dataString += (String)gpsData.longitude_dir + ",";
  
  dataString += (String)gpsData.velocity + ",";
  dataString += (String)gpsData.angle + ",";
  dataString += (String)gpsData.altitude + ",";
  dataString += (String)gpsData.satellites + ",";
  
  dataString += (String)honeywellData[honeywell.TCA_INSIDE_SENSOR].pressure + ",";
  dataString += (String)honeywellData[honeywell.TCA_INSIDE_SENSOR].temperature + ",";
  dataString += (String)honeywellData[honeywell.TCA_INSIDE_SENSOR].status + ",";
  dataString += (String)honeywellData[honeywell.TCA_INSIDE_SENSOR].rawPressure + ",";
  dataString += (String)honeywellData[honeywell.TCA_INSIDE_SENSOR].rawTemperature + ",";
  dataString += (String)honeywellData[honeywell.TCA_INSIDE_SENSOR].el + ",";

  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].pressure + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].temperature + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].status + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].rawPressure + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].rawTemperature + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].el + ",";
  
  /*dataString += (String)bmeData[0].pressure + ",";
  dataString += (String)bmeData[0].temperature + ",";
  dataString += (String)bmeData[0].humidity + ",";
  dataString += (String)bmeData[0].altitude + ",";

  dataString += (String)bmeData[1].pressure + ",";
  dataString += (String)bmeData[1].temperature + ",";
  dataString += (String)bmeData[1].humidity + ",";
  dataString += (String)bmeData[1].altitude + ",";

  dataString += (String)bmeData[2].pressure + ",";
  dataString += (String)bmeData[2].temperature + ",";
  dataString += (String)bmeData[2].humidity + ",";
  dataString += (String)bmeData[2].altitude + ",";*/

  dataString += (String)ascentVelocity + ",";
  dataString += (String)actuator.position() + ",";
  dataString += (String)valveIsOpen + ",";
  
  dataString += (String)valveHasOpened + ",";
  dataString += (String)valveAlreadyClosed;
  
  datalog.write(dataString);
}

int availableMemory() {
    // Use 1024 with ATmega168
    int size = 2048;
    byte *buf;
    while ((buf = (byte *) malloc(--size)) == NULL);
        free(buf);
    return size;
}
