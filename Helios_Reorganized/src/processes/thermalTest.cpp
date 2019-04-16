#include "../../include/processes/processes.h"

uint32_t millisLast = 0;

void sThermalTest(){
  //THERMAL TEST CODE
  delay(5000); //wait to initialize so we can connect anything we might need to
  Serial.begin(115200); //start communication with computer
  ledStat.initialize();
  ledStat.setStatus(ledStat.YELLOW);  //yellow indicates power on and starting up
  ledArmed.initialize();
  ledArmed.setStatus(ledArmed.RED); //green indicates that the system is currently armed
  if(!xbee.initialize()){
    //ledStat.setStatus(ledStat.RED); // we don't actually care about the xbee this time
    delay(5000);  }
  else{
    ledStat.setStatus(ledStat.BLUE);  //blue indicates the xbee is functional, likely the most important component
    delay(2000);
  }
  actuator.initialize();
  motor.initialize();

  pinMode(ACT2_READ, INPUT_PULLUP);
  if (digitalRead(ACT2_READ) == LOW){ //Only actuate on startup if the switch is set to do so.
    actuator.openValve();
    while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
    actuator.closeValve();
    while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
    actuator.stopValve();
    motor.startFan();
    delay(2000);
    motor.stopFan();
  }
  honeywell.initialize(allData.honeywellBalloonData, allData.honeywellAtmosphereData);
  if(!bme.initialize(allData.bmeBalloonData, allData.bmeAtmosphereData)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);  }
  if(!datalog.initialize()){
    ledStat.setStatus(ledStat.RED);
    delay(5000);  }
  else
    ledStat.setStatus(ledStat.WHITE);
  if (!datalog.write(HEADER_STRING)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);  }
  if(!gps.initialize(allData.gpsData)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);  }
  for (int i=0; i<10; i++){
    ledStat.setStatus(ledStat.OFF);
    delay(500);
    ledStat.setStatus(ledStat.GREEN);
    delay(500);  }
}

void lThermalTest(){

  //FOR THERMAL TEST
  //stop actuator if it is moving and about to hit its endpoints
  if (valve.state == open && actuator.position() < actuator.START) //if valve has finished opening, turn it off
    actuator.stopValve();
  else if (actuator.position() > actuator.END) //if valve has finished closing, turn it off
    actuator.stopValve();

  if(xbee.checkForMessage() != xbee.NO_PACKET && (millis() - millisLast) > xbee.WAIT_TIME_AFTER_COMMAND){ //if the xbee does receive a command
    //xbeeCommand();  //exectue separate function that handles the command
    millisLast = millis(); //set the time at which the last command was received to prevent duplicates
  }

  if (millis() > 40*60*1000){
    actuator.openValve();
    motor.startFan();
    while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
    actuator.stopValve();
    delay(10*1000);
    actuator.closeValve();
    while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
    actuator.stopValve();
    motor.stopFan();
  }
}