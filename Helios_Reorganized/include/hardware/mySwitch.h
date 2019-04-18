
#ifndef SwitchFunctions
#define SwitchFunctions

#include "Arduino.h"
#include "../../myPins.h"

class mySwitch{
  private:
    int pin;
    unsigned long timeFlippedOn = 0;
    unsigned long timeFlippedOff = 0;
    bool timerOnActive = 0;
    bool timerOffActive = 0;
    int lastState;

  public:
    mySwitch(int);
    void initialize(void);
    bool getStatus(void);
    void checkStatus(void);
    unsigned long timerOnStartTime(void);
    unsigned long timerOffStartTime(void); //not yet sure what this would be used for
    bool isOnActive(void);
    bool isOffActive(void);
};



#endif