#include "SensorReadings.hpp"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

#define TEMP_SENSOR 27
#define PH_SENSOR 34
#define TURBIDITY 36

OneWire oneWire(TEMP_SENSOR);
DallasTemperature ds18b20(&oneWire);

float ph4 = 3.18;
float ph7 = 2.6;
float po, ph_step;
unsigned long int avgval;
int buffer_arr[10], temp;

float readTemperature()
{
    ds18b20.requestTemperatures();
    float tempC = ds18b20.getTempCByIndex(0);
    if (tempC != DEVICE_DISCONNECTED_C)
    {
        return tempC;
    }
    else
    {
        return -1;
    }
}

float readTurbidity()
{
    float ADC = analogRead(TURBIDITY);
    float voltage = ADC * 3.3 / 4096;
    float NTU = (-0.6393 * ADC) + 1472.6;
    if (NTU < 0 || voltage <= 0.5)
    {
        return -1;
    }
    else
    {
        return NTU;
    }
}

float readPH()
{
    for (int i = 0; i < 10; i++)
    {
        buffer_arr[i] = analogRead(PH_SENSOR);
        delay(30);
    }
    for (int i = 0; i < 9; i++)
    {
        for (int j = i + 1; j < 10; j++)
        {
            if (buffer_arr[i] > buffer_arr[j])
            {
                temp = buffer_arr[i];
                buffer_arr[i] = buffer_arr[j];
                buffer_arr[j] = temp;
            }
        }
    }
    avgval = 0;
    for (int i = 2; i < 8; i++)
        avgval += buffer_arr[i];
    float volt = (float)avgval * 3.3 / 4096 / 6;
    ph_step = (ph4 - ph7) / 2.86;
    po = 6.86 + ((ph7 - volt) / ph_step);
    if (po > 14 || po < 0 || volt == 0)
    {
        return -1;
    }
    else
    {
        return po;
    }
}

void sensorSetup()
{
    pinMode(TURBIDITY, INPUT);
    pinMode(PH_SENSOR, INPUT);
    ds18b20.begin();
}
