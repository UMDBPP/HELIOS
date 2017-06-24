#include "ccsds_xbee.h"
#include <Arduino.h>

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#define _NO_RTC_
#define PKT_MAX_LEN 100
#define XbeeSerial Serial3 //Pins 14 and 15, the location of the xbee on balloonduino board

class XBEE{

  private:

    const int XBEE_WRITE_ADDR = 0x0002;
    const int XBEE_THIS_ADDR = 0x0008;
    const int XBee_PAN_ID = 0x0B0B;
    const int AP_ID_CMD = 800;   //the AP_ID that we must see whenever we receive a packet
    const int AP_ID_TLM = 810;   //the AP_ID that will be used to send all telemetry packets
    const int AP_ID_ERR = 890;   //the AP_ID that will be used to send all error packets
    const int AP_ID_CON = 850;   //the AP_ID used to send a confirmation
    const uint32_t TIME_TO_OPEN_REVERSE = 120000; //this is the time for which the valve will blow air into the balloon if commanded in reverse

    CCSDS_Xbee xbee;
    uint16_t numTries = 1; //number of transmissions we have made since startup
    uint8_t lastCommand;
    int32_t lastCommandedTime;

    boolean packet_processing(uint8_t Pkt_Buff[]){
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
        //return 0;
      }

      if (HELIOS_DEBUG) DEBUG_SERIAL.println((String)millis() + "Valid packet has been received");
      
      //if everything is good, then we will actually process the command
      uint8_t command = getCmdFunctionCode(Pkt_Buff);
      if (command == 0){ //this is the receive telemetry command
        lastCommand = COMMAND_REQUEST_DATA; //send the most recent data
        return 1;
      } else if (command == 1){ //this is the abort command
        lastCommand = COMMAND_ABORT;
        return 1;
      } else if (command == 127){ //this is the reverse fan command
        lastCommand = COMMAND_REVERSE;
        lastCommandedTime = TIME_TO_OPEN_REVERSE;
        return 1;
      }
      else if (command > 1){ //this is the open now command
        lastCommand = COMMAND_OPEN_NOW;
        lastCommandedTime = ((int32_t)command-1)*10000; //in this case, the time open is the command value times 10 in seconds
        return 1;
      }
    }
  
  public:

    const int WAIT_TIME_AFTER_COMMAND = 1000;    //the wait time before another command can be received

    const static int COMMAND_REQUEST_DATA = 0;
    const static int COMMAND_ABORT = 1;
    const static int COMMAND_REVERSE = 2;
    const static int COMMAND_OPEN_NOW = 3;

    const static int ERROR_CODE_NEGATIVE = 1;
    const static int ERROR_CODE_BAD_LENGTH = 2;
    const static int ERROR_CODE_BAD_ID = 3;
    const static int ERROR_CODE_NOT_COMMAND = 4;
    const static int ERROR_CODE_BAD_CHECKSUM = 5;

    const static int CONFIRM_CODE_ABORT = 1;
    const static int CONFIRM_CODE_REVERSE = 200;

    uint32_t getCommandedTime(){return lastCommandedTime;}
    uint8_t getLastCommand(){return lastCommand;}

    int initialize(){
      XbeeSerial.begin(9600); //start commmunication with xbee
      uint8_t initstat = xbee.init(XBEE_THIS_ADDR, XBee_PAN_ID, XbeeSerial);
      if(!initstat) {
        Serial.println("XBee Initialized!");
        return 1;
      } else {
        Serial.print("XBee Failed to Initialize with Error Code: ");
        Serial.println(initstat);
        return 0;
      }
    }
    
    void sendData(uint32_t altitude, float ascentVelocity){
      uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
      uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
      payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
      payload_size = addIntToTlm(altitude, payload_buff, payload_size);
      payload_size = addFloatToTlm(ascentVelocity, payload_buff, payload_size); //the payload is the send number, the altitude, and the ascent velocity
      uint32_t pre_send = xbee.getSentPktCtr();
    
      xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_TLM, payload_buff, payload_size); //note that we are not checking that the packet sent successfully; if it does not send, it is up to the user to ask again
      
      numTries++; //increment the number of messages we have sent
    }
    
    void sendError(uint8_t errCode){
      uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
      uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
      payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
      payload_size = addIntToTlm(errCode, payload_buff, payload_size);
      uint32_t pre_send = xbee.getSentPktCtr();
    
      xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_ERR, payload_buff, payload_size);
      
      numTries++; //increment the number of messages we have sent
    }
    
    void sendConf(int32_t confCode){
      uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
      uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
      payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
      payload_size = addIntToTlm(confCode, payload_buff, payload_size);
      uint32_t pre_send = xbee.getSentPktCtr();
    
      xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_CON, payload_buff, payload_size);
      
      numTries++; //increment the number of messages we have sent
    }
    
    boolean receive(){
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

    //command signals explanation
    /*
     * 
     * If nothing is read, the class does nothing
     * If corrupted data is read, the class sends back an error packet with code 1 or 2 to alert the user that an error was detected and no action was taken
     *    If data is read correctly but has the wrong AP_ID, the class will send back error packet 3
     *    If data is read correctly but is not a command packet, the class will send back error packet 4
     *    If data is read correctly but does not have a valid checksum, the class will send back error packet 5
     * If the data is good, the following commands apply
     *    A command code of 0 means altitude data will be sent as normal
     *    A command code of 1 means abort so the xbee will send back a confirmation packet with time 1
     *    A command code of 127 means to open in reverse for a hard coded amount of time, so the xbee will send back a confirmation packet with time 200
     *    Any other command code means open for the number times 10 seconds so the xbee will send back a confirmation packet with the time
     *    
     * The data received will always be "Helios" followed by the transmission number followed by the altitude followed by the ascent velocity
     */
};

