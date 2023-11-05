#include <Arduino.h>

#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Firebase_ESP_Client.h>

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

#define WIFI_SSID "Wijaya 1A"
#define WIFI_PASSWORD "wijayahebat"

#define API_KEY "AIzaSyCgq5luz_flyabS8u-am327XvnDxiZN0ew"
#define DATABASE_URL "https://skm-kakoi-rtdb-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app


// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;


int relay[4] = {17, 23, 18, 19};
float waterTemp;
float waterNTU;

OneWire oneWire(TEMP_SENSOR);
DallasTemperature ds18b20(&oneWire);

void relaySetup();
float tempRead();
float turbidityRead();

void setup()
{
  Serial.begin(115200);
  // put your setup code here, to run once:
  relaySetup();
  pinMode(TURBIDITY, INPUT);
  ds18b20.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  // put yo ur main code here, to run repeatedly:

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    waterTemp = tempRead();
    waterNTU = turbidityRead();
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setFloat(&fbdo, "/paramreal/suhu", waterTemp)){
      Serial.println("PASSED");
      Serial.print("PATH: "); Serial.println(fbdo.dataPath());
      Serial.println("TYPE: "); Serial.println(fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.print("REASON: "); Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/paramreal/kekeruhan", waterNTU)){
      Serial.println("PASSED");
      Serial.print("PATH: "); Serial.println(fbdo.dataPath());
      Serial.println("TYPE: "); Serial.println(fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.print("REASON: "); Serial.println(fbdo.errorReason());
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
  float voltage = analogRead(TURBIDITY) * 3.3 / (4096);
  float NTU = (-2572.2 * voltage * voltage) + (8700.5 * voltage) - 4352.9;
  // Serial.print("Water NTU is: ");
  // Serial.println(NTU);
  Serial.print("Voltage is: ");
  Serial.println(voltage);
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
  // delay(500);
  // for (int i = 3; i >= 0; i--)
  // {
  //   digitalWrite(relay[i], HIGH);
  //   delay(50);
  //   digitalWrite(relay[i], LOW);
  // }
  // delay(200);
  // for (int i = 0; i < 4; i++)
  // {
  //   digitalWrite(relay[i], HIGH);
  //   delay(50);
  //   digitalWrite(relay[i], LOW);
  // }
  // delay(1000);
  // for (int i = 0; i < 4; i++)
  // {
  //   digitalWrite(relay[i], HIGH);
  // }
  // delay(500);
  // for (int i = 0; i < 4; i++)
  // {
  //   digitalWrite(relay[i], LOW);
  // }
  // delay(500);
  // for (int i = 0; i < 4; i++)
  // {
  //   digitalWrite(relay[i], HIGH);
  // }
  // delay(500);
  // for (int i = 0; i < 4; i++)
  // {
  //   digitalWrite(relay[i], LOW);
  // }
};