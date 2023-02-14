#include "Simple_ACE.h"
#include "uFire_SHT20.h"
#include "Screen.h"
#include "Calibration.h"
#include <PID.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>
#include <math.h>
#include <time.h>
#include <TFT_eSPI.h>
#include "SPIFFS.h"
#include "Cloud_storage.h"
#include "History_Data.h"


extern TFT_eSPI tft; 
Adafruit_ADS1115 ads;
uFire_SHT20 sht20;
// BlynkTimer timer;

// const char* ntpServer = "pool.ntp.org";
// char auth[] = BLYNK_AUTH_TOKEN;
// char ssid[] = SSID;
// char password[] = PASSWORD;

int dutyCycle_pump = 120; //to be changed
double upload_buffer;
double upload_buffer_1;
double upload_buffer_2;
double upload_buffer_3; 

short Sensor_arr[store_size]={0};
bool control = true;

void pinSetup(){
  pinMode(pumpPin, OUTPUT);
  pinMode(colPin,OUTPUT);
  pinMode(NTCC,INPUT);
  pinMode(btn_rst, INPUT);
  pinMode(sensor_h,OUTPUT);
}

void analogSetup(){
  ledcSetup(colChannel, freq, resolution);
  ledcAttachPin(colPin, colChannel);
  // ledcWrite(colChannel, dutyCycle_col);
  
  dacWrite(sensor_h, 240);
  delay(5000); // turn on sensor heater with DAC 220 is HS ~1.9V
  dacWrite(sensor_h, 220);
  // dacWrite(senH,220);
}

void warm_up(){
  extern double Setpoint;
  unsigned long counttime = 0;
  double warm_up_length = 0;
  double ntcc_bar_base  = (double)analogRead(NTCC) - Setpoint;
  int boundary = 10;
  
  tft.drawRoundRect(15,210, 200,15,7,TFT_NEIGHBOUR_BEIGE);
  while(abs(analogRead(NTCC)-(int)Setpoint) > boundary){ 
    // Serial.print("difference: "); Serial.println(abs(analogRead(NTCC)-(int)Setpoint));
    PID_control();
    warm_up_length = abs ((double)analogRead(NTCC)-Setpoint);
    // tft.fillRect(15, 210, (int)(200 * (1-(warm_up_length / ntcc_bar_base))), 5, TFT_NEIGHBOUR_BEIGE);
    tft.fillRoundRect(15, 210, (int)(200 * (1-(warm_up_length / ntcc_bar_base))), 15, 7, TFT_NEIGHBOUR_BEIGE);
    delay(10);
  }
  // Serial.print("Analog read:");Serial.println(analogRead(NTCC));
  tft.fillRect(20,200,200,80,TFT_NEIGHBOUR_GREEN);   // cover graph 
}

void pump_control(bool control){
  if(control == true){
  dacWrite(pumpPin, 225);
  delay(200);
  dacWrite(pumpPin,150);
  delay(200);
  dacWrite(pumpPin,100);
  delay(200);
  dacWrite(pumpPin, dutyCycle_pump);
  }
  else{
    dacWrite(pumpPin, 0);
    delay(100);
  }
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
   //To rewrite each file from the first file
  EEPROM_setup();
  sht20.begin();

  ads.setGain(GAIN_ONE); 
  if (!ads.begin(0x48)) {
  Serial.println("Failed to initialize ADS.");
  while (1);
  }
  PID_setup();
  Serial.println("Setup Complete."); 
}

void restore_humidity(){
  while(1){
    float previous = sht20.humidity();
    Serial.println(sht20.humidity());
    if (sht20.humidity() - previous  < 2) {
      printf("Humiditty Restored\n");
      dacWrite(pumpChannel, dutyCycle_pump);
      delay(5);
      break;
    }    
  }
}

double read_humidity(){ 
  float value;
  value = sht20.humidity();
  return value;
}

