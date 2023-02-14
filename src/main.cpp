#include <Arduino.h>
// #include <BlynkSimpleEsp32.h>
// BlynkWifi Blynk(_blynkTransport);
#include <Simple_ACE.h>
#include <Screen.h>
#include <PID.h>
#include <Cloud_storage.h>
#include <Wifi_connection.h>

// ////////////////////////SPIFFS File//////////////////////////////////////
// String format_1 = "/";
// String format_2 = ".txt";
// String file[255];
//////////////////////////Request time//////////////////////////////////////
unsigned long previous_time;
//////////////////////////functions/////////////////////////////////////////
int data_logging(double value, double value_1, double value_2, double value_3, int storage);
double mapping(double CO2, double O2);
///////////////////////////////////Global Data.//////////////////////////////////////
double avg_ratio_Ace;
double avg_ratio_O2;
double rq;
double map_rq;
int file_label;

void setup()
{
  Serial.begin(115200);
  tft_setup();
  pinSetup();
  analogSetup();
  checkSetup();
  // warm_up();
  firebase_setup();
  Serial.println("Setup done");
  update_check_time();
  HomeScreen();
}
unsigned long previous_checktime = 0;

void loop()
{
  PID_control();
  TouchScreen();
  if (millis() - previous_checktime > 10000)
  {
    check_sensor_life();
    previous_checktime = millis();
  }

  // if(isWifi == true){
  //   if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >2000)) {
  //     // Serial.print(millis());
  //     Serial.println("Reconnecting to WiFi...");
  //     WiFi.disconnect();
  //     delay(2000);
  //     WiFi.reconnect();
  //     delay(2000);
  //     previousMillis = millis();
  //   }
  // }
}
