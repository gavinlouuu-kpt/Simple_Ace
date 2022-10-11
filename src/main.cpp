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
  // if(click a button){
  //   enter calibration mode
  // }  
  // sample_collection();

  // output_result();

  // Use only when SPIFFS is enabel here//
  // if ( store == true) {
  //   file_label = EEPROM.read(EEP_add);
  //   Serial.println(file_label);
  //   String filename =  format_1 + (String)file_label + format_2;
  //   Serial.println(filename);
  //   // File dat_file_app = SPIFFS.open(filename, FILE_WRITE);
  //   // for (int i = 0; i < plot_size; i++) {
  //   //   dat_file_app.print((unsigned long)millis()); dat_file_app.print(" , "); dat_file_app.print(CO2_arr[i]);
  //   //   Serial.println(i);
  //   // }
  //   // dat_file_app.close();
  //   Serial.println("saved");
  //   Serial.print(EEP_add); Serial.print("\t"); Serial.println(file_label, DEC);
  //   file_label = file_label + 1;
  //   Serial.println(file_label);
  //   EEPROM.write(EEP_add, file_label);
  //   EEPROM.commit();
  // }

  // delay(1000);
  // previous_time = millis();
  // power_saving(previous_time);
  // // mapping(avg_ratio_CO2, avg_ratio_O2);

  // // Blynk.connect();
  // // blynk_upload(avg_ratio_Ace, avg_ratio_O2, rq, map_rq);//editted

  // // data_logging(avg_ratio_CO2, avg_ratio_O2,rq , 0 , 1 );
  // // Serial.print("Fat Burn effeciency: "); Serial.print(rq); Serial.print (" "); Serial.println(map_rq);
  // data_logging(avg_ratio_Ace, avg_ratio_O2, rq , 0, 5 );
}

// int data_logging(double value, double value_1, double value_2, double value_3,  int storage) {
//   if (!dat_file_app) {
//     Serial.println("There was an error opening the file for appending");
//   }
//   switch (storage) {
//     case 1 :
//       dat_file_app.println(value);
//       break;
//     case 2 :
//       dat_file_app.print(value); dat_file_app.print(" , "); dat_file_app.println(value_1);
//       break;
//   }
//   return(0);
// }
