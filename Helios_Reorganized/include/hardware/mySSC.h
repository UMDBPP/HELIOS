/*
 *  Functions to interact with Honeywell SSC Pressure Sensors
 */

#ifndef HoneywellFunctions
#define HoneywellFunctions

#include "Arduino.h"
#include "myMulti.h"

struct myHoneywellData{ //Preferred data structure for each sensor
  float pressure;
  float temperature;
  uint8_t status;
  uint16_t rawPressure;
  uint16_t rawTemperature;
  uint8_t el;
};

class myHoneywell{
  private:
    const static uint8_t SSC_ADDR = 0x28;         // Address of the sensor
    const static uint16_t SSC_MIN = 0;            // Minimum value the sensor returns
    const static uint16_t SSC_MAX = 0x3fff;       // 2^14 - 1
    const static float PRESSURE_MIN = 0.0;        // Min is 0 for sensors that give absolute values
    const static float PRESSURE_MAX = 206842.7;   // Max presure of the 30psi for this sensor converted to Pascals

    void selectSensor(uint8_t index);

  public:
    const static uint8_t INDEX_BALLOON = 0;   //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
    const static uint8_t INDEX_ATMOS = 1;  //SD#/SC# for the pressure sensor outside the balloon on the multiplexer

    void initialize(myHoneywellData &data1, myHoneywellData &data2);
    void read(myHoneywellData *data, uint8_t sensor);
};

 #endif
