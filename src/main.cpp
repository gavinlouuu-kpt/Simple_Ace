#include <Arduino.h>
#include <Simple_ACE.h>
#include <Screen.h>
#include <PID.h>
#include <Cloud_storage.h>
#include <Wifi_connection.h>

unsigned long millis_previousCheckTime =0;
int setup_address = 18;

void setup() {
  Serial.begin(115200);
  tft_setup();
  pinSetup();
  checkSetup();
  firebase_setup();
  screen_count(setup_address);
  // Warmup_Screen();
  update_check_time();
  HomeScreen();
}

void loop()
{
  PID_control();
  Navigation();
  
}
