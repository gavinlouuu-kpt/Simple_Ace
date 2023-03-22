#include <Arduino.h>
#include <Simple_ACE.h>
#include <Screen.h>
#include <PID.h>
#include <Cloud_storage.h>
#include <Wifi_connection.h>

unsigned long millis_previousCheckTime =0;

void setup() {
  Serial.begin(115200);
  tft_setup();
  pinSetup();
  checkSetup();
  firebase_setup();
  // Warmup_Screen();
  // Serial.println( "Setup done" );
  update_check_time();
  HomeScreen();
}

void loop()
{
  PID_control();
  Navigation();
  // if(millis()-millis_previousCheckTime > 10000){
  //   check_sensor_life();
  //   millis_previousCheckTime = millis();
  // }
}
