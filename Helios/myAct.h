/* 
 * Class for managing actuator functions 
 */
#ifndef ActuatorFunctions
#define ActuatorFunctions

#define HELIOS_DEBUG false
#include "Arduino.h"

class myActuator{
  private:
    const static int PIN_ACTUATOR_A = 3; //Turning PIN_A high will make the actuator extend
    const static int PIN_ACTUATOR_B = 2;
    const static int PIN_ACTUATOR_PWM = 5;
    const static int PIN_ACTUATOR_READ = A0;
    
  public:
    const static float START = 1.5;
    const static float END = 48.5;
    int initialize();
    float position();
    uint8_t openValve();
    uint8_t closeValve();
    uint8_t stopValve();
};

#endif
