/*
 * Functions for interacting with Adafruit GPS
 */

#ifndef GPSFunctions
#define GPSFunctions

#include "Arduino.h"
#include "../../myPins.h"
#include <Adafruit_GPS.h>

//boolean usingInterrupt = false;

struct myGPSData{ //Data structure for storing gps info
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t millisecond;//the time at which the last GPS sentence was read
  uint8_t day;
  uint8_t month;
  uint16_t year;
  byte fix;
  uint8_t latitude_deg;//degrees
  float latitude_min;//decimal minutes
  char latitude_dir;//N/S
  uint8_t longitude_deg;
  float longitude_min;
  char longitude_dir;//W/E
  float velocity;//knots
  float angle;//direction gps thinks we're moving
  int32_t altitude=11;//meters
  uint8_t satellites;//number of satellites
};

#if USING_GPS

class myGPS{
  private:
    Adafruit_GPS GPS;
    void recordGPS(myGPSData *gpsData);

  public:
    myGPS();
    int initialize(myGPSData &gpsData);
    void read(myGPSData *gpsData);
};

#endif

#endif
