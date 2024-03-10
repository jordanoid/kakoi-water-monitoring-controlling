#include "OLED.hpp"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM image_Ble_disconnected_15x15_bits[] = {0x07, 0xc0, 0x18, 0x30, 0x21, 
                                                                          0x08, 0x41, 0x84, 0x49, 0x44, 
                                                                          0x85, 0x22, 0x83, 0x42, 0x81, 
                                                                          0x82, 0x83, 0x42, 0x85, 0x22, 
                                                                          0x49, 0x40, 0x41, 0x84, 0x21, 
                                                                          0x08, 0x18, 0x30, 0x07, 0xc0};
static const unsigned char PROGMEM image_Ble_connected_15x15_bits[] = {0x07, 0xc0, 0x1f, 0xf0, 0x3e,
                                                                       0xf8, 0x7e, 0x7c, 0x76, 0xbc, 
                                                                       0xfa, 0xde, 0xfc, 0xbe, 0xfe, 
                                                                       0x7e, 0xfc, 0xbe, 0xfa, 0xde, 
                                                                       0x76, 0xbc, 0x7e, 0x7c, 0x3e, 
                                                                       0xf8, 0x1f, 0xf0, 0x07, 0xc0};
static const unsigned char PROGMEM image_Wifi_15x15_bits[] = {0x00, 0x00, 0x0f, 0xe0, 0x3f,
                                                              0xf8, 0x7c, 0x7c, 0xe7, 0xce, 
                                                              0xdf, 0xf6, 0x3e, 0xf8, 0x73, 
                                                              0x9c, 0x0f, 0xe0, 0x1f, 0xf0, 
                                                              0x0d, 0x70, 0x03, 0x80, 0x03, 
                                                              0x80, 0x03, 0x80, 0x00, 0x00};
static const unsigned char PROGMEM image_Wifi_7x7_bits[] = {0x00, 0x7c, 0xba, 0x54, 0x38, 0x10, 0x00};
static const unsigned char PROGMEM image_Crossed_Wifi_7x7_bits[] = {0x82, 0x7c, 0xba, 0x54, 0x28, 0x54, 0x82};
static const unsigned char PROGMEM image_Check_7x7_bits[] = {0x06, 0x0e, 0x8e, 0xfc, 0xf8, 0x70, 0x30};
static const unsigned char PROGMEM image_Cross_7x7_bits[] = {0xc6, 0xfe, 0xfe, 0x38, 0xfe, 0xfe, 0xc6};

void oledSetup()
{
    display.begin(SCREEN_ADDRESS, true);
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
}

void oledInitWaitforBT()
{
    display.clearDisplay();
    display.drawBitmap(57, 5, image_Ble_disconnected_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(14, 24);
    display.setTextWrap(false);
    display.print("Waiting for a new");
    display.setCursor(26, 34);
    display.print("BT Connection");
    display.setCursor(26, 45);
    display.print("in 15 Seconds");
    display.display();
}

void oledWaitforBT()
{
    display.clearDisplay();
    display.drawBitmap(57, 5, image_Ble_disconnected_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(26, 25);
    display.setTextWrap(false);
    display.print("Waiting for a");
    display.setCursor(5, 37);
    display.print("Bluetooth Connection");
    display.display();
}

void oledBTConnected()
{
    display.clearDisplay();
    display.drawBitmap(57, 5, image_Ble_connected_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(7, 25);
    display.setTextWrap(false);
    display.print("Bluetooth Connected");
    display.setCursor(46, 37);
    display.print("Insert");
    display.setCursor(13, 49);
    display.print("SSID and Password");
    display.display();
}

void oledConnectingtoWiFi()
{
    display.clearDisplay();
    display.drawBitmap(57, 15, image_Wifi_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(35, 34);
    display.setTextWrap(false);
    display.print("Connecting");
    display.setCursor(35, 45);
    display.print("to WiFi...");
    display.display();
}

void oledWiFiConnected()
{
    display.clearDisplay();
    display.drawBitmap(57, 15, image_Wifi_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(23, 36);
    display.setTextWrap(false);
    display.print("WiFi Connected");
    display.display();
}

void oledWiFiFailed()
{
    display.clearDisplay();
    display.drawBitmap(57, 5, image_Wifi_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(13, 24);
    display.setTextWrap(false);
    display.print("Failed to Connect");
    display.setCursor(43, 35);
    display.print("to WiFi");
    display.display();
}

void oledWiFiFailedInsertNew()
{
    display.clearDisplay();
    display.drawBitmap(57, 5, image_Wifi_15x15_bits, 15, 15, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(13, 24);
    display.setTextWrap(false);
    display.print("Failed to Connect");
    display.setCursor(43, 35);
    display.print("to WiFi");
    display.setCursor(7, 44);
    display.print("Insert new SSID and");
    display.setCursor(39, 54);
    display.print("Password");
    display.display();
}

void oledWiFiLimit()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(15, 19);
    display.setTextWrap(false);
    display.print("Failed to connect");
    display.setCursor(42, 30);
    display.print("to WiFi");
    display.setCursor(4, 41);
    display.print("Max attempts reached");
    display.display();
}

void oledMainDisplay(bool wifiStatus, bool autoPH, bool autoTemp, float waterTemp, float waterPH, float waterNTU)
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(3, 16);
    display.setTextWrap(false);
    display.print("Suhu:");
    display.setCursor(3, 32);
    display.print("PH  :");
    display.setCursor(3, 49);
    display.print("NTU :");
    display.setCursor(65, 15);
    if (waterTemp == -1)
    {
        display.print("Err");
    }
    else
    {
        display.print(waterTemp);
    }
    display.setCursor(65, 32);
    if (waterPH == -1)
    {
        display.print("Err");
    }
    else
    {
        display.print(waterPH);
    }

    display.setCursor(65, 49);
    if (waterNTU == -1)
    {
        display.print("Err");
    }
    else
    {
        display.print(int(waterNTU));
    }
    display.drawLine(0, 12, 127, 12, SH110X_WHITE);

    if (wifiStatus)
    {
        display.drawBitmap(4, 2, image_Wifi_7x7_bits, 7, 7, SH110X_WHITE);
    }
    else
    {
        display.drawBitmap(4, 2, image_Crossed_Wifi_7x7_bits, 7, 7, SH110X_WHITE);
    }

    display.setTextSize(1);
    display.setCursor(16, 2);
    display.print("Control:");
    display.setCursor(64, 2);
    display.print("PH");
    if (autoPH)
    {
        display.drawBitmap(78, 2, image_Check_7x7_bits, 7, 7, SH110X_WHITE);
    }
    else
    {
        display.drawBitmap(78, 2, image_Cross_7x7_bits, 7, 7, SH110X_WHITE);
    }

    display.setCursor(91, 2);
    display.print("Suhu");
    if (autoTemp)
    {
        display.drawBitmap(117, 2, image_Check_7x7_bits, 7, 7, SH110X_WHITE);
    }
    else
    {
        display.drawBitmap(117, 2, image_Cross_7x7_bits, 7, 7, SH110X_WHITE);
    }

    display.display();
}