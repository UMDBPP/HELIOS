#include <SPI.h>
#include <SD.h>
#include "Arduino.h"
#include "myLog.h"

int myDatalog::initialize(){  //intialize the SD library and SD card and return if something doesn't work
  if (HELIOS_DEBUG) Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CHIP_SELECT)) {
    if (HELIOS_DEBUG) Serial.println("Card failed, or not present");
    return 0;
  }
  else{
    if (HELIOS_DEBUG) Serial.println("card initialized.");
    return 1;
  }
}

int myDatalog::write(String str){ //write to the SD card and return false is something doesn't work
  File dataFile = SD.open("Datalog.txt", FILE_WRITE);
  if (dataFile) {// if the file is available, write to it:
    dataFile.println(str);
    if (HELIOS_DEBUG) Serial.println(str);
    dataFile.close();
    return 1;
  }
  else { // if the file isn't open, pop up an error:
    if (HELIOS_DEBUG) Serial.println("error opening datalog.txt");
    dataFile.close();
    return 0;
  }
}
