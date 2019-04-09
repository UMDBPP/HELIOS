#ifndef MULTIFUNCTIONS
#define MULTIFUNCTIONS

#include <Arduino.h>
#include "../../myPins.h"
#include <Wire.h>

class myMulti{
  private:
    const static uint8_t MULTI_ADDR = 0x70;
    const static uint8_t UNUSED_ADDR = TCA_UNUSED;
  public:
    const static uint8_t BME_BALLOON = TCA_BME_INSIDE_ADDR;
    const static uint8_t BME_ATMOS = TCA_BME_OUTSIDE_ADDR;
    const static uint8_t HONEYWELL_BALLOON = TCA_HONEYWELL_INSIDE_ADDR;
    const static uint8_t HONEYWELL_ATMOS = TCA_HONEYWELL_OUTSIDE_ADDR;

    static void selectSensor(uint8_t i);
    static void initialize(void);
};

#endif