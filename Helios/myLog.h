/*
 * Functions to help manage data logging using SD writer
 */

#ifndef LogFunctions
#define LogFunctions

#define HELIOS_DEBUG false
#include "Arduino.h"

class myDatalog{
  private:
    const static int SD_CHIP_SELECT = 53; //chip select for SPI card writer on a balloonduino
    
  public:
    int initialize();
    int write(String str);
};

#endif

