#include <Arduino.h>
#include <WiFi.h>
#include "FirebaseFunctions.hpp"
#include "SensorReadings.hpp"

#define PHUP 17
#define PHDOWN 23
#define PELT 18
#define HEATER 19

#define WIFI_SSID "JORDANO 8600"
#define WIFI_PASSWORD "12345678"

int relay[4] = {17, 23, 18, 19};
float waterTemp;
float waterNTU;
float waterPH;

void relaySetup();

void setup()
{
  Serial.begin(115200);
  relaySetup();
  sensorSetup();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  FirebaseSetup();
  
}

void loop()
{
  waterTemp = readTemperature();
  waterNTU = readTurbidity();
  RTDBSend(waterTemp, waterNTU);
  FirestoreSend(waterTemp, waterNTU);
}

void relaySetup()
{
  for (int i = 0; i < 4; i++)
    pinMode(relay[i], OUTPUT);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(relay[i], HIGH);
    delay(100);
  }
  delay(500);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(relay[i], LOW);
    delay(100);
  }
};