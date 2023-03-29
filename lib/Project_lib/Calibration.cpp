#include "Calibration.h"
#include "SPIFFS.h"
#include "Simple_ACE.h"
#include <EEPROM.h>
#include <Adafruit_ADS1X15.h>
#include "TFT_eSPI.h"
#include "Image_assets/Loadingcopy.h"
#include "PID.h"
#include "Neighbour_color.h"
#include "Cloud_storage.h"
#include "Screen.h"

extern TFT_eSPI tft;
extern Adafruit_ADS1115 ads;
extern short Sensor_arr[store_size];
int peak_position[10] = {0};
int ref_position[2];

void Calibration();                     //  record calibrating gas sample
void EEPROM_setup(bool factory);                    //  initialize EEPROM
void find_peak();                       //  locate the local maxima of the gas data
void store_calibiration_data();         //  store calibration gas sample into SPIFFS
void update_parameters(int unit);       //  store gas maxima positions into EEPROM

void EEPROM_setup(bool factory){
  if(factory == false){
    if(!EEPROM.begin(512)){
        printf("failed to mount EEPROM");
    }
    else{
      printf("EEPROM begin\n");
    }
  }
  else{
    extern int index_address;
    extern int lifecount_address;
    extern int sample_address; 
    extern int plot_address; 
    extern int blow_address; 
    extern int setup_address;
    extern bool isCal;
    isCal= true;
    Serial.println("Factory setting");
    EEPROM.begin(512);
    EEPROM.put(index_address, 0);
    delay(100); 
    EEPROM.put(lifecount_address,10);
    delay(100);
    int lifecount;
    EEPROM.get(lifecount_address, lifecount);
    Serial.print("lifecount = "); Serial.println(lifecount);
    delay(100);
    EEPROM.put(sample_address, 0);
    delay(100);
    EEPROM.put(plot_address, 0);
    delay(100);
    EEPROM.put(blow_address, 0);
    delay(100);
    EEPROM.put(setup_address, 0);
    EEPROM.commit();
    delay(500);
    EEPROM.end();
    delay(500);
  }
}

void update_parameters(int unit){
  EEPROM.begin(20);
  printf("EEPROM begin\n");
    int past = 0; 
    byte address = 0;
    EEPROM.get(0,past);
    printf("EEPROM write value : %d\n", past);

    ref_position[0] = peak_position[0] * unit;
    ref_position[1] = peak_position[1] * unit;
    printf("updated: %d\n",ref_position[0]);
   
    EEPROM.put(address, ref_position[0]);  
    delay(100);  
    printf("EEPROM address: %d, value: %d\n", address, ref_position[0]);
    address += sizeof(int);
    EEPROM.put(address, ref_position[1]);  
    delay(100); 
    printf("EEPROM address: %d, value: %d\n", address,  ref_position[1]);

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

void Calibration() {
  PID_control();
  int sampling_index = 0;
  int display_index = 0;
  int countdown_index = 3;
  bool fillscreen = true;
  float millisdelay =1000;
  long millisPreviousTime = 0;
  long millisPreviousTime_1 =0 ;
  long millisPreviousTime_gif =0 ;
  long millisStartTime = millis();
  float print_time=0;
  tft.setTextColor(TFT_TextBrown, TFT_NEIGHBOUR_BEIGE);
  for(int i =0; i<store_size; i++){
    Sensor_arr[i]=0;
  }
  while(millis() - millisStartTime < sampletime+1){
    PID_control();
    unsigned long present_millis =millis();
    if(present_millis - millisPreviousTime > millisdelay && countdown_index > 0){
      tft.drawString("Sample in ",110,120,4);
      tft.drawFloat(float(countdown_index),0,180,120,4);
      countdown_index--;
      millisPreviousTime = millis();
    }

    if(present_millis-millisPreviousTime > millisdelay && countdown_index == 0 && fillscreen == true){
      tft.fillRect(0,100,240,40,TFT_NEIGHBOUR_BEIGE);
      fillscreen = false;
    }

    if(present_millis - millisPreviousTime > millisdelay){
      tft.setTextDatum(CC_DATUM);
      tft.drawString("Remain ",100,120,4);
      tft.fillRect(155,100,65,40,TFT_NEIGHBOUR_BEIGE);
      if (sampletime - (present_millis- millisStartTime)<0)
      {
        print_time = 0;
      }else{
        print_time = float((sampletime-(present_millis - millisStartTime))/millisdelay);
      }
      tft.drawNumber((int)print_time,170,120,4);
      millisPreviousTime= millis();
    }

    if(present_millis- millisPreviousTime_gif > 100){
      display_loading(display_index);display_index++;
      millisPreviousTime_gif = millis();
    }

    if (present_millis-millisPreviousTime_1>10){
      Sensor_arr[sampling_index] = ads.readADC_SingleEnded(Sensor_channel);
      Serial.println(Sensor_arr[sampling_index]);
      sampling_index += 1;
      millisPreviousTime_1 = millis();
    }
  }
  
  find_peak();//part to be corrected
  for(int i = 0; i < store_size; i++){
    Serial.println(Sensor_arr[i]);
  }
  int millisExposedTime = millis()-millisStartTime;
  Serial.print("exposed time:");Serial.println(millisExposedTime);
  int millisUnitTime = millisExposedTime/sampling_index;
  Serial.print("Unit time:");Serial.println(millisUnitTime);
  //print the peak position convert to time
  Serial.print("Peak position:");Serial.println(peak_position[1]*millisUnitTime);
  //print sensor array
  update_parameters(millisUnitTime);
  // store_calibiration_data();
}

void find_peak(){
  int peak_1=0;
  int peak_2=0;
  for(int i = 20; i <100; i++){
    // Serial.println(Sensor_arr[i]);
    if(Sensor_arr[i]>peak_1){
      peak_1=Sensor_arr[i];
      peak_position[0] = i;
      // Serial.println(position[0]);
    }
  }
  Serial.println();
  for(int j = 400; j < sizeof(Sensor_arr)/2; j++){ //specified region of sample to look for peaks
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
    for(int i =0; i <store_size; i++){
      if(Sensor_arr[i] !=0){
        file.print(Sensor_arr[i]);file.print(',');file.write('\n'); 
      }
    }
    file.close();
}