#include "../../include/processes/processes.h"

void sBench(void){
  delay(5000); //wait to initialize so we can connect anything we might need to

  Serial.begin(115200); //start communication with computer
  Serial.println("I am starting");
  ledStat.initialize();
  ledStat.setStatus(ledStat.YELLOW);  //yellow indicates power on and starting up
  delay(5e3);
  ledArmed.initialize();
  ledArmed.setStatus(ledStat.GREEN); //green indicates that the system is currently armed
  delay(5e3);

  if(!xbee.initialize()){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }
  else{
    ledStat.setStatus(ledStat.BLUE);  //blue indicates the xbee is functional, likely the most important component
    delay(5000);
  }

  actuator.initialize();
  motor.initialize();

  pinMode(ACT2_READ, INPUT_PULLUP);
  if (digitalRead(ACT2_READ) == LOW){ //Only actuate on startup if the switch is set to do so.
    //open and close valve
    actuator.openValve();
    while(actuator.position() > actuator.START); //wait for the valve to close, then turn it off
    actuator.closeValve();
    while(actuator.position() < actuator.END); //wait for the valve to close, then turn it off
    actuator.stopValve();

    //Turn fan on and off
    motor.startFan();
    //delay(2000);
    motor.stopFan();
  }
  while(1);

/*
  valve.state = armed;

  honeywell.initialize(allData.honeywellBalloonData, allData.honeywellAtmosphereData);

  if(!bme.initialize(allData.bmeBalloonData, allData.bmeAtmosphereData)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }

  //implement something to check that all the pressure sensors are recording the correct data

  if(!datalog.initialize()){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }
  else{
    ledStat.setStatus(ledStat.WHITE);
    delay(2000);
  }

  //Write header string on the SD card
  if (!datalog.write(HEADER_STRING)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }

#if (USING_GPS)
  if(!gps.initialize(allData.gpsData)){
    ledStat.setStatus(ledStat.RED);
    delay(5000);
  }
  //wait for the gps to get a fix before starting up
  //while(!gpsData.fix) gps.read(&gpsData);
  ledStat.setStatus(ledStat.MAGENTA); //magenta is a reminder that the gps is active
  delay(2000);
#else
  allData.gpsData = {};
#endif

  for (int i=0; i<10; i++){
    ledStat.setStatus(ledStat.OFF);
    delay(500);
    ledStat.setStatus(LEDStatusColor);
    delay(500); //led blinks green to confirm it has finished setup successfully; led will turn off once there is a gps fix
  }
  if(HELIOS_DEBUG) Serial.println("Starting Main Loop");
*/
}

void lBench(void){
    ;
}