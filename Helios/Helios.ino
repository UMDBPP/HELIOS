#include <Wire.h> //Needed for I2C
#include "hsc_ssc_i2c.h" //Needed for Honeywell Pressure Sensor
#include <SPI.h> //SD
#include <SD.h> //SD-> ~5250 bytes Storage and ~800 bytes global variables
#include <Adafruit_GPS.h>

//GPS hardware serial startup
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false

// see hsc_ssc_i2c.h for a description of these values
// these defaults are found in the Honeywell SSC Datasheet and Honeywell I2C

#define SSC_ADDR 0x28      // Address of the sensor, not needed while using the multiplexer
#define MULTI_ADDR 0x70            // Address of the multiplexer
#define SSC_MIN 0            // Minimum pressure the sensor detects
#define SSC_MAX 0x3fff       // 2^14 - 1
#define PRESSURE_MIN 0.0        // Min is 0 for sensors that give absolute values
#define PRESSURE_MAX 206842.7   // Max presure of the 30psi for this sensor converted to Pascals
#define HEADER_STRING "Year,Month,Day,Hour,Minute,Second,Millisecond,Latitude_deg,Latitude_min,Latitude_dir,Longitude_deg,Longitude_min,Longitude_dir,Velocity,Angle,Altitude,Num_Satellites,In_Pressure,In_Temperature,In_Status,In_Pressure_Raw,In_Temperature_Raw,Out_Pressure,Out_Temperature,Out_Status,Out_Pressure_Raw,Out_Temperature_Raw,Valve_Open,Valve_Closed"

struct PRESSURE_SENSOR{
  float pressure;
  float temperature;
  uint8_t status;
  uint16_t rawPressure;
  uint16_t rawTemperature;
};

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
  int32_t altitude;//meters
  uint8_t satellites;//number of satellites
};

//Pin Numbers
const int PIN_ACTUATOR_A = 13;
const int PIN_ACTUATOR_B = 12;
const int PIN_ACTUATOR_PWM = 11;
const int PIN_MOTOR_A = 10;
const int PIN_MOTOR_B = 9;
const int PIN_MOTOR_PWM = 6;
const int PIN_LED_DATA = 8;

//Control Parameters
const uint8_t MOTOR_SPEED = 255; //PWN oscillation between 0 and 255
const uint16_t VALVE_TIMER = 9000; //milliseconds
const uint16_t ALTITUDE_TO_OPEN = 20000;
const uint64_t TIME_TO_OPEN = 2100000; //miliseconds
const uint32_t TIME_OPEN = 100000; //miliseconds defalut 100000 milliseconds
const uint8_t NUM_OF_CHECKS = 40; //the number of times the GPS must confirm altitude to open the valve (2 minutes)
const uint8_t CHIP_SELECT = 4;        // This is the Chip Select for the adafruit feather
const byte INSIDE_SENSOR = 0; //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
const byte OUTSIDE_SENSOR = 1;  //SD#/SC# for the pressure sensor outside the balloon on the multiplexer

//Global Variables

//For Valve
unsigned long valve_time_at_open = 400000000; //must be large, does not have to be that <<<
uint8_t valve_counter=0;
boolean valve_open=0;
boolean valve_already_closed=0;

//For Pressure
PRESSURE_SENSOR pressures[2];

//For GPS
MY_GPS gpsData = {};
boolean usingInterrupt = false;
uint32_t timer = millis();

void setup() {
  //Wait half a second
  delay(500);
  Serial.begin(115200);

  //Start acutator and motor pins
  pinMode(PIN_ACTUATOR_A, OUTPUT);
  pinMode(PIN_ACTUATOR_B, OUTPUT);
  pinMode(PIN_ACTUATOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_A, OUTPUT);
  pinMode(PIN_MOTOR_B, OUTPUT);
  pinMode(PIN_MOTOR_PWM, OUTPUT);

  //open valve & close valve
  valveOpen();
  delay(1000);
  valveClose();

  //Turn fan on and off
  fanOn();
  delay(2000);
  fanOff();
  
  //Wire setup for use of the Mulitplexer
  Wire.begin();
  
  //Begin logging data to SD Card
  Serial.print("\n\nInitializing SD card...");
  if (!SD.begin(CHIP_SELECT)) { // see if the card is present and can be initialized:
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  //Write "Starting" on the SD card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {// if the file is available, write to it:
    dataFile.println("Starting:");
    dataFile.println(HEADER_STRING);
    Serial.println("Starting:");
    Serial.println(HEADER_STRING);
    dataFile.close();
  }
  else { // if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }

  //GPS startup
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); // 1 Hz update rate
  //GPS.sendCommand(PGCMD_ANTENNA); // Request updates on antenna status, comment out to keep quiet
  //GPSSerial.println(PMTK_Q_RELEASE);  // Ask for firmware version
  //useInterrupt(true);
}

