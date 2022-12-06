#include <WiFi.h>
#include "Wifi_connection.h"
#include "Cloud_storage.h"

bool isWifi=false;

void checkstatus(){
// Serial.print("Connecting to Wi-Fi");
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//   unsigned long countdown= millis();
//   while(millis()-countdown < 5000){
//     if (WiFi.status() != WL_CONNECTED){
//       Serial.print(".");
//       delay(300);
//     }
//     else{
//       isWifi = true;
//       Serial.print("Connected with IP: ");
//       Serial.println(WiFi.localIP());
//       firebase_setup();
//       break;
//     }
//   } 
  if(WiFi.status() == WL_CONNECTED){
    isWifi = true;
    Serial.println("Connected");
  }
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Failed");
    isWifi = false;
  }
}

void Wifi_connect(){
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long countdown= millis();
  while(millis()-countdown < 5000){
    if (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    }
    else{
      isWifi = true;
      Serial.print("Connected with IP: ");
      Serial.println(WiFi.localIP());
      firebase_setup();
      break;
    }
  }
}

void Wifi_disconnect(){
  WiFi.disconnect();
  isWifi = false;
  Serial.println("Disonnected");
}