#include "Simple_ACE.h"
#include "Screen.h"
#include "Calibration.h"
#include "SPIFFS.h"
#include "Cloud_storage.h"
#include "History_Data.h"
#include "Image_assets/Beagle.h"
#include "Neighbour_color.h"
#include "SHTSensor.h"

#include <PID.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>
#include <math.h>
#include <TFT_eSPI.h>

double baselineRead(int channel); //average out baseline candidate
int restore_baseline();           //define sensor baseline for new set of gas data        
void breath_check();              //  check if sensor value exceed threshold
void checkSetup(void);            //  initialize I2C protocol,EEPROM and SPIFFS memory, PID control
void output_result();             //  return sensor response in ratio
void pinSetup(void);              //  define pin cofig for pump, sensor, and sensor heater
void pump_control(bool control);  //  functions conrol high and low of the pump
void sample_collection();         //  integrate function to analysis one gas sample
void storing_data();              //  background storage of gas data into Firebase/ local SPIFFS

extern TFT_eSPI tft; 
Adafruit_ADS1115 ads;
SHTSensor sht(SHTSensor::SHT4X);

short Sensor_arr[store_size]={0};
short temporal_baseline = 0;
bool isStore = false;

int dutyCycle_pump = 120;         
int baseline = 0;
int fail_count = 0;
int millisUnitTime = 0;

void pinSetup(){
  pinMode(pumpPin_1,OUTPUT);
  pinMode(pumpPin_2,OUTPUT);
  pinMode(colPin_1,OUTPUT);
  pinMode(colPin_2,OUTPUT);
  pinMode(sensor_h,OUTPUT);
  pinMode(battery_EN, OUTPUT);
  pinMode(btn_rst, INPUT);
  pinMode(battery_read,INPUT);

  digitalWrite(battery_EN,1);           //  enable battery monitor
  dacWrite(sensor_h,255);             //  enable senosr heater
  ledcSetup(colChannel_1, 5000, 8);
  ledcSetup(colChannel_2, 5000, 8);
  ledcSetup(pumpChannel_1, freq, resolution);
  ledcSetup(pumpChannel_2, freq, resolution);
  ledcAttachPin(pumpPin_1,pumpChannel_1);
  ledcAttachPin(pumpPin_2,pumpChannel_2);
  ledcAttachPin(colPin_1,colChannel_1);
  ledcAttachPin(colPin_2,colChannel_2);
}

