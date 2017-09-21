/* 
 *  Functions to interact with Honeywell SSC Pressure Sensors
 */

#ifndef HoneywellFunctions
#define HoneywellFunctions

#define HELIOS_DEBUG false
#include "Arduino.h"

struct myHoneywellData{ //Preferred data structure for each sensor
  float pressure;
  float temperature;
  uint8_t status;
  uint16_t rawPressure;
  uint16_t rawTemperature;
};

class myHoneywell{
  private:
    //const static uint8_t TCAADDR 0x70;
    const static uint8_t SSC_ADDR = 0x28;         // Address of the sensor
    const static uint8_t MULTI_ADDR = 0x70;       // Address of the multiplexer
    const static uint16_t SSC_MIN = 0;            // Minimum value the sensor returns
    const static uint16_t SSC_MAX = 0x3fff;       // 2^14 - 1
    const static float PRESSURE_MIN = 0.0;        // Min is 0 for sensors that give absolute values
    const static float PRESSURE_MAX = 206842.7;   // Max presure of the 30psi for this sensor converted to Pascals

    void selectSensor(uint8_t i);

  public:
    const static uint8_t TCA_INSIDE_SENSOR = 0;   //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
    const static uint8_t TCA_OUTSIDE_SENSOR = 1;  //SD#/SC# for the pressure sensor outside the balloon on the multiplexer

    int initialize(myHoneywellData *data1, myHoneywellData *data2);
    void read(myHoneywellData *data, uint8_t sensor);
};

 #endif
