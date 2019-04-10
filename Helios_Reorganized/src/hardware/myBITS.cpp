#include "../../include/hardware/myBITS.h"

int myBITS::initialize(void){
    Xbee_Serial.begin(9600);
    xbee.setSerial(Xbee_Serial);
    if (HELIOS_DEBUG) Serial.println("Xbee initialized");
}

bool myBITS::sendToGround(char* message, uint8_t length){
  /**
   * This function will assign char* message to a packet, and send it to BITS to send to the ground
  */
}

int myBITS::checkForMessage(void){
  /**
   * This function should check the xbee for a message, and assign it to a char* or uint8_t* array.
   * If there is a message, it will call processMessage and return its output
  */
}

int myBITS::processMessage(void){
  /**
   * This function will compare the string in XbeeReceiveBuf to the list of valid commands
   * If it matches one of these commands, it will return the corresponding command number as an integer.
   * If it does not match any known command, it will return COMMAND_ERROR
  */
}


