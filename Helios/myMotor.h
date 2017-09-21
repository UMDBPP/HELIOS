/*
 * Functions to control motor
 */

#ifndef MotorFunctions
#define MotorFunctions

#define HELIOS_DEBUG false
#include "Arduino.h"

class myMotor{
  private:
    const static int PIN_MOTOR_A = 0; // Turning PIN_A high will make the motor blow air out of the balloon
    const static int PIN_MOTOR_B = 6;
    const static int PIN_MOTOR_PWM = 4;
    const static int MOTOR_SPEED = 155; //PWM oscillation between 0 and 255
  public:
    int initialize();
    void startFan(void);
    void reverseFan(void);
    void stopFan(void);
};

#endif
