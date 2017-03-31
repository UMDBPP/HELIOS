/*Board "Pro Trinket 3V/12MHz (USB)" Programmer "USBTiny"*/
#include<Wire.h>

//I2C configuration
#define TRINKET_ADDR 9
//#define M0_ADDR 8
#define PIN_M0_OUT 8

//Xbee configuration
#define _NO_RTC_
#define PKT_MAX_LEN 100
#define XBEE_WRITE_ADDR 0x0002
#define XBEE_THIS_ADDR 0x0008
#define XBee_PAN_ID 0x0B0B
#define AP_ID_CMD 800   //the AP_ID that we must see whenever we receive a packet
#define AP_ID_TLM 810   //the AP_ID that will be used to send all telemetry packets
#define AP_ID_ERR 890   //the AP_ID that will be used to send all error packets
#define AP_ID_CON 850   //the AP_ID used to send a confirmation
#define WAIT_TIME_SUCCESS 1000    //the wait time before another command can be received
#define WAIT_TIME_NONE 100    //the wait time between checking for more data
#include "ccsds_xbee.h"
CCSDS_Xbee xbee;
uint16_t numTries = 0; //number of transmissions we have made since startup
uint8_t sentCounter = 0;

int32_t altitude; //last altitude in meters
float ascentVelocity; //last speed
uint32_t timeOpen = 100000; //this theoretically can be initialized to anything, but I have chosen to initialize it to the same value as the M0 will initialize
uint8_t valveOpen = 0; //initialized to not doing anything

uint8_t receivedBytes[5];
uint8_t receivedCounter = 0;

//This program will only receive single byte command packets and send eight byte telemetry packets. All other xbee operations are unused.

void setup(){
  Wire.begin(TRINKET_ADDR);
  Wire.onRequest(sendI2C);
  Wire.onReceive(receiveI2C);

  pinMode(PIN_M0_OUT, OUTPUT);
  digitalWrite(PIN_M0_OUT, LOW);
  
  Serial.begin(9600);
  uint8_t initstat = xbee.init(XBEE_THIS_ADDR, XBee_PAN_ID, Serial);
  if(!initstat) {
    Serial.println("XBee Initialized!");
  } else {
    Serial.print("XBee Failed to Initialize with Error Code: ");
    Serial.println(initstat);
    digitalWrite(PIN_M0_OUT, HIGH); //tell the m0 that we have a problem so that it can alert the user via the status led
  }
}

void loop(){
  delay(100);
  receiveXbee();
  /*
   * All this processor even does is look for and send xbee messages
   * xbee messages will only be sent when asked, so all we have to do is check if we've been asked
   * In general, this program should process messages fairly quickly, though it may need to wait no more than 800 milliseconds for the m0 to respond
   */
}

//command signals explanation
/*
 * If good data is received from link, the trinket automatically notifies the m0
 * The first byte sent is an action command; it must be true or the m0 will not do anything
 * If the action command is false, the m0 will ignore it and write the normal altitude and ascent rate back as confirmation
 * If the action command is true, the m0 will force the valve to open for the amount of time specified in the next four bits
 *    In this case, the m0 will write back the amount of time open followed by a zero to confirm the command
 * If the action command is true, and the time specified is zero, the m0 will prevent the valve from opening unless commanded otherwise later 
 *    In this case, the m0 will write back a one followed by a zero
 * In all of these cases, the m0 will stop receiving data for WAIT_TIME (current 2 seconds) after the command
 * The m0 will also continuously update the ascent velocity and altitude which can be called by link at any time
 * 
 * On startup, the m0 will send the data stream {-1 -2.0} to the trinket to confirm that it can communicate
 * The user should then check link to ensure the xbee also transmits that data
 * 
 * If nothing is read, the trinket does nothing
 * If corrupted data is read, the trinket sends back an error packet with code 1 or 2 to alert the user that an error was detected and no action was taken
 *    If data is read correctly but has the wrong AP_ID, the trinket will send back error packet 3
 *    If data is read correctly but is not a command packet, the trinket will send back error packet 4
 *    If data is read correctly but does not have a valid checksum, the trinket will send back error packet 5
 * If the data is good, the following commands apply
 *    A command code of 0 means altitude data will be sent as normal
 *    A command code of 1 means abort so the next data packet should be {1, 0} and the xbee will send back a confirmation packet with time 1
 *    Any other command code means open for the number times 20 seconds so the next data packt should be {0, time in millis}, and the xbee will send back a confirmation packet with the time
 *    
 * The data received will always be "Helios" followed by the transmission number followed by the altitude followed by the ascent velocity
 */

void sendXbee(){
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

void sendConf(int32_t tvalue){
  uint8_t payload_buff[PKT_MAX_LEN]; // create a buffer in which to compile the payload of the packet
  uint8_t payload_size = 0; // initalize a counter to keep track of the length of the payload
  payload_size = addIntToTlm(numTries, payload_buff, payload_size);  //compile the payload
  payload_size = addIntToTlm(tvalue, payload_buff, payload_size);
  uint32_t pre_send = xbee.getSentPktCtr();

  xbee.sendTlmMsg(XBEE_WRITE_ADDR, AP_ID_CON, payload_buff, payload_size);
  
  numTries++; //increment the number of messages we have sent
}

void receiveXbee(){
  uint8_t Pkt_Buff[PKT_MAX_LEN];
  uint8_t pktLength = 0;
  uint8_t bytes_read = 0;
  bytes_read = xbee.readMsg(Pkt_Buff);

  if(bytes_read < 0){
    sendError(1);  //an error has occurred so we will let the user know
    return;
    }
  else if(bytes_read == 0){
    delay(WAIT_TIME_NONE);
    return;
  }
  else if(bytes_read != getPacketLength(Pkt_Buff)){
    sendError(2);
    return; //an error has occurred so we will let the user know
  }
  else{
    packet_processing(Pkt_Buff); //if the data looks good, we'll actually look at the content
  }
}

void packet_processing(uint8_t Pkt_Buff[]){
  if(getAPID(Pkt_Buff) != AP_ID_CMD){ //check that we have the correct AP_ID
    sendError(3);
    return;
  }
  if(getPacketType(Pkt_Buff) != CCSDS_CMD_PKT){ //check that what we have received is a command
    sendError(4);
    return;
  }
  if(!packetHasValidChecksum(Pkt_Buff)){  //check that we have received a valid checksum
    sendError(5);
    return;
  }

  //if everything is good, then we will actually process the command
  uint8_t command = getCmdFunctionCode(Pkt_Buff);
  if (command == 0){ //this is the receive telemetry command
    sendXbee(); //send the most recent data
    //digitalWrite(PIN_M0_OUT, HIGH);
  } else if (command == 1){ //this is the abort command
    valveOpen = 1;
    timeOpen = 0;
    digitalWrite(PIN_M0_OUT, HIGH); //writing this high will cause the m0 to trigger the xbee to send data
  } else if (command == 127){ //this is the reverse fan command
    valveOpen = 1;
    timeOpen = 100;
    digitalWrite(PIN_M0_OUT, HIGH);
  }
  else if (command > 1){ //this is the open now command
    valveOpen = 1;
    timeOpen = (command-1)*30000; //in this case, the time open is the command value times 30 in seconds
    digitalWrite(PIN_M0_OUT, HIGH); 
  }
  delay(WAIT_TIME_SUCCESS);
}

void sendI2C(){
  union {
    uint32_t v;
    uint8_t bytes[4];
  } a;
  a.v = timeOpen;
  if (sentCounter == 0){
    Wire.write(valveOpen);
    sentCounter++;
  } else if (sentCounter == 1){
    Wire.write(a.bytes[0]);
    sentCounter++;
  } else if (sentCounter == 2){
    Wire.write(a.bytes[1]);
    sentCounter++;
  } else if (sentCounter == 3){
    Wire.write(a.bytes[2]);
    sentCounter++;
  } else if (sentCounter == 4){
    Wire.write(a.bytes[3]);
    sentCounter = 0;
  }
  //bytes[0] = timeOpen % (uint32_t)(pow(2, 8));
  //bytes[1] = timeOpen % (uint32_t)(pow(2, 16)) / pow(2, 8);
  //bytes[2] = timeOpen % (uint32_t)(pow(2, 24)) / pow(2, 16);
  //bytes[3] = timeOpen / (uint32_t)(pow(2, 24));
  //for (int i=0; i<4; i++){ Wire.write(2); delay(5);}
}

void receiveI2C(){
  receivedBytes[receivedCounter] = Wire.read();
  receivedCounter++;
  if (receivedCounter == 9){
    receivedCounter = 0;
    uint8_t command = receivedBytes[0];
    union {
      int32_t data;
      byte bytes[4];
    } a;
    union {
      float data;
      byte bytes[4];
    } v;
    for (int i=0; i<4; i++) a.bytes[i] = receivedBytes[i+1];
    for (int i=0; i<4; i++) v.bytes[i] = receivedBytes[i+5];
  
    if(command == 1){
      digitalWrite(PIN_M0_OUT, LOW);
      valveOpen = 0;  //reset the trinket so that the arduino doesn't open continuously
      if(v.data != 0.0){ //nothing happened
        sendError(100);
      }
      else if (a.data == 120001){ //the fan is spinning backwards
        sendConf(-1);
      }
      else{
        sendConf(a.data); //everything is good
      }//no need to update the altitude and ascent velocity if we receive a command code.
    }
    else{
      altitude = a.data;
      ascentVelocity = v.data;
    }
  }
}

