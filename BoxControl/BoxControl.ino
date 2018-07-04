#include "DHT.h"
#include "Timer.h" 
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHT_IN_PIN 2     
#define DHT_OUT_PIN 7  
 
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
boolean MODE = false; //true = bloom mode //false = growing mode

const double G_HOURON = 4;
const double G_MINUTEON = 0;
const double G_HOUROFF = 22;
const double G_MINUTEOFF = 0;

const double B_HOURON = 5;
const double B_MINUTEON = 0;
const double B_HOUROFF = 17;
const double B_MINUTEOFF = 0;

//EXTRACTOR
const int tempON = 24;
const int tempOFF = 23;

const int minMOI = 820;
const int unMOI = 500;

const unsigned long Time_DHT = 1000;
const unsigned long Time_LCD = 3000;
const unsigned long Time_EXT = 500;
const unsigned long Time_WATER = 60000;
const unsigned long Time_MSD = 10000;

const unsigned long Time_LIGHT = 10000;
const unsigned long Time_MOI = 1000;
const unsigned long Time_FAN = 30000;

float DHTData[4] = {0,0,0,0}; 
const int HR_in = 0;
const int T_in = 1;
const int HR_out = 2;
const int T_out = 3;

int moi = 0;


DHT dht_in(DHT_IN_PIN, DHTTYPE);
DHT dht_out(DHT_OUT_PIN, DHTTYPE);

Timer timer;
DS1307 rtc;

void setup() {
  pinMode(EXTPIN, OUTPUT);
  pinMode(LIGHTPIN, OUTPUT);
  digitalWrite(LIGHTPIN, HIGH);
  pinMode(FANPIN, OUTPUT);
  digitalWrite(FANPIN, HIGH);
  analogReference(EXTERNAL);


  
  Serial.begin(115200);
  Serial.println("PANDORA GROWING BOX START");
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PANDORA BOX");

//  delay(1000);
//  lcd.setCursor(0, 1);
//  if(SD.begin(SDENABLEPIN)){lcd.print("SD READY");}else{lcd.print("SD FAILED");}
//  delay(1000);
//  if (writeHEADER){lcd.print("  OK");}else{lcd.print("  ERROR");}

  timer.every(Time_DHT, readDHT);
  timer.every(Time_MOI, readMOI);
  timer.every(Time_EXT, extCTRL);
  timer.every(Time_LIGHT, lightCTRL);
  timer.every(Time_FAN, fanCTRL);
  timer.every(Time_WATER, waterCTRL);
  
  timer.every(Time_LCD, writeLCD);
  //timer.every(Time_MSD, writeMSD);

  dht_in.begin();
  dht_out.begin();
  Wire.begin(8);
  rtc.begin();

//  /Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  timer.update();
}

void requestEvent() {
  Wire.write("HALLO "); // respond with message of 6 bytes
  // as expected by master
}
  
void readDHT(){
    DHTData[HR_in] = dht_in.readHumidity();
    DHTData[T_in] = dht_in.readTemperature();
    
    DHTData[HR_out] = dht_out.readHumidity();
    DHTData[T_out] = dht_out.readTemperature();
  }
  
void extCTRL(){
    if (DHTData[T_in] >= tempON){EXTRACTOR = 100;}else{
      if (DHTData[T_in] <= tempOFF){EXTRACTOR = 0;}else{
          if (isnan(DHTData[T_in])){if(LIGHT){EXTRACTOR = 100;}else{EXTRACTOR = 0;};}
        }
    }
    int PWM = map(EXTRACTOR, 0, 100, 0, 255);
    analogWrite(EXTPIN,PWM);

//    int potval = analogRead(POTPIN); Serial.print(potval);
//    potval = map(potval, 0, 1023, 0, 255);Serial.print("  ");
//    analogWrite(EXTPIN,potval);Serial.println(potval);
  }
void readMOI(){
    int moival = analogRead(MOIPIN);
    moi = moival;
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
  double horaON;
  double horaOFF;
  if (MODE){
      horaON = B_HOURON + B_MINUTEON/60; 
      horaOFF = B_HOUROFF + B_MINUTEOFF/60;
    }
    else{
        horaON = G_HOURON + G_MINUTEON/60; 
        horaOFF = G_HOUROFF + G_MINUTEOFF/60;
      } 
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

//bool writeHEADER(){
//    Serial.print("DD.MM.YYYY,hh:mm:ss,");
//    Serial.print("TEMP");
//    Serial.print(",");
//    Serial.print("HR");
//    Serial.print(",");
//    Serial.print("MOI");
//    Serial.print(",");
//    Serial.print("LIGHT");
//    Serial.print(",");
//    Serial.print("FAN");
//    Serial.print(",");
//    Serial.print("EXT");
//    Serial.println();
//
//    Data = SD.open("test.txt", FILE_WRITE);
//    if (!Data){
//        Data.print("DD.MM.YYYY,hh:mm:ss,");
//        Data.print("TEMP");
//        Data.print(",");
//        Data.print("HR");
//        Data.print(",");
//        Data.print("MOI");
//        Data.print(",");
//        Data.print("LIGHT");
//        Data.print(",");
//        Data.print("FAN");
//        Data.print(",");
//        Data.print("EXT");
//        Data.println();
//      }
//      else{Data.close(); return false;}
//    Data.close();
//    return true;
//  }
//void writeMSD(){
//    DateTime now = rtc.now();
//    char buf[100];
//    strncpy(buf,"DD.MM.YYYY,hh:mm:ss,\0",100);
//    Serial.print(now.format(buf));
//    Serial.print(temp);
//    Serial.print(",");
//    Serial.print(hum);
//    Serial.print(",");
//    Serial.print(moi);
//    Serial.print(",");
//    if (LIGHT){Serial.print("ON");}else{Serial.print("OFF");}
//    Serial.print(",");
//    if (FAN){Serial.print("ON");}else{Serial.print("OFF");}
//    Serial.print(",");
//    Serial.print(EXTRACTOR);
//    Serial.println();
//  }


void writeLCD(){
    switch (screen) {
      case 0:          
          screenZero();
          screen=1;
          break;
      case 1:
          screenOne();
          screen = 2;
        break;
      case 2:
          screenTwo();
          screen = 3;
        break;
      case 3:
          screenThree();
          screen = 0;
        break;
    default:
        screen = 0;
    }

  }
void screenZero(){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("INSIDE");

          lcd.setCursor(0, 1);
          lcd.print("T:");
          lcd.print(DHTData[T_in]);
          lcd.print("C");

          lcd.setCursor(10, 1);
          lcd.print("HR:");
          lcd.print(DHTData[HR_in]);
          lcd.print("%");
  }
void screenOne(){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("OUTSIDE");

          lcd.setCursor(0, 1);
          lcd.print("T:");
          lcd.print(DHTData[T_out]);
          lcd.print("C");

          lcd.setCursor(10, 1);
          lcd.print("HR:");
          lcd.print(DHTData[HR_out]);
          lcd.print("%");
  }
void screenTwo(){
          lcd.clear();

//          DateTime now = rtc.now();
//          lcd.setCursor(0, 0);
//          lcd.print(now.hour());
//          lcd.print(':');
//          lcd.print(now.minute());

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
    
    void screenThree(){
          lcd.clear();

          DateTime now = rtc.now();
          lcd.setCursor(0, 0);
          lcd.print(now.hour());
          lcd.print(':');
          lcd.print(now.minute());

          lcd.setCursor(0, 1);
          if (MODE){lcd.print("BLOOM MODE");} else{lcd.print("GROWING MODE");}
        
    }
