#include <Arduino.h>
#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

class Actuator{ //static class

  private:
    const static int PIN_ACTUATOR_A = 3; //Turning PIN_A high will make the actuator extend
    const static int PIN_ACTUATOR_B = 2;
    const static int PIN_ACTUATOR_PWM = 5;
    const static int PIN_ACTUATOR_READ = A0;
    
  public:
    const static float START = 1.5;
    const static float END = 48.5;

    Actuator(){}
  
    static int initialize(){
      pinMode(PIN_ACTUATOR_A, OUTPUT);
      pinMode(PIN_ACTUATOR_B, OUTPUT);
      pinMode(PIN_ACTUATOR_PWM, OUTPUT);
      pinMode(A0, INPUT);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Valve pins have been initialized");
      return 1;
    }

    static float position(){
      return 50.0*analogRead(A0)/1032.0;
    }

    static uint8_t openValve(void){//opens the valve by retracting the piston
      digitalWrite(PIN_ACTUATOR_A, HIGH);
      digitalWrite(PIN_ACTUATOR_B, LOW);
      digitalWrite(PIN_ACTUATOR_PWM, HIGH);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Valve Opening");
      return 1;
    }

    static uint8_t closeValve(void){//closes the valve by extending the piston 
      digitalWrite(PIN_ACTUATOR_A, LOW);
      digitalWrite(PIN_ACTUATOR_B, HIGH);
      digitalWrite(PIN_ACTUATOR_PWM, HIGH);
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Valve Closing");
      return 0;
    }

    static void stopValve(void){//puts zero voltage on each side of valve
      digitalWrite(PIN_ACTUATOR_A, LOW);
      digitalWrite(PIN_ACTUATOR_B, LOW);
      digitalWrite(PIN_ACTUATOR_PWM, LOW);
      //if (HELIOS_DEBUG) DEBUG_SERIAL.println("Valve off");
      return;
    }
};





