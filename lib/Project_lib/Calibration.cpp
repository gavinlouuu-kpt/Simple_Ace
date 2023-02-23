#include "Calibration.h"
#include "SPIFFS.h"
#include "Simple_ACE.h"
#include <EEPROM.h>
#include <Adafruit_ADS1X15.h>
#include "TFT_eSPI.h"
#include "Image_assets/Loading.h"
#include "PID.h"
#include "Neighbour_color.h"

extern TFT_eSPI tft;
extern Adafruit_ADS1115 ads;
extern short Sensor_arr[store_size];

int peak_position[10] = {0};
int ref_position[2];

void find_peak();
void store_calibiration_data();
void update_parameters(int unit);
void calibration();

void EEPROM_setup(){
  if(!EEPROM.begin(20)){
      printf("failed to mount EEPROM");
  }
  else{
    printf("EEPROM begin\n");
    }
}

void update_parameters(int unit){
  EEPROM.begin(20);
  printf("EEPROM begin\n");
    int past =0; 
    byte address = 0;
    EEPROM.get(0,past);
    printf("EEPROM write value : %d\n", past);

    ref_position[0] = peak_position[0] * unit;
    ref_position[1] = peak_position[1] * unit;
    printf("updated: %d\n",ref_position[0]);
   
    EEPROM.put(address, ref_position[0]);  
    delay(100);  
    // printf("EEPROM address: %d, value: %d\n", address,position[0]);
    address += sizeof(int);
    EEPROM.put(address, ref_position[1]);  
    delay(100); 
    // printf("EEPROM address: %d, value: %d\n", address, position[1]);

    EEPROM.commit();
    address = 0;
    EEPROM.get(address,past);
    delay(500);
    printf("EEPROM write address: %d, value : %d\n",address, past);
    address += sizeof(int);
    EEPROM.get(address,past);
    delay(500);
    printf("EEPROM write address: %d, value : %d\n",address, past);
    EEPROM.end();
    delay(500);
}

void calibration() {
  PID_control();
  int sampling_index = 0;
  int display_index = 0;
  int countdown_index = 3;
  bool fillscreen = true;
  const int millisdelay =1000;
  long millisPreviousTime = 0;
  long millisPreviousTime_1 =0 ;
  long millisStartTime = millis();
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  for(int i =0; i<store_size; i++){
    Sensor_arr[i]=0;
  }
  while(millis() - millisStartTime < sampletime){
     PID_control();
    if(millis() - millisPreviousTime > millisdelay && countdown_index > 0){
      tft.drawString("Sample in ",110,120,4);
      tft.drawFloat(float(countdown_index),0,180,120,4);
      countdown_index--;
      millisPreviousTime = millis();
    }

    if(millis()-millisPreviousTime > millisdelay && countdown_index == 0 && fillscreen == true){
    tft.fillRect(0,100,240,40,TFT_NEIGHBOUR_GREEN);
    fillscreen = false;
    }

    if(millis() - millisPreviousTime > millisdelay){
      tft.setTextDatum(4);
      tft.drawString("Remain ",100,120,4);
      tft.fillRect(155,100,65,40,TFT_NEIGHBOUR_GREEN);
      tft.drawFloat(float((sampletime-(millis() - millisStartTime))/millisdelay),0,170,120,4);
      millisPreviousTime= millis();
    }

    tft.pushImage(90, 150, LoadingWidth  ,LoadingHeight, Loading[display_index%11]);
    delay(10); display_index++;

    if (millis()-millisPreviousTime_1>10){
      Sensor_arr[sampling_index] = ads.readADC_SingleEnded(0);
      Serial.println(Sensor_arr[sampling_index]);
      sampling_index += 1;
      //printf("Counter 1: %d\n", sampling_index);
      millisPreviousTime_1 = millis();
    }
  }
  
  find_peak();//part to be corrected
  int millisExposedTime = millis()-millisStartTime;
  Serial.print("exposed time:");Serial.println(millisExposedTime);
  int millisUnitTime = millisExposedTime/sampling_index;
  Serial.print("Unit time:");Serial.println(millisUnitTime);
  update_parameters(millisUnitTime);
  store_calibiration_data();
}

void find_peak(){
  int peak_1=0;
  int peak_2=0;
  for(int i = 50; i <250; i++){
    // Serial.println(Sensor_arr[i]);
    if(Sensor_arr[i]>peak_1){
      peak_1=Sensor_arr[i];
      peak_position[0] = i;
      // Serial.println(position[0]);
    }
  }
  Serial.println();
  for(int j = 800; j <1500; j++){ //specified region of sample to look for peaks
    // Serial.println(Sensor_arr[j]);
    if(Sensor_arr[j]> peak_2){
      peak_2=Sensor_arr[j];
      peak_position[1] = j;
      // Serial.println(position[1]);
    }
  }

}

void store_calibiration_data(){
  if(SPIFFS.exists("/Calibration")){
      SPIFFS.remove("/Calibration");
      delay(500);
      printf("removed file: %s\n","/Calibration");
    }
    printf("Storing into %s\n","/Calibration");

    File file = SPIFFS.open("/Calibration",FILE_WRITE);
    file.print(',');file.write('\n'); 
    for(int i =0; i <2000; i++){
      if(Sensor_arr[i] !=0){
        file.print(Sensor_arr[i]);file.print(',');file.write('\n'); 
      }
    }
    file.close();
}