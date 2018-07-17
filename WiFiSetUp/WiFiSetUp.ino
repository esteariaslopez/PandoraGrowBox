#include <Wire.h>

#include <ESP8266WiFi.h>
#include "Timer.h"   

const char* ssid = "Motoratones";
const char* password = "trama1234";
//
//const char* ssid = "HUAWEI P10 lite";
//const char* password = "holamundo";

const char* host ="192.168.100.100";
//const char* host ="192.168.43.81";

const unsigned long PERIOD1 = 10000;    //one second
const unsigned long PERIOD2 = 20000;   //ten seconds
Timer t;                               //instantiate the timer object

int dataSerie = 222;
int charData = 25;
int DeviceNum = 8;

float DHTData[5] = {0};
boolean StateData[3] = {false,false,false};
byte ActuatorData[2] = {0};

byte Data[25] = {0};

void setup()
{
	Serial.begin(115200);
	delay(100);

   Wire.begin(0,2); 

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid,password);

	while (WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print(".");
	}

	Serial.print("");
	Serial.print("WiFi connected");
	Serial.print("IP address: ");
	Serial.print(WiFi.localIP());

  t.every(PERIOD1,SendToServer);
//  t.every(PERIOD1,requestFun);
}

int value = 0;

void loop()
{
	  t.update();
}

void SendToServer (){
  requestFun();
  
  Serial.print("Connecting to ");
  Serial.println(host);

  //Instancia WIFICLIENT
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)){
    Serial.println("connection failed");
    return;
  }


  String url = "http://192.168.100.100/prueba.php";

  String data = "SERIE="+String(dataSerie)+"&HR_IN="+String(DHTData[0])+"&TEMP_IN="+String(DHTData[1])+"&HR_OUT="+String(DHTData[2])+"&TEMP_OUT="+String(DHTData[3])
  +"&MOI="+String(DHTData[4])+"&MODE="+String(StateData[0])+"&LIGHT="+String(StateData[1])+"&FAN="+String(StateData[2])+"&EXT="+String(ActuatorData[0])+"&WPUMP="+String(ActuatorData[1]);

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.0\r\n"+
    "Host: " + host + "\r\n" +
    "Accept: *" + "/" + "*\r\n" +
    "Content-Length: " + data.length() + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + data);

//  Serial.println(String("POST ") + url + " HTTP/1.0\r\n"+
//    "Host: " + host + "\r\n" +
//    "Accept: *" + "/" + "*\r\n" +
//    "Content-Length: " + data.length() + "\r\n" +
//    "Content-Type: application/x-www-form-urlencoded\r\n" +
//    "\r\n" + data);
  delay(10);

  Serial.println("Respond: ");
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();

  Serial.println("closing connection");

  Serial.println("|----------------------------------|");
}

void requestFun(){
      Wire.requestFrom(DeviceNum, charData);    // request 6 bytes from slave device #8  
      int count = 0;
      while (Wire.available()) { // slave may send less than requested
        Data[count] = Wire.read(); // receive a byte as character
        count++;
      }

      byte *ptr = (byte*) &DHTData[0];
    *(ptr) = Data[0];
    *(ptr+1) = Data[1];
    *(ptr+2) = Data[2];
    *(ptr+3) = Data[3];

    ptr = (byte*) &DHTData[1];
    *(ptr) = Data[4];
    *(ptr+1) = Data[5];
    *(ptr+2) = Data[6];
    *(ptr+3) = Data[7];

    ptr = (byte*) &DHTData[2];
    *(ptr) = Data[8];
    *(ptr+1) = Data[9];
    *(ptr+2) = Data[10];
    *(ptr+3) = Data[11];

    ptr = (byte*) &DHTData[3];
    *(ptr) = Data[12];
    *(ptr+1) = Data[13];
    *(ptr+2) = Data[14];
    *(ptr+3) = Data[15];

    ptr = (byte*) &DHTData[4];
    *(ptr) = Data[16];
    *(ptr+1) = Data[17];
    *(ptr+2) = Data[18];
    *(ptr+3) = Data[19];

    ptr = (byte*) &StateData[0];
    *(ptr) = Data[20];

    ptr = (byte*) &StateData[1];
    *(ptr) = Data[21];

    ptr = (byte*) &StateData[2];
    *(ptr) = Data[22];

    ptr = (byte*) &ActuatorData[0];
    *(ptr) = Data[23];

    ptr = (byte*) &ActuatorData[1];
    *(ptr) = Data[24];
    
    Serial.println("FLOAT: ");
    Serial.println(DHTData[0]);
    Serial.println(DHTData[1]);
    Serial.println(DHTData[2]);
    Serial.println(DHTData[3]);
    Serial.println(DHTData[4]);
    Serial.println(StateData[0]);
    Serial.println(StateData[1]);
    Serial.println(StateData[2]);
    Serial.println(ActuatorData[0]);
    Serial.println(ActuatorData[1]);

  }
