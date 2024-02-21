#include <Arduino.h>
#include <WiFi.h>
#include "FirebaseFunctions.hpp"
#include "SensorReadings.hpp"
#include "FuzzyControl.hpp"
#include "connectionSetup.hpp"
#include "OLED.hpp"

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

TaskHandle_t controllerTask, firebaseTask;
SemaphoreHandle_t flag;

void controllerLoop(void *parameter)
{
  for (;;)
  {
    xSemaphoreTake(flag, portMAX_DELAY);
    waterTemp = readTemperature();
    waterNTU = readTurbidity();
    waterPH = readPH();
    fuzzyControl(waterTemp, waterPH, waterMinTemp, waterMaxTemp, waterAutoTemp, waterMinPH, waterMaxPH, waterAutoPH);
    oledMainDisplay(checkWiFiStatus(), waterAutoPH, waterAutoTemp, waterTemp, waterPH, waterNTU);
    xSemaphoreGive(flag);
  }
}

void firebaseLoop(void *parameter)
{
  for (;;)
  {
    if (millis() - controlPrevMillis >= 5000 || controlPrevMillis == 0)
    {
      controlPrevMillis = millis();
      waterMinTemp = getMinTemp();
      waterMaxTemp = getMaxTemp();
      waterAutoTemp = getAutoTemp();
      waterMinPH = getMinPH();
      waterMaxPH = getMaxPH();
      waterAutoPH = getAutoPH();
    }

    if (millis() - sendDataPrevMillis >= 2000 || sendDataPrevMillis == 0)
    {
      sendDataPrevMillis = millis();
      RTDBSend(waterTemp, waterNTU, waterPH);
      FirestoreSend(waterTemp, waterNTU, waterPH);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  oledSetup();
  connectionSetup();
  controllerSetup();
  FirebaseSetup();

  waterMinTemp = getMinTemp();
  waterMaxTemp = getMaxTemp();
  waterAutoTemp = getAutoTemp();
  waterMinPH = getMinPH();
  waterMaxPH = getMaxPH();
  waterAutoPH = getAutoPH();

  sensorSetup();
  flag = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
      controllerLoop,
      "ControllerTask",
      8192,
      NULL,
      1,
      &controllerTask,
      0);

  xTaskCreatePinnedToCore(
      firebaseLoop,
      "FirebaseTask",
      8192,
      NULL,
      1,
      &firebaseTask,
      1);
}

void loop() {}