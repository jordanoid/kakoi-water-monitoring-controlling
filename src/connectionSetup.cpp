#include "ConnectionSetup.hpp"
#include <WiFi.h>
#include <Preferences.h>
#include "deviceUID.hpp"
#include "OLED.hpp"
#include <BluetoothSerial.h>

const long wifiTimeout = 15000;
unsigned long startTime;

BluetoothSerial SerialBT;
Preferences preferences;

void waitForBluetoothConnection(unsigned long timeout);
void receiveAndStoreNewCredentials();
void connectToWiFi();
void configureWiFiWithBluetooth();
void loadWiFiCredentials();

void connectionSetup()
{
  preferences.begin("wifi-cred", false);
  SerialBT.begin(DEVICE_IDENTIFIER);
  oledInitWaitforBT();
  waitForBluetoothConnection(wifiTimeout);
  if (SerialBT.connected())
  {
    // A Bluetooth device is connected, receive and store new credentials.
    oledBTConnected();
    receiveAndStoreNewCredentials();
    SerialBT.end();
  }
  else
  {
    // No Bluetooth device connected, proceed with usual setup.
    SerialBT.end();
    loadWiFiCredentials();
    connectToWiFi();
  }
  preferences.end();
}

void connectToWiFi()
{
  WiFi.begin(preferences.getString("ssid", "").c_str(), preferences.getString("password", "").c_str());
  startTime = millis();
  oledConnectingtoWiFi();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout)
  {
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    oledWiFiConnected();
    delay(3000);
  }
  else
  {
    oledWiFiFailed();
    delay(3000);
    oledWaitforBT();
    configureWiFiWithBluetooth();
  }
}

void configureWiFiWithBluetooth()
{
  int attempts = 0;
  SerialBT.begin(DEVICE_IDENTIFIER);
  while (1)
  {
    if (SerialBT.connected())
    {
      if (attempts == 0)
      {
        oledBTConnected();
      }
    }
    if (SerialBT.available())
    {
      SerialBT.println("Please enter new WiFi credentials in the format: SSID,password");
      String input = SerialBT.readStringUntil('\n');
      input.trim();

      int commaIndex = input.indexOf(',');
      if (commaIndex != -1)
      {
        String newSSID = input.substring(0, commaIndex);
        String newPassword = input.substring(commaIndex + 1);

        preferences.putString("ssid", newSSID.c_str());
        preferences.putString("password", newPassword.c_str());

        // Attempt to connect with the new credentials
        WiFi.begin(newSSID.c_str(), newPassword.c_str());
        startTime = millis();
        oledConnectingtoWiFi();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout)
        {
          delay(500);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
          SerialBT.println("connected");
          delay(500);
          oledWiFiConnected();
          delay(3000);
          SerialBT.end();
          break; // Exit the loop if connected
        }
        else
        {
          oledWiFiFailedInsertNew();
          SerialBT.println("not-connected");
          attempts++;
        }

        if (attempts >= 3)
        {
          oledWiFiLimit();
          delay(3000);
          SerialBT.end();
          break;
        }
      }
    }
  }
}

void loadWiFiCredentials()
{

  String storedSSID = preferences.getString("ssid", "");
  String storedPassword = preferences.getString("password", "");

}

void waitForBluetoothConnection(unsigned long timeout)
{
  unsigned long startTime = millis();
  while (!SerialBT.connected() && millis() - startTime < timeout)
  {
    delay(500);
  }
}

void receiveAndStoreNewCredentials()
{
  int attempts = 0;
  while (1)
  {
    if (SerialBT.available())
    {
      String input = SerialBT.readStringUntil('\n');
      input.trim();

      int commaIndex = input.indexOf(',');
      if (commaIndex != -1)
      {
        String newSSID = input.substring(0, commaIndex);
        String newPassword = input.substring(commaIndex + 1);

        preferences.putString("ssid", newSSID.c_str());
        preferences.putString("password", newPassword.c_str());


        // Attempt to connect with the new credentials
        WiFi.begin(newSSID.c_str(), newPassword.c_str());
        startTime = millis();
        oledConnectingtoWiFi();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout)
        {
          delay(500);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
          SerialBT.println("connected");
          delay(500);
          oledWiFiConnected();
          SerialBT.end();
          delay(3000);
          break; // Exit the loop if connected
        }
        else
        {
          SerialBT.println("not-connected");
          oledWiFiFailedInsertNew();
          attempts++;
        }

        // Retry up to 3 times (adjust as needed)
        if (attempts >= 3)
        {
          oledWiFiLimit();
          break;
        }
      }
    }
  }
}

bool checkWiFiStatus()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }
  else
  {
    return false;
  }
}
