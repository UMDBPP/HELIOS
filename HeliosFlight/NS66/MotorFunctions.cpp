#include <Arduino.h>

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

class Motor{ //static class

  private:
    const static int PIN_MOTOR_A = 0; // Turning PIN_A high will make the motor blow air out of the balloon
    const static int PIN_MOTOR_B = 6;
    const static int PIN_MOTOR_PWM = 4;
    const static int MOTOR_SPEED = 155; //PWM oscillation between 0 and 255

  public:

    Motor(){}

    static int initialize(){
      pinMode(PIN_MOTOR_A, OUTPUT);
      pinMode(PIN_MOTOR_B, OUTPUT);
      pinMode(PIN_MOTOR_PWM, OUTPUT);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Motor pins have been initialized");
      return 1;
    }

    static void startFan(void){//turns fan on to prespecified speed
      //Serial.println("Turning fan on");
      digitalWrite(PIN_MOTOR_A, HIGH);
      digitalWrite(PIN_MOTOR_B, LOW);
      analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Fan is now on");
    }

    static void reverseFan(void){//turns fan on to prespecified speed
      //Serial.println("Turning fan on");
      digitalWrite(PIN_MOTOR_A, LOW);
      digitalWrite(PIN_MOTOR_B, HIGH);
      analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Fan is now running in reverse");
    }

    static void stopFan(void){//turns fan off
      //Serial.println("Turning fan off");
      digitalWrite(PIN_MOTOR_A, LOW);
      digitalWrite(PIN_MOTOR_B, LOW);
      digitalWrite(PIN_MOTOR_PWM, LOW);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Fan is now off");
    }
};


