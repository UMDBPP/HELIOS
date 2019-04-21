#include "../../include/hardware/myBITS.h"

const char myBITS::PACKET_REQUEST_DATA[myBITS::commandLength]   = "HELIOS:DATA";
const char myBITS::PACKET_DROP_NOW[myBITS::commandLength]       = "HELIOS:DROP";
const char myBITS::PACKET_VENT_NOW[myBITS::commandLength]       = "HELIOS:VENT";
const char myBITS::PACKET_ABORT_VENT[myBITS::commandLength]     = "HELIOS:ABORT";
const char myBITS::PACKET_ABORT_DROP[myBITS::commandLength]     = "HELIOS:RISE";
const char myBITS::PACKET_TEST_OPEN[myBITS::commandLength]      = "HELIOS:OPEN";
const char myBITS::PACKET_TEST_CLOSE[myBITS::commandLength]     = "HELIOS:CLOSE";
const char myBITS::PACKET_TEST_FWD[myBITS::commandLength]       = "HELIOS:FWD";
const char myBITS::PACKET_ENABLE_VENT[myBITS::commandLength]    = "HELIOS:ARM";
const char myBITS::PACKET_ENABLE_DROP[myBITS::commandLength]    = "HELIOS:PREP";
const char myBITS::PACKET_TEST_HEAT[myBITS::commandLength]      = "HELIOS:HEAT";

int myBITS::initialize(void){
    Xbee_Serial.begin(9600);
    xbee.setSerial(Xbee_Serial);
    memcpy(xbeeSendBufFull, OFFSET, offsetSize);
    if (HELIOS_DEBUG) Serial.println("Xbee initialized");
    return sendToGround("Helios is on.");
}

bool myBITS::sendToGround(String message){
  int length = message.length();
  memset(xbeeSendBuf, 0, xbeeSendBufSize);
  if (length <= xbeeSendBufSize){
    message.toCharArray(xbeeSendBuf, length);
    return sendToGround(xbeeSendBufFull, length+offsetSize);
  } else {
    if (HELIOS_DEBUG) Serial.println("Warning: Message exceeds size that can be sent. Message trimmed.");
    message.toCharArray(xbeeSendBuf, xbeeSendBufSize-1);
    xbeeSendBuf[xbeeSendBufSize-1] = '\0';
    return sendToGround(xbeeSendBufFull, xbeeSendBufSizeFull);
  }
}

bool myBITS::sendToGround(char* message, uint8_t length){
  /**
   * This function will assign char* message to a packet, and send it to BITS to send to the ground
  */
	XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,  BitsSL);
	ZBTxRequest zbTx = ZBTxRequest(TargetAddress, message, length); //Assembles Packet to be sent
	xbee.send(zbTx); //Sends packet
  if (xbee.readPacket(500)) {  //Checks Reception, waits up to 500 ms
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) { //If the response packet is of the type we're expecting
      xbee.getResponse().getZBTxStatusResponse(txStatus); //copy the status packet
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        if (HELIOS_DEBUG) Serial.println("Successful BITS Transmit");
        return true;
      } else {
        if (HELIOS_DEBUG) Serial.println("Status packet indicates send failure");
        return false;
      }
    }
  } else if (xbee.getResponse().isError()) {
    if (HELIOS_DEBUG){
      Serial.print("Error reading status return packet. Error code: ");
      Serial.println(xbee.getResponse().getErrorCode());
    }
  } else {
    if (HELIOS_DEBUG) Serial.println("Message sent. No response from BITS.");
  }
  return false; //failure
}
  
