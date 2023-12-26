#include <Arduino.h>

#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define PHUP 17
#define PHDOWN 23
#define PELT 18
#define HEATER 19
#define TEMP_SENSOR 27
#define PH_SENSOR 36
#define TURBIDITY 35

#define WIFI_SSID "JORDANO 8600"
#define WIFI_PASSWORD "12345678"

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

int relay[4] = {17, 23, 18, 19};
float waterTemp;
float waterNTU;
float waterPH;
bool FirestoreState = false;

OneWire oneWire(TEMP_SENSOR);
DallasTemperature ds18b20(&oneWire);

void relaySetup();
float tempRead();
float turbidityRead();

void setup()
{
  Serial.begin(115200);
  relaySetup();
  pinMode(TURBIDITY, INPUT);
  ds18b20.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
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

void loop()
{

  timeClient.update();
  // Serial.println(timeClient.getFormattedTime());
  // Serial.println(timeClient.getEpochTime());

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    waterTemp = tempRead();
    waterNTU = turbidityRead();
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

    // Firestore
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
    else if(timeClient.getMinutes() == 31 || timeClient.getMinutes() == 1)
    {
      FirestoreState = false;
    }
  }
}

float tempRead()
{
  Serial.print("Requesting temperatures...");
  ds18b20.requestTemperatures();
  Serial.println("DONE");
  float tempC = ds18b20.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
    return tempC;
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
    return -1;
  }
}

float turbidityRead()
{
  float ADC = analogRead(TURBIDITY);
  float voltage = ADC * 3.3 / (4096);
  float NTU = (-2572.2 * voltage * voltage) + (8700.5 * voltage) - 4352.9;
  Serial.print("ADC is: ");
  Serial.println(ADC);
  Serial.print("Voltage is: ");
  Serial.println(voltage);
  if (NTU < 0)
  {
    NTU = -1;
  }
  return NTU;
}

void relaySetup()
{
  for (int i = 0; i < 4; i++)
    pinMode(relay[i], OUTPUT);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(relay[i], HIGH);
    delay(100);
  }
  delay(500);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(relay[i], LOW);
    delay(100);
  }
};