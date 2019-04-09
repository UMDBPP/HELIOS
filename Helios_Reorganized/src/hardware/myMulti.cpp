#include "../../include/hardware/myMulti.h"

static void myMulti::selectSensor(uint8_t addr) {//selects which pressure sensor we're talking to with the I2C multiplexer
  if (addr > 7){ //if invalid
      if (HELIOS_DEBUG) Serial.println("Invalid multiplexer address called.");
      return; 
  }
  Wire.beginTransmission(MULTI_ADDR);  //call the multiplexer
  Wire.write(1 << addr);
  Wire.endTransmission();
}

static void myMulti::initialize(void){
  if (TWCR == 0){ // if Wire library is not already initialized
    Wire.begin();
  }
}