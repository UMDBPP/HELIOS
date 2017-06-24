/*Now using in house custom Balloonduino board*/

#define USING_GPS true

//Import Custom Libraries
#include "GPSFunctions.cpp"
#include "HoneywellFunctions.cpp"
#include "LEDFunctions.cpp"
#include "MotorFunctions.cpp"
#include "XbeeFunctions.cpp"
#include "ActuatorFunctions.cpp"
#include "LogFunctions.cpp"
//#include "BMEFunctions.cpp"


//First line printed identifies order of data
const String HEADER_STRING = "Starting:\nYear,Month,Day,Hour,Minute,Second,Millisecond,Latitude_deg,Latitude_min,Latitude_dir,Longitude_deg,Longitude_min,Longitude_dir,Velocity,Angle,Altitude,Num_Satellites,In_Pressure,In_Temperature,In_Status,In_Pressure_Raw,In_Temperature_Raw,Out_Pressure,Out_Temperature,Out_Status,Out_Pressure_Raw,Out_Temperature_Raw,valveHasOpened,Valve_Closed";

//Control Parameters
#define VALVE_MOVE_TIME 9000 //milliseconds
#define ALTITUDE_TO_OPEN 18000 //meters
int32_t timeOpen = 20000; //milliseconds
#define NUM_OF_CHECKS_BEFORE_OPEN 40 //the number of times the GPS must confirm altitude to open the valve
#define LOG_FREQUENCY 500 //time in milliseconds between logging
#define ASCENT_CALC_FREQUENCY 10 //this is the number of times we will log data before we will recalculate the ascent velocity

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Global Variables
//For Valve
unsigned long valveTimeAtOpen=400000000; //must be large, does not have to be that
uint8_t valveAltitudeCheckCounter=0;
boolean valveHasOpened=0;
boolean valveAlreadyClosed=0;
boolean valveIsOpen; //true for open, false for closed

//For ascent velocity calculation
float ascentVelocity = 0.0;
int32_t oldAltitude = 0;
unsigned long oldTime = 0;
uint8_t logsCounter = 0;

//Data structures
MY_HONEYWELL* honeywellData = new MY_HONEYWELL[2];
MY_GPS gpsData;

//For log timing
uint32_t log_timer;

//For command timing
uint32_t command_timer;

//Create objects
LED led;
XBEE xbee;
Actuator actuator;
Datalog datalog;
Honeywell honeywell;
Motor motor;
#if (USING_GPS)
  AGPS gps;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  delay(5000); //wait to initialize so we can connect anything we might need to
  
  Serial.begin(115200); //start communication with computer

  led.initialize();

  if(!xbee.initialize())
    led.setStatus(led.RED);
  
  if(!actuator.initialize())
    led.setStatus(led.RED);

  //open and close valve
  valveIsOpen = actuator.openValve();
  while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
  valveIsOpen = actuator.closeValve();
  while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
  actuator.stopValve();
  
  if(!motor.initialize())
    led.setStatus(led.RED);

  //Turn fan on and off
  motor.startFan();
  delay(2000);
  motor.stopFan();

  if(!honeywell.initialize(&honeywellData))
    led.setStatus(led.RED);

  /*if(!bme.initialize())
    led.setStatus(led.RED);*/

  //implement something to check that all the pressure sensors are recording the correct data

  if(!datalog.initialize())
    led.setStatus(led.RED);

  //Write header string on the SD card
  if (!datalog.write(HEADER_STRING))
    led.setStatus(led.RED);

  #if (USING_GPS)
    if(!gps.initialize(&gpsData))
      led.setStatus(led.RED);

    //wait for the gps to get a fix before starting up
    while(!gpsData.fix)
      gps.read(&gpsData);
  #else
    gpsData = {};
  #endif
  
  for (int i=0; i<10; i++){
    led.setStatus(led.OFF);
    delay(500);
    led.setStatus(led.GREEN);
    delay(500); //led blinks green to confirm it has finished setup successfully; led will turn off once there is a gps fix
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  //update essential data every loop
  #if (USING_GPS)
    gps.read(&gpsData);
  #endif

  //stop actuator if it is moving and about to hit its endpoints
  if (valveIsOpen && actuator.position() < actuator.START) //if valve has finished opening, turn it off
    actuator.stopValve();
  if (!valveIsOpen && actuator.position() > actuator.END) //if valve has finished closing, turn it off
    actuator.stopValve();

  if(xbee.receive() && (millis() - command_timer) > xbee.WAIT_TIME_AFTER_COMMAND){
    xbeeCommand();
    command_timer = millis();
  }

  //log data
  if ((millis() - log_timer) > LOG_FREQUENCY)
    logData();
    
  //Opens the plunger and expells helium at ALTIDUDE_TO_OPEN meters for timeOpen seconds
  if(!valveAlreadyClosed){//if the valve has not already been opened and closed
    if(gpsData.altitude > ALTITUDE_TO_OPEN){
      valveAltitudeCheckCounter++;//add to the altitude verifier
    }
    if(valveAltitudeCheckCounter >= NUM_OF_CHECKS_BEFORE_OPEN && !valveHasOpened){//if it is time to open the valve
      actuator.openValve();//this causes a 3 second delay where no data is taken.
      motor.startFan();
      valveTimeAtOpen = millis();
      valveHasOpened=1;
    }
    else if(millis() > (valveTimeAtOpen + timeOpen)){//if we've been open long enough
      valveAlreadyClosed=1;
      motor.stopFan();
      valveIsOpen = actuator.closeValve();
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void xbeeCommand(){
  if(xbee.getLastCommand() == xbee.COMMAND_REQUEST_DATA)
    xbee.sendData(gpsData.altitude, ascentVelocity);
  else if (xbee.getLastCommand() == xbee.COMMAND_ABORT){
    valveAlreadyClosed = 1;
    motor.stopFan();
    valveIsOpen = actuator.closeValve();
    xbee.sendConf(xbee.CONFIRM_CODE_ABORT);
  }else if (xbee.getLastCommand() == xbee.COMMAND_REVERSE){
    valveAlreadyClosed = 0;
    valveIsOpen = actuator.openValve();
    motor.reverseFan();
    timeOpen = xbee.getCommandedTime();
    valveTimeAtOpen = millis();
    valveHasOpened = 1;
    xbee.sendConf(xbee.CONFIRM_CODE_REVERSE);
  }else if (xbee.getLastCommand() == xbee.COMMAND_OPEN_NOW){
    valveAlreadyClosed = 0;
    valveIsOpen = actuator.openValve();
    motor.startFan();
    timeOpen = xbee.getCommandedTime();
    valveTimeAtOpen = millis();
    valveHasOpened = 1;
    xbee.sendConf(timeOpen);
  }
}

void logData(){
  honeywell.read(&honeywellData[honeywell.INSIDE_SENSOR], honeywell.INSIDE_SENSOR);
  honeywell.read(&honeywellData[honeywell.OUTSIDE_SENSOR], honeywell.OUTSIDE_SENSOR);
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
  
  dataString += (String)honeywellData[0].pressure + ",";
  dataString += (String)honeywellData[0].temperature + ",";
  dataString += (String)honeywellData[0].status + ",";
  dataString += (String)honeywellData[0].rawPressure + ",";
  dataString += (String)honeywellData[0].rawTemperature + ",";

  dataString += (String)honeywellData[1].pressure + ",";
  dataString += (String)honeywellData[1].temperature + ",";
  dataString += (String)honeywellData[1].status + ",";
  dataString += (String)honeywellData[1].rawPressure + ",";
  dataString += (String)honeywellData[1].rawTemperature + ",";

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

