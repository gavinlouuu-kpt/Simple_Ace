#include <WiFi.h>
#include "Wifi_connection.h"

bool isWifi=false;
void checkstatus(){
Serial.print("Connecting to Wi-Fi");
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
    }
  } 
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Failed");
    isWifi = false;
  }
}