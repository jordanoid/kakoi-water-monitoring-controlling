#include <Arduino.h>
#include <WiFi.h>
#include "FirebaseFunctions.hpp"
#include "SensorReadings.hpp"
#include "FuzzyControl.hpp"
#include "connectionSetup.hpp"
#include "OLED.hpp"

#include <random>

#define PHUP 17
#define PHDOWN 23
#define PELT 18
#define HEATER 19

float waterTemp;
float waterNTU;
float waterPH;
float waterMinTemp;
float waterMaxTemp;
bool waterAutoTemp;
float waterMinPH;
float waterMaxPH;
bool waterAutoPH;

unsigned long sendDataPrevMillis = 0;
unsigned long controlPrevMillis = 0;

std::random_device rd;                                 // Seed for the random number engine
std::mt19937 gen(rd());                                // Mersenne Twister 19937 generator
std::uniform_real_distribution<float> dis(6.0f, 8.0f); // Distribution between 6 and 9

void setup()
{
  Serial.begin(115200);
  oledSetup();
  connectionSetup();
  controllerSetup();
  FirebaseSetup();
  sensorSetup();
}

void loop()
{

  waterTemp = readTemperature();
  waterNTU = readTurbidity();
  waterPH = readPH();

  if (millis() - controlPrevMillis >= 15000 || controlPrevMillis == 0)
  {
    controlPrevMillis = millis();
    Serial.println("Control Data");
    waterMinTemp = getMinTemp();
    waterMaxTemp = getMaxTemp();
    waterAutoTemp = getAutoTemp();
    waterMinPH = getMinPH();
    waterMaxPH = getMaxPH();
    waterAutoPH = getAutoPH();
    Serial.println(waterMinTemp);
    Serial.println(waterMaxTemp);
    Serial.println(waterAutoTemp);
    Serial.println(waterMinPH);
    Serial.println(waterMaxPH);
    Serial.println(waterAutoPH);
  }

  fuzzyControl(waterTemp, waterPH, waterMinTemp, waterMaxTemp, waterAutoTemp, waterMinPH, waterMaxPH, waterAutoPH);

  if (millis() - sendDataPrevMillis >= 2000 || sendDataPrevMillis == 0)
  {
    sendDataPrevMillis = millis();
    RTDBSend(waterTemp, waterNTU, waterPH);
    FirestoreSend(waterTemp, waterNTU, waterPH);
    Serial.print("Free Heap : ");
    Serial.println(ESP.getFreeHeap());
  }

  oledMainDisplay(checkWiFiStatus(), waterAutoPH, waterAutoTemp, waterTemp, waterPH, waterNTU);
}