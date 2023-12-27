#ifndef FIREBASE_FUNCTIONS_HPP
#define FIREBASE_FUNCTIONS_HPP

void FirebaseSetup();
void RTDBSend(float waterTemp, float waterNTU);
void FirestoreSend(float waterTemp, float waterNTU);

#endif