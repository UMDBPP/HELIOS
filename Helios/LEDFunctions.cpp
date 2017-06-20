#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LED{
  private:
    Adafruit_NeoPixel led;
    const int PIN_LED = A0;

  public:

    LED(){} //constructor

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
      return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    }

    const static uint32_t OFF = (0 << 16) | (0 << 8) | 0;
    const static uint32_t BLUE = (0 << 16) | (0 << 8) | 255;
    const static uint32_t GREEN = (0 << 16) | (255 << 8) | 0;
    const static uint32_t RED = (255 << 16) | (0 << 8) | 0;
    const static uint32_t WHITE = (255 << 16) | (255 << 8) | 255;
    const static uint32_t YELLOW = (255 << 16) | (255 << 8) | 0;
    const static uint32_t PURPLE = (0 << 16) | (255 << 8) | 255;
    const static uint32_t TURQUOISE = (255 << 16) | (0 << 8) | 255;
    const static uint32_t ORANGE = (128 << 16) | (255 << 8) | 0;
    const static uint32_t HOTPINK = (100 << 16) | (255 << 8) | 180;


  
    int initialize(){
      led = Adafruit_NeoPixel(1, PIN_LED, NEO_GRB + NEO_KHZ800);
      led.begin();
      led.show(); //initialize the status led
      setStatus(OFF);
    }

    void setStatus(uint32_t color){
      led.setPixelColor(0, color);
      led.show();
    }
};
/*     
    static const uint32_t OFF = 0;
    const uint32_t BLUE = 255;
    const uint32_t GREEN = 65280;
    const uint32_t RED = 16711680;
    const uint32_t WHITE = 16777215;
    const uint32_t YELLOW = 16776960;
    const uint32_t PURPLE = 65535;
    const uint32_t TURQUOISE = 16711935;
    const uint32_t ORANGE = 8388863;
    const uint32_t HOTPINK = 6619060;
 */


