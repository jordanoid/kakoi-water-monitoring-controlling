#ifndef OLED_HPP
#define OLED_HPP

void oledSetup();
void oledInitWaitforBT();
void oledWaitforBT();
void oledBTConnected();
void oledConnectingtoWiFi();
void oledWiFiConnected();
void oledWiFiFailed();
void oledWiFiFailedInsertNew();
void oledWiFiLimit();
void oledMainDisplay(bool wifiStatus, bool autoPH, bool autoTemp, float waterTemp, float waterPH, float waterNTU);

#endif