#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 14  // GPIO14 = D5 see https://www.reddit.com/r/esp8266/comments/9q3314/nodemcu_v1_v2_v3_size_and_pinout_comparison/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#if defined(ESP8266)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#else
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#endif
//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//String apiKey = "API_keyL"; // <<<<< YOUR API KEY
String apiKey = "APIkey"; // <<<<< YOUR API KEY
const char* server = "api.thingspeak.com";

unsigned long delayTime;

WiFiClient client;


void setup() {
    Serial.begin(9600);
    Serial.println(F("DS18B20 test"));

sensors.begin();  //for dallas temp sensor
 // 9 bit resolution by default 
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  int resolution = 11;
  sensors.setResolution(resolution);
  delay(750/ (1 << (12-resolution)));
//  checkSensors();   //check all available sensors and save values to RAM

//WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
}


void loop() { 
    printValues();
  //  delay(delayTime);
  
      if (client.connect(server, 80))
    {
      String postStr = apiKey;
      postStr += "&field4=";
      postStr += String(sensors.getTempCByIndex(0));
      postStr += "\r\n\r\n";
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      Serial.println(postStr);
    }
    client.stop();
    Serial.println("I'm waitting new data...");
   // delay(57000);
    delay(5000);


}  // end main loop


void printValues() {
 sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
//  tempC = DallasTemperature::toFahrenheit(tempC);
  char buffer[10];
  String strTemp = dtostrf(tempC, 4, 1, buffer);
   
Serial.println(strTemp);
    Serial.println();
}
