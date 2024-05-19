#include <Arduino.h>
//#include <WiFi.h>               //we are using the ESP32
#include <ESP8266WiFi.h>      // uncomment this line if you are using esp8266 and comment the line above
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

const int tempPin = A0;

TinyGPSPlus gps;
SoftwareSerial SerialGPS(4, 5);
float Latitude , Longitude;
String DateString , TimeString , LatitudeString , LongitudeString;


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Kavya Chougule"
#define WIFI_PASSWORD "kavya6170"

// Insert your Firebase project API Key
#define API_KEY "AIzaSyC1SCRxX2sHdkfZ457FALfxwuX-oHHZzoQ" 

// Insert your RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://baby-safety-5abd9-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;


FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;                     //since we are doing an anonymous sign in 

void setup(){

  //Serial.begin(9600);
  Serial.begin(115200);
  SerialGPS.begin(9600);
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

void loop(){

  //temperature and humidity measured should be stored in variables so the user
  //can use it later in the database
  

        int tempReading = analogRead(tempPin);
        float voltage = tempReading * (3.3/ 1023.0);
        float temp = voltage * 100;
        float temperature=(temp*9/5)+45;
        Serial.print("Temperature : ");
        Serial.println(temperature);
        delay(1000);

       if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)){
        //since we want the data to be updated every second
        sendDataPrevMillis = millis();
        // Enter Temperature in to the DHT_11 Table
        if (Firebase.RTDB.setInt(&fbdo, "Temperature", temperature)){
       // This command will be executed even if you dont serial print but we will make sure its working
       }
        else {
        Serial.println("Failed to Read from the LM-35 Sensor");
        Serial.println("REASON: " + fbdo.errorReason());
        }
       }
         while (SerialGPS.available() > 0)
        if (gps.encode(SerialGPS.read()))
       {
        if (gps.location.isValid() && gps.location.age()<2000)
        {
        Latitude = gps.location.lat();
        LatitudeString = String(Latitude , 6);
        Longitude = gps.location.lng();
        LongitudeString = String(Longitude , 6);
        Serial.println(LatitudeString + "," + LongitudeString);
        delay(3000);
       
        if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)){
        //since we want the data to be updated every second
        sendDataPrevMillis = millis();
        if (Firebase.RTDB.setInt(&fbdo, "GPS/Latitude", Latitude)){
        // This command will be executed even if you dont serial print but we will make sure its working
         }
       else {
        Serial.println("Failed to Read from the GPS Sensor");
        Serial.println("REASON: " + fbdo.errorReason());
       }
        if (Firebase.RTDB.setInt(&fbdo, "GPS/Longitude", Longitude)){
        // This command will be executed even if you dont serial print but we will make sure its working
        }
       else {
        Serial.println("Failed to Read from the GPS Sensor");
        Serial.println("REASON: " + fbdo.errorReason());
        }
        }
      }
    }
}