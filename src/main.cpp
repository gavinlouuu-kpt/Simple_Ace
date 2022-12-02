#include <Arduino.h>
// #include <BlynkSimpleEsp32.h>
// BlynkWifi Blynk(_blynkTransport);
#include <Simple_ACE.h>
#include <Screen.h>

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
  draw_framework();
  Serial.println( "Setup done" );
  // use only when data has to write into spiffs //
  // only flush the file when EEPROM is rebooted
  // if (EEPROM.read(EEP_add) == 0) {
  //   printf("Clearing Files");
  //   for (int i = 0; i < 255 ; i ++) {
  //     String filename = format_1 + (String)i + format_2;
  //     SPIFFS.remove(filename);
  //     if (!filename) {
  //       File dat_file_w = SPIFFS.open(filename, FILE_WRITE);
  //       Serial.println("cleared");
  //       if (!dat_file_w) {
  //         Serial.println("There was an error opening the file for writing");
  //         return;
  //       }
  //       dat_file_w.close();
  //     }
  //   }
  // }
}

void loop() {

  TouchScreen();
  // // Blynk.connect();
  // // blynk_upload(avg_ratio_Ace, avg_ratio_O2, rq, map_rq);//editted

  // // data_logging(avg_ratio_CO2, avg_ratio_O2,rq , 0 , 1 );
  // // Serial.print("Fat Burn effeciency: "); Serial.print(rq); Serial.print (" "); Serial.println(map_rq);
  // data_logging(avg_ratio_Ace, avg_ratio_O2, rq , 0, 5 );
}

