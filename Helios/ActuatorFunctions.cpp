#include <Arduino.h>

class Actuator{ //static class

  private:
    const static int PIN_ACTUATOR_A = 10; //Turning PIN_A high will make the actuator extend
    const static int PIN_ACTUATOR_B = 9;
    const static int PIN_ACTUATOR_PWM = 6;
    const static int PIN_ACTUATOR_READ = A2;
    
  public:
    const static float START = 1.0;
    const static float END = 49.0;

    Actuator(){}
  
    static int initialize(){
      pinMode(PIN_ACTUATOR_A, OUTPUT);
      pinMode(PIN_ACTUATOR_B, OUTPUT);
      pinMode(PIN_ACTUATOR_PWM, OUTPUT);
      pinMode(PIN_ACTUATOR_READ, INPUT);
      return 1;
    }

    static float position(){
      return 50.0*analogRead(PIN_ACTUATOR_READ)/1032.0;
    }

    static uint8_t openValve(void){//opens the valve by retracting the piston
      //Serial.println("Opening Valve");
      digitalWrite(PIN_ACTUATOR_A, HIGH);
      digitalWrite(PIN_ACTUATOR_B, LOW);
      digitalWrite(PIN_ACTUATOR_PWM, HIGH);
      return 1;
    }

    static uint8_t closeValve(void){//closes the valve by extending the piston
      //Serial.println("Closing Valve");  
      digitalWrite(PIN_ACTUATOR_A, LOW);
      digitalWrite(PIN_ACTUATOR_B, HIGH);
      digitalWrite(PIN_ACTUATOR_PWM, HIGH);
      return 0;
    }

    static void stopValve(void){//puts zero voltage on each side of valve
      //Serial.println("Turning Valve Off");
      digitalWrite(PIN_ACTUATOR_A, LOW);
      digitalWrite(PIN_ACTUATOR_B, LOW);
      digitalWrite(PIN_ACTUATOR_PWM, LOW);
    }
};





