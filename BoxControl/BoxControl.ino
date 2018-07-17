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

LiquidCrystal lcd(38, 36, 34, 32, 30, 28);//RS-EN-D4-D5-D6-D7
int screen = 0;

const double G_HOURON = 4;
const double G_MINUTEON = 0;
const double G_HOUROFF = 22;
const double G_MINUTEOFF = 0;

const double B_HOURON = 5;
const double B_MINUTEON = 0;
const double B_HOUROFF = 17;
const double B_MINUTEOFF = 0;

//EXTRACTOR
const int tempON = 21;
const int tempOFF = 20;

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

int charData = 25;
int DeviceNum = 8;

float DHTData[5] = {0};
boolean StateData[3] = {true,false,true};
byte ActuatorData[2] = {0};

byte Data[25];
  
float     HR_IN   = 0;
float     T_IN     = 0;
float     HR_OUT   = 0;
float     T_OUT    = 0;
float     MOI      = 0;
boolean   MODE     = false; //true = bloom mode //false = growing mode
boolean   LIGHT    = false;
boolean   FAN      = false;
byte      EXTRACTOR = 0;
byte      WPUMP     =0;


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

  timer.every(Time_DHT, readDHT);
  timer.every(Time_MOI, readMOI);
  timer.every(Time_EXT, extCTRL);
  timer.every(Time_LIGHT, lightCTRL);
  timer.every(Time_FAN, fanCTRL);
//  timer.every(Time_WATER, waterCTRL);
  
  timer.every(Time_LCD, writeLCD);

  dht_in.begin();
  dht_out.begin();
  Wire.begin(DeviceNum);
  rtc.begin();

  Wire.onRequest(requestEvent); // register event
  
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
 StateData[0] = MODE;
}

void loop() {
  timer.update();
}

void requestEvent() { 
        byte *ptr = (byte*) &DHTData[0];
        Data[0]= byte(*(ptr));
        Data[1]= byte(*(ptr+1));
        Data[2]= byte(*(ptr+2));
        Data[3]= byte(*(ptr+3));
        
        ptr = (byte*) &DHTData[1];
        Data[4]= byte(*(ptr));
        Data[5]= byte(*(ptr+1));
        Data[6]= byte(*(ptr+2));
        Data[7]= byte(*(ptr+3));
        
        ptr = (byte*) &DHTData[2];
        Data[8]= byte(*(ptr));
        Data[9]= byte(*(ptr+1));
        Data[10]= byte(*(ptr+2));
        Data[11]= byte(*(ptr+3));
        
        ptr = (byte*) &DHTData[3];
        Data[12]= byte(*(ptr));
        Data[13]= byte(*(ptr+1));
        Data[14]= byte(*(ptr+2));
        Data[15]= byte(*(ptr+3));
        
        ptr = (byte*) &DHTData[4];
        Data[16]= byte(*(ptr));
        Data[17]= byte(*(ptr+1));
        Data[18]= byte(*(ptr+2));
        Data[19]= byte(*(ptr+3));
        
        ptr = (byte*) &StateData[0];
        Data[20]= byte(*(ptr));
        
        ptr = (byte*) &StateData[1];
        Data[21]= byte(*(ptr));
        
        ptr = (byte*) &StateData[2];
        Data[22]= byte(*(ptr));
        
        ptr = (byte*) &ActuatorData[0];
        Data[23]= byte(*(ptr));
        
        ptr = (byte*) &ActuatorData[1];
        Data[24]= byte(*(ptr));
        
        
        for (int i = 0; i < charData; i++){
          Wire.write(Data[i]); // respond with message of 6 bytes
        }
}
  
void readDHT(){
    HR_IN = dht_in.readHumidity();
    T_IN = dht_in.readTemperature();
    
    HR_OUT = dht_out.readHumidity();
    T_OUT = dht_out.readTemperature();

    DHTData[0] = HR_IN;
    DHTData[1] = T_IN;
    DHTData[2] = HR_OUT;
    DHTData[3] = T_OUT;
  }
void readMOI(){
    int moival = analogRead(MOIPIN);
    MOI = int(moival);
    DHTData[4] = MOI;
  }
  
void extCTRL(){
    if (T_IN >= tempON){EXTRACTOR = 100;}else{
      if (T_IN <= tempOFF){EXTRACTOR = 0;}else{
          if (isnan(T_IN)){if(LIGHT){EXTRACTOR = 100;}else{EXTRACTOR = 0;};}
        }
    }
    int PWM = map(EXTRACTOR, 0, 100, 0, 255);
    analogWrite(EXTPIN,PWM);//StateData
    ActuatorData[0] = EXTRACTOR;
  }
  
void waterCTRL(){ 
    if (MOI <= minMOI && MOI >= unMOI){
      WPUMP = 100;
      int PWM = map(WPUMP, 0, 100, 0, 255);
      analogWrite(WPUMPPIN,PWM);
      ActuatorData[1] = WPUMP;
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WATER PUMP ON:");
      delay(10000);

      writeLCD();
      
      WPUMP = 0;
      PWM = map(WPUMP, 0, 100, 0, 255);
      analogWrite(WPUMPPIN,PWM);
      }else{WPUMP = 0;};
      ActuatorData[1] = WPUMP;
      
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
    StateData[1] = LIGHT;
  }

  
void fanCTRL(){
    if (FAN){FAN = false;} else{FAN = true;}      
    digitalWrite(FANPIN, !FAN);
    StateData[2] = FAN;

  }

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
          lcd.print(T_IN);
          lcd.print("C");

          lcd.setCursor(10, 1);
          lcd.print("HR:");
          lcd.print(HR_IN);
          lcd.print("%");
  }
void screenOne(){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("OUTSIDE");

          lcd.setCursor(0, 1);
          lcd.print("T:");
          lcd.print(T_OUT);
          lcd.print("C");

          lcd.setCursor(10, 1);
          lcd.print("HR:");
          lcd.print(HR_OUT);
          lcd.print("%");
  }
void screenTwo(){
          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("M:");
          lcd.print(int(MOI));
          
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