int myBITS::checkForMessage(void){
  /**
   * This function should check the xbee for a message, and assign it to a char* or uint8_t* array.
   * If there is a message, it will call processMessage and return its output
  */  
 
  xbee.readPacket(); //read buffer, does not wait
  if (xbee.getResponse().isAvailable()) { //if a packet arrive
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { //if the packet is of the type we're looking for
      xbee.getResponse().getZBRxResponse(rx); //copy packet data to rx
      uint32_t incominglsb = rx.getRemoteAddress64().getLsb(); //get the address from which the packet was sent
      if (HELIOS_DEBUG){
        Serial.print("Incoming Packet From: ");
        Serial.println(incominglsb,HEX);
      }
      if(rx.getPacketLength()>=xbeeReceiveBufSize){
        if (HELIOS_DEBUG){
          Serial.print("Oversized Message, Length: ");
          Serial.print(rx.getPacketLength());
          Serial.println(", Message truncated.");
        }
      }
      memset(xbeeReceiveBuf, 0, xbeeReceiveBufSize); // Clears old buffer
      memcpy(xbeeReceiveBuf,rx.getData(),rx.getPacketLength()); //copies packet to new buffer
      if(incominglsb == BitsSL){
        lastCommand = processMessage();
	      return lastCommand;
      }	
    }
    lastCommand = COMMAND_ERROR;
    return COMMAND_ERROR;
  } else{
    lastCommand = NO_PACKET;
    return NO_PACKET;
  }
}

int myBITS::processMessage(void){
  /**
   * This function will compare the string in XbeeReceiveBuf to the list of valid commands
   * If it matches one of these commands, it will return the corresponding command number as an integer.
   * If it does not match any known command, it will return COMMAND_ERROR
  */

  if (HELIOS_DEBUG){ //Print the data as received to the Serial monitor
    Serial.print("ReceiveFromBits: ");
    Serial.write(xbeeReceiveBuf,xbeeReceiveBufSize);
    Serial.println("");
  }

  // See documentation for function "strstr". Note that the code "if (NULL)" returns false, while all other pointers return true, which is how this works.
  if (strstr(xbeeReceiveBuf, "HELIOS")){ // Check if the packet was pre-prended with the phrase "HELIOS"
    if (strstr(xbeeReceiveBuf, PACKET_REQUEST_DATA)){ //Check if the packet matches each command type
       if (HELIOS_DEBUG) Serial.println("Request data packet received.");
       return COMMAND_REQUEST_DATA;
    } else if (strstr(xbeeReceiveBuf, PACKET_DROP_NOW)){
      if (HELIOS_DEBUG) Serial.println("Command packet to turn on nichrome now received.");
      return COMMAND_DROP_NOW;
    } else if (strstr(xbeeReceiveBuf, PACKET_VENT_NOW)){
      if (HELIOS_DEBUG) Serial.println("Command packet to turn on vent now received.");
      return COMMAND_VENT_NOW;
    } else if (strstr(xbeeReceiveBuf, PACKET_ABORT_VENT)){
      if (HELIOS_DEBUG) Serial.println("Command packet to turn off vent now received.");
      return COMMAND_ABORT_VALVE;
    } else if (strstr(xbeeReceiveBuf, PACKET_ABORT_DROP)){
      if (HELIOS_DEBUG) Serial.println("Command packet to turn off nichrome now received.");
      return COMMAND_ABORT_DROP;
    } else if (strstr(xbeeReceiveBuf, PACKET_TEST_OPEN)){
      if (HELIOS_DEBUG) Serial.println("Manual Test: Valve commanded to open.");
      return COMMAND_TEST_OPEN;
    } else if (strstr(xbeeReceiveBuf, PACKET_TEST_CLOSE)){
      if (HELIOS_DEBUG) Serial.println("Manual Test: Valve commanded to close.");
      return COMMAND_TEST_CLOSE;
    } else if (strstr(xbeeReceiveBuf, PACKET_TEST_FWD)){
      if (HELIOS_DEBUG) Serial.println("Manual Test: Valve commanded to spin forward.");
      return COMMAND_TEST_FWD;
    } else if (strstr(xbeeReceiveBuf, PACKET_ENABLE_VENT)){
      if (HELIOS_DEBUG) Serial.println("Command packet to enable vent now received.");
      return COMMAND_ENABLE_VENT;
    } else if (strstr(xbeeReceiveBuf, PACKET_ENABLE_DROP)){
      if (HELIOS_DEBUG) Serial.println("Command packet to enable nichrome now received.");
      return COMMAND_ENABLE_DROP;
    } else if (strstr(xbeeReceiveBuf, PACKET_TEST_HEAT)){
      if (HELIOS_DEBUG) Serial.println("Command packet to test nichrome now received.");
      return COMMAND_TEST_HEAT;
    }
  }

  if (HELIOS_DEBUG) Serial.println("Unrecognized packet received.");
  return COMMAND_ERROR;
}


