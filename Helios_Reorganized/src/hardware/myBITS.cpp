#include "../../include/hardware/myBITS.h"

int myBITS::initialize(void){
    Xbee_Serial.begin(9600);
    xbee.setSerial(Xbee_Serial);
    if (HELIOS_DEBUG) Serial.println("Xbee initialized");
}

bool myBITS::sendToGround(char* message, uint8_t length){
	XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,  BitsSL);
	ZBTxRequest zbTx = ZBTxRequest(TargetAddress, (unit8_t*)message, length); //Assembles Packet to be sent
	xbee.send(zbTx);              //Sends packet
	  memset(xbeeSendBuf, 0, xbeeSendBufSize);
  if (xbee.readPacket(500)) {                                       //Checks Reception, waits up to 500 ms
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        //Serial.println("SuccessfulTransmit");
        return true;
      } else {
        //Serial.println("TxFail");
        return false;
      }
    }
  } 
    else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  }
  return false;
}
  /**
   * This function will assign char* message to a packet, and send it to BITS to send to the ground
  */


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


