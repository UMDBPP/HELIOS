/*
 * Functions to control motor
 */

#ifndef MotorFunctions
#define MotorFunctions

#define HELIOS_DEBUG true
#include "Arduino.h"
#include "myPins.h"

#define SLA5074 false //Using the SLA5074 H bridge or the plain n channel fet

class myMotor{
  private:
    const static int PIN_MOTOR_A = NC; // Turning PIN_A high will make the motor blow air out of the balloon
    const static int PIN_MOTOR_B = NC;
    const static int PIN_MOTOR_PWM = SWITCH1;
    const static int MOTOR_SPEED = 155; //PWM oscillation between 0 and 255
    const static int PIN_SLA_FWD_HIGH = BRIDGE3_A;
    const static int PIN_SLA_BACK_LOW = BRIDGE3_B;
    const static int PIN_SLA_BACK_HIGH = BRIDGE3_C;
    const static int PIN_SLA_FWD_LOW = BRIDGE3_D;
  public:
    void initialize();
    void startFan(void);
    void reverseFan(void);
    void stopFan(void);
};

#endif