void breath_check(){
  while (true) {
    PID_control();
    float arr[3];
    float humd;
    double gradient;
    long previous;  
    for (int i = 0; i < 3; i++) {
      arr[i] = sht20.humidity();
      previous= millis();
    }
    // Serial.print("Average baseline:");Serial.println(average);
    draw_sensor(ads.readADC_SingleEnded(0));
    gradient  = (arr[2] - arr[0]) * 7 ;
    if (gradient > 1) {
      printf("breath real...");
      break;
    }
  }
}

double baselineRead(int channel) {
  int toSort[baseSample];
  float mean = 0;
  for (int i = 0; i < baseSample; ++i ) {
    toSort[i] = ads.readADC_SingleEnded(channel);
    delay(5);
  }
  for (int i = 0; i < baseSample; ++i) {
    mean += toSort[i];
  }
  mean /= baseSample;
  return mean;
}

int restore_baseline(){
  extern double Setpoint;
  double temp=0;
  double ref=0;
  Serial.print("Duty Cycle");Serial.println(dutyCycle_pump);
  pump_control(true);
  // dacWrite(pumpPin, dutyCycle_pump);
  ledcWrite(colChannel, 255);
  int counter=0 ;
  unsigned long cleaning_counter = millis();
  while(millis()-cleaning_counter <10000){
    // Serial.println("removing residues...");
    draw_loading(counter);counter ++;
  }
  while(abs(analogRead(NTCC)-(int)Setpoint) > 10){
    PID_control();
    draw_loading(counter);counter ++;
  } 

  unsigned long previous_time= millis();
  double slope =0 ;
  double buffer[5]= {0};
  int count = 0;
  while (1) {
    // if(millis()-previous_time > 20000){   //RESTORE TIMER 
    //   break;
    // }
    PID_control();
    draw_loading(counter);counter ++;
    temp = baselineRead(CO2_channel);
    Serial.print("Temp value:");Serial.println(temp);
    delay(10);
    ref = baselineRead(CO2_channel);
    Serial.print("Ref value:");Serial.println(ref);
    slope = (temp - ref)/0.5;
    Serial.print("Slope:");Serial.println(slope);
    if (abs(slope)< 8) { //wait baseline drop flat
      buffer[count] = slope;
      count ++; 
      if(count > 4){
        Serial.print("Found Baseline:");Serial.println(temp);
        Serial.println("Start forecasting...");
        return (int)ref;
      }
    }
    else{
      for(int i=0; i<5; i++){
        buffer[i]= 0;
      }
      count = 0;
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

double conc_Ace;
double conc_CO2;
bool isStore;
int baseline;
int fail_count = 0 ;
int unit;
void sample_collection(){
  int a = 0;
  float bar_time;
  float bar_percentage;
  int q = 0;
  float previous ;
  short adc_CO2;
  restore_humidity();
  baseline = restore_baseline();
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.fillRect(90,250,70,70,TFT_NEIGHBOUR_GREEN);  //cover loading
  tft.drawString("HUFF now", 120, 245, 4);
  // set_range(baseline);
  // delay(1);
  breath_check();
  isStore = false;
  previous = millis();
  int previous_counter;
  int previous_counter2;
  draw_wait();
  for(int i =0; i<store_size; i++){
    Sensor_arr[i]=0;
  }
  
  long start_time = millis();
  // draw_time(time);
  while (millis() - previous <= sampletime + 1) {
    int time =0 ;
    bar_time = millis() - previous;
    bar_percentage = (bar_time/45000)*100;
    draw_progress(bar_time,bar_percentage);


    if (millis()-previous_counter2 >10){ 
      // Sensor_arr[q]= analogRead(NTCC);
      Sensor_arr[q]= ads.readADC_SingleEnded(CO2_channel);
      // Serial.println(Sensor_arr[q]);
      draw_sensor(Sensor_arr[q]); 
      q ++;
      previous_counter2 = millis();      
    }
    PID_control();
    if (isStore == false) {
      fail_count += 1 ;
      if (fail_count== 50){
       break;
      }
      if (read_humidity() > 40) {
        isStore = true;
        fail_count= 0;
        // Serial.println("Certain a breathe. Recording...");
      }
    }
  }
  if(fail_count==50){
    return;
  }
  Serial.print("Number of sample:");Serial.println(q);
  int expose = millis() - start_time;
  Serial.print("Exposed time");Serial.println(expose);
  unit= expose/q;
}

int peak_value(int address, int unittime) {
  int peak = 0;
  int position;
  int peak_time;
  EEPROM.begin(20);
  // int start = EEPROM.get(address,position)-200;
  position =  EEPROM.get(address, peak_time)/unittime;
  int start = position-200;
  delay(100); 
  if(start<0){start=0;}
  // int end =  EEPROM.get(address,position) + 200;
  int end = position + 200;
  delay(100);
  EEPROM.end();
  printf("start: %d , end: %d\n", (int)start, (int)end);
  for (int i = start ; i < end; i++){
    // printf("value: %d\n", Sensor_arr[i]);
    if ( Sensor_arr[i] > peak) {
      peak = Sensor_arr[i];
      printf("Replaced %d\n", i);
    }
  }
  Serial.print("Peak value is");Serial.println(peak);
return (peak);
}

double ratio_calibration(double base_resist, double peak_resist, int formula){
  double concentration;
  double buffer;
  buffer =  peak_resist / base_resist;
  // buffer =   base_resist/peak_resist ;
  switch (formula) { 
    case (1):
      { // CO2 concentration
        // const float ref_baseline_resist= 4700.3;// assignn value
        // float ratio_baseline = base_resist/ref_baseline_resist;
        // // float correct_factor = 1.8764 * ratio_baseline * ratio_baseline * ratio_baseline - 3.9471 * ratio_baseline * ratio_baseline + 2.3844 * ratio_baseline + 0.6869;
        // float correct_factor = -0.208 * log(ratio_baseline) + 0.9922;
        // printf(" baseline ratio: %.6f\n", ratio_baseline);
        // printf("buffer: %.6f\n",buffer);
        
        // buffer = buffer*correct_factor;
        
        // // float coeff_1 = 0.596;
        // // float coeff_2 = -1.0194;
        // // float coeff_3 = 0.4467;
        // concentration = 1.9433 * exp(-6.143* buffer) ;
        // // return concentration;
        return buffer;
      }
    case (2): // acetone _concentration
      {
        // float slope = 1;
        // float constant = 0;
        // concentration = (buffer - constant) / slope;
        // // return concentration;
        return buffer;
      }
  }
}

double ads_convert(int value, bool resist) {
  double volt;
  const double load_r = 47*1000;
  const double V_in = 3.3;
  const double off_volt= 2.27272727273;
  double sen_r;
  volt = value * LSB;      
  double Vout = off_volt + volt;
  printf("adc value: %d\n",value);
  switch (resist) {
    case (false):           //voltage of adc reading 
      Serial.println(volt);
      return volt;
      break;
    case (true): // resistance of adc reading
      sen_r = load_r*(V_in-Vout)/Vout;
      printf("Sensor Resistance: %.2f\n",sen_r);
      return sen_r;
      break;
  }
}


void output_result(){
  if(fail_count==50){
    draw_result(0,0);
    return;
  }
  int CO2_peak = peak_value(0,unit);
  int ace_peak = peak_value(4,unit);
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


//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  // printf("Breath Analysis Result:\n");
  // printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, CO2(%): %.6f\n", peak_resist_CO2 , baseline_resist , conc_CO2);
  // printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, Ratio_Acetone: %.6f\n", peak_resist_Ace , baseline_resist , conc_Ace);
  
  // Serial.print("peal_value: "); Serial.println(peak_resist_Ace, 6); Serial.print("Baseline Resistance (Ohm): "); Serial.println(baseline_resist_Ace, 6); Serial.print("Ratio_Acetone: "); Serial.println(ratio_Ace, 6);
  draw_result(conc_CO2,conc_Ace);
  cloud_upload();
  // control=false;
  // pump_control(control);
}// not in percentage






