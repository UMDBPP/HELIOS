#include <Arduino.h>

class Motor{ //static class

  private:
    const static int PIN_MOTOR_A = 12; // Turning PIN_A high will make the motor blow air out of the balloon
    const static int PIN_MOTOR_B = 13;
    const static int PIN_MOTOR_PWM = 11;
    const static int MOTOR_SPEED = 255; //PWM oscillation between 0 and 255

  public:

    Motor(){}

    static int initialize(){
      pinMode(PIN_MOTOR_A, OUTPUT);
      pinMode(PIN_MOTOR_B, OUTPUT);
      pinMode(PIN_MOTOR_PWM, OUTPUT);
      return 1;
    }

    static void startFan(void){//turns fan on to prespecified speed
      Serial.println("Turning fan on");
      digitalWrite(PIN_MOTOR_A, HIGH);
      digitalWrite(PIN_MOTOR_B, LOW);
      analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
    }

    static void reverseFan(void){//turns fan on to prespecified speed
      Serial.println("Turning fan on");
      digitalWrite(PIN_MOTOR_A, LOW);
      digitalWrite(PIN_MOTOR_B, HIGH);
      analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
    }

    static void stopFan(void){//turns fan off
      Serial.println("Turning fan off");
      digitalWrite(PIN_MOTOR_A, LOW);
      digitalWrite(PIN_MOTOR_B, LOW);
      analogWrite(PIN_MOTOR_PWM, 0);
    }
};


