#ifndef DATA_TYPES
#define DATA_TYPES

#include "../hardware/myGPS.h"  //defines functions for interacting with GPS and handles all the Adafruit GPS library nonsense
#include "../hardware/mySSC.h"  //defines functions for interacting with Honeywell SSC Pressure sensors
#include "../hardware/myBME.h"  //defines functions for working with BME280 pressure sensors

enum State{armed, disarmed, open};
struct valveState{
  unsigned long millisWhenOpened=0; //this variable becomes the time from millis() at which the valve opens (it is initialized to a large value because an older version of the code required as such)
  uint8_t numAltitudeChecks=0;  //the number of times the GPS has read a value above minAltitudeToOpen
  State state; //becomes true as soon as the valve first opens, becomes false after a re-enable command
};
extern valveState valve;

//Create data objects

struct myData{
  myGPSData gpsData;
  myHoneywellData honeywellBalloonData;
  myHoneywellData honeywellAtmosphereData;
  myBMEData bmeBalloonData;
  myBMEData bmeAtmosphereData;
};
extern myData allData;

#endif