#include "Arduino.h"
#include "myAct.h"
  
int myActuator::initialize(){
  pinMode(PIN_ACTUATOR_A, OUTPUT);
  pinMode(PIN_ACTUATOR_B, OUTPUT);
  pinMode(PIN_ACTUATOR_PWM, OUTPUT);
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, LOW);
  pinMode(A0, INPUT);
  if (HELIOS_DEBUG) Serial.println("Valve pins have been initialized");
  return 1;
}

float myActuator::position(){
  return 50.0*analogRead(A0)/1032.0;
}

uint8_t myActuator::openValve(void){//opens the valve by retracting the piston
  digitalWrite(PIN_ACTUATOR_A, HIGH);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  if (HELIOS_DEBUG) Serial.println("Valve Opening");
  return 1;
}

uint8_t myActuator::closeValve(void){//closes the valve by extending the piston 
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, HIGH);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  if (HELIOS_DEBUG) Serial.println("Valve Closing");
  return 0;
}

uint8_t myActuator::stopValve(void){//puts zero voltage on each side of valve
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, LOW);
  //if (HELIOS_DEBUG) Serial.println("Valve off");
  return 2;
}
