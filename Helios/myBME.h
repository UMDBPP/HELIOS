#ifndef BMEFunctions
#define BMEFunctions

#include "Arduino.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "myPins.h"

struct myBMEData{ //Preferred data structure for each sensor
  float pressure;
  float temperature;
  float altitude;
  float humidity;
};

class myBME{
  private:
    const static float SEA_LEVEL_PRESSURE_HPA  = 1013.25;
    const static uint8_t MULTI_ADDR = 0x70;       // Address of the multiplexer
    const static uint8_t INSIDE_ADDR = TCA_BME_INSIDE_ADDR;
    const static uint8_t OUTSIDE_ADDR = TCA_BME_OUTSIDE_ADDR;
    const static uint8_t UNUSED_ADDR = TCA_UNUSED;
    Adafruit_BME280 bme[2];
    static void selectSensor(uint8_t i);
  
  public:
    myBME();
    const static uint8_t TCA_INSIDE_SENSOR = 1;   //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
    const static uint8_t TCA_OUTSIDE_SENSOR = 0; 

    int initialize(myBMEData *data1, myBMEData *data2);
    void read(myBMEData *data, uint8_t sensor);
    uint8_t getSensorLocation(uint8_t sensor_number);
};

#endif
