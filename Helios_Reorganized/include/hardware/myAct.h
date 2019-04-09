/*
 * Class for managing actuator functions
 */
#ifndef ActuatorFunctions
#define ActuatorFunctions

#include "Arduino.h"
#include "../../myPins.h"

class myActuator{
  private:
    const static int PIN_ACTUATOR_A = BRIDGE1_A; //Turning PIN_A high will make the actuator extend
    const static int PIN_ACTUATOR_B = BRIDGE1_B;  //these values are defined in the myPins sheet
    const static int PIN_ACTUATOR_PWM = BRIDGE1_PWM;
    const static int PIN_ACTUATOR_READ = ACT1_READ;
    const static boolean CLOSED = false;
    const static boolean OPEN = true;

  public:
    const static float START = 1.5;
    const static float END = 48.5;
    void initialize();
    float position();
    boolean openValve();
    boolean closeValve();
    void stopValve();
};

#endif
