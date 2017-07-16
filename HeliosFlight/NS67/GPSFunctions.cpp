//#include <Adafruit_Sensor.h>
#include <Adafruit_GPS.h> //include gps library
#include <Arduino.h>  //include arduino library to recognize keywords

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#if USING_GPS
#define GPSECHO false
#endif

//Data structure for storing gps info
struct MY_GPS{
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


//boolean usingInterrupt = false;

#if USING_GPS
class AGPS{

  private:
    
    void recordGPS(MY_GPS *gpsData, Adafruit_GPS *GPS){  //save current data to the structure indicated by the pointer passed
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

  public:

    AGPS(){}  //constructor that does nothing
    
    int initialize(MY_GPS *gpsData, Adafruit_GPS *GPS){  //initialize the gps
      GPS->begin(9600);
      GPS->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
      GPS->sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
      GPS->sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
      GPS->sendCommand(PGCMD_ANTENNA);
      delay(1000);
      Serial1.println(PMTK_Q_RELEASE);
      gpsData = {}; //initailize the structure to all zeros
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("GPS initialized");
      return 1; //return successful
    }

    void read(MY_GPS *gpsData, Adafruit_GPS *GPS){ //function to read data and save it to the pointer passed
      char c = GPS->read();  //read the gps
      if (GPSECHO)  //echo the raw data if true
        if (c) DEBUG_SERIAL.println(c);
      if (GPS->newNMEAreceived()) {  //if there is new data, try to parse it
        if (!GPS->parse(GPS->lastNMEA())) // this also sets the newNMEAreceived() flag to false
          ; // we can fail to parse a sentence in which case we should just wait for another
        recordGPS(gpsData, GPS);  //if there is new data we will record it, if not, this will simply write the old data again
      }
      return;
    }
};
#endif

