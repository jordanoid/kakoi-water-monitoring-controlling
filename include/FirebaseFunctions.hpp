#ifndef FIREBASE_FUNCTIONS_HPP
#define FIREBASE_FUNCTIONS_HPP

void FirebaseSetup();
void RTDBSend(float waterTemp, float waterNTU);
void FirestoreSend(float waterTemp, float waterNTU);

float getMinTemp();
float getMaxTemp();
bool getAutoTemp();

float getMinPH();
float getMaxPH();
bool getAutoPH();

#endif