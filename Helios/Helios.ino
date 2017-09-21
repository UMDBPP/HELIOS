/*Now using in house custom Balloonduino board*/

//confirmed that SD card writer works correctly
//confirmed that gps works correctly
//confirmed that xbee works correctly

#define USING_GPS true
#define HELIOS_DEBUG true //this makes every function output what it is doing to serial as well
#define DEBUG_MODE false //this makes the main code ignore the main setup and loop and instead follow an alternative code sequence
#define GPS_Serial Serial1

//Import Custom Libraries
#include "myPins.h"
#include "myGPS.h"
#include "mySSC.h"
#include "myLED.h"
#include "myMotor.h"
#include "myXbee.h"
#include "myAct.h"
#include "myLog.h"
//#include "myBME.h"

#include<Wire.h>

//First line printed identifies order of data
const String HEADER_STRING = "Starting:\nYear,Month,Day,Hour,Minute,Second,Millisecond,Latitude_deg,Latitude_min,Latitude_dir,Longitude_deg,Longitude_min,Longitude_dir,Velocity,Angle,Altitude,Num_Satellites,In_Pressure,In_Temperature,In_Status,In_Pressure_Raw,In_Temperature_Raw,Out_Pressure,Out_Temperature,Out_Status,Out_Pressure_Raw,Out_Temperature_Raw,valveHasOpened,Valve_Closed";

//Control Parameters
int32_t altitudeToOpen = 18000; //meters
int32_t timeOpen = 60000; //milliseconds
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

//uint16_t loopNum = 0;

//For ascent velocity calculation
float ascentVelocity = 0.0;
int32_t oldAltitude = 0;
unsigned long oldTime = 0;
uint8_t logsCounter = 0;

//Data structures
myHoneywellData honeywellData[2];
myGPSData gpsData;

//For log timing
uint32_t log_timer = 0;

//For xbee command timing
uint32_t command_timer= 0;

//Create objects
myLED led(LED1_PIN1, LED1_PIN2, LED1_PIN3);
myXbee xbee;
myActuator actuator;
myDatalog datalog;
myHoneywell honeywell;
myMotor motor;
#if (USING_GPS)
  //Required for GPS to work
  Adafruit_GPS GPS(&GPS_Serial);
  myAGPS gps;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if DEBUG_MODE  //if we are in debug mode, skip turning on everything

void setup(){
  delay(5000);
  Serial.begin(115200);/*
  led.initialize();
  if(!actuator.initialize()) Serial.println("Valve error");
  if(!motor.initialize()) Serial.println("Fan error");
  if(!xbee.initialize()) Serial.println("XBEE Error");
  if(!datalog.initialize()) Serial.println("Log error");
  if(!gps.initialize(&gpsData, &GPS)) Serial.println("GPS Error");*/
  if(!honeywell.initialize(&honeywellData[0], &honeywellData[1])) Serial.println("Honeywell Error");
  delay(1000);
}

void loop(){
  honeywell.read(&honeywellData[honeywell.TCA_INSIDE_SENSOR], honeywell.TCA_INSIDE_SENSOR);
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

  if(!xbee.initialize())
    led.setStatus(led.RED);

  led.setStatus(led.YELLOW);
  
  if(!actuator.initialize())
    led.setStatus(led.RED);

  //open and close valve
  valveIsOpen = actuator.openValve();
  while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
  valveIsOpen = actuator.closeValve();
  while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
  actuator.stopValve();

  led.setStatus(led.BLUE);
  
  if(!motor.initialize())
    led.setStatus(led.RED);

  //Turn fan on and off
  motor.startFan();
  delay(2000);
  motor.stopFan();

  if(!honeywell.initialize(&honeywellData[0], &honeywellData[1]))
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
    if(!gps.initialize(&gpsData, &GPS))
      led.setStatus(led.RED);

    //wait for the gps to get a fix before starting up
    //while(!gpsData.fix)
      gps.read(&gpsData, &GPS);
  #else
    gpsData = {};
  #endif
  
  for (int i=0; i<10; i++){
    led.setStatus(led.OFF);
    delay(500);
    led.setStatus(led.GREEN);
    delay(500); //led blinks green to confirm it has finished setup successfully; led will turn off once there is a gps fix
  }
  Serial.println("Starting Main Loop");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  //update essential data every loop
  //Serial.println("Loop G");
  /*if (millis()/1000 > loopNum){
    loopNum = millis()/1000;
    Serial.println(availableMemory());
  }*/
  
  #if (USING_GPS)
    gps.read(&gpsData, &GPS);
  #endif

  if (gpsData.fix)
    led.setStatus(led.OFF);
  //stop actuator if it is moving and about to hit its endpoints
  if (valveIsOpen && actuator.position() < actuator.START) //if valve has finished opening, turn it off
    actuator.stopValve();
  if (!valveIsOpen && actuator.position() > actuator.END) //if valve has finished closing, turn it off
    actuator.stopValve();
  //Serial.println("Loop"); 
  if(xbee.receive() && (millis() - command_timer) > xbee.WAIT_TIME_AFTER_COMMAND){
    //Serial.println("Test");
    xbeeCommand();
    command_timer = millis();
  }
  //Serial.println("Loop X");
  //log data
  if ((millis() - log_timer) > LOG_FREQUENCY)
    logData();
  //Serial.println("Loop L");
  //Opens the plunger and expells helium at ALTIDUDE_TO_OPEN meters for timeOpen seconds
  if(!valveAlreadyClosed){//if the valve has not already been opened and closed
    if(gpsData.altitude > altitudeToOpen){
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
  //Serial.println("Loop A");
}

#endif  //this ends the if/else sequence used while debugging

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Below are assisting functions which are included in this file because they have access to global variables

void xbeeCommand(){
  if (xbee.getLastCommand() == xbee.COMMAND_ENABLE){
    valveAlreadyClosed = 0;
    valveAltitudeCheckCounter = 0;
    valveHasOpened = 0;
    xbee.sendConf(xbee.CONFIRM_CODE_ENABLE, timeOpen);
    xbee.sendConf(xbee.CONFIRM_CODE_ENABLE, altitudeToOpen);
  }else if(xbee.getLastCommand() == xbee.COMMAND_REQUEST_DATA){
    xbee.sendData(gpsData.altitude, ascentVelocity);
    if(HELIOS_DEBUG) Serial.println("xbee data sent");
  }
  else if (xbee.getLastCommand() == xbee.COMMAND_ABORT){
    valveAlreadyClosed = 1;
    motor.stopFan();
    valveIsOpen = actuator.closeValve();
    xbee.sendConf(xbee.CONFIRM_CODE_ABORT, 0);
    if(HELIOS_DEBUG) Serial.println("xbee has commanded abort");
  }else if (xbee.getLastCommand() == xbee.COMMAND_VENT_NOW){
    valveAlreadyClosed = 0;
    valveIsOpen = actuator.openValve();
    motor.startFan();
    timeOpen = xbee.getCommandedTime();
    valveTimeAtOpen = millis();
    valveHasOpened = 1;
    xbee.sendConf(xbee.CONFIRM_CODE_VENT, timeOpen);
    if (HELIOS_DEBUG) Serial.println("Helios will open valve for " + (String)timeOpen + " milliseconds");
  }else if (xbee.getLastCommand() == xbee.COMMAND_SET_TIME){
    timeOpen = xbee.getCommandedTime();
    xbee.sendConf(xbee.CONFIRM_CODE_SET_VAR, timeOpen);
  }else if (xbee.getLastCommand() == xbee.COMMAND_SET_ALT){
    altitudeToOpen = xbee.getCommandedTime();
    xbee.sendConf(xbee.CONFIRM_CODE_SET_VAR, -1*altitudeToOpen);
  }else if (xbee.getLastCommand() == xbee.COMMAND_REVERSE_NOW){
    valveAlreadyClosed = 0;
    valveIsOpen = actuator.openValve();
    motor.reverseFan();
    timeOpen = xbee.getCommandedTime();
    valveTimeAtOpen = millis();
    valveHasOpened = 1;
    xbee.sendConf(xbee.CONFIRM_CODE_REVERSE, timeOpen);
    if(HELIOS_DEBUG) Serial.println("Helios will run fan in reverse for " + (String)timeOpen + " seconds");
  }else if (xbee.getLastCommand() == xbee.COMMAND_TEST_OPEN){
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
  }else if (xbee.getLastCommand() == xbee.COMMAND_KILL){
    // make a kill switch in the LVC maybe
    xbee.sendConf(xbee.CONFIRM_CODE_KILL, 0);
  }
  datalog.write("XBEE COMMAND RECEIVED: " + (String)(xbee.getLastCommand()) + ", " + (String)(xbee.getCommandedTime()));
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

  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].pressure + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].temperature + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].status + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].rawPressure + ",";
  dataString += (String)honeywellData[honeywell.TCA_OUTSIDE_SENSOR].rawTemperature + ",";
  
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
