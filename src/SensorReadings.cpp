#include "SensorReadings.hpp"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>


#define TEMP_SENSOR 27
#define PH_SENSOR 36
#define TURBIDITY 35

OneWire oneWire(TEMP_SENSOR);
DallasTemperature ds18b20(&oneWire);

float readTemperature()
{
    // Serial.print("Requesting temperatures...");
    ds18b20.requestTemperatures();
    // Serial.println("DONE");
    float tempC = ds18b20.getTempCByIndex(0);

    if (tempC != DEVICE_DISCONNECTED_C)
    {
        Serial.print("Temperature for the device 1 (index 0) is: ");
        Serial.println(tempC);
        return tempC;
    }
    else
    {
        Serial.println("Error: Could not read temperature data");
        return -1;
    }
}

float readTurbidity()
{
    float ADC = analogRead(TURBIDITY);
    float voltage = ADC * 3.3 / 4096;
    float NTU = (-6539.8 * voltage) + 10475;
    Serial.print("NTU: ");
    Serial.println(NTU);
    Serial.print("Voltage is: ");
    Serial.println(voltage);
    if (NTU < 0)
    {
        return -1;
    }
    else
    {
        return NTU;
    }
    
}

// float readPH()
// {
    
// }

void sensorSetup()
{
    pinMode(TURBIDITY, INPUT);
    ds18b20.begin();
}
