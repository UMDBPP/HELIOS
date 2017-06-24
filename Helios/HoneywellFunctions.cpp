#include <Wire.h>
#include <Arduino.h>

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

//Data structure
struct MY_HONEYWELL{
  float pressure;
  float temperature;
  uint8_t status;
  uint16_t rawPressure;
  uint16_t rawTemperature;
};

class Honeywell{ //static class
  private:

    const static uint8_t SSC_ADDR = 0x28;         // Address of the sensor
    const static uint8_t MULTI_ADDR = 0x70;       // Address of the multiplexer
    const static uint16_t SSC_MIN = 0;            // Minimum value the sensor returns
    const static uint16_t SSC_MAX = 0x3fff;       // 2^14 - 1
    const static float PRESSURE_MIN = 0.0;        // Min is 0 for sensors that give absolute values
    const static float PRESSURE_MAX = 206842.7;   // Max presure of the 30psi for this sensor converted to Pascals
    
    static void selectSensor(uint8_t i) {//selects which pressure sensor we're talking to with the I2C multiplexer
      if (i > 7) return; //if invalid
      Wire.beginTransmission(MULTI_ADDR);  //call the multiplexer
      Wire.write(1 << i);
      Wire.endTransmission();  
    }
    
  public:

    const static uint8_t INSIDE_SENSOR = 0;   //SD#/SC# for the pressure sensor inside the balloon on the multiplexer
    const static uint8_t OUTSIDE_SENSOR = 1;  //SD#/SC# for the pressure sensor outside the balloon on the multiplexer

    Honeywell(){}
    
    static int initialize(MY_HONEYWELL **data){
      Wire.begin();
      data[0] = {};
      data[1] = {};
      if (HELIOS_DEBUG) DEBUG_SERIAL.println("Two honeywell sensors initialized");
      return 1;
    }

    static void read(MY_HONEYWELL *data, uint8_t sensor){
      selectSensor(sensor); //select to talk to the desired sensor
      uint8_t val[4] = {0};  //four bytes to store sensor data
      Wire.requestFrom(SSC_ADDR, (uint8_t) 4);    //request sensor data
      for (uint8_t i = 0; i <= 3; i++) {
          delay(4);                        // sensor might be missing, do not block by using Wire.available()
          val[i] = Wire.read();
      }
      data->status = (val[0] & 0xc0) >> 6; // first 2 bits from first byte are the status
      data->rawPressure = ((val[0] & 0x3f) << 8) + val[1];
      data->rawTemperature = ((val[2] << 8) + (val[3] & 0xe0)) >> 5;
      if (data->rawTemperature == 65535)
          data->status = 4;
      if (data->status == 4){
        data->pressure = -1;
        data->temperature = -1;
      }
      else{
        data->pressure = 1.0 * (data->rawPressure - SSC_MIN) * (PRESSURE_MAX - PRESSURE_MIN) / (SSC_MAX - SSC_MIN) + PRESSURE_MIN;
        data->temperature = (data->rawTemperature * 0.0977) - 50;
      }
    }
};


