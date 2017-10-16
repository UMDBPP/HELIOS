#include "ccsds_xbee.h"
#include "Arduino.h"
#include "myXbee.h"
#include "myPins.h"

boolean myXbee::packet_processing(uint8_t Pkt_Buff[]){
  if(getAPID(Pkt_Buff) != AP_ID_CMD){ //check that we have the correct AP_ID
    sendError(ERROR_CODE_BAD_ID);
    return 0;
  }
  if(getPacketType(Pkt_Buff) != CCSDS_CMD_PKT){ //check that what we have received is a command
    sendError(ERROR_CODE_NOT_COMMAND);
    return 0;
  }
  if(!packetHasValidChecksum(ERROR_CODE_BAD_CHECKSUM)){  //check that we have received a valid checksum
    sendError(5);
    //return 0; //this was removed because it was happening repeatedly even when signals went through, should be investigated further
  }

  if (HELIOS_DEBUG) Serial.println((String)millis() + "Valid packet has been received");
  
  //if everything is good, then we will actually process the command
  uint8_t command = getCmdFunctionCode(Pkt_Buff);
  
  if (command == 0){ //this is the enable command
    lastCommand = COMMAND_ENABLE;
    return 1;
  }else if (command == 1){ //this is the receive telemetry command
    lastCommand = COMMAND_REQUEST_DATA; //send the most recent data
    return 1;
  } else if (command == 2){ //this is the abort command
    lastCommand = COMMAND_ABORT;
    return 1;
  } else if (command == 3){ //this is the open/test for five seconds command
    lastCommand = COMMAND_VENT_NOW;
    lastCommandedTime = 5000;
    return 1;
  } else if (command == 4){ //this is the vent for a predefined period command
    lastCommand = COMMAND_VENT_NOW;
    lastCommandedTime = (int32_t)TIME_TO_VENT;
    return 1;
  } else if (command == 5){ //this is the open for x seconds command
    lastCommand = COMMAND_VENT_NOW;
    uint16_t pkt_param = 0;
    extractFromTlm(pkt_param, Pkt_Buff, PARAM_POS);
    lastCommandedTime = (int32_t)pkt_param*1000; 
    return 1;
  } else if (command == 6){ //this is the open forever command
    lastCommand = COMMAND_VENT_NOW;
    lastCommandedTime = 2147483647; 
    return 1;
  } else if (command == 7){ //this is the set new amount of time command
    lastCommand = COMMAND_SET_TIME;
    uint16_t pkt_param = 0;
    extractFromTlm(pkt_param, Pkt_Buff, PARAM_POS);
    lastCommandedTime = (int32_t)pkt_param*1000; 
    return 1;
  } else if (command == 8){ //this is the set new altitude command
    lastCommand = COMMAND_SET_ALT;
    uint32_t pkt_param = 0;
    extractFromTlm(pkt_param, Pkt_Buff, PARAM_POS);
    lastCommandedTime = (int32_t)pkt_param; 
    return 1;
  } else if (command == 9){ //this is the reverse for a predefined period command
    lastCommand = COMMAND_REVERSE_NOW;
    lastCommandedTime = (int32_t)TIME_TO_REVERSE; 
    return 1;
  } else if (command == 10){ //this is the reverse for x seconds command
    lastCommand = COMMAND_REVERSE_NOW;
    uint16_t pkt_param = 0;
    extractFromTlm(pkt_param, Pkt_Buff, PARAM_POS);
    lastCommandedTime = (int32_t)pkt_param*1000; 
    return 1;
  } else if (command == 11){ //this is the reverse forever command
    lastCommand = COMMAND_REVERSE_NOW;
    lastCommandedTime = 2147483647; 
    return 1;
  } else if (command == 12){ //this is the command to open the valve indefinitely with nothing else
    lastCommand = COMMAND_TEST_OPEN;
    return 1;
  } else if (command == 13){ //this is the command to close the valve indefinitely with nothing else
    lastCommand = COMMAND_TEST_CLOSE;
    return 1;
  } else if (command == 14){ //this is the command to run the fan forwards indefinitely with nothing else
    lastCommand = COMMAND_TEST_FWD;
    return 1;
  } else if (command == 15){ //this is the command to run the fan backwards indefinitely with nothing else
    lastCommand = COMMAND_TEST_REV;
    return 1;
  } else if (command == 16){ //this is the reset command, FYI this currently does nothing
    lastCommand = COMMAND_RESET;
    return 1;
  } else if (command == 17){
    lastCommand = COMMAND_ALL_DATA;
    return 1;
  }
  return 0;
}

