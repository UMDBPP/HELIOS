
#include "../../include/hardware/mySwitch.h"

mySwitch::mySwitch(int inputPin){ pin = inputPin; }

void mySwitch::initialize(void){
  pinMode(pin, INPUT_PULLUP);
  lastState = digitalRead(pin);
  if (HELIOS_DEBUG) Serial.println("An input pin was initialized");
}

bool mySwitch::getStatus(void){
  checkStatus(NULL);
  return (lastState == LOW);
}

unsigned long mySwitch::timerOnStartTime(void){
  return timeFlippedOn;
}

unsigned long mySwitch::timerOffStartTime(void){
  return timeFlippedOff;
}

bool mySwitch::isOnActive(void){ return timerOnActive; }
bool mySwitch::isOffActive(void){ return timerOffActive; }

void mySwitch::checkStatus(myBITS* xbee){
  int newState = digitalRead(pin);
  if (newState != lastState){
    if (newState == LOW){ //The switch is on if it reads low
      timeFlippedOn = millis();
      timerOnActive = 1;
      if (HELIOS_DEBUG) Serial.println("Switch has been toggled to ON at: " + (String)timeFlippedOn + " ms");
      if (xbee){
        String str = "Started timer: " + (String)allData.gpsData.hour + ":" + (String)allData.gpsData.minute + ":" + (String)allData.gpsData.second + ".";
        xbee->sendToGround(str);
      }
    } else{
      timeFlippedOff = millis();
      timerOffActive = 1;
      if (HELIOS_DEBUG) Serial.println("Switch has been toggled to OFF at: " + (String)timeFlippedOn + " ms");
    }
    lastState = newState;
  }
}