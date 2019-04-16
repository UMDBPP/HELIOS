/*
 * Functions for working with BITS
*/

#ifndef XbeeFunctions
#define XbeeFunctions

#include "Arduino.h"
#include "../../myPins.h"
#include <Xbee.h>

class myBITS{
  private:
    XBee xbee;
    XBeeResponse response;

    //last 32 bits of the xbee address, varies by target
    const uint32_t BitsSL = 0x417B4A3B; // address of BITS, this is the only address we care about
    //const uint32_t MarsSL = 0x417B4A3A; // address of Mars, should not be used
    //const uint32_t GroundSL = 0x417B4A36; // address of the ground station, should not be used

    //first 32 bits of the xbee address - the same for everything
    const uint32_t UniSH = 0x0013A200; 

    // Everything ending in "Request" deals with sending
    // Everything ending in "Response" deals with receiving
    ZBTxStatusResponse txStatus; //the check on whether a transmitted packet was actually sent
    ZBRxResponse rx; //the received packet

    const static int xbeeReceiveBufSize = 50; //Rec must be ~15bytes larger than send
    const static int xbeeSendBufSize = 35;
    uint8_t xbeeReceiveBuf[xbeeReceiveBufSize];
    uint8_t xbeeSendBuf[xbeeSendBufSize];

    // These are intentionally rather different strings to make it hard for human error to mistype something
    const static char PACKET_REQUEST_DATA[16]   = "HELIOS:DATA";
    const static char PACKET_DROP_NOW[16]       = "HELIOS:DROP";
    const static char PACKET_VENT_NOW[16]       = "HELIOS:VENT";
    const static char PACKET_ABORT_VENT[16]     = "HELIOS:ABORT";
    const static char PACKET_ABORT_DROP[16]     = "HELIOS:RISE";
    const static char PACKET_TEST_OPEN[16]      = "HELIOS:OPEN";
    const static char PACKET_TEST_CLOSE[16]     = "HELIOS:CLOSE";
    const static char PACKET_TEST_FWD[16]       = "HELIOS:FWD";
    const static char PACKET_ENABLE_VENT[16]    = "HELIOS:ARM";
    const static char PACKET_ENABLE_DROP[16]    = "HELIOS:PREP";

    int processMessage(void);

  public:
    int initialize();

    bool sendToGround(char*,uint8_t); //sends a character array to ground
    int checkForMessage(void); //checks for messages and if so, calls processMessage
    
    const static int COMMAND_REQUEST_DATA = 0;
    const static int COMMAND_ABORT_VALVE = 1;
    const static int COMMAND_ABORT_DROP = 2;
    const static int COMMAND_VENT_NOW = 3;
    const static int COMMAND_DROP_NOW = 4;
    const static int COMMAND_ENABLE_VENT = 5;
    const static int COMMAND_ENABLE_DROP = 6;
    const static int COMMAND_TEST_OPEN = 7;
    const static int COMMAND_TEST_CLOSE = 8;
    const static int COMMAND_TEST_FWD = 9;
    const static int COMMAND_ERROR = 10;

};

#endif