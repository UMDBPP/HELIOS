#ifndef PROCESS_TYPES
#define PROCESS_TYPES

//Import Custom Libraries
#include "../../myPins.h" //defines pin numbers for use elsewhere
#include "../hardware/myGPS.h"  //defines functions for interacting with GPS and handles all the Adafruit GPS library nonsense
#include "../hardware/mySSC.h"  //defines functions for interacting with Honeywell SSC Pressure sensors
#include "../hardware/myLED.h"  //defines functions for changing LED color
#include "../hardware/myMotor.h"  //defines functions for handling fan motor
#include "../hardware/myBITS.h" //defines functions for interacting with xbees - note that this libary is just about communications, the actual command handling is at the end of this file
#include "../hardware/myAct.h"  //defines functions for extending and reading the actuator
#include "../hardware/myLog.h"  //defines functions for interacting with SD card logging
#include "../hardware/myBME.h"  //defines functions for working with BME280 pressure sensors
#include "../hardware/myNichrome.h"
#include "../hardware/mySwitch.h"
#include "data.h"
#include "functions.h"
//#include <Wire.h>  //Required for I2C communication with SSC and BME sensors

//Create hardware objects
extern myLED ledStat; //create an LED object for status purposes
extern myLED ledArmed; //create an LED object to indicate if the system is armed
extern myBITS xbee;  //create an Xbee object
extern myActuator actuator;  //create an Actuator object
extern myNichrome nichrome;
extern myDatalog datalog;  //create an SD Card logger object
extern myHoneywell honeywell;  //create a honeywell SSC sensor and multiplexer module
extern myBME bme; //create a BME280 sensor object to manage the BMEs
extern myMotor motor;  //create a motor module
extern mySwitch extSwitch;
#if (USING_GPS) //The GPS is often troublesome, so we nest this inside an if statement so it can be readily removed for other testing
  extern myGPS gps;  //if using gps, create a GPS module
#endif

#define HEADER_STRING " "


void sFlight(void);
void lFlight(void);


void sThermalTest(void);
void lThermalTest(void);
void sPressureTest(void);
void lPressureTest(void);
void sBench(void);
void lBench(void);

#endif