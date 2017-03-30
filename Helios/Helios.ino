#include <Wire.h> //Needed for I2C
#include <SPI.h> //SD
#include <SD.h> //SD-> ~5250 bytes Storage and ~800 bytes global variables
#include <Adafruit_GPS.h>
#include<Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>

//GPS hardware serial startup
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false

//Honeywell sensor values
#define SSC_ADDR 0x28      // Address of the sensor, not needed while using the multiplexer
#define MULTI_ADDR 0x70            // Address of the multiplexer
#define SSC_MIN 0            // Minimum pressure the sensor detects
#define SSC_MAX 0x3fff       // 2^14 - 1
#define PRESSURE_MIN 0.0        // Min is 0 for sensors that give absolute values
#define PRESSURE_MAX 206842.7   // Max presure of the 30psi for this sensor converted to Pascals

//BME280 Values
#define SEALEVELPRESSURE_HPA (1013.25)

//Trinket Values
#define TRINKET_ADDR 9
#define vCounts 10  //the number of counts we wait to get a more accurate average altitude
#define XBEE_WAIT_TIME 5000   //the wait time to receive commands; I built this in so that we cannot receive duplicate commands

//First line printed identifies order of data
#define HEADER_STRING "Year,Month,Day,Hour,Minute,Second,Millisecond,Latitude_deg,Latitude_min,Latitude_dir,Longitude_deg,Longitude_min,Longitude_dir,Velocity,Angle,Altitude,Num_Satellites,In_Pressure,In_Temperature,In_Status,In_Pressure_Raw,In_Temperature_Raw,Out_Pressure,Out_Temperature,Out_Status,Out_Pressure_Raw,Out_Temperature_Raw,Valve_Open,Valve_Closed"

//Control Parameters
#define MOTOR_SPEED 255 //PWM oscillation between 0 and 255
#define VALVE_MOVE_TIME 9000 //milliseconds
#define ALTITUDE_TO_OPEN 20000 //meters
int32_t TIME_OPEN = 100000; //milliseconds
#define NUM_OF_CHECKS 40 //the number of times the GPS must confirm altitude to open the valve (2 minutes)
#define SD_CHIP_SELECT 4        // This is the Chip Select for the adafruit feather
#define INSIDE_SENSOR 0 //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
#define OUTSIDE_SENSOR 1  //SD#/SC# for the pressure sensor outside the balloon on the multiplexer
#define FREQUENCY 500 //time in milliseconds between logging

//Pin Numbers
#define PIN_MOTOR_A 13 // Turning PIN_A high will make the actuator extend
#define PIN_MOTOR_B 12
#define PIN_MOTOR_PWM 11
#define PIN_ACTUATOR_READ A2
#define PIN_ACTUATOR_A 10 //Turning PIN_A high will make the motor blow forwards
#define PIN_ACTUATOR_B 9
#define PIN_ACTUATOR_PWM 6
#define PIN_LED A0
#define PIN_TRINKET_IN A1  //When this pin goes high, the arduino knows that there is data from the xbee waiting

//LED Setup
Adafruit_NeoPixel led = Adafruit_NeoPixel(1, PIN_LED, NEO_GRB + NEO_KHZ800);
const uint32_t off = led.Color(0,0,0);
const uint32_t blue = led.Color(0,0,255);
const uint32_t green = led.Color(0,255,0);
const uint32_t red = led.Color(255,0,0);
const uint32_t white = led.Color(255,255,255);
const uint32_t yellow = led.Color(255, 255, 0);

//Data structures
struct MY_HONEYWELL{
  float pressure;
  float temperature;
  uint8_t status;
  uint16_t rawPressure;
  uint16_t rawTemperature;
};

struct MY_BME{
  uint32_t pressure;
  float temperature;
  float humidity;
  int32_t altitude;
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
  int32_t altitude=-1;//meters
  uint8_t satellites;//number of satellites
};

//Global Variables
//For Valve
unsigned long valve_time_at_open = 400000000; //must be large, does not have to be that <<<
uint8_t valve_counter=0;
boolean valve_open=0;
boolean valve_already_closed=0;
float actuator_pos=25.0; //mm
boolean valveState = 0; //true for open, false for closed

//For communication
float ascentVelocity = -2.0;
int32_t oldAltitude = 0;
unsigned long oldTime = 0;
uint8_t counter = 0;
boolean command = 0;

//For Honeywell
MY_HONEYWELL honeywells[2];

//For BME280s
Adafruit_BME280 bme[3];
MY_BME bmeData[3];

//For GPS
MY_GPS gpsData = {};
boolean usingInterrupt = false;
uint32_t timer = millis();

/////////////////////////////////////////////////////
/*
 * To Do:
 * xxxProgram LED
 * Program Xbee
 * Program BME280
 * xxxEnsure valve works as desired
 * xxxLogging every half second - make this a variable
 */

//one of the most important things to test will be how the valve works without limit switches
//Normal startup sequence is blue --> green --> blinking green --> solid green --> off.
void setup() {
  //Wait half a second
  delay(5000);
  Serial.begin(115200);
  
  //Start acutator and motor pins
  pinMode(PIN_ACTUATOR_A, OUTPUT);
  pinMode(PIN_ACTUATOR_B, OUTPUT);
  pinMode(PIN_ACTUATOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_A, OUTPUT);
  pinMode(PIN_MOTOR_B, OUTPUT);
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_TRINKET_IN, INPUT);

  led.begin();
  led.show(); //initialize the status led
  led.setPixelColor(0, blue);
  led.show(); //blue led means the code has just started and the valve is opening
  
  //open valve & close valve
  /*valveOpen();
  while(actuator_pos > 1.0) //wait for the valve to close, then turn it off
    actuator_pos = 50.0*analogRead(PIN_ACTUATOR_READ)/1032.0;
  valveClose();
  while(actuator_pos < 49.0) //wait for the valve to close, then turn it off
    actuator_pos = 50.0*analogRead(PIN_ACTUATOR_READ)/1032.0;
  valveOff();
  */
  //Initialize I2C
  Wire.begin();
  if (digitalRead(PIN_TRINKET_IN) == HIGH){
    led.setPixelColor(0, red);
    led.show();
    delay(5000); //if the trinker thinks something is wrong, turn the led red
  }
  sendData(); //send the data stream {-1 -2} to confirm xbee functionality 
  for (int i=0; i<10; i++){
      led.setPixelColor(0, off);
      led.show();
      delay(500);
      led.setPixelColor(0, blue);
      led.show();
      delay(500); //led blinks blue to confirm it has communicated with the trinket; at this point, check that xbee has sent data
  }

  //Turn fan on and off
  fanOn();
  digitalWrite(12, HIGH);
  delay(20000000);
  fanOff();

  led.setPixelColor(0, green);
  led.show(); //green indicates that the fan should have worked.

  //delay(10000000);
  
  //Begin logging data to SD Card
  Serial.print("\n\nInitializing SD card...");
  if (!SD.begin(SD_CHIP_SELECT)) { // see if the card is present and can be initialized:
    Serial.println("Card failed, or not present");
    // don't do anything more:
    led.setPixelColor(0, red);
    led.show(); //red indicates an error
    delay(5000);
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
    led.setPixelColor(0, red);
    led.show(); //red indicates an error
    delay(5000);
  }

  delay(1000);
  led.setPixelColor(0, blue);
  led.show();

  /*for (int i=0; i<3; i++){
    tcaselect(i+2);
    if (!bme[i].begin()) {  
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      led.setPixelColor(0, red);
      led.show(); //red indicates an error
      delay(5000);
    }
  }

  //GPS startup
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); // 1 Hz update rate
  //GPS.sendCommand(PGCMD_ANTENNA); // Request updates on antenna status, comment out to keep quiet
  //GPSSerial.println(PMTK_Q_RELEASE);  // Ask for firmware version
  //useInterrupt(true);

  //implement something to check that all the pressure sensors are recording the correct data
  */
  for (int i=0; i<10; i++){
    led.setPixelColor(0, off);
    led.show();
    delay(500);
    led.setPixelColor(0, green);
    led.show();
    delay(500); //led blinks green to confirm it has finished setup successfully; led will turn off once there is a gps fix
  }

  led.setPixelColor(0, off);
  led.show();
}

void loop() {
  led.setPixelColor(0, yellow); led.show();
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
    if (gpsData.fix){
      led.setPixelColor(0, off); //led turns off once we have a gps fix
      led.show();
    }    
  }
  actuator_pos = 50.0*analogRead(PIN_ACTUATOR_READ)/1032.0;
  if (valveState && actuator_pos < 1.0) //if valve has finished opening, turn it off
    valveOff();
  if (!valveState && actuator_pos > 49.0) //if valve has finished closing, turn it off
    valveOff();
  if (digitalRead(PIN_TRINKET_IN) == HIGH && abs(millis() - valve_time_at_open) > XBEE_WAIT_TIME){
    //union {
    //  uint32_t v;
      uint8_t bytes[4];
    //} commandData;
    Wire.requestFrom(TRINKET_ADDR, 4, 1);
    uint8_t toOpen=0;// = Wire.read();
    //delay(5);
    for (int i=0; i<4; i++){
      bytes[i] = Wire.read();
      delay(5);
    }
    uint32_t commandTime = bytes[3]*pow(2, 24) + bytes[2]*pow(2, 16) + bytes[1]*pow(2, 8) + bytes[0];
    //Serial.println(toOpen);
    Serial.println(commandTime);
    if (toOpen){ //doing this check is sort of unneeded, but seems like a logical safety
      valve_already_closed = 0;
      valve_open = 0; //these two lines reset the valve tracker to think that it is okay to open again
      valve_counter = NUM_OF_CHECKS; //makes the valve open immediately without checking the altitude
      TIME_OPEN = commandTime; //assign a new amount of time to open
      if (commandTime == 0){//if we have been commanded to not open the valve at all
        valve_open = 1;
        valve_time_at_open = 0; //this tricks the valve into thinking it has already been opened, so it forces itself closed
        TIME_OPEN = 1; //in this case, I opted to send 1 instead of zero so the user can be sure it is not blank data
        led.setPixelColor(0, white); led.show();
      }
      else if (commandTime == 100){
        valveOpen();
        fanBack();
        TIME_OPEN = 120001;
        valve_open = 1;
        valve_time_at_open = millis();
        led.setPixelColor(0, yellow); led.show();
      }
      sendCommand(1);
    }
    else
      sendCommand(0);
  }
  led.setPixelColor(0, off); led.show(); delay(100);
  if (millis() - timer > FREQUENCY) {
    getHoneywell(0);
    getHoneywell(1);
    getBME(2);
    getBME(3);
    getBME(4);
    timer = millis(); 
    writeLog();
    counter++;
    if (counter == vCounts){ //send data less frequently to the trinket
      ascentVelocity = 1.0*(gpsData.altitude - oldAltitude)/(millis() - oldTime);
      oldTime = millis();
      oldAltitude = gpsData.altitude;
      counter = 0;
      sendData();
    }
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

//////////////////////////////////////////////////////////////////////////////////////////

void recordGPS(){
  gpsData.hour = GPS.hour;//this whole sequence could be more efficient if we were memory limited, but as is, this helps keep data organized and isolated
  gpsData.minute = GPS.minute;
  gpsData.second = GPS.seconds;
  gpsData.millisecond = GPS.milliseconds;
  gpsData.day = GPS.day;
  gpsData.month = GPS.month;
  gpsData.year = 2000+GPS.year;
  gpsData.fix = GPS.fix;
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

void getHoneywell(uint8_t sensor){
  tcaselect(sensor); //select to talk to the desired sensor
  uint8_t val[4] = {0};  //four bytes to store sensor data
  Wire.requestFrom(SSC_ADDR, (uint8_t) 4);    //request sensor data
  for (uint8_t i = 0; i <= 3; i++) {
      delay(4);                        // sensor might be missing, do not block by using Wire.available()
      val[i] = Wire.read();
  }
  honeywells[sensor].status = (val[0] & 0xc0) >> 6; // first 2 bits from first byte are the status
  honeywells[sensor].rawPressure = ((val[0] & 0x3f) << 8) + val[1];
  honeywells[sensor].rawTemperature = ((val[2] << 8) + (val[3] & 0xe0)) >> 5;
  if (honeywells[sensor].rawTemperature == 65535)
      honeywells[sensor].status = 4;
  if (honeywells[sensor].status == 4){
    honeywells[sensor].pressure = -1;
    honeywells[sensor].temperature = -1;
  }
  else{
    honeywells[sensor].pressure = 1.0 * (honeywells[sensor].rawPressure - SSC_MIN) * (PRESSURE_MAX - PRESSURE_MIN) / (SSC_MAX - SSC_MIN) + PRESSURE_MIN;
    honeywells[sensor].temperature = (honeywells[sensor].rawTemperature * 0.0977) - 50;
  }
}

void getBME(uint8_t sensor){
  tcaselect(sensor);
  bmeData[sensor-2].pressure = bme[sensor-2].readPressure();
  bmeData[sensor-2].temperature = bme[sensor-2].readTemperature();
  bmeData[sensor-2].humidity = bme[sensor-2].readHumidity();
  bmeData[sensor-2].altitude = bme[sensor-2].readAltitude(SEALEVELPRESSURE_HPA);
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
  
  dataString += (String)honeywells[0].pressure + ",";
  dataString += (String)honeywells[0].temperature + ",";
  dataString += (String)honeywells[0].status + ",";
  dataString += (String)honeywells[0].rawPressure + ",";
  dataString += (String)honeywells[0].rawTemperature + ",";

  dataString += (String)honeywells[1].pressure + ",";
  dataString += (String)honeywells[1].temperature + ",";
  dataString += (String)honeywells[1].status + ",";
  dataString += (String)honeywells[1].rawPressure + ",";
  dataString += (String)honeywells[1].rawTemperature + ",";

  dataString += (String)bmeData[0].pressure + ",";
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
  dataString += (String)bmeData[2].altitude + ",";
  
  dataString += (String)valve_open + ",";
  dataString += (String)actuator_pos + ",";
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
  valveState = 1;
}

void valveClose(void){//closes the valve by extending the piston
  Serial.println("Closing Valve");  
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, HIGH);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  valveState = 0;
}

void valveOff(void){//puts zero voltage on each side of valve
  //Serial.println("Turning Valve Off");
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

void fanBack(void){//turns fan on to prespecified speed
  Serial.println("Turning fan on");
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, HIGH);
  analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
}

void fanOff(void){//turns fan off
  Serial.println("Turning fan off");
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, LOW);
  analogWrite(PIN_MOTOR_PWM, 0);
}

void sendCommand(boolean opened){
  if (!opened){ //if unsuccessful, write normal data
    union {
      int32_t v;
      byte bytes[4];
    } a;
    union {
      float f;
      byte bytes[4];
    } u;
    u.f = ascentVelocity; //used to convert float to bytes which can be sent
    a.v = gpsData.altitude;
    Wire.beginTransmission(TRINKET_ADDR);
    Wire.write(1);
    for (int i=0; i<4; i++) Wire.write(a.bytes[i]);
    for (int i=0; i<4; i++) Wire.write(u.bytes[i]);
    Wire.endTransmission();
  }
  else{ //if successful, write the data received
    union {
      int32_t v;
      byte bytes[4];
    } a;
    union {
      float f;
      byte bytes[4];
    } u;
    u.f = 0.0;
    a.v = TIME_OPEN;
    Wire.beginTransmission(TRINKET_ADDR);
    Wire.write(1);
    for (int i=0; i<4; i++) Wire.write(a.bytes[i]);
    for (int i=0; i<4; i++) Wire.write(u.bytes[i]);
    Wire.endTransmission();
  }
  delay(200); //we want the trinket to process the command before we move on
}

void sendData(){
  union {
      int32_t v;
      byte bytes[4];
    } a;
  union {
    float f;
    byte bytes[4];
  } u;
  u.f = ascentVelocity;
  a.v = gpsData.altitude;
  Wire.beginTransmission(TRINKET_ADDR);
  Wire.write(0);
  for (int i=0; i<4; i++) Wire.write(a.bytes[i]);
  for (int i=0; i<4; i++) Wire.write(u.bytes[i]);
  Wire.endTransmission();
}

