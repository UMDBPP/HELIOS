#ifndef BMEFunctions
#define BMEFunctions

#include "Arduino.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "myMulti.h"

struct myBMEData{ //Preferred data structure for each sensor
  float pressure;
  float temperature;
  float altitude;
  float humidity;
};

class myBME{
  private:
    const static float SEA_LEVEL_PRESSURE_HPA  = 1013.25;
    Adafruit_BME280 bme[2];
    void selectSensor(uint8_t sensorIndex);

  public:
    myBME();
    const static uint8_t INDEX_BALLOON = 0;
    const static uint8_t INDEX_ATMOS = 1;
    int initialize(myBMEData &data1, myBMEData &data2);
    void read(myBMEData *data, uint8_t sensor);
};

#endif
