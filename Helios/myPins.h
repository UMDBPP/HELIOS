#include "Arduino.h"

//these parameters correspond to the PCB print
#define LED1_PINR 32
#define LED1_PING 35
#define LED1_PINB 33

#define LED2_PINR 34
#define LED2_PING 36
#define LED2_PINB 37

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

#define NC A2 //this is a pin which should remain unconnected because it will be used as a placeholder

#define CS_SD 31  //chip select for the SD card
#define SPI_SS1 46  //Slave Select pins for various SPI peripherals
#define SPI_SS2 47
#define SPI_SS3 48
#define SPI_SS4 49

#define GPS_Serial Serial2
#define Xbee_Serial Serial3
