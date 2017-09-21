/*
 * Functions to manage LEDs
 */
#ifndef LEDFunctions
#define LEDFunctions

#define HELIOS_DEBUG false
#include "Arduino.h"

class myLED{
  private:
    int PIN_R;
    int PIN_G;
    int PIN_B;

  public:
    const static char OFF = 'o';
    const static char RED = 'r';
    const static char GREEN = 'g';
    const static char BLUE = 'b';
    const static char MAGENTA = 'a';
    const static char YELLOW = 'y';
    const static char CYAN = 'c';
    const static int WHITE = 'w';
  
    myLED(int r, int g, int b);
    int initialize();
    void setStatus(char c);
};

#endif
