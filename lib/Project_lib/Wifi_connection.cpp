#include <WiFi.h>
#include "Wifi_connection.h"
#include "Cloud_storage.h"
#include "Screen.h"

bool isWifi=false;
bool isConnect =false; 

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

  // if (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(millis());
  //   Serial.println("Reconnecting to WiFi...");
  //   WiFi.disconnect();
  //   WiFi.reconnect();
  // }
  Wifi_reconnect();
  if(WiFi.status() == WL_CONNECTED){
    isConnect = true;
    Serial.println("Connected");
  }
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Not connected");
    isConnect = false;
  }
}

void Wifi_able(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  isWifi= true;
  draw_Wifi();
  unsigned long countdown= millis();
  while(millis()-countdown < 5000){
    if (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    }
    else{
      isConnect = true;
      Serial.print("Connected with IP: ");
      Serial.println(WiFi.localIP());
      break;
    }
  }
  isConnect = false;
}

void Wifi_disable(){
  WiFi.disconnect();
  isWifi = false;
  isConnect = false;
  Serial.println("Wifi_off");
}

unsigned long previousMillis =0;
void Wifi_reconnect(){
  if(isWifi == true){
    if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >2000)) {
      // Serial.print(millis());
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      delay(2000);
      WiFi.reconnect();
      delay(2000);
      previousMillis = millis();
    }
  }
}