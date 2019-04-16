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
	XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,  BitsSL);
	ZBTxRequest zbTx = ZBTxRequest(TargetAddress, message, length); //Assembles Packet to be sent
	xbee.send(zbTx); //Sends packet
  memset(xbeeSendBuf, 0, xbeeSendBufSize);
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
	      return processMessage();
      }	
    }
  }
  return COMMAND_ERROR;
}

int myBITS::processMessage(void){
  /**
   * This function will compare the string in XbeeReceiveBuf to the list of valid commands
   * If it matches one of these commands, it will return the corresponding command number as an integer.
   * If it does not match any known command, it will return COMMAND_ERROR
  */

  Serial.println("ReceiveFromBits: ");
  Serial.write(xbeeReceiveBuf,xbeeReceiveBufSize);
  
  if(strstr((char*)xbeeReceiveBuf,"test")){ //Checks if "test" is within buffer
      //Serial.println("");
      //Serial.println("ackTest");
      String("PacketAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      //xbeeSend(BitsSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeReceiveBuf,"TB")){ //Checks if "test" is within buffer
      //Serial.println("");
      //Serial.println("ToBits");
      String("ToBitsAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      //xbeeSend(BitsSL,xbeeSendBuf);
  }  

  return 0;
}


