#include <Arduino.h>

#define HELIOS_DEBUG true //this makes every function output what it is doing to serial as well, this can be individually enabled/disabled for every file
#define USING_GPS true //turning this false will tell the compiler to ignore anything involving the GPS, which is sometimes annoying during other tests

//these parameters correspond to the PCB print
#define LED1_PINR 32
#define LED1_PING 33
#define LED1_PINB 35

#define LED2_PINR 34
#define LED2_PING 37
#define LED2_PINB 36

#define BRIDGE1_PWM 2
#define BRIDGE1_A 40
#define BRIDGE1_B 41
#define ACT1_READ A0

#define BRIDGE2_PWM 3
#define BRDIGE2_A 42
#define BRIDGE2_B 43
#define ACT2_READ A1

#define BRIDGE3_A 4
#define BRIDGE3_B 5
#define BRIDGE3_C 6
#define BRIDGE3_D 7

#define SWITCH1 38
#define SWITCH2 39

#define NICHROME_PIN 16

#define NC A2 //this is a pin which should remain unconnected because it will be used as a placeholder

#define CS_SD 31  //chip select for the SD card
#define SPI_SS1 46  //Slave Select pins for various SPI peripherals
#define SPI_SS2 47
#define SPI_SS3 48
#define SPI_SS4 49

#define TCA_HONEYWELL_INSIDE_ADDR 1
#define TCA_HONEYWELL_OUTSIDE_ADDR 0
#define TCA_BME_INSIDE_ADDR 2
#define TCA_BME_OUTSIDE_ADDR 3
#define TCA_UNUSED 5

#define GPS_Serial Serial2
#define Xbee_Serial Serial3
//#include <SoftwareSerial.h>
