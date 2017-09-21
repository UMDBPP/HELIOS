#include <Wire.h>
#include "Arduino.h"
#include "mySSC.h"
#include "hsc_ssc_i2c.h"

//#define TCAADDR 0x70

void myHoneywell::selectSensor(uint8_t i){//selects which pressure sensor we're talking to with the I2C multiplexer
  if (i > 7) return; //if invalid
  Wire.beginTransmission(MULTI_ADDR);  //call the multiplexer
  Wire.write(1 << i);
  Wire.endTransmission();
}

int myHoneywell::initialize(myHoneywellData *data1, myHoneywellData *data2) {
  Wire.begin();
  data1 = {};
  data2 = {};
  if (HELIOS_DEBUG) Serial.println("Two honeywell sensors initialized");
  return 1;
}

void myHoneywell::read(myHoneywellData *data, uint8_t sensor) {  //Read data from a specific sensor and store it in the given memory structure
  selectSensor(sensor); //select to talk to the desired sensor
  struct cs_raw ps;
  char p_str[10], t_str[10];
  uint8_t el;
  float p, t;
  el = ps_get_raw(SSC_ADDR, &ps);
  if ( el == 4 ) {
    Serial.println("err sensor missing");
  } else {
    if ( el == 3 ) {
      Serial.print("err diagnostic fault ");
      Serial.println(ps.status, BIN);
    }
    if ( el == 2 ) {
      // if data has already been feched since the last
      // measurement cycle
      Serial.print("warn stale data ");
      Serial.println(ps.status, BIN);
    }
    if ( el == 1 ) {
      // chip in command mode
      // no clue how to end up here
      Serial.print("warn command mode ");
      Serial.println(ps.status, BIN);
    }
    ps_convert(ps, &p, &t, SSC_MIN, SSC_MAX, PRESSURE_MIN,
               PRESSURE_MAX);
    dtostrf(p, 2, 2, p_str);
    dtostrf(t, 2, 2, t_str);
    //Serial.print("pressure    (Pa) ");
    //Serial.println(p_str);
    //Serial.print("temperature (dC) ");
    //Serial.println(t_str);
    data->status=ps.status;
    data->rawPressure=ps.bridge_data;
    data->rawTemperature=ps.temperature_data;
    data->pressure = p;
    data->temperature = t;
  }
  return;
}
