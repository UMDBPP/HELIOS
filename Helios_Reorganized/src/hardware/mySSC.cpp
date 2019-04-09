#include <Wire.h>
#include <Arduino.h>
#include "../../include/hardware/mySSC.h"
#include <hsc_ssc_i2c.h>

void myHoneywell::initialize(myHoneywellData &data1, myHoneywellData &data2) { //initialize the sensors by initializing their data structures
  //this method doesn't actually do anything to check that the sensors are connected
  if (TWCR == 0){ // if Wire library is not already initialized
    Wire.begin();
  }
  data1 = {};
  data2 = {};
  if (HELIOS_DEBUG) Serial.println("Two honeywell sensors initialized");
}

void myHoneywell::selectSensor(uint8_t index){
  //returns the physical location of the sensor when given the array index of the sensor
  switch(index){
    case INDEX_BALLOON:
      myMulti::selectSensor(myMulti::HONEYWELL_BALLOON);
      break;
    case INDEX_ATMOS:
      myMulti::selectSensor(myMulti::HONEYWELL_ATMOS);
      break;
  }
}

void myHoneywell::read(myHoneywellData *data, uint8_t sensorIndex) {  //Read data from a specific sensor and store it in the given memory structure
  if (sensorIndex != INDEX_BALLOON && sensorIndex != INDEX_ATMOS) return; //if not a valid array position
  selectSensor(sensorIndex); //select to talk to the desired sensor
  struct cs_raw ps; //initialize a structure and other variables to use with sensor library
  //char p_str[10], t_str[10];
  uint8_t el;
  float p, t;
  el = ps_get_raw(SSC_ADDR, &ps); //get data from sensor
  if ( el == 4 ) {  //if status == 4, then sensor missing
    if(HELIOS_DEBUG){
      Serial.print("Error. Honeywell sensor missing: ");
      Serial.println(sensorIndex);
    }
  } else {
    if ( el == 3 ) {  //if status == 3, then error
      if(HELIOS_DEBUG) Serial.print("err diagnostic fault ");
      if(HELIOS_DEBUG) Serial.println(ps.status, BIN);
    }
    if ( el == 2 ) {  //if status == 2, then data is bad
      // if data has already been feched since the last
      // measurement cycle
      if(HELIOS_DEBUG) Serial.print("warn stale data ");
      if(HELIOS_DEBUG) Serial.println(ps.status, BIN);
    }
    if ( el == 1 ) {  //if status == 1, then chip is in command mode, which shouldn't happen
      // chip in command mode
      // no clue how to end up here
      if(HELIOS_DEBUG) Serial.print("warn command mode ");
      if(HELIOS_DEBUG) Serial.println(ps.status, BIN);
    }
    ps_convert(ps, &p, &t, SSC_MIN, SSC_MAX, PRESSURE_MIN, PRESSURE_MAX);//convert the raw values to pressure and temperature and return the results in p and t
    //dtostrf(p, 2, 2, p_str);  //convert p and t to strings for printing
    //dtostrf(t, 2, 2, t_str);
    //Serial.print("pressure    (Pa) ");
    //Serial.println(p_str);
    //Serial.print("temperature (dC) ");
    //Serial.println(t_str);
    data->status=ps.status; //copy the raw and converted data to the SSC data structure for logging
    data->rawPressure=ps.bridge_data;
    data->rawTemperature=ps.temperature_data;
    data->pressure = p;
    data->temperature = t;
    data->el = el;
  }
  return;
}