uint32_t myXbee::getCommandedTime(){return lastCommandedTime;}  //utility functions to get variables

uint8_t myXbee::getLastCommand(){return lastCommand;}

int myXbee::initialize(){ //initialize the xbee and return valve if there is an error
  Xbee_Serial.begin(9600); //start commmunication with xbee
  uint8_t initstat = xbee.init(XBEE_THIS_ADDR, XBee_PAN_ID, Xbee_Serial);
  if(!initstat) {
    if(HELIOS_DEBUG) Serial.println("XBee Initialized!");
    return 1;
  } else {
    if(HELIOS_DEBUG) Serial.print("XBee Failed to Initialize with Error Code: ");
    if(HELIOS_DEBUG) Serial.println(initstat);
    return 0;
  }
}
    
void myXbee::sendData(uint32_t altitude, float ascentVelocity){ //function to send the altitude and ascent velocity upon request
  uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
  uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
  payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
  payload_size = addIntToTlm(altitude, payload_buff, payload_size);
  payload_size = addFloatToTlm(ascentVelocity, payload_buff, payload_size); //the payload is the send number, the altitude, and the ascent velocity
  uint32_t pre_send = xbee.getSentPktCtr();

  xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_TLM, payload_buff, payload_size); //note that we are not checking that the packet sent successfully; if it does not send, it is up to the user to ask again
  
  numTries++; //increment the number of messages we have sent
}

void myXbee::sendAllData(uint32_t altitude, uint8_t lat_deg, float lat_min, uint8_t lon_deg, float lon_min, float velocity, float pres_in, float pres_out, float temp_in, float temp_out, float position){
  uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
  uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
  payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
  payload_size = addIntToTlm(altitude, payload_buff, payload_size);
  payload_size = addIntToTlm(lat_deg, payload_buff, payload_size);
  payload_size = addFloatToTlm(lat_min, payload_buff, payload_size);
  payload_size = addIntToTlm(lon_deg, payload_buff, payload_size);
  payload_size = addFloatToTlm(lon_min, payload_buff, payload_size);
  payload_size = addFloatToTlm(velocity, payload_buff, payload_size);
  payload_size = addFloatToTlm(pres_in, payload_buff, payload_size);
  payload_size = addFloatToTlm(pres_out, payload_buff, payload_size);
  payload_size = addFloatToTlm(temp_in, payload_buff, payload_size);
  payload_size = addFloatToTlm(temp_out, payload_buff, payload_size);
  payload_size = addFloatToTlm(position, payload_buff, payload_size);
  uint32_t pre_send = xbee.getSentPktCtr();
  xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_DATA, payload_buff, payload_size); //note that we are not checking that the packet sent successfully; if it does not send, it is up to the user to ask again
  numTries++; //increment the number of messages we have sent
}

void myXbee::sendError(uint8_t errCode){  //function to send an error code
  uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
  uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
  payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
  payload_size = addIntToTlm(errCode, payload_buff, payload_size);
  uint32_t pre_send = xbee.getSentPktCtr();

  xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_ERR, payload_buff, payload_size);
  
  numTries++; //increment the number of messages we have sent
}
    
void myXbee::sendConf(uint8_t confCode, int32_t confTime){  //function to send a confirmation code plus a number indicated the vlaue received
  uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
  uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
  payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
  payload_size = addIntToTlm(confCode, payload_buff, payload_size);
  payload_size = addIntToTlm(confTime, payload_buff, payload_size);
  uint32_t pre_send = xbee.getSentPktCtr();

  xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_CON, payload_buff, payload_size);
  
  numTries++; //increment the number of messages we have sent
}
    
boolean myXbee::receive(){  //function to check continuously if the xbee received any data
  uint8_t Pkt_Buff[PKT_MAX_LEN];
  uint8_t pktLength = 0;
  uint8_t bytes_read = 0;
  bytes_read = xbee.readMsg(Pkt_Buff);

  if(bytes_read < 0){
    sendError(ERROR_CODE_NEGATIVE);  //an error has occurred so we will let the user know
    return 0;
    }
  else if(bytes_read == 0){
    return 0; //no packets read, no action required
  }
  else if(bytes_read != getPacketLength(Pkt_Buff)){
    sendError(ERROR_CODE_BAD_LENGTH);
    return 0; //an error has occurred so we will let the user know
  }
  else{
    return packet_processing(Pkt_Buff); //if the data looks good, we'll actually look at the content
  }
}
