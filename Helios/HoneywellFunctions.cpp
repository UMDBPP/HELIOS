#include <Wire.h>
#include <Arduino.h>
#include "hsc_ssc_i2c.h"

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#define TCAADDR 0x70

//Data structure
struct MY_HONEYWELL {
  float pressure;
  float temperature;
  uint8_t status;
  uint16_t rawPressure;
  uint16_t rawTemperature;
};

class Honeywell { //static class
  private:

    const static uint8_t SSC_ADDR = 0x28;         // Address of the sensor
    const static uint8_t MULTI_ADDR = 0x70;       // Address of the multiplexer
    const static uint16_t SSC_MIN = 0;            // Minimum value the sensor returns
    const static uint16_t SSC_MAX = 0x3fff;       // 2^14 - 1
    const static float PRESSURE_MIN = 0.0;        // Min is 0 for sensors that give absolute values
    const static float PRESSURE_MAX = 206842.7;   // Max presure of the 30psi for this sensor converted to Pascals

    static void selectSensor(uint8_t i) {//selects which pressure sensor we're talking to with the I2C multiplexer
      if (i > 7) return; //if invalid
      Serial.println("Trying to find multiplexer");
      Wire.beginTransmission(TCAADDR);  //call the multiplexer
      Wire.write(1 << i);
      Wire.endTransmission();
      Serial.println("End of multiplexer");
    }

  public:

    const static uint8_t INSIDE_SENSOR = 0;   //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
    const static uint8_t OUTSIDE_SENSOR = 1;  //SD#/SC# for the pressure sensor outside the balloon on the multiplexer

    Honeywell() {}

    static int initialize(MY_HONEYWELL *data1, MY_HONEYWELL *data2) {
      Wire.begin();
      data1 = {};
      data2 = {};
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Two honeywell sensors initialized");
      return 1;
    }

    static void read(MY_HONEYWELL *data, uint8_t sensor) {
      selectSensor(sensor); //select to talk to the desired sensor
      /*uint8_t val[4] = {0};  //four bytes to store sensor data
        Wire.requestFrom(SSC_ADDR, (uint8_t) 4);    //request sensor data
        for (uint8_t i = 0; i <= 3; i++) {
          delay(4);                        // sensor might be missing, do not block by using Wire.available()
          val[i] = Wire.read();
        }
        Serial.println("Data Received");*/
      struct cs_raw ps;
      char p_str[10], t_str[10];
      uint8_t el;
      float p, t;
      el = ps_get_raw(SSC_ADDR, &ps);
      // for some reason my chip triggers a diagnostic fault
      // on 50% of powerups without a notable impact
      // to the output values.
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
        //Serial.print("status      ");
        //Serial.println(ps.status, BIN);
        //Serial.print("bridge_data ");
        //Serial.println(ps.bridge_data, DEC);
        //Serial.print("temp_data   ");
        //Serial.println(ps.temperature_data, DEC);
        //Serial.println("");
        ps_convert(ps, &p, &t, SSC_MIN, SSC_MAX, PRESSURE_MIN,
                   PRESSURE_MAX);
        // floats cannot be easily printed out
        dtostrf(p, 2, 2, p_str);
        dtostrf(t, 2, 2, t_str);
        Serial.print("pressure    (Pa) ");
        Serial.println(p_str);
        Serial.print("temperature (dC) ");
        Serial.println(t_str);
        //Serial.println("");
        data->status=ps.status;
        data->rawPressure=ps.bridge_data;
        data->rawTemperature=ps.temperature_data;
        data->pressure = p;
        data->temperature = t;
        /*data->status = (val[0] & 0xc0) >> 6; // first 2 bits from first byte are the status
        data->rawPressure = ((val[0] & 0x3f) << 8) + val[1];
        data->rawTemperature = ((val[2] << 8) + (val[3] & 0xe0)) >> 5;
        if (data->rawTemperature == 65535)
          data->status = 4;
        if (data->status == 4) {
          data->pressure = -1;
          data->temperature = -1;
        }
        else {
          data->pressure = 1.0 * (data->rawPressure - SSC_MIN) * (PRESSURE_MAX - PRESSURE_MIN) / (SSC_MAX - SSC_MIN) + PRESSURE_MIN;
          data->temperature = (data->rawTemperature * 0.0977) - 50;
        }*/
      }
      return;
    }
};