void checkSetup(){
  if (!Wire.begin(21,22)) {
  Serial.println("Failed to initialize wire library");
  while (1);
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  EEPROM_setup(false);
  if (sht.init()) {
      Serial.print("init(): success\n");
  } else {
      Serial.print("init(): failed\n");
  }
  sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
  ads.setGain(GAIN_ONE); 
  if (!ads.begin(0x48)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  PID_setup();
  Serial.println("Setup Complete."); 
}

void pump_control(bool control){
  if(control == true){
  dacWrite(pumpChannel_1,150);
  delay(200);
  dacWrite(pumpChannel_1,100);
  delay(200);
  ledcWrite(pumpChannel_1, dutyCycle_pump);
  }
  else{
    ledcWrite(pumpChannel_1, 0);
    delay(100);
  }
}

// void restore_humidity(){
//   while(1){
//     float previous = sht20.humidity();
//     Serial.println(sht20.humidity());
//     if (sht20.humidity() - previous  < 2) {
//       printf("Humiditty Restored\n");
//       dacWrite(pumpChannel, dutyCycle_pump);
//       delay(5);
//       break;
//     }    
//   }
// }

// double read_humidity(){ 
//   float value;
//   value = sht20.humidity();
//   return value;
// }
void breath_check(){
  long previoustime = millis();
  while (true) {
    PID_control();
    // float arr[3];
    // float humd;
    // double gradient;
    // long previous; 
   
    // for (int i = 0; i < 3; i++) {
    //   arr[i] = sht20.humidity();
    //   previous= millis();
    // }
    if(millis() - previoustime > 1000){
      // if(buffer == 0){
      //   buffer = ads.readADC_SingleEnded(0);
      //   Serial.print("update");Serial.println(buffer);
      // }
      if(abs(ads.readADC_SingleEnded(Sensor_channel) - temporal_baseline) > 200){
        Serial.println("Start recording");
        break;
      }
      temporal_baseline = ads.readADC_SingleEnded(Sensor_channel);
      previoustime =millis();
    }
    draw_sensor(ads.readADC_SingleEnded(Sensor_channel));
    // if (gradient > 1) {
    //   printf("breath real...");
    //   break;
    // }
  }
}

double baselineRead(int channel) {
  int toSort[baseline_window];
  float mean = 0;
  for (int i = 0; i < baseline_window; ++i ) {
    toSort[i] = ads.readADC_SingleEnded(channel);
    delay(5);
  }
  for (int i = 0; i < baseline_window; ++i) {
    mean += toSort[i];
  }
  mean /= baseline_window;
  return mean;
}

int restore_baseline(){
  extern double PID_Setpoint;
  double temporal_read = 0;
  double reference_read = 0;
  Serial.print("Duty Cycle");Serial.println(dutyCycle_pump);
  pump_control(true);
  int loading_index=0 ;
  unsigned long millisCleanStart = millis();
  while(millis()-millisCleanStart < 10000){
    PID_control();
    display_loading(loading_index);loading_index ++;
  }
  // while(abs(ads.readADC_SingleEnded(NTCC_channel)-(int)PID_Setpoint) > 10){
  //   PID_control();
  //   Serial.println(ads.readADC_SingleEnded(NTCC_channel));
  //   draw_loading(counter);counter ++;
  // } 

  double slope = 0 ;
  double flat_slope[5]= {0};
  int flat_count = 0;
  while (1) {
    PID_control();
    display_loading(loading_index);loading_index ++;
    temporal_read = baselineRead(Sensor_channel);
    Serial.print("Temp value:");Serial.println(temporal_read);delay(10);
    reference_read = baselineRead(Sensor_channel);
    Serial.print("Ref value:");Serial.println(reference_read);
    slope = (temporal_read - reference_read)/0.5; // timelapse of two value retrieved
    Serial.print("Slope:");Serial.println(slope);
    if (abs(slope) < 8) { //wait baseline drop flat, change according to the strigency of your baseline
      flat_slope[flat_count] = slope;
      flat_count ++; 
      if(flat_count > 4){
        temporal_baseline = reference_read; //update sensor_baseline
        Serial.print("Found Baseline:");Serial.println(temporal_read);
        Serial.println("Start forecasting...");
        return (int)reference_read;
      }
    }
    else{
      for(int i=0; i<5; i++){
        flat_slope[i]= 0;
      }
      flat_count = 0;
    }
  }
}

// void power_saving(unsigned long last_time){
//   while(1){
//     delay(5);
//     if (digitalRead(btn_rst) == HIGH) {
//       Serial.println("New loop");
//       dacWrite(pumpChannel, dutyCycle_pump);
//       break;
//     }
//     if (millis() - last_time > wait_time) {
//       dacWrite(pumpChannel, 0);
//     }
//   }
// }

void sample_collection(){
  int a = 0;
  float bar_time;
  float bar_percentage;
  int data_size = 0;
  short adc_CO2;
  // restore_humidity();
  baseline = restore_baseline();   //reduncding value
  for(int i =0; i<store_size; i++){
    Sensor_arr[i]=0;
  }
  tft.fillRect(0,30,240,60,TFT_NEIGHBOUR_BEIGE);
  tft.fillRect(90, 200, 70, 70, TFT_NEIGHBOUR_BEIGE );  //cover loading
  tft.setTextColor(TFT_TextBrown, TFT_NEIGHBOUR_BEIGE);
  tft.setTextDatum(CC_DATUM);
  tft.drawString("Huff for 3 seconds", 120, 245, 4);
  breath_check();
  isStore = true;
  int previousDrawLoad = 0;
  write_analyzing();
  long millisStartSample = millis();
  while (millis() - millisStartSample <= sampletime + 1) {
    int time =0 ;
    bar_time = millis() - (float)millisStartSample;
    bar_percentage = (bar_time/45000)*100;
    draw_sample_progress(bar_time,bar_percentage);

    if (millis()-previousDrawLoad >10){ 
      Sensor_arr[data_size]= ads.readADC_SingleEnded(Sensor_channel);
      draw_sensor(Sensor_arr[data_size]); 
      data_size ++;
      previousDrawLoad = millis();      
    }
    PID_control();
  
    // if (isStore == false) {
    //   fail_count += 1 ;
    //   if (fail_count== 50){
    //    break;
    //   }
    //   if (read_humidity() > 40) {
    //     isStore = true;
    //     fail_count= 0;
    //     // Serial.println("Certain a breathe. Recording...");
    //   }
    // }
  }
  if(fail_count==50){
    return;
  }
  Serial.print("Number of sample:");Serial.println(data_size);
  int expose = millis() - millisStartSample;
  Serial.print("Exposed time");Serial.println(expose);
  millisUnitTime= expose/data_size;
}

int find_peak_value(int address, int unittime) {
  int peak_value = 0;
  int peak_position = 0;
  int peak_time = 0;
  EEPROM.begin(20);
  // int check_peak_start = EEPROM.get(address,peak_position)-200;
  peak_position =  EEPROM.get(address, peak_time)/unittime;
  int check_peak_start = peak_position - 200;
  delay(100); 
  if(check_peak_start < 0){check_peak_start = 0;}
  // int check_peak_end =  EEPROM.get(address,peak_position) + 200;
  int check_peak_end = peak_position + 200;
  delay(100);
  EEPROM.end();
  printf("check_peak_start: %d , check_peak_end: %d\n", (int)check_peak_start, (int)check_peak_end);
  for (int i = check_peak_start ; i < check_peak_end; i++){

    if (Sensor_arr[i] > peak_value) {
      peak_value = Sensor_arr[i];
      printf("Replaced %d\n", i);
    }
  }
  Serial.print("Peak value is");Serial.println(peak_value);
  return (peak_value);
}

// double ratio_calibration(double base_resist, double peak_resist, int formula){
//   double concentration;
//   double buffer;
//   buffer =  peak_resist / base_resist;
//   // buffer =   base_resist/peak_resist ;
//   switch (formula) { 
//     case (1):
//       { // CO2 concentration
//         // const float ref_baseline_resist= 4700.3;// assignn value
//         // float ratio_baseline = base_resist/ref_baseline_resist;
//         // // float correct_factor = 1.8764 * ratio_baseline * ratio_baseline * ratio_baseline - 3.9471 * ratio_baseline * ratio_baseline + 2.3844 * ratio_baseline + 0.6869;
//         // float correct_factor = -0.208 * log(ratio_baseline) + 0.9922;
//         // printf(" baseline ratio: %.6f\n", ratio_baseline);
//         // printf("buffer: %.6f\n",buffer);
        
//         // buffer = buffer*correct_factor;
        
//         // // float coeff_1 = 0.596;
//         // // float coeff_2 = -1.0194;
//         // // float coeff_3 = 0.4467;
//         // concentration = 1.9433 * exp(-6.143* buffer) ;
//         // // return concentration;
//         return buffer;
//       }
//     case (2): // acetone _concentration
//       {
//         // float slope = 1;
//         // float constant = 0;
//         // concentration = (buffer - constant) / slope;
//         // // return concentration;
//         return buffer;
//       }
//   }
// }

// double ads_convert(int value, bool resist) {
//   double volt;
//   const double load_r = 47*1000;
//   const double V_in = 3.3;
//   const double off_volt= 2.27272727273;
//   double sen_r;
//   volt = value * LSB;      
//   double Vout = off_volt + volt;
//   printf("adc value: %d\n",value);
//   switch (resist) {
//     case (false):           //voltage of adc reading 
//       Serial.println(volt);
//       return volt;
//       break;
//     case (true): // resistance of adc reading
//       sen_r = load_r*(V_in-Vout)/Vout;
//       printf("Sensor Resistance: %.2f\n",sen_r);
//       return sen_r;
//       break;
//   }
// }


void output_result(){
  if(fail_count==50){
    draw_result(0,0);
    return;
  }
  double conc_Ace = 0;
  double conc_CO2 = 0;
  int CO2_peak = find_peak_value(0,millisUnitTime);
  int ace_peak = find_peak_value(4,millisUnitTime);
  // double baseline_resist = ads_convert(baseline, true); 
  // double peak_resist_CO2 = ads_convert(CO2_peak, true);
  // double peak_resist_Ace = ads_convert(ace_peak, true);
    // conc_CO2 = ratio_calibration(baseline_resist, peak_resist_CO2, 1);
    // conc_Ace = ratio_calibration(baseline_resist, peak_resist_Ace, 2);
  conc_CO2 = (double)CO2_peak/(double)baseline;
  conc_Ace = (double)ace_peak/(double)baseline;
  Serial.println(conc_Ace);
  Serial.println(conc_CO2);
  store_result(conc_Ace,conc_CO2);
    
    // conc_CO2 = (double)CO2_peak/(double)baseline;
    // conc_Ace = (double)ace_peak/(double)baseline;

    conc_Ace = 1.01; //dummydata
    conc_CO2 = 1.20; //dummydata

    Serial.println(conc_Ace);
    Serial.println(conc_CO2);
    store_result(conc_Ace,conc_CO2);


//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  // printf("Breath Analysis Result:\n");
  // printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, CO2(%): %.6f\n", peak_resist_CO2 , baseline_resist , conc_CO2);
  // printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, Ratio_Acetone: %.6f\n", peak_resist_Ace , baseline_resist , conc_Ace);
  
  // Serial.print("peal_value: "); Serial.println(peak_resist_Ace, 6); Serial.print("Baseline Resistance (Ohm): "); Serial.println(baseline_resist_Ace, 6); Serial.print("Ratio_Acetone: "); Serial.println(ratio_Ace, 6);
  draw_result(conc_CO2,conc_Ace);
  store_data();
}// not in percentage






