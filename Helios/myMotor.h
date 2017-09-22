/*
 * Functions to control motor
 */

#ifndef MotorFunctions
#define MotorFunctions

#define HELIOS_DEBUG false
#include "Arduino.h"
#include "myPins.h"

class myMotor{
  private:
    const static int PIN_MOTOR_A = SWITCH1; // Turning PIN_A high will make the motor blow air out of the balloon
    const static int PIN_MOTOR_B = NC;
    const static int PIN_MOTOR_PWM = NC;
    const static int MOTOR_SPEED = 155; //PWM oscillation between 0 and 255
  public:
    void initialize();
    void startFan(void);
    void reverseFan(void);
    void stopFan(void);
};

#endif
