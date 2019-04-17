#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <Arduino.h>
#include "../hardware/myLog.h"  //defines functions for interacting with SD card logging
#include "data.h"  //defines functions for working with BME280 pressure sensors

void logData(const myDatalog &datalog, const myData &allData, const valveState &valve, const valveState &cutdown, const float &actPosition);

int availableMemory(void);

#endif