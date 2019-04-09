#include "../../include/processes/processes.h"

void sPressureTest(){
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

void lPressureTest(){
  bme.read(&allData.bmeBalloonData, bme.INDEX_BALLOON);
  bme.read(&allData.bmeAtmosphereData, bme.INDEX_ATMOS);
  Serial.print("Inside: ");
  Serial.print(allData.bmeBalloonData.pressure); Serial.print(", ");
  Serial.print(allData.bmeBalloonData.temperature); Serial.print(", ");
  Serial.print(allData.bmeBalloonData.humidity); Serial.print(", ");
  Serial.println(allData.bmeBalloonData.altitude);
  Serial.print("Outside: ");
  Serial.print(allData.bmeAtmosphereData.pressure); Serial.print(", ");
  Serial.print(allData.bmeAtmosphereData.temperature); Serial.print(", ");
  Serial.print(allData.bmeAtmosphereData.humidity); Serial.print(", ");
  Serial.println(allData.bmeAtmosphereData.altitude);
  
  delay(500);

  honeywell.read(&allData.honeywellBalloonData, honeywell.INDEX_BALLOON);
  Serial.print(allData.honeywellBalloonData.pressure);
  Serial.print("   ");
  Serial.print(allData.honeywellBalloonData.temperature);
  Serial.print("   ");
  Serial.println(allData.honeywellBalloonData.el);
  honeywell.read(&allData.honeywellAtmosphereData, honeywell.INDEX_ATMOS);
  Serial.print(allData.honeywellAtmosphereData.pressure);
  Serial.print("   ");
  Serial.print(allData.honeywellAtmosphereData.temperature);
  Serial.print("   ");
  Serial.println(allData.honeywellAtmosphereData.el);
  delay(250);
  datalog.write("the logger is working"); delay(2000);
}