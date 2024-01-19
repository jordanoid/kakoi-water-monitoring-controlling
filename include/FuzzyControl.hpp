#ifndef FUZZY_CONTROL_HPP
#define FUZZY_CONTROL_HPP

void controllerSetup();
void fuzzyControl(float currentTemp, float currentPH, float minTemp, float maxTemp, bool autoTemp, float minPH, float maxPH, bool autoPH);

#endif