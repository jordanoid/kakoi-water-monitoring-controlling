#include <Arduino.h>
#include <WiFi.h>
#include "FirebaseFunctions.hpp"
#include "SensorReadings.hpp"
#include "FuzzyControl.hpp"
#include "connectionSetup.hpp"

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
bool getControlData = false;

unsigned long sendDataPrevMillis = 0;
unsigned long controlPrevMillis = 0;

void setup()
{
  Serial.begin(115200);
  connectionSetup();
  controllerSetup();
  sensorSetup();
  FirebaseSetup();
}

void loop()
{

  waterTemp = readTemperature();
  waterNTU = readTurbidity();


  if (millis() - controlPrevMillis >= 15000 || controlPrevMillis == 0)
  {
    controlPrevMillis = millis();
    getControlData = true;
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
    fuzzyControl(16, 10, waterMinTemp, waterMaxTemp, waterAutoTemp, waterMinPH, waterMaxPH, waterAutoPH);

  }

  if (millis() - sendDataPrevMillis >= 2000 || sendDataPrevMillis == 0 && getControlData == false)
  {
    sendDataPrevMillis = millis();
    RTDBSend(waterTemp, waterNTU);
    FirestoreSend(waterTemp, waterNTU);
    Serial.print("Free Heap : ");
    Serial.println(ESP.getFreeHeap());
  }
  
  getControlData = false;
}