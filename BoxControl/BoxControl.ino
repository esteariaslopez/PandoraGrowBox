#include <SPI.h>
#include <SD.h>

#include "DHT.h"
#include "Timer.h" 
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTPIN 2     
#define MOIPIN 0
#define EXTPIN 3 
#define LIGHTPIN 23
#define FANPIN 22
#define WPUMPPIN 4
#define SDENABLEPIN 53

boolean LIGHT = false;
boolean FAN = false;
int EXTRACTOR = 0;
int WPUMP =0;

LiquidCrystal lcd(38, 36, 34, 32, 30, 28);//RS-EN-D4-D5-D6-D7
int screen = 0;

//LIGHT TIMER
const double HOURON = 4;
const double MINUTEON = 0;
const double HOUROFF = 22;
const double MINUTEOFF = 0;

//EXTRACTOR
const int tempON = 24;
const int tempOFF = 23;

const int minMOI = 820;
const int unMOI = 500;

const unsigned long Time_DHT = 1000;
const unsigned long Time_MOI = 1000;
const unsigned long Time_LCD = 3000;
const unsigned long Time_EXT = 500;
const unsigned long Time_LIGHT = 10000;
const unsigned long Time_FAN = 30000;
const unsigned long Time_WATER = 60000;
const unsigned long Time_MSD = 10000;

float hum = 0;
float temp = 0;
int moi = 0;


DHT dht(DHTPIN, DHTTYPE);
Timer timer;
DS1307 rtc;
File Data;

void setup() {
  pinMode(SDENABLEPIN, OUTPUT);
  pinMode(EXTPIN, OUTPUT);
  pinMode(LIGHTPIN, OUTPUT);
  digitalWrite(LIGHTPIN, HIGH);
  pinMode(FANPIN, OUTPUT);
  digitalWrite(FANPIN, HIGH);
  
  Serial.begin(9600);
  Serial.println("SmartGrowing Start");
  
  analogReference(EXTERNAL);
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GROWING BOX");

  delay(1000);
  lcd.setCursor(0, 1);
  if(SD.begin(SDENABLEPIN)){lcd.print("SD READY");}else{lcd.print("SD FAILED");}
  delay(1000);
  if (writeHEADER){lcd.print("  OK");}else{lcd.print("  ERROR");}

  timer.every(Time_DHT, readDHT);
  timer.every(Time_MOI, readMOI);
  timer.every(Time_LCD, writeLCD);
  timer.every(Time_EXT, extCTRL);
  timer.every(Time_LIGHT, lightCTRL);
  timer.every(Time_FAN, fanCTRL);
  timer.every(Time_WATER, waterCTRL);
  timer.every(Time_MSD, writeMSD);

  dht.begin();
  Wire.begin();
  rtc.begin();
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  timer.update();
}
void readMOI(){
    int moival = analogRead(MOIPIN);
    moi = moival;
  }
void readDHT(){
    hum = dht.readHumidity();
    temp = dht.readTemperature();
  }
void extCTRL(){
    if (temp >= tempON){EXTRACTOR = 100;}else{
      if (temp <= tempOFF){EXTRACTOR = 0;}else{
          if (isnan(temp)){if(LIGHT){EXTRACTOR = 100;}else{EXTRACTOR = 0;};}
        }
    }
    int PWM = map(EXTRACTOR, 0, 100, 0, 255);
    analogWrite(EXTPIN,PWM);

//    int potval = analogRead(POTPIN); Serial.print(potval);
//    potval = map(potval, 0, 1023, 0, 255);Serial.print("  ");
//    analogWrite(EXTPIN,potval);Serial.println(potval);
  }

void waterCTRL(){ 
    if (moi <= minMOI && moi >= unMOI){
      WPUMP = 100;
      int PWM = map(WPUMP, 0, 100, 0, 255);
      analogWrite(WPUMPPIN,PWM);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WATER PUMP ON:");
      delay(10000);

      writeLCD();
      
      WPUMP = 0;
      PWM = map(WPUMP, 0, 100, 0, 255);
      analogWrite(WPUMPPIN,PWM);
      }else{WPUMP = 0;};
  }
void lightCTRL(){
    double horaON = HOURON + MINUTEON/60; 
    double horaOFF = HOUROFF + MINUTEOFF/60;
    
    DateTime now = rtc.now();
    double hora = double(now.hour())+double(now.minute())/60;
    
    if ((horaON<= hora) && (hora<horaOFF)){
        LIGHT = true;
      }else{LIGHT = false;}
      
    digitalWrite(LIGHTPIN, !LIGHT);
  }
void fanCTRL(){
    if (FAN){FAN = false;} else{FAN = true;}      
    digitalWrite(FANPIN, !FAN);
  }

bool writeHEADER(){
    Serial.print("DD.MM.YYYY,hh:mm:ss,");
    Serial.print("TEMP");
    Serial.print(",");
    Serial.print("HR");
    Serial.print(",");
    Serial.print("MOI");
    Serial.print(",");
    Serial.print("LIGHT");
    Serial.print(",");
    Serial.print("FAN");
    Serial.print(",");
    Serial.print("EXT");
    Serial.println();

    Data = SD.open("test.txt", FILE_WRITE);
    if (!Data){
        Data.print("DD.MM.YYYY,hh:mm:ss,");
        Data.print("TEMP");
        Data.print(",");
        Data.print("HR");
        Data.print(",");
        Data.print("MOI");
        Data.print(",");
        Data.print("LIGHT");
        Data.print(",");
        Data.print("FAN");
        Data.print(",");
        Data.print("EXT");
        Data.println();
      }
      else{Data.close(); return false;}
    Data.close();
    return true;
  }
void writeMSD(){
    DateTime now = rtc.now();
    char buf[100];
    strncpy(buf,"DD.MM.YYYY,hh:mm:ss,\0",100);
    Serial.print(now.format(buf));
    Serial.print(temp);
    Serial.print(",");
    Serial.print(hum);
    Serial.print(",");
    Serial.print(moi);
    Serial.print(",");
    if (LIGHT){Serial.print("ON");}else{Serial.print("OFF");}
    Serial.print(",");
    if (FAN){Serial.print("ON");}else{Serial.print("OFF");}
    Serial.print(",");
    Serial.print(EXTRACTOR);
    Serial.println();
  }
void writeLCD(){
    switch (screen) {
      case 0:          
          screenZero();
          screen=1;
          break;
      case 1:
          screenOne();
          screen = 0;
        break;
    default:
        screen = 0;
    }

  }

void screenZero(){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("HR:");
          lcd.print(hum);
          lcd.print("%");

          lcd.setCursor(0, 0);
          lcd.print("HR:");
          lcd.print(hum);
          lcd.print("%");

          lcd.setCursor(0, 1);
          lcd.print("T:");
          lcd.print(temp);
          lcd.print("C");

          lcd.setCursor(10, 1);
          lcd.print("M:");
          lcd.print(moi);
  }
void screenOne(){
          lcd.clear();

          DateTime now = rtc.now();
          lcd.setCursor(0, 0);
          lcd.print(now.hour());
          lcd.print(':');
          lcd.print(now.minute());

          lcd.setCursor(8, 0);
          lcd.print("EXT:");
          lcd.print(EXTRACTOR);

          lcd.setCursor(0, 1);
          lcd.print("LED:");
          if(LIGHT){lcd.print("ON");}else{lcd.print("OFF");}

          lcd.setCursor(8, 1);
          lcd.print("FAN:");
          if(FAN){lcd.print("ON");}else{lcd.print("OFF");}
    }
