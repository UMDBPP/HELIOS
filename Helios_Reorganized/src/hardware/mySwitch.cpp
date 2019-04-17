
#include "../../include/hardware/mySwitch.h"

mySwitch::mySwitch(int inputPin){ pin = inputPin; }

void mySwitch::initialize(void){
  pinMode(pin, INPUT);
  lastState = digitalRead(pin);
  if (HELIOS_DEBUG) Serial.println("An input pin was initialized");
}

int mySwitch::getStatus(void){
  checkStatus();
  return lastState;
}

unsigned long mySwitch::timerStartTime(void){
  return timeFlippedOn;
}

unsigned long mySwitch::timerOtherTime(void){
  return timeFlippedOff;
}

void mySwitch::checkStatus(void){
  int newState = digitalRead(pin);
  if (newState != lastState){
    if (newState == HIGH){
      timeFlippedOn = millis();
    } else{
      timeFlippedOff = millis();
    }
    lastState = newState;
  }
}