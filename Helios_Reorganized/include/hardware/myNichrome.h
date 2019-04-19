
#ifndef NichromeFunctions
#define NichromeFunctions

#include <Arduino.h>
#include "../../myPins.h"

class myNichrome{
  private:
    int state=2;
    const static int PIN = NICHROME_PIN;
  public:
    void initialize();
    void startHeat(void);
    void endHeat(void);
};

#endif