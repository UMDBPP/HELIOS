//#include <Adafruit_Sensor.h>
#include <Adafruit_GPS.h>
#include <Arduino.h>

HardwareSerial GPS_Serial =  Serial1; //Pins 18 and 19 on Arduino mega
Adafruit_GPS GPS(&Serial1);

//Data structure
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

class AGPS{

  private:
    const static boolean GPSECHO =  false;
    
    void recordGPS(MY_GPS *gpsData){
      gpsData->hour = GPS.hour;//this whole sequence could be more efficient if we were memory limited, but as is, this helps keep data organized and isolated
      gpsData->minute = GPS.minute;
      gpsData->second = GPS.seconds;
      gpsData->millisecond = GPS.milliseconds;
      gpsData->day = GPS.day;
      gpsData->month = GPS.month;
      gpsData->year = 2000+GPS.year;
      gpsData->fix = GPS.fix;
      if(GPS.fix){
        gpsData->latitude_deg = GPS.latitude/100;
        gpsData->latitude_min = GPS.latitude - 100*gpsData->latitude_deg;
        gpsData->latitude_dir = GPS.lat;
        gpsData->longitude_deg = GPS.longitude/100;
        gpsData->longitude_min = GPS.longitude - 100*gpsData->longitude_deg;
        gpsData->longitude_dir = GPS.lon;
        gpsData->velocity = GPS.speed;
        gpsData->angle = GPS.angle;
        gpsData->altitude = GPS.altitude;
        gpsData->satellites = GPS.satellites;
      }
    }

  public:

    AGPS(){}
    
    int initialize(MY_GPS *gpsData){
      TIMSK0 &= ~_BV(OCIE0A);
      GPS.begin(9600);
      GPS.sendCommand("$PGCMD,33,0*6D");
      GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //turn on RMC (recommended minimum) and GGA (fix data) including altitude
      GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
      //GPS.sendCommand(PGCMD_ANTENNA); // Request updates on antenna status, comment out to keep quiet
      //GPS_Serial.println(PMTK_Q_RELEASE);  // Ask for firmware version
      //useInterrupt(true);

      while(!GPS.newNMEAreceived()){
        char c = GPS.read();
      }
      gpsData = {};
      return 1;
    }

    void read(MY_GPS *gpsData){
      char c = GPS.read();
      if (GPSECHO)
        if (c) Serial.print(c);
      if (GPS.newNMEAreceived()) {
        if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
          ; // we can fail to parse a sentence in which case we should just wait for another
        recordGPS(gpsData);  
      }
    }
};


