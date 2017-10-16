/*
 * Functions for managing Xbee communications
 */

#ifndef XbeeFunctions
#define XbeeFunctions

#define HELIOS_DEBUG true
#include "Arduino.h"
#include "ccsds_xbee.h"

#define _NO_RTC_
#define PKT_MAX_LEN 100

class myXbee{
  private:
    const static int XBEE_WRITE_ADDR = 0x0002;
    const static int XBEE_THIS_ADDR = 0x0008;
    const static int XBee_PAN_ID = 0x0B0B;
    const static int AP_ID_CMD = 800;   //the AP_ID that we must see whenever we receive a packet
    const static int AP_ID_TLM = 810;   //the AP_ID that will be used to send all telemetry packets
    const static int AP_ID_DATA = 820;
    const static int AP_ID_ERR = 890;   //the AP_ID that will be used to send all error packets
    const static int AP_ID_CON = 850;   //the AP_ID used to send a confirmation
    const static uint32_t TIME_TO_VENT = 200000; //this is the time for which the valve will vent air if commanded forward without a time
    const static uint32_t TIME_TO_REVERSE = 120000; //this is the time for which the valve will blow air into the balloon if commanded backwards without a time
    const static uint8_t PARAM_POS = 8; //the packet position to look for the command parameters; for this particular command set, this value is a constant

    CCSDS_Xbee xbee;
    uint16_t numTries = 1; //number of transmissions we have made since startup
    uint8_t lastCommand;
    int32_t lastCommandedTime;
    boolean packet_processing(uint8_t Pkt_Buff[]);
  
  public:
    int initialize();
    uint32_t getCommandedTime();
    uint8_t getLastCommand();
    void sendData(uint32_t altitude, float ascentVelocity);
    void sendAllData(uint32_t altitude, uint8_t lat_deg, float lat_min, uint8_t lon_deg, float lon_min, float velocity, float pres_in, float pres_out, float temp_in, float temp_out, float position);
    void sendError(uint8_t errCode);
    void sendConf(uint8_t confCode, int32_t confTime);    
    boolean receive();
  
    const static int WAIT_TIME_AFTER_COMMAND = 1000;    //the wait time before another command can be received
    const static int COMMAND_REQUEST_DATA = 0;
    const static int COMMAND_ABORT = 1;
    const static int COMMAND_REVERSE_NOW = 2;
    const static int COMMAND_VENT_NOW = 3;
    const static int COMMAND_ENABLE = 4;
    const static int COMMAND_TEST_OPEN = 5;
    const static int COMMAND_TEST_CLOSE = 6;
    const static int COMMAND_TEST_FWD = 7;
    const static int COMMAND_TEST_REV = 8;
    const static int COMMAND_RESET = 9;
    const static int COMMAND_SET_TIME = 10;
    const static int COMMAND_SET_ALT = 11;
    const static int COMMAND_ALL_DATA = 12;

    const static int ERROR_CODE_NEGATIVE = 1;
    const static int ERROR_CODE_BAD_LENGTH = 2;
    const static int ERROR_CODE_BAD_ID = 3;
    const static int ERROR_CODE_NOT_COMMAND = 4;
    const static int ERROR_CODE_BAD_CHECKSUM = 5;

    const static int CONFIRM_CODE_VENT = 0;
    const static int CONFIRM_CODE_ABORT = 1;
    const static int CONFIRM_CODE_REVERSE = 2;
    const static int CONFIRM_CODE_ENABLE = 3;
    const static int CONFIRM_CODE_SET_VAR = 4;
    const static int CONFIRM_CODE_TEST = 5;
    const static int CONFIRM_CODE_KILL = 6;

    const static int CONFIRM_STATE_FWD = 1;
    const static int CONFIRM_STATE_REV = 2;
    const static int CONFIRM_STATE_OPEN = 3;
    const static int CONFIRM_STATE_CLOSE = 4;
};

#endif
