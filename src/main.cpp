#include <Arduino.h>
#include <Simple_ACE.h>
#include <Screen.h>
#include <PID.h>
#include <Cloud_storage.h>
#include <Wifi_connection.h>
#include <Adafruit_ADS1X15.h>
extern Adafruit_ADS1115 ads;

unsigned long millis_previousCheckTime =0;

void setup() {
  Serial.begin(115200);
  tft_setup();
  pinSetup();
  checkSetup();
  firebase_setup();
  // Warmup_Screen();
  update_check_time();
  HomeScreen();
}

unsigned long previous_switch_heater =0;
extern double Output;
int dac_power=255;

void loop() {
  PID_control();
  Navigation();
  Serial.print(ads.readADC_SingleEnded(Sensor_channel));
  Serial.print(",");
  Serial.print(ads.readADC_SingleEnded(Heater_channel));
  Serial.print(",");
  Serial.print(ads.readADC_SingleEnded(Offset_channel));
  Serial.print(",");
  Serial.print(ads.readADC_SingleEnded(NTCC_channel));
  Serial.print(",");
  Serial.println(Output); 
  if(millis()- previous_switch_heater >1){
    previous_switch_heater = millis();
    if(dac_power == 255){
      dac_power = 0;
      dacWrite(sensor_h, dac_power);
      // delay(5);
    }
    else{
      dac_power = 255;
      dacWrite(sensor_h, dac_power);
      // delay(5);
    } 
  }
  
  // if(millis()-millis_previousCheckTime > 10000){
  //   check_sensor_life();
  //   millis_previousCheckTime = millis();
  // }
}

