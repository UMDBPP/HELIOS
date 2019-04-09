#include "../../include/hardware/myMotor.h"


#if SLA5074

void myMotor::initialize(){
  pinMode(PIN_SLA_FWD_HIGH, OUTPUT);
  pinMode(PIN_SLA_FWD_LOW, OUTPUT);
  pinMode(PIN_SLA_BACK_HIGH, OUTPUT);
  pinMode(PIN_SLA_BACK_LOW, OUTPUT);
  digitalWrite(PIN_SLA_FWD_HIGH, LOW);
  digitalWrite(PIN_SLA_FWD_LOW, LOW);
  digitalWrite(PIN_SLA_BACK_HIGH, LOW);
  digitalWrite(PIN_SLA_BACK_LOW, LOW);
  if (HELIOS_DEBUG) Serial.println("Motor pins have been initialized");
}

void myMotor::startFan(void){//turns fan on to prespecified speed
  digitalWrite(PIN_SLA_BACK_HIGH, LOW);
  digitalWrite(PIN_SLA_BACK_LOW, LOW);
  analogWrite(PIN_SLA_FWD_HIGH, 255);
  digitalWrite(PIN_SLA_FWD_LOW, LOW);
  if (HELIOS_DEBUG) Serial.println("Fan is now on");
}

void myMotor::reverseFan(void){//turns fan on to prespecified speed
  digitalWrite(PIN_SLA_FWD_HIGH, LOW);
  digitalWrite(PIN_SLA_FWD_LOW, LOW);
  analogWrite(PIN_SLA_BACK_HIGH, 255);
  digitalWrite(PIN_SLA_BACK_LOW, HIGH);
  if (HELIOS_DEBUG) Serial.println("Fan is now running in reverse");
}

void myMotor::stopFan(void){//turns fan off
  digitalWrite(PIN_SLA_BACK_HIGH, LOW);
  digitalWrite(PIN_SLA_BACK_LOW, LOW);
  digitalWrite(PIN_SLA_FWD_HIGH, LOW);
  digitalWrite(PIN_SLA_FWD_LOW, LOW);
  if (HELIOS_DEBUG) Serial.println("Fan is now off");
}

#else

void myMotor::initialize(){
  pinMode(PIN_MOTOR_A, OUTPUT);
  pinMode(PIN_MOTOR_B, OUTPUT);
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  digitalWrite(PIN_MOTOR_A, LOW);
  digitalWrite(PIN_MOTOR_B, LOW);
  digitalWrite(PIN_MOTOR_PWM, LOW);
  if (HELIOS_DEBUG) Serial.println("Motor pins have been initialized");
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

#endif
