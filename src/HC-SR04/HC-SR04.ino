#include <M5Stack.h>
#include <WiFi.h>

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

void setup() {
  M5.begin();

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
  
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Letter size
  M5.Lcd.setTextSize(4);
}

void loop() {
  M5.update();
  
  
  // Make the LCD background white
  //M5.Lcd.fillScreen(BLACK);

  if(flag==1){
    if(loopnum == 1){
       //Generate audible tone for caliberation
       tonegen = millis();
       M5.Speaker.tone(900, 100);
       loopnum = 0;
      }
      
    // Ultrasonic sound generation
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    // Receive ultrasonic sound
    times = pulseIn(ECHO, HIGH);
    distance = (times * 0.017); //(340m/s)*(10**(-4))/2   <----- Take half the ToF, multiply it with the velocity of sound and convert to meter from micro meter
    file.println((String)(millis()-tonegen) + "," + (String)distance);
    // Display on the LCD screen
    //M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,80);
    M5.Lcd.print("Collecting...");
    /*M5.Lcd.print(distance);
    M5.Lcd.print(" cm");
    delay(500);*/
  }
  
  if (M5.BtnB.wasPressed()) { //Only sense if button B is pressed
    flag = (flag+1)%2;
    if(flag==1) {
      loopnum = 1;
      //getFileName();
      if(!SD.open(getFileName(), FILE_WRITE)){
        Serial.println("\nError creating file.");
        }
      file = SD.open(getFileName(), FILE_WRITE); //Make sure that the filename has a "/" mark at the front
    }
    if(flag==0) {
      file.close();
      //M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,80);
      M5.Lcd.println("             ");
      M5.Lcd.setCursor(0,80);
      M5.Lcd.println("Stopped!");
    }
  }  
}
