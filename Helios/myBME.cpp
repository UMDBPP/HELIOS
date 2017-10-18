#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Arduino.h"
#include "myBME.h"
#include <Wire.h>

myBME::myBME(){} //try myBME::myBME():bme(){} if this doesn't work, or myBME::myBME():bme({{1}, {2}}){} for SPI, though theorertically the first one is unnecessary and the second one shouldn't work 

static void myBME::selectSensor(uint8_t i) {//selects which pressure sensor we're talking to with the I2C multiplexer
  if (i > 7) return; //if invalid
  Wire.beginTransmission(MULTI_ADDR);  //call the multiplexer
  Wire.write(1 << i);
  Wire.endTransmission();  
}

int myBME::initialize(myBMEData *data1, myBMEData *data2){
  if (TWCR == 0){ // if Wire library is not already initialized
    Wire.begin();
  }
  selectSensor(getSensorLocation(TCA_INSIDE_SENSOR));
  if (!bme[TCA_INSIDE_SENSOR].begin()) {  
    if(HELIOS_DEBUG) Serial.println("Could not find a valid BME280 sensor, check wiring!");
    return 0;
  }
  selectSensor(getSensorLocation(TCA_OUTSIDE_SENSOR));
  if (!bme[TCA_OUTSIDE_SENSOR].begin()) {  
    if(HELIOS_DEBUG) Serial.println("Could not find a valid BME280 sensor, check wiring!");
    return 0;
  }
  data1 = {};
  data2 = {};
  if (HELIOS_DEBUG) Serial.println("Two BME sensors initialized");
  return 1;
}

uint8_t myBME::getSensorLocation(uint8_t sensor_number){
  //returns the physical location of the sensor when given the array index of the sensor
  switch(sensor_number){
    case TCA_INSIDE_SENSOR:
      return INSIDE_ADDR;
      break;
    case TCA_OUTSIDE_SENSOR:
      return OUTSIDE_ADDR;
      break;
    default:
      return UNUSED_ADDR;
  }
}

void myBME::read(myBMEData *data, uint8_t sensor){
  if (sensor != TCA_INSIDE_SENSOR && sensor != TCA_OUTSIDE_SENSOR) return; //if not a valid array position
  selectSensor(getSensorLocation(sensor));
  data->pressure = bme[sensor].readPressure();
  data->temperature = bme[sensor].readTemperature();
  data->humidity = bme[sensor].readHumidity();
  data->altitude = bme[sensor].readAltitude(SEA_LEVEL_PRESSURE_HPA);
}




