#include "Arduino.h"
#include "myLED.h"

myLED::myLED(int r, int g, int b){
  PIN_R = r;
  PIN_G = g;
  PIN_B = b;
}

int myLED::initialize(){
  setStatus(OFF);
}

void myLED::setStatus(char c){
  switch(c){
    case OFF:
      digitalWrite(PIN_R, LOW);
      digitalWrite(PIN_G, LOW);
      digitalWrite(PIN_B, LOW);
      break;
    case RED:
      digitalWrite(PIN_R, HIGH);
      digitalWrite(PIN_G, LOW);
      digitalWrite(PIN_B, LOW);
      break;
    case GREEN:
      digitalWrite(PIN_R, LOW);
      digitalWrite(PIN_G, HIGH);
      digitalWrite(PIN_B, LOW);
      break;
    case BLUE:
      digitalWrite(PIN_R, LOW);
      digitalWrite(PIN_G, LOW);
      digitalWrite(PIN_B, HIGH);
      break;
    case YELLOW:
      digitalWrite(PIN_R, HIGH);
      digitalWrite(PIN_G, HIGH);
      digitalWrite(PIN_B, LOW);
      break;
    case MAGENTA:
      digitalWrite(PIN_R, HIGH);
      digitalWrite(PIN_G, LOW);
      digitalWrite(PIN_B, HIGH);
      break;
    case CYAN:
      digitalWrite(PIN_R, LOW);
      digitalWrite(PIN_G, HIGH);
      digitalWrite(PIN_B, HIGH);
      break;
    case WHITE:
      digitalWrite(PIN_R, HIGH);
      digitalWrite(PIN_G, HIGH);
      digitalWrite(PIN_B, HIGH);
      break;
  }
}


/*
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef Serial
#define Serial Serial
#endif

class ALED{
  private:
    const int PIN_LED = 17;
    Adafruit_NeoPixel myled = Adafruit_NeoPixel(1, PIN_LED, NEO_GRB + NEO_KHZ800);

  public:

    ALED(){} //constructor

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
      return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    }

    static const uint32_t OFF = 0;
    static const uint32_t BLUE = 255;
    static const uint32_t GREEN = 65280;
    static const uint32_t RED = 16711680;
    static const uint32_t WHITE = 16777215;
    static const uint32_t YELLOW = 16776960;
    static const uint32_t TURQOISE = 65535;
    static const uint32_t PURPLE = 16711935;
    static const uint32_t ORANGE = 8388863;
    static const uint32_t HOTPINK = 6619060;
  
    void initialize(){
      myled.begin();
      myled.show(); //initialize the status led
      if (HELIOS_DEBUG) Serial.println("Status LED initialized");
      setStatus(OFF);
    }

    void setStatus(uint32_t color){
      myled.setPixelColor(0, color);
      myled.show();
      //if (HELIOS_DEBUG) Serial.println("LED status changed");
    }
};*/
