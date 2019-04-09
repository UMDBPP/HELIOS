#include <Arduino.h>
#include "../../include/hardware/myAct.h"

void myActuator::initialize(){  //initialize all pins and set to low
  pinMode(PIN_ACTUATOR_A, OUTPUT);
  pinMode(PIN_ACTUATOR_B, OUTPUT);
  pinMode(PIN_ACTUATOR_PWM, OUTPUT);
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, LOW);
  pinMode(PIN_ACTUATOR_READ, INPUT);
  if (HELIOS_DEBUG) Serial.println("Valve pins have been initialized");
}

float myActuator::position(){ //return the position of the valve by linear interpolation of signal read
  return 50.0*analogRead(PIN_ACTUATOR_READ)/1032.0;
}

boolean myActuator::openValve(void){//opens the valve by retracting the piston
  digitalWrite(PIN_ACTUATOR_A, HIGH);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  if (HELIOS_DEBUG) Serial.println("Valve Opening");
  return OPEN;
}

boolean myActuator::closeValve(void){//closes the valve by extending the piston
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, HIGH);
  digitalWrite(PIN_ACTUATOR_PWM, HIGH);
  if (HELIOS_DEBUG) Serial.println("Valve Closing");
  return CLOSED;
}

void myActuator::stopValve(void){//puts zero voltage on each side of valve
  digitalWrite(PIN_ACTUATOR_A, LOW);
  digitalWrite(PIN_ACTUATOR_B, LOW);
  digitalWrite(PIN_ACTUATOR_PWM, LOW);
  //if (HELIOS_DEBUG) Serial.println("Valve off");
}
