
#include "../../include/hardware/myNichrome.h"

void myNichrome::initialize(){
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);
  if (HELIOS_DEBUG) Serial.println("Nichrome pins have been initialized");
}

void myNichrome::startHeat(){
  digitalWrite(PIN, HIGH);
  if (HELIOS_DEBUG) Serial.println("Nichrome has been turned on.");
}

void myNichrome::endHeat(){
  digitalWrite(PIN, LOW);
  if (HELIOS_DEBUG) Serial.println("Nichrome has been turned off.");
}