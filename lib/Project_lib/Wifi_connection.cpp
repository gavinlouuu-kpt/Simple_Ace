#include <WiFi.h>
#include "Wifi_connection.h"
#include "Cloud_storage.h"
#include "Screen.h"

bool isWifi=false;
bool isConnect =false; 

void checkstatus(){
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
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  unsigned long countdown= millis();
  isConnect =false; 
  while(millis()-countdown < 5000){
    if (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    }
    else{
      isConnect = true;  
      Serial.print("Connected with IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("Wifi status:");Serial.println(WiFi.status());
      delay(500);
      break;
    } 
  }
}

void Wifi_disable(){
  WiFi.disconnect(true,true);
  delay(1000);
  isWifi = false;
  isConnect = false;
  Serial.println("Wifi_off");
  Serial.print("Wifi status:");Serial.println(WiFi.status());
}

unsigned long previousMillis =0;
void Wifi_reconnect(){
  if(isWifi == true){
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
    unsigned long countdown= millis();
    isConnect =false; 
    while(millis()-countdown < 5000){
      if (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(300);
      }
      else{
        isConnect = true;  
        Serial.print("Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("Wifi status:");Serial.println(WiFi.status());
        delay(500);
        break;
      } 
    }
  }
}