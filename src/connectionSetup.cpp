#include "connectionSetup.hpp"
#include <WiFi.h>
#include <BluetoothSerial.h>
#include <Preferences.h>

const long wifiTimeout = 15000;  // 30 seconds
unsigned long startTime;

BluetoothSerial SerialBT;
Preferences preferences;

void connectionSetup() {
  Serial.print("Open prefecences : ");
  Serial.println(preferences.begin("wifi-cred", false));
  loadWiFiCredentials();
  connectToWiFi();
  preferences.end();
}

void connectToWiFi() {

  WiFi.begin(preferences.getString("ssid", "").c_str(), preferences.getString("password", "").c_str());
  startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
  } else {
    Serial.println("Failed to connect to WiFi. Starting Bluetooth configuration.");
    configureWiFiWithBluetooth();
  }
}

void configureWiFiWithBluetooth() {
  SerialBT.begin("ESP32_BT");
  while (1) {
    if (SerialBT.available()) {
      SerialBT.println("Please enter new WiFi credentials in the format: SSID,password");
      String input = SerialBT.readStringUntil('\n');
      input.trim();

      int commaIndex = input.indexOf(',');
      if (commaIndex != -1) {
        String newSSID = input.substring(0, commaIndex);
        String newPassword = input.substring(commaIndex + 1);

        preferences.putString("ssid", newSSID.c_str());
        preferences.putString("password", newPassword.c_str());
        Serial.println("New WiFi credentials received.");
        Serial.print("SSID: ");
        Serial.println(preferences.getString("ssid", ""));
        Serial.print("Password: ");
        Serial.println(preferences.getString("password", ""));

        // Attempt to connect with the new credentials
        WiFi.begin(newSSID.c_str(), newPassword.c_str());
        startTime = millis();

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiTimeout) {
          delay(500);
          Serial.println("Connecting to WiFi with new credentials...");
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to WiFi with new credentials");
          btStop();
          break; // Exit the loop if connected
        } else {
          Serial.println("Failed to connect to WiFi with new credentials. Retrying...");
          attempts++;
        }

        // Retry up to 3 times (adjust as needed)
        if (attempts >= 3) {
          Serial.println("Max retry attempts reached. Please try again later.");
          break;
        }
      } else {
        Serial.println("Invalid input format. Please enter SSID and password separated by a comma.");
      }
    }
  }
}

void loadWiFiCredentials() {

  String storedSSID = preferences.getString("ssid", "");
  String storedPassword = preferences.getString("password", "");

  Serial.println("Stored WiFi credentials:");
  Serial.print("SSID: ");
  Serial.println(storedSSID);
  Serial.print("Password: ");
  Serial.println(storedPassword);

}