#include "FirebaseFunctions.hpp"
#include "deviceUID.hpp"
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define FIREBASE_ID "skm-kakoi-rtdb"
#define API_KEY "AIzaSyCgq5luz_flyabS8u-am327XvnDxiZN0ew"
#define DATABASE_URL "https://skm-kakoi-rtdb-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

#define USER_EMAIL "skmkakoi@test.com"
#define USER_PASS "skmkakoi"

const long utcOffsetInSeconds = 7 * 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String DEVICE_UID = DEVICE_IDENTIFIER;
String temp_RTDB_node = DEVICE_UID + "/suhu";
String NTU_RTDB_node = DEVICE_UID + "/NTU";
String PH_RTDB_node = DEVICE_UID + "/ph";

String tempControl_RTDB_node = DEVICE_UID + "/auto_suhu";
String minTemp_RTDB_node = DEVICE_UID + "/suhu_min";
String maxTemp_RTDB_node = DEVICE_UID + "/suhu_max";

String PHControl_RTDB_node = DEVICE_UID + "/auto_PH";
String minPH_RTDB_node = DEVICE_UID + "/PH_min";
String maxPH_RTDB_node = DEVICE_UID + "/PH_max";

String timestamp_RTDB_node = DEVICE_UID + "/last_updated";

FirebaseJson tempContent;
FirebaseJson PHContent;
FirebaseJson NTUContent;

String documentPath = "device/" + DEVICE_UID + "/log";

String temp_FS_path = documentPath + "/temp";
String NTU_FS_path = documentPath + "/turbid";
String PH_FS_path = documentPath + "/ph";
String fields;

bool FirestoreState = false;

void FirebaseSetup()
{
    timeClient.begin();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASS;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

/*

RTDB Send Function

*/

void RTDBSend(float waterTemp, float waterNTU, float waterPH)
{
    timeClient.update();
    String timestamp = String(timeClient.getEpochTime());

    if (Firebase.ready())
    {
        // RTDB
        Firebase.RTDB.setFloat(&fbdo, temp_RTDB_node.c_str(), waterTemp);
        Firebase.RTDB.setFloat(&fbdo, NTU_RTDB_node.c_str(), waterNTU);
        Firebase.RTDB.setFloat(&fbdo, PH_RTDB_node.c_str(), waterPH);
        Firebase.RTDB.setString(&fbdo, timestamp_RTDB_node.c_str(), timestamp);
    }
}

/*

RTDB Get Temp Function

*/

float getMaxTemp()
{
    if (Firebase.ready())
    {
        if (Firebase.RTDB.getFloat(&fbdo, maxTemp_RTDB_node.c_str()))
        {
            if (fbdo.dataType() == "float")
            {
                return fbdo.floatData();
            }
            else
            {
                return float(fbdo.floatData());
            }
        }
        else
        {
            return -1;
        }
    }

    return -1;
}

float getMinTemp()
{
    if (Firebase.ready())
    {
        if (Firebase.RTDB.getFloat(&fbdo, minTemp_RTDB_node.c_str()))
        {
            if (fbdo.dataType() == "float")
            {
                return fbdo.floatData();
            }
            else
            {
                return float(fbdo.floatData());
            }
        }
        else
        {
            return -1;
        }
    }

    return -1;
}

bool getAutoTemp()
{
    if (Firebase.ready())
    {
        if (Firebase.RTDB.getBool(&fbdo, tempControl_RTDB_node.c_str()))
        {
            if (fbdo.dataType() == "bool")
            {
                return fbdo.boolData();
            }
            {
                return bool(fbdo.boolData());
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

/*

RTDB Get PH Function

*/

float getMaxPH()
{
    if (Firebase.ready())
    {
        if (Firebase.RTDB.getFloat(&fbdo, maxPH_RTDB_node.c_str()))
        {
            if (fbdo.dataType() == "float")
            {
                return fbdo.floatData();
            }
            else
            {
                return float(fbdo.floatData());
            }
        }
        else
        {
            return -1;
        }
    }

    return -1;
}

float getMinPH()
{
    if (Firebase.ready())
    {
        if (Firebase.RTDB.getFloat(&fbdo, minPH_RTDB_node.c_str()))
        {
            if (fbdo.dataType() == "float")
            {
                return fbdo.floatData();
            }
            else
            {
                return float(fbdo.floatData());
            }
        }
        else
        {
            return -1;
        }
    }

    return -1;
}

bool getAutoPH()
{
    if (Firebase.ready())
    {
        if (Firebase.RTDB.getBool(&fbdo, PHControl_RTDB_node.c_str()))
        {
            if (fbdo.dataType() == "bool")
            {
                return fbdo.boolData();
            }
            else
            {
                return bool(fbdo.boolData());
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

/*

Firestore Function

*/

void FirestoreSend(float waterTemp, float waterNTU, float waterPH)
{
    timeClient.update();

    String epoch = "E" + String(timeClient.getEpochTime());
    fields = "fields/" + epoch + "/doubleValue";

    if (Firebase.ready())
    {

        tempContent.clear();
        PHContent.clear();
        NTUContent.clear();
        if (FirestoreState == false && (timeClient.getMinutes() % 5 == 0))
        {
            if (waterTemp != -1)
            {
                if (!Firebase.Firestore.getDocument(&fbdo, FIREBASE_ID, "", temp_FS_path.c_str()))
                {
                    tempContent.set(fields.c_str(), String(waterTemp));
                    Firebase.Firestore.createDocument(&fbdo, FIREBASE_ID, "", temp_FS_path.c_str(), tempContent.raw());
                }
                else
                {
                    tempContent.set(fields, String(waterTemp));
                    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_ID, "", temp_FS_path.c_str(), tempContent.raw(), epoch);
                }
            }
            if (waterNTU != -1)
            {
                if (!Firebase.Firestore.getDocument(&fbdo, FIREBASE_ID, "", NTU_FS_path.c_str()))
                {
                    NTUContent.set(fields.c_str(), String(waterNTU));
                    Firebase.Firestore.createDocument(&fbdo, FIREBASE_ID, "", NTU_FS_path.c_str(), NTUContent.raw());
                }
                else
                {
                    NTUContent.set(fields, String(waterNTU));
                    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_ID, "", NTU_FS_path.c_str(), NTUContent.raw(), epoch);
                }
            }

            if (waterPH != -1)
            {
                if (!Firebase.Firestore.getDocument(&fbdo, FIREBASE_ID, "", PH_FS_path.c_str()))
                {

                    PHContent.set(fields.c_str(), String(waterPH));
                    Firebase.Firestore.createDocument(&fbdo, FIREBASE_ID, "", PH_FS_path.c_str(), PHContent.raw());
                }
                else
                {
                    PHContent.set(fields, String(waterPH));
                    Firebase.Firestore.patchDocument(&fbdo, FIREBASE_ID, "", PH_FS_path.c_str(), PHContent.raw(), epoch);
                }
            }

            FirestoreState = true;
        }
        else if (timeClient.getMinutes() % 5 != 0)
        {
            FirestoreState = false;
        }
    }
}