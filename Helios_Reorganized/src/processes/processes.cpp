#include "../../include/processes/processes.h"

myLED ledStat(LED1_PINR, LED1_PING, LED1_PINB); //create an LED object for status purposes
myLED ledArmed(LED2_PINR, LED2_PING, LED2_PINB); //create an LED object to indicate if the system is armed
myBITS xbee;  //create an Xbee object
myActuator actuator;  //create an Actuator object
myNichrome nichrome; //create a Nichrome wire object
mySwitch extSwitch(ACT2_READ);
myDatalog datalog;  //create an SD Card logger object
myHoneywell honeywell;  //create a honeywell SSC sensor and multiplexer module
myBME bme; //create a BME280 sensor object to manage the BMEs
myMotor motor;  //create a motor module
#if (USING_GPS) //The GPS is often troublesome, so we nest this inside an if statement so it can be readily removed for other testing
  myGPS gps;  //if using gps, create a GPS module
#endif

valveState valve;
valveState cutdown;
myData allData;
