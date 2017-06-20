#include <SPI.h>
#include <SD.h>
#include <Arduino.h>

class Datalog{ //static class

  private:
    #define FILE_NAME "Datalog.txt"
    const static int SD_CHIP_SELECT = 4;
    
  public:

    Datalog(){}
  
    static int initialize(){
      Serial.print("\n\nInitializing SD card...");
      if (!SD.begin(SD_CHIP_SELECT)) {
        Serial.println("Card failed, or not present");
        return 0;
      }
      else{
        Serial.println("card initialized.");
        return 1;
      }
    }

    static int write(String str){
      File dataFile = SD.open(FILE_NAME, FILE_WRITE);
      if (dataFile) {// if the file is available, write to it:
        dataFile.println(str);
        Serial.println(str);
        dataFile.close();
        return 1;
      }
      else { // if the file isn't open, pop up an error:
        Serial.println("error opening datalog.txt");
        dataFile.close();
        return 0;
      }
    }
};

