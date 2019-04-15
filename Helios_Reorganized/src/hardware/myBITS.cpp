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
    
  xbee.readPacket(); //read buffer, does not wait
    if (xbee.getResponse().isAvailable()) { //got something
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { //got a TxRequestPacket
        xbee.getResponse().getZBRxResponse(rx);
        
        uint32_t incominglsb = rx.getRemoteAddress64().getLsb();
        //Serial.print("Incoming Packet From: ");
        //Serial.println(incominglsb,HEX);
        
	if(rx.getPacketLength()>=xbeeReceiveBufSize){
        //Serial.print("Oversized Message: ");
        //Serial.println(rx.getPacketLength());
        }
        memset(xbeeReceiveBuf, 0, xbeeReceiveBufSize); // Clears old buffer
        memcpy(xbeeReceiveBuf,rx.getData(),rx.getPacketLength());
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
      xbeeSend(BitsSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeReceiveBuf,"TB")){ //Checks if "test" is within buffer
      //Serial.println("");
      //Serial.println("ToBits");
      String("ToBitsAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(BitsSL,xbeeSendBuf);
  }  

}


