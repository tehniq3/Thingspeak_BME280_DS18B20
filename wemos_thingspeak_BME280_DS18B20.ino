// Nicu FLORICA (niq_ro) combine more sketch in one :))
//scl na D1    sda na D2

#include <BME280_MOD-1022.h>
#include <Wire.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 14  // GPIO14 = D5 see https://www.reddit.com/r/esp8266/comments/9q3314/nodemcu_v1_v2_v3_size_and_pinout_comparison/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Wifi and ThingSpeak settings
#include <ESP8266WiFi.h>
 
const char* ssid = "SSID";
const char* password = "password";
 
const char* server = "api.thingspeak.com";
const char* api_key = "yourAPIkey";

// Measurement interval (seconds)
const int interval = 100; //5 mins

#define LED D4
 
WiFiClient client;

void printFormattedFloat(float x, uint8_t precision) {
char buffer[10];

  dtostrf(x, 7, precision, buffer);
  Serial.print(buffer);

}

void printCompensatedMeasurements(void) {

float temp, humidity,  pressure, pressureMoreAccurate;
double tempMostAccurate, humidityMostAccurate, pressureMostAccurate;
char buffer[80];
float tempDS;

  temp      = BME280.getTemperature();
  humidity  = BME280.getHumidity();
  pressure  = BME280.getPressure();
  
  pressureMoreAccurate = BME280.getPressureMoreAccurate();  // t_fine already calculated from getTemperaure() above
  
  tempMostAccurate     = BME280.getTemperatureMostAccurate();
  humidityMostAccurate = BME280.getHumidityMostAccurate();
  pressureMostAccurate = BME280.getPressureMostAccurate();
  
  Serial.print("Temperature: ");
  printFormattedFloat(tempMostAccurate, 2);
  Serial.println();
  
  Serial.print("Humidity: ");
  printFormattedFloat(humidityMostAccurate, 2);
  Serial.println();

  Serial.print("Pressure: ");
  printFormattedFloat(pressureMostAccurate, 2);
  Serial.println();

  sensors.requestTemperatures();
  tempDS = sensors.getTempCByIndex(0);
//  tempC = DallasTemperature::toFahrenheit(tempC);

  Serial.print("DS18B20 Temperature: ");
  printFormattedFloat(tempDS, 2);
  Serial.println();

  // Post data to ThingSpeak
  postData(tempMostAccurate, humidityMostAccurate, pressureMostAccurate, tempDS);
  Serial.println();
}

void postData(float tempbme, float humidity, float pressure, float tempdese){
  // Send data to ThingSpeak
  if (client.connect(server,80)) {
  Serial.println("Connect to ThingSpeak - OK"); 

  String dataToThingSpeak = "";
  dataToThingSpeak+="GET /update?api_key=";
  dataToThingSpeak+=api_key;
   
  dataToThingSpeak+="&field1=";
  dataToThingSpeak+=String(tempbme);

  dataToThingSpeak+="&field2=";
  dataToThingSpeak+=String(humidity);

  dataToThingSpeak+="&field3=";
  dataToThingSpeak+=String(pressure);

  dataToThingSpeak+="&field4=";
  dataToThingSpeak+=String(tempdese);
   
  dataToThingSpeak+=" HTTP/1.1\r\nHost: a.c.d\r\nConnection: close\r\n\r\n";
  dataToThingSpeak+="";
  client.print(dataToThingSpeak);

  int timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println("Error: Client Timeout!");
      client.stop();
      return;
    }
  }
}
 while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}


// Setup wire and serial
void setup()
{
  Wire.begin();
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  delay(10);

  sensors.begin();  //for dallas temp sensor
  // 9 bit resolution by default 
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  int resolution = 11;
  sensors.setResolution(resolution);
  delay(750/ (1 << (12-resolution)));

  Serial.println("Connecting to wifi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    
    // Blink LED when connecting to wifi
    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
  }
  Serial.println("WiFi connected");
  

  // Prepare LED to turn on when measuring and send data
  
}

// main loop
void loop()
{ 
  // need to read the NVM compensation parameters
  BME280.readCompensationParams();

  // We'll switch into normal mode for regular automatic samples  
  BME280.writeStandbyTime(tsb_0p5ms);         // tsb = 0.5ms
  BME280.writeFilterCoefficient(fc_16);       // IIR Filter coefficient 16
  BME280.writeOversamplingPressure(os16x);    // pressure x16
  BME280.writeOversamplingTemperature(os2x);  // temperature x2
  BME280.writeOversamplingHumidity(os1x);     // humidity x1
  
  BME280.writeMode(smNormal);
   
  while (1) {
    digitalWrite(LED, LOW);
    while (BME280.isMeasuring()) {
       //Serial.println("Measuring...");
       //delay(100);
    }
    
    // read out the data - must do this before calling the getxxxxx routines
    BME280.readMeasurements();
    printCompensatedMeasurements();
    digitalWrite(LED, HIGH);
    
    delay(interval*500);
    Serial.println();
  }
}