void loop() {
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    // Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
    recordGPS();
    getPressure(0);
    getPressure(1);
    if (millis() - timer > 1000) {
      timer = millis(); 
      writeLog();
      //Opens the plunger and expells helium at ALTIDUDE_TO_OPEN meters for TIME_OPEN seconds
      if(!valve_already_closed){//if the valve has not already been opened and closed
        if(gpsData.altitude > ALTITUDE_TO_OPEN){
          valve_counter++;//add to the altitude verifier
        }
        if(valve_counter >= NUM_OF_CHECKS && !valve_open){//if it is time to open the valve
          valveOpen();//this causes a 3 second delay where no data is taken.
          fanOn();
          valve_time_at_open = millis();
          valve_open=1;
        }
        else if(millis() > (valve_time_at_open + TIME_OPEN)){//if we've been open long enough
          valve_already_closed=1;
          fanOff();
          valveClose();
        }
      }
    }
  }
  

}

//////////////////////////////////////////////////////////////////////////////////////////

void recordGPS(){
  gpsData.hour = GPS.hour;//this whole sequence could be more efficient if we were memory limited, but as is, this helps keep data organized and isolated
  gpsData.minute = GPS.minute;
  gpsData.second = GPS.seconds;
  gpsData.millisecond = GPS.milliseconds;
  gpsData.day = GPS.day;
  gpsData.month = GPS.month;
  gpsData.year = 2000+GPS.year;
  if(GPS.fix){
    gpsData.latitude_deg = GPS.latitude/100;
    gpsData.latitude_min = GPS.latitude- 100*gpsData.latitude_deg;
    gpsData.latitude_dir = GPS.lat;
    gpsData.longitude_deg = GPS.longitude/100;
    gpsData.longitude_min = GPS.longitude - 100*gpsData.longitude_deg;
    gpsData.longitude_dir = GPS.lon;
    gpsData.velocity = GPS.speed;
    gpsData.angle = GPS.angle;
    gpsData.altitude = GPS.altitude;
    gpsData.satellites = GPS.satellites;
  }
}

void getPressure(uint8_t sensor)
{
  tcaselect(sensor); //select to talk to the desired sensor
  struct cs_raw ps; //declare variable for raw data 
  uint8_t el; //location for status vairable
  float p, t; //location for converted variables before storage
  el = ps_get_raw(SSC_ADDR, &ps); //get status and data
  if ( el == 4 ) {  //if sensor missing
    pressures[sensor].pressure = -1;
    pressures[sensor].temperature = -1;
  } else {
    pressures[sensor].status = ps.status;
    pressures[sensor].rawPressure = ps.bridge_data;
    pressures[sensor].rawTemperature = ps.temperature_data;
    ps_convert(ps, &p, &t, SSC_MIN, SSC_MAX, PRESSURE_MIN, PRESSURE_MAX); //convert raw data
    pressures[sensor].pressure = p;
    pressures[sensor].temperature = t;
  }
}

void tcaselect(uint8_t i) {//selects which pressure sensor we're talking to with the I2C multiplexer
  if (i > 7) return; //if invalid
  Wire.beginTransmission(MULTI_ADDR);  //call the multiplexer
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void writeLog(void){
  // make a string for assembling the data to log:
  String dataString = "";
  dataString += (String)millis() + ",";

  dataString += (String)gpsData.year + ",";
  dataString += (String)gpsData.month + ","; 
  dataString += (String)gpsData.day + ",";
  dataString += (String)gpsData.hour + ",";
  dataString += (String)GPS.minute + ",";
  dataString += (String)gpsData.second + ",";
  dataString += (String)gpsData.millisecond + ",";
  
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
  
  dataString += (String)pressures[0].pressure + ",";
  dataString += (String)pressures[0].temperature + ",";
  dataString += (String)pressures[0].status + ",";
  dataString += (String)pressures[0].rawPressure + ",";
  dataString += (String)pressures[0].rawTemperature + ",";

  dataString += (String)pressures[1].pressure + ",";
  dataString += (String)pressures[1].temperature + ",";
  dataString += (String)pressures[1].status + ",";
  dataString += (String)pressures[1].rawPressure + ",";
  dataString += (String)pressures[1].rawTemperature + ",";

  dataString += (String)valve_open + ",";
  dataString += (String)valve_already_closed;
  
  writeSD(dataString);
}

void writeSD(String dataString){
  // open the file. note that only one file can be open at a time
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString); 
  } 
  else {// if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }
}

void valveOpen(void){//opens the valve by retracting the piston
  Serial.println("Opening Valve");
  digitalWrite(PIN_ACTUATOR_A, HIGH);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  delay(VALVE_TIMER);
  valveOff();
}

void valveClose(void){//closes the valve by extending the piston
  Serial.println("Closing Valve");  
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, HIGH);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  delay(VALVE_TIMER);
  valveOff();
}

void valveOff(void){//puts zero voltage on each side of valve
  Serial.println("Turning Valve Off");
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, LOW);
}

void fanOn(void){//turns fan on to prespecified speed
  Serial.println("Turning fan on");
  digitalWrite(PIN_MOTOR_A, HIGH);
  digitalWrite(PIN_MOTOR_B, LOW);
  analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
}

void fanOff(void){//turns fan off
  Serial.println("Turning fan off");
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, LOW);
  analogWrite(PIN_MOTOR_PWM, 0);
}
