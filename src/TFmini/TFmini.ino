/*
 * Using TFMini sensor to measure distance paired with M5Stack basic model 
 */
#include <M5Stack.h>
#include <WiFi.h>
#include "SD.h"

#define TFMINI_FRAME_SIZE 7
#define TRIG 2
#define ECHO 5

#define JST (3600L * 9)

const char* ssid = "Galaxy S20 Ultra 5G0525"; //Only connects to 2.4GHz
const char* password = "ylke5622";

int times;
float distance;
unsigned long flag=0;
unsigned long tonegen=0;
int loopnum = 0;

File file;

void writeFile(fs::FS &fs, const char * path, const char * message){
    //Serial.printf("Writing file: %s\n", path);
    //M5.Lcd.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
      M5.Lcd.setTextSize(2);
      M5.Lcd.println("Failed to open directory");
      M5.Lcd.println("Check SD card");
      for(int x=0;x<5;x++){
        M5.Speaker.tone(1500, 100);
        delay(500);
      }
        return;
    }
    if(file.print(message)){
        //Serial.println("File written");
        M5.Lcd.println("SD check completed");
    } else {
        //Serial.println("Write failed");
        M5.Lcd.println("SD check failed failed");
    }
}


/****** Function to get the filename ******/
String getFileName(){

  char year[10];//For some reason just the number of digits in date or time is not enough
  char month[10];
  char day[10];
  char hour[10];
  char minute[10];
  char second[2];
  
  char filename[200];

  struct tm tm;
  getLocalTime(&tm);

  sprintf(year,"%2d",tm.tm_year+1900);
  sprintf(month,"%02d",tm.tm_mon+1);
  sprintf(day,"%02d",tm.tm_mday);
  sprintf(hour,"%02d",tm.tm_hour);
  sprintf(minute,"%02d",tm.tm_min);
  sprintf(second,"%02d",tm.tm_sec);
  
  //M5.Rtc.GetTime(&RTC_TimeStruct);
  //M5.Rtc.GetData(&RTC_DateStruct);
  
  /*printf(year,"%04d",RTC_DateStruct.Year);
  sprintf(month,"%02d",(int)RTC_DateStruct.Month);
  sprintf(day,"%02d",(int)RTC_DateStruct.Date);
  sprintf(hour,"%02d",(int)RTC_TimeStruct.Hours);
  sprintf(minute,"%02d",(int)RTC_TimeStruct.Minutes);
  sprintf(second,"%02d",(int)RTC_TimeStruct.Seconds);
  sprintf(msec,"%03d",(unsigned int)mSec);*/
  strcpy(filename, "/");
  strcat(filename, year);
  strcat(filename, month);
  strcat(filename, day);
  strcat(filename, hour);
  strcat(filename, minute);
  strcat(filename, second);
  strcat(filename, ".csv");
  Serial.print(filename);
  return filename;
  }

float getTFminiData(Stream* streamPtr, float* distance, float * strength) {
    uint8_t lastChar = 0x00;
    uint8_t frame[TFMINI_FRAME_SIZE];

    while (1) {
        if (streamPtr->available()) {      
            uint8_t curChar = streamPtr->read();
            if ((lastChar == 0x59) && (curChar == 0x59)) {
                break;  // Break to begin frame
            } else {
                lastChar = curChar;
            }
        }
    }

    uint8_t checksum = 0x59 + 0x59;
    for (int i=0; i<TFMINI_FRAME_SIZE; i++) {
        while (!streamPtr->available()) {
        }
        frame[i] = streamPtr->read();

        if (i < TFMINI_FRAME_SIZE-1) {
            checksum += frame[i];
        }
    }
    uint8_t checksumByte = frame[TFMINI_FRAME_SIZE-1];
    if (checksum != checksumByte) {
        *distance = -1;
        *strength = -1;
#ifdef DEBUG
        Serial.printf("ERROR: bad checksum %02x, %02x\n\r", checksum, checksumByte);
#endif
        return -1;
    }

    uint16_t dist = (frame[1] << 8) + frame[0];
    uint16_t st = (frame[3] << 8) + frame[2];
    uint8_t reserved = frame[4];
    uint8_t originalSignalQuality = frame[5];

    *distance = dist;
    *strength = st;

    return 0;
}

void setup() {
    M5.begin();
    Serial1.begin(115200, SERIAL_8N1, 5, 2);

    M5.Lcd.setTextSize(2);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        M5.Lcd.print('.');
    }
  
    M5.Lcd.print("\r\nWiFi connected\r\nIP address: ");
    M5.Lcd.println(WiFi.localIP());
    delay(500);
    configTime(JST,0,"ntp.nict.jp", "time.google.com",
            "ntp.jst.mfeed.ad.jp");
            
    //Perform SD check
    writeFile(SD, "/test.txt", "Hello world from M5Stack !!");

    // Letter size
    M5.Lcd.setTextSize(4);
}

void loop() {
    M5.update();


    
    if(flag==1){
      if(loopnum == 1){
         //Generate audible tone for caliberation
         tonegen = millis();
         M5.Speaker.tone(900, 100);
         loopnum = 0;
      }
         float distance = 0.0;
         float strength = 0.0;
        

        if (getTFminiData(&Serial1, &distance, &strength) == 0.0) {
            //Serial.println(distance);
            //M5.Lcd.fillScreen(BLACK);
            //M5.Lcd.setCursor(20, 20);
            //M5.Lcd.printf("%4dcm\r\n", distance);
            //M5.Lcd.printf("s: %d", strength);
    
            //delay(10);
            file.println((String)(millis()-tonegen) + "," + (String)distance + "," + (String)strength);
        }
        M5.Lcd.setTextSize(4);
        M5.Lcd.setCursor(0,80);
        M5.Lcd.print("Collecting...");
    
    }

    if (M5.BtnB.wasPressed()) { //Only sense if button B is pressed
      flag = (flag+1)%2;
      if(flag==1) {
        loopnum = 1;
        //getFileName();
        if(!SD.open("/dist" +getFileName(), FILE_WRITE)){
          Serial.println("\nError creating file.");
          }
        file = SD.open("/dist" + getFileName(), FILE_WRITE); //Make sure that the filename has a "/" mark at the front
        file.println("millis,dist,strength");
      }
      if(flag==0) {
        file.close();
        //M5.Lcd.fillScreen(BLACK);
        M5.Speaker.tone(1500, 100);
        M5.Lcd.setTextSize(4);
        M5.Lcd.setCursor(0,80);
        M5.Lcd.println("             ");
        M5.Lcd.setCursor(0,80);
        M5.Lcd.println("Stopped!");
      }
  }  
    

}
