#include <SPI.h>
#include <SD.h>
#include <Arduino.h>

#ifndef HELIOS_DEBUG
#define HELIOS_DEBUG true
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

class Datalog{ //static class

  private:
    #define FILE_NAME "Datalog.txt"
    const static int SD_CHIP_SELECT = 53; //chip select for SPI card writer on a balloonduino
    
  public:

    Datalog(){}
  
    static int initialize(){
      if (HELIOS_DEBUG) DEBUG_SERIAL.print("Initializing SD card...");
      if (!SD.begin(SD_CHIP_SELECT)) {
        if (HELIOS_DEBUG) DEBUG_SERIAL.println("Card failed, or not present");
        return 0;
      }
      else{
        if (HELIOS_DEBUG) DEBUG_SERIAL.println("card initialized.");
        return 1;
      }
    }

    static int write(String str){
      File dataFile = SD.open(FILE_NAME, FILE_WRITE);
      if (dataFile) {// if the file is available, write to it:
        dataFile.println(str);
        if (HELIOS_DEBUG) DEBUG_SERIAL.println(str);
        dataFile.close();
        return 1;
      }
      else { // if the file isn't open, pop up an error:
        if (HELIOS_DEBUG) DEBUG_SERIAL.println("error opening datalog.txt");
        dataFile.close();
        return 0;
      }
    }
};

