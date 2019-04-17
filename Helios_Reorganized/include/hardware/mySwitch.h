
#ifndef SwitchFunctions
#define SwitchFunctions

#include "Arduino.h"
#include "../../myPins.h"

class mySwitch{
  private:
    int pin;
    unsigned long timeFlippedOn=0;
    unsigned long timeFlippedOff=0;
    int lastState;

  public:
    mySwitch(int);
    void initialize(void);
    int getStatus(void);
    unsigned long timerStartTime(void);
    void checkStatus(void);
};



#endif