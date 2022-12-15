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
int data_logging(double value, double value_1, double value_2, double value_3,  int storage);
double mapping(double CO2, double O2);
///////////////////////////////////Global Data.//////////////////////////////////////
double avg_ratio_Ace;
double avg_ratio_O2 ;
double rq;
double map_rq;
int file_label;


void setup() {
  Serial.begin(115200);
  tft_setup();
  pinSetup();
  analogSetup();
  checkSetup();
  warm_up();
  firebase_setup();
  Serial.println( "Setup done" );
}

void loop() {
  PID_control();
  TouchScreen();
  // // Blynk.connect();
  // // blynk_upload(avg_ratio_Ace, avg_ratio_O2, rq, map_rq);//editted

  // // data_logging(avg_ratio_CO2, avg_ratio_O2,rq , 0 , 1 );
  // // Serial.print("Fat Burn effeciency: "); Serial.print(rq); Serial.print (" "); Serial.println(map_rq);
  // data_logging(avg_ratio_Ace, avg_ratio_O2, rq , 0, 5 );
// if WiFi is down, try reconnecting
// if(isWifi ==true){
//   if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >2000)) {
//     // Serial.print(millis());
//     Serial.println("Reconnecting to WiFi...");
//     WiFi.disconnect();
//     delay(100);
//     WiFi.reconnect();
//     delay(100);
//     previousMillis = currentMillis;
//     }
//   }
// Wifi_reconnect();
}

