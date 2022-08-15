#include <Arduino.h>

#include "SPIFFS.h"
#include <BlynkSimpleEsp32.h>
BlynkWifi Blynk(_blynkTransport);
#include <Simple_COCO.h>

//////////////////////////SPIFFS File//////////////////////////////////////
String format_1 = "/";
String format_2 = ".txt";
String file[255];
//////////////////////////Request time//////////////////////////////////////
unsigned long previous_time;
//////////////////////////functions/////////////////////////////////////////
int data_logging(double value, double value_1, double value_2, double value_3,  int storage);
double mapping(double CO2, double O2);
double concentration_ethanol( double temp, int baseline);
///////////////////////////////////Global Data.//////////////////////////////////////
double avg_ratio_CO2;
double avg_ratio_O2 ;
double rq;
double map_rq;
int file_label;
int temperate = 55;
short CO2_arr[store_size] = {0};
short O2_arr[store_size] = {0};
File dat_file_app;

double mapping(double CO2, double O2) {
  //  int vol_in = 7.5;
  //  int vol_out = 7.5;
  rq = ( CO2 / ( 21 -  O2));
  map_rq = 10 - ((rq - 0.7) * (9) / (1 - 0.7));
  if (map_rq > 10) {
    map_rq = 10;
  }
  if (map_rq < 1) {
    map_rq = 1;
  }
  return(0);
}


double concentration_ethanol( double temp, int baseline) {
  double acetone_start = 6923;
  double acetone_end = 9000;
  double coec  = 55 / temp;
  int peak = 0;
  acetone_start = acetone_start * coec;
  acetone_end = acetone_end * coec;
  printf("%d , %d\n", (int)acetone_start, (int)acetone_end);
  for ( int i = (int)acetone_start - 1 ; i <= (int) acetone_end - 1; i++) {
    printf("%d\n", CO2_arr[i]);
    printf("%d\n", i);
    if ( CO2_arr[i] > peak) {
      peak = CO2_arr[i];
      printf("Replaced");
    }
  }
  double ratio =  peak / baseline;
  printf("%d\n", baseline);
  printf(" Acetone Concentration: %.5f \n", ratio);
  return(0);
}

void setup() {
  Serial.begin(115200);
  tftSetup();
  pinSetup();
  analogSetup();
  checkSetup();
  //only flush the file when EEPROM is rebooted
  if (EEPROM.read(EEP_add) == 0) {
    printf("Clearing Files");
    for (int i = 0; i < 255 ; i ++) {
      String filename = format_1 + (String)i + format_2;
      SPIFFS.remove(filename);
      if (!filename) {
        File dat_file_w = SPIFFS.open(filename, FILE_WRITE);
        Serial.println("cleared");
        if (!dat_file_w) {
          Serial.println("There was an error opening the file for writing");
          return;
        }
        dat_file_w.close();
      }
    }
  }
}

void loop() {

  for (int i = 0; i < 3; i++) {
    int baseline = baselineRead(CO2_channel);
    sample_collection(i);
    concentration_ethanol(temperate, baseline);
    if ( store == true) {
      file_label = EEPROM.read(EEP_add);
      Serial.println(file_label);
      String filename = format_1 + (String)file_label + format_2;
      Serial.println(filename);
      File dat_file_app = SPIFFS.open(filename, FILE_WRITE);
      for (int i = 0; i < plot_size; i++) {
        dat_file_app.print((unsigned long)millis()); dat_file_app.print(" , "); dat_file_app.print(CO2_arr[i]); dat_file_app.print(" , "); dat_file_app.println(O2_arr[i]);
        Serial.println(i);
      }
      dat_file_app.close();
      Serial.println("saved");
      Serial.print(EEP_add); Serial.print("\t"); Serial.println(file_label, DEC);
      file_label = file_label + 1;
      Serial.println(file_label);
      EEPROM.write(EEP_add, file_label);
      EEPROM.commit();
    }
    ;
    delay(4000);
  }


  avg_ratio_CO2 =  sort_reject(ratio_CO2, 3);
  avg_ratio_O2 = sort_reject(ratio_O2, 3);
  mapping(avg_ratio_CO2, avg_ratio_O2);
  Blynk.connect();
  blynk_upload(avg_ratio_CO2, avg_ratio_O2, rq, map_rq);//editted

  Serial.print("Ratio is: "); Serial.println(avg_ratio_CO2, 6); Serial.print("Ratio_O2 is: "); Serial.println(avg_ratio_O2, 6);
  //    data_logging(avg_ratio_CO2, avg_ratio_O2,rq , 0 , 1 );
  Serial.print("Fat Burn effeciency: "); Serial.print(rq); Serial.print (" "); Serial.println(map_rq);
  data_logging(avg_ratio_CO2, avg_ratio_O2, rq , 0, 5 );
  tft.fillScreen(BLACK);
  //  tft.setTextSize(2); tft.setCursor(15, 90); tft.println("Fat Burn");
  //  tft.setTextSize(3); tft.setCursor(35, 110); tft.print(map_rq, 1);
  // comment the following two lines if using 2 inch screen
  tft.setTextSize(4); tft.setCursor(25, 130); tft.println("Fat Burn");
  tft.setTextSize(5); tft.setCursor(55, 180); tft.print(map_rq, 1);
  previous_time = getTime();

  power_saving(previous_time);
}

int data_logging(double value, double value_1, double value_2, double value_3,  int storage) {
  if (!dat_file_app) {
    Serial.println("There was an error opening the file for appending");
  }
  switch (storage) {
    case 1 :
      dat_file_app.println(value);
      break;
    case 2 :
      dat_file_app.print(value); dat_file_app.print(" , "); dat_file_app.println(value_1);
      break;
      //    case 3 :
      //      dat_file_app.println("/////////////////////////////////////////////////////////////////////////");
      //      dat_file_app.print("CO2 Baseline_r"); dat_file_app.print(" , "); dat_file_app.print("CO2 Response_r"); dat_file_app.print(" , "); dat_file_app.print("CO2 ratio"); dat_file_app.print(" , "); dat_file_app.println("Max_grad");
      //      dat_file_app.println(value); dat_file_app.println(value_1);  dat_file_app.println(value_2); dat_file_app.println(value_3);
      //      break;
      //    case 4 :
      //      dat_file_app.println("/////////////////////////////////////////////////////////////////////////");
      //      dat_file_app.print("O2 Baseline_v"); dat_file_app.print(" , "); dat_file_app.print("O2 Response_v"); dat_file_app.print(" , "); dat_file_app.print("O2 ratio"); dat_file_app.print(" , "); dat_file_app.println("Max_grad");
      //      dat_file_app.println(value); dat_file_app.println(value_1);  dat_file_app.println(value_2); dat_file_app.println(value_3);
      //      break;
      //    case 5:
      //      dat_file_app.println("/////////////////////////////////////////////////////////////////////////");
      //      dat_file_app.print("Average CO2 ratio"); dat_file_app.print(" , "); dat_file_app.print("Average O2 ratio"); dat_file_app.print(" , "); dat_file_app.print("Resipiratory quotient");
      //      dat_file_app.println(value); dat_file_app.println(value_1);  dat_file_app.println(value_2);
      //      dat_file_app.println("/////////////////////////////////////////////////////////////////////////");
      //      break;
  }
  return(0);
}

