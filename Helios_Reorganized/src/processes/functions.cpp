#include "../../include/processes/functions.h"

void logData(const myDatalog &datalog, const myData &allData, const valveState &valve, const float &actPosition){
  /*logsCounter++;
  if (logsCounter == ASCENT_CALC_FREQUENCY){ //send data less frequently to the trinket
    ascentVelocity = 1.0*(allData.gpsData.altitude - oldAltitude)/(millis() - oldTime);
    oldTime = millis();
    oldAltitude = allData.gpsData.altitude;
    logsCounter = 0;
  }*/

  String dataString = "";
  dataString += (String)millis() + ",";

  dataString += (String)allData.gpsData.year + ",";
  dataString += (String)allData.gpsData.month + ",";
  dataString += (String)allData.gpsData.day + ",";
  dataString += (String)allData.gpsData.hour + ",";
  dataString += (String)allData.gpsData.minute + ",";
  dataString += (String)allData.gpsData.second + ",";
  dataString += (String)allData.gpsData.millisecond + ",";
  dataString += (String)allData.gpsData.fix + ",";

  dataString += (String)allData.gpsData.latitude_deg + ",";
  dataString += (String)allData.gpsData.latitude_min + ",";
  dataString += (String)allData.gpsData.latitude_dir + ",";
  dataString += (String)allData.gpsData.longitude_deg + ",";
  dataString += (String)allData.gpsData.longitude_min + ",";
  dataString += (String)allData.gpsData.longitude_dir + ",";

  dataString += (String)allData.gpsData.velocity + ",";
  dataString += (String)allData.gpsData.angle + ",";
  dataString += (String)allData.gpsData.altitude + ",";
  dataString += (String)allData.gpsData.satellites + ",";

  dataString += (String)allData.honeywellBalloonData.pressure + ",";
  dataString += (String)allData.honeywellBalloonData.temperature + ",";
  dataString += (String)allData.honeywellBalloonData.status + ",";
  dataString += (String)allData.honeywellBalloonData.rawPressure + ",";
  dataString += (String)allData.honeywellBalloonData.rawTemperature + ",";
  dataString += (String)allData.honeywellBalloonData.el + ",";

  dataString += (String)allData.honeywellAtmosphereData.pressure + ",";
  dataString += (String)allData.honeywellAtmosphereData.temperature + ",";
  dataString += (String)allData.honeywellAtmosphereData.status + ",";
  dataString += (String)allData.honeywellAtmosphereData.rawPressure + ",";
  dataString += (String)allData.honeywellAtmosphereData.rawTemperature + ",";
  dataString += (String)allData.honeywellAtmosphereData.el + ",";

  dataString += (String)allData.bmeBalloonData.pressure + ",";
  dataString += (String)allData.bmeBalloonData.temperature + ",";
  dataString += (String)allData.bmeBalloonData.humidity + ",";
  dataString += (String)allData.bmeBalloonData.altitude + ",";

  dataString += (String)allData.bmeAtmosphereData.pressure + ",";
  dataString += (String)allData.bmeAtmosphereData.temperature + ",";
  dataString += (String)allData.bmeAtmosphereData.humidity + ",";
  dataString += (String)allData.bmeAtmosphereData.altitude + ",";

  /*dataString += (String)bmeData[bme.TCA_BOX_SENSOR].pressure + ",";
  dataString += (String)bmeData[bme.TCA_BOX_SENSOR].temperature + ",";
  dataString += (String)bmeData[bme.TCA_BOX_SENSOR].humidity + ",";
  dataString += (String)bmeData[bme.TCA_BOX_SENSOR].altitude + ",";*/

  //dataString += (String)ascentVelocity + ",";
  dataString += (String)actPosition + ",";
  dataString += (String)valve.state + ",";
  dataString += (String)valve.numAltitudeChecks;

  datalog.write(dataString);
}

int availableMemory() {
    // Use 1024 with ATmega168
    int size = 2048;
    byte *buf;
    while ((buf = (byte *) malloc(--size)) == NULL);
        free(buf);
    return size;
}