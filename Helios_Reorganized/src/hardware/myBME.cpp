#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include "../../include/hardware/myBME.h"
#include <Wire.h>

myBME::myBME(){} //try myBME::myBME():bme(){} if this doesn't work, or myBME::myBME():bme({{1}, {2}}){} for SPI, though theorertically the first one is unnecessary and the second one shouldn't work

int myBME::initialize(myBMEData &data1, myBMEData &data2){
  if (TWCR == 0){ // if Wire library is not already initialized
    Wire.begin();
  }
  selectSensor(INDEX_BALLOON);
  if (!bme[INDEX_BALLOON].begin()) {
    if(HELIOS_DEBUG) Serial.println("Could not find a valid BME280 sensor inside balloon, check wiring!");
    return 0;
  }
  selectSensor(INDEX_ATMOS);
  if (!bme[INDEX_ATMOS].begin()) {
    if(HELIOS_DEBUG) Serial.println("Could not find a valid BME280 sensor for atmosphere, check wiring!");
    return 0;
  }
  data1 = {};
  data2 = {};
  if (HELIOS_DEBUG) Serial.println("Two BME sensors initialized");
  return 1;
}

void myBME::selectSensor(uint8_t sensorIndex){
  //switches the multiplexer to the correct sensor channel
  switch(sensorIndex){
    case INDEX_BALLOON:
      myMulti::selectSensor(myMulti::BME_BALLOON);
      break;
    case INDEX_ATMOS:
      myMulti::selectSensor(myMulti::BME_ATMOS);
      break;
  }
}

void myBME::read(myBMEData *data, uint8_t index){
  if (index != INDEX_BALLOON && index != INDEX_ATMOS) return; //if not a valid array position
  selectSensor(index);
  data->pressure = bme[index].readPressure();
  data->temperature = bme[index].readTemperature();
  data->humidity = bme[index].readHumidity();
  data->altitude = bme[index].readAltitude(SEA_LEVEL_PRESSURE_HPA);
  if(isnan(bme[index].readPressure())){ //if the bme was disconnected, try restarting it
    bme[index].begin();
  }
}
