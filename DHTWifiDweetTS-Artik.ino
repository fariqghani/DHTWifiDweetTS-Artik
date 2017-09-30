// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

#include "DHT.h"
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <CytronWiFiShield.h>
#include <CytronWiFiClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>

#define DHTPIN 10     // what digital pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "FG Tijarah SB";
char pass[] = "fgt98765";
ESP8266Client client;

// Hardware Serial on Mega, Leonardo, Micro...
//#define EspSerial Serial1

// Software Serial on Uno, Nano...
SoftwareSerial EspSerial(2, 3); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 9600

/** ARTIK Cloud REST Initialization **/
char server[] = "api.artik.cloud";    // Samsung ARTIK Cloud API Host
int port = 443;                       // 443 for HTTPS 

char buf[200];                        // body data to store the JSON to be sent to the ARTIK cloud 

String deviceID = "ee9ca858269d4eadb2d94753c93ee1e6"; // put your device id here created from tutorial 
String deviceToken = "c0ef4ecd978b493cb20a4dca6d5af63a"; // put your device token here created from tutorial

float t;
float h;
boolean onFire = false; 

const int LED = 6; 
int ledState = 0; 


void setup() {
  Serial.begin(9600);
  Serial.println("DHTxx test!");
    // Set ESP8266 baud rate
  delay(10);
   
  if(!wifi.begin(2, 3))
  {
    Serial.println(F("Error talking to shield"));
    while(1);
  }
  Serial.println(F("Start wifi connection"));
  if(!wifi.connectAP(ssid, pass))
  {
    Serial.println(F("Error connecting to WiFi"));
    while(1);
  } 
  Serial.print(F("Connected to "));Serial.println(wifi.SSID());
  Serial.println(F("IP address: "));
  Serial.println(wifi.localIP()); 

  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(500);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  Serial.println("==========================================="); 
  Serial.println("We will send these json data"); 
  //print to json format
  Serial.println("data: { ");
  Serial.print("temperature: ");
  Serial.print(t);
  Serial.print(" , humidity: ");
  Serial.print(h);
  Serial.println("} ");

  Serial.println("");
  
  Serial.println("Start sending data"); 
  String contentType = "application/json"; 
  String AuthorizationData = "Bearer " + deviceToken; //Device Token 
  int len = loadBuffer(t,h);   
  Serial.println("Sending temperature: "+String(t) +" and humidity: "+String(h) );  
  Serial.println("Send POST to ARTIK Cloud API"); 
  client.beginRequest(); 
  client.post("/v1.1/messages"); //, contentType, buf 
  client.sendHeader("Authorization", AuthorizationData); 
  client.sendHeader("Content-Type", "application/json"); 
  client.sendHeader("Content-Length", len); 
  client.endRequest(); 
  client.print(buf); 
  
  // print response from api
  int statusCode = client.responseStatusCode(); 
  String response = client.responseBody(); 
  Serial.println("");
  Serial.print("Status code: "); 
  Serial.println(statusCode); 
  Serial.print("Response: "); 
  Serial.println(response);   
  delay(5000); // delay of update 
}

/*Buffer to send on REST*/
int loadBuffer(float temp, float humidity ) {   
  StaticJsonBuffer<200> jsonBuffer; // reserve spot in memory 
  JsonObject& root = jsonBuffer.createObject(); // create root objects 
  root["sdid"] =  deviceID;   
  root["type"] = "message"; 
  JsonObject& dataPair = root.createNestedObject("data"); // create nested objects 
  dataPair["temp"] = temp;   
  dataPair["humidity"] = humidity; 
  root.printTo(buf, sizeof(buf)); // JSON-print to buffer 
  return (root.measureLength()); // also return length 
} 
