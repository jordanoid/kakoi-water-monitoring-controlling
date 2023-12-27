#include "FirebaseFunctions.hpp"
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define FIREBASE_ID "skm-kakoi-rtdb"
#define API_KEY "AIzaSyCgq5luz_flyabS8u-am327XvnDxiZN0ew"
#define DATABASE_URL "https://skm-kakoi-rtdb-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

const long utcOffsetInSeconds = 7 * 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

String DEVICE_UID = "1X";
String temp_RTDB_node = DEVICE_UID + "/suhu";
String NTU_RTDB_node = DEVICE_UID + "/NTU";
String ph_RTDB_node = DEVICE_UID + "/ph";
bool FirestoreState = false;


void FirebaseSetup()
{
    timeClient.begin();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;
    }
    else
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void RTDBSend(float waterTemp, float waterNTU)
{
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

        // RTDB
        if (Firebase.RTDB.setFloat(&fbdo, temp_RTDB_node.c_str(), waterTemp))
        {
            Serial.println("PASSED");
            Serial.print("PATH: ");
            Serial.println(fbdo.dataPath());
            Serial.print("TYPE: ");
            Serial.println(fbdo.dataType());
        }
        else
        {
            Serial.println("FAILED");
            Serial.print("REASON: ");
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.RTDB.setFloat(&fbdo, NTU_RTDB_node.c_str(), waterNTU))
        {
            Serial.println("PASSED");
            Serial.print("PATH: ");
            Serial.println(fbdo.dataPath());
            Serial.print("TYPE: ");
            Serial.println(fbdo.dataType());
        }
        else
        {
            Serial.println("FAILED");
            Serial.print("REASON: ");
            Serial.println(fbdo.errorReason());
        }
    }
}

void FirestoreSend(float waterTemp, float waterNTU)
{
    timeClient.update();

    if (Firebase.ready() && signupOK)
    {
        FirebaseJson tempContent;
        FirebaseJson NTUContent;

        String documentPath = "device/" + DEVICE_UID;

        String temp_FS_path = documentPath + "/temp/" + String(timeClient.getEpochTime());
        String NTU_FS_path = documentPath + "/turbid/" + String(timeClient.getEpochTime());
        String ph_FS_path = documentPath + "/ph/" + String(timeClient.getEpochTime());

        String fields = "fields/value/doubleValue";

        tempContent.set(fields, String(waterTemp));
        NTUContent.set(fields, String(waterNTU));

        if (FirestoreState == false && (timeClient.getMinutes() == 30 || timeClient.getMinutes() == 0))
        {
            FirestoreState = true;
            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_ID, "" /* databaseId can be (default) or empty */, temp_FS_path.c_str(), tempContent.raw()))
            {
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_ID, "" /* databaseId can be (default) or empty */, NTU_FS_path.c_str(), NTUContent.raw()))
            {
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }
        }
        else if (timeClient.getMinutes() == 31 || timeClient.getMinutes() == 1)
        {
            FirestoreState = false;
        }
    }
}