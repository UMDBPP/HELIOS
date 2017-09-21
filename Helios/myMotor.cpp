#include "Arduino.h"
#include "myMotor.h"

int myMotor::initialize(){
  pinMode(PIN_MOTOR_A, OUTPUT);
  pinMode(PIN_MOTOR_B, OUTPUT);
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, LOW);
  digitalWrite(PIN_MOTOR_PWM, LOW);
  if (HELIOS_DEBUG) Serial.println("Motor pins have been initialized");
  return 1;
}

void myMotor::startFan(void){//turns fan on to prespecified speed
  //Serial.println("Turning fan on");
  digitalWrite(PIN_MOTOR_A, HIGH);
  digitalWrite(PIN_MOTOR_B, LOW);
  analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
  if (HELIOS_DEBUG) Serial.println("Fan is now on");
}

void myMotor::reverseFan(void){//turns fan on to prespecified speed
  //Serial.println("Turning fan on");
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, HIGH);
  analogWrite(PIN_MOTOR_PWM, MOTOR_SPEED);
  if (HELIOS_DEBUG) Serial.println("Fan is now running in reverse");
}

void myMotor::stopFan(void){//turns fan off
  //Serial.println("Turning fan off");
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, LOW);
  digitalWrite(PIN_MOTOR_PWM, LOW);
  if (HELIOS_DEBUG) Serial.println("Fan is now off");
}
