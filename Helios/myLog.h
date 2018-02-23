/*
 * Functions to help manage data logging using SD writer
 */

#ifndef LogFunctions
#define LogFunctions

#include "Arduino.h"
#include "myPins.h"

class myDatalog{
  private:
    const static int SD_CHIP_SELECT = CS_SD; //chip select for SPI card writer on a balloonduino
    
  public:
    int initialize();
    int write(String str);
};

#endif

