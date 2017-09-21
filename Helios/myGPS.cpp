//#include <Adafruit_Sensor.h>
#include <Adafruit_GPS.h> //include gps library
#include <Arduino.h>  //include arduino library to recognize keywords
#include "myGPS.h"

#if USING_GPS
#define GPSECHO false

void myAGPS::recordGPS(myGPSData *gpsData, Adafruit_GPS *GPS){  //save current data to the structure indicated by the pointer passed
  gpsData->hour = GPS->hour;//this whole sequence could be more efficient if we were memory limited, but as is, this helps keep data organized and isolated
  gpsData->minute = GPS->minute;
  gpsData->second = GPS->seconds;
  gpsData->millisecond = GPS->milliseconds;
  gpsData->day = GPS->day;
  gpsData->month = GPS->month;
  gpsData->year = 2000+GPS->year;
  gpsData->fix = GPS->fix;
  if(GPS->fix){  //only save location data if we have a fix
    gpsData->latitude_deg = GPS->latitude/100;
    gpsData->latitude_min = GPS->latitude - 100*gpsData->latitude_deg;
    gpsData->latitude_dir = GPS->lat;
    gpsData->longitude_deg = GPS->longitude/100;
    gpsData->longitude_min = GPS->longitude - 100*gpsData->longitude_deg;
    gpsData->longitude_dir = GPS->lon;
    gpsData->velocity = GPS->speed;
    gpsData->angle = GPS->angle;
    gpsData->altitude = GPS->altitude;
    gpsData->satellites = GPS->satellites;
  }
}

int myAGPS::initialize(myGPSData *gpsData, Adafruit_GPS *GPS){  //initialize the gps
  GPS->begin(9600);
  GPS->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS->sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS->sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
  GPS->sendCommand(PGCMD_ANTENNA);
  delay(1000);
  Serial1.println(PMTK_Q_RELEASE);
  gpsData = {}; //initailize the structure to all zeros
  if (HELIOS_DEBUG) Serial.println("GPS initialized");
  return 1; //return successful
}

void myAGPS::read(myGPSData *gpsData, Adafruit_GPS *GPS){ //function to read data and save it to the pointer passed
  char c = GPS->read();  //read the gps
  if (GPSECHO)  //echo the raw data if true
    if (c) Serial.println(c);
  if (GPS->newNMEAreceived()) {  //if there is new data, try to parse it
    if (!GPS->parse(GPS->lastNMEA())) // this also sets the newNMEAreceived() flag to false
      ; // we can fail to parse a sentence in which case we should just wait for another
    recordGPS(gpsData, GPS);  //if there is new data we will record it, if not, this will simply write the old data again
  }
  return;
}

#endif

