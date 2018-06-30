#include <Wire.h>

#include <ESP8266WiFi.h>
#include "Timer.h"   

const char* ssid = "Motoratones";
const char* password = "trama1234";

const char* host ="192.168.100.2";

const unsigned long PERIOD1 = 1000;    //one second
const unsigned long PERIOD2 = 20000;   //ten seconds
Timer t;                               //instantiate the timer object


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

  t.every(PERIOD2,SendToServer);
}

int value = 0;

void loop()
{
	  t.update();
}

void SendToServer (){

  //delay(2000);
  //++value;

  Serial.print("Connecting to ");
  Serial.println(host);

  //Instancia WIFICLIENT
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)){
    Serial.println("connection failed");
    return;
  }

  value = random(-15, 40);

  String url = "http://192.168.100.2/prueba.php";

  String data = "serie=888&temp="+String(value);

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
      Wire.requestFrom(8, 6);    // request 6 bytes from slave device #8
      
      while (Wire.available()) { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      Serial.println();         // print the character
      Serial.print("Received from Uno: ");         // print the character
      Serial.println(c);         // print the character
  }
  }
