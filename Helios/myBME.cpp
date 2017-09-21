#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <Wire.h>

class bme{ //static class, not yet fully implemented
  private:
    const static float SEALEVELPRESSURE_HPA  = 1013.25;
    const static uint8_t MULTI_ADDR = 0x70;       // Address of the multiplexer

    static void selectSensor(uint8_t i) {//selects which pressure sensor we're talking to with the I2C multiplexer
      if (i > 7) return; //if invalid
      Wire.beginTransmission(MULTI_ADDR);  //call the multiplexer
      Wire.write(1 << i);
      Wire.endTransmission();  
    }
  
  public:
  
    static int initialize(){
      //declare BMEs, then begin
      /*for (int i=0; i<3; i++){
        selectSensor(i+2);
        if (!bme[i].begin()) {  
          Serial.println("Could not find a valid BME280 sensor, check wiring!");
          return 0;
        }
      }*/
      return 1;
    }
};




