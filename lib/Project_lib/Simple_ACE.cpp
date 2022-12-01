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

#include "Cloud_storage.h"
#include "Loading.h"

// #define PASSWORD            "10200718"
// #define SSID                "KPTESP32"

extern TFT_eSPI tft; 
Adafruit_ADS1115 ads;
uFire_SHT20 sht20;
// BlynkTimer timer;

// const char* ntpServer = "pool.ntp.org";
// char auth[] = BLYNK_AUTH_TOKEN;
// char ssid[] = SSID;
// char password[] = PASSWORD;


double upload_buffer;
double upload_buffer_1;
double upload_buffer_2;
double upload_buffer_3; 

short Sensor_arr[store_size]={0};

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
  ledcWrite(colChannel, dutyCycle_col);
  dacWrite(pumpPin, 225);
  delay(100);
  dacWrite(pumpPin,150);
  dacWrite(pumpPin, dutyCycle_pump);
  dacWrite(sensor_h, 220); // turn on sensor heater with DAC 220 is HS ~1.9V
  // dacWrite(senH,220);
}

void checkSetup(){
  // WiFi.begin(ssid,password);
  // configTime(0, 0, ntpServer);
  // unsigned long clk = getTime();
  // while (1) {
  //   if (clk - getTime() < 10) {
  //     Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  //     break;
  //   }
  // }

  // if (Blynk.connect() == false) {
  //   ESP.restart();        //custom function I wrote to check wifi connection
  // }

  // timer.setInterval(1000L, myTimerEvent);
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

  if (!ads.begin(0x49)) {
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
    short adc_CO2 = ads.readADC_SingleEnded(CO2_channel);
    // printf("%d\n",adc_CO2);
    draw_sensor((double)adc_CO2);
    // PID_control();
    gradient  = (arr[2] - arr[0]) * 7 ;
    if (gradient > 0.4) {
      printf("breath real...");
      break;
    }
  }
}

int baselineRead(int channel) {
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
  return int(mean);
}

int restore_baseline(){
  dacWrite(pumpPin,200);
  ledcWrite(colChannel, 150);
  while (1) {
      int temp = baselineRead(CO2_channel );
      for(int i= 0;i<10;i++){
        tft.pushImage(80, 250, LoadingWidth  ,LoadingHeight, Loading[i]);
        delay(100);
      }
      tft.fillRect(80,250,70,70,TFT_NEIGHBOUR_GREEN);
      int ref = baselineRead(CO2_channel );

      if (temp + 4 >= ref && temp - 4 <= ref) {
        printf("Found Baseline %d\n", temp);
        delay(10);  
        dacWrite(pumpPin, dutyCycle_pump);
        ledcWrite(colChannel, dutyCycle_col);  
        return temp;
        break;
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
bool store;
int baseline;
int fail_count = 0 ;
int unit;
void sample_collection(){
  int q = 0;
  unsigned long previous ;
  short adc_CO2;
  PID_control();
  restore_humidity();
  baseline = restore_baseline();
  set_range(baseline);
  delay(1);
  // printf("Blow Now\n");
  breath_check();
  store = false;
  previous = millis();
  int previous_counter;
  int previous_counter2;
  draw_wait();
  for(int i =0; i<store_size; i++){
    Sensor_arr[i]=0;
  }
  
  long start_time = millis();
  while (millis() - previous < sampletime + 1) {
    if (millis() -previous_counter >1000){
      int time;
      time = ((sampletime-((millis()-previous)))/1000);
      previous_counter= millis();
      draw_time(time);
    }
    if (millis()-previous_counter2 >10){ 
      Sensor_arr[q]= ads.readADC_SingleEnded(CO2_channel);
      draw_sensor((double)Sensor_arr[q]); 
      q = q + 1;
      previous_counter2 = millis();      
    }
    PID_control();
    if (store == false) {
      fail_count += 1 ;
      if (fail_count== 50){
        // printf("This is a failed breath");
        break;
      }
      if (read_humidity() > 40) {
        store = true;
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

void storing_data(){
  cloud_upload();
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
        break;
      }
    case (2): // acetone _concentration
      {
        // float slope = 1;
        // float constant = 0;
        // concentration = (buffer - constant) / slope;
        // // return concentration;
        return buffer;
        break;
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


//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  // printf("Breath Analysis Result:\n");
  // printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, CO2(%): %.6f\n", peak_resist_CO2 , baseline_resist , conc_CO2);
  // printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, Ratio_Acetone: %.6f\n", peak_resist_Ace , baseline_resist , conc_Ace);
  
  // Serial.print("peal_value: "); Serial.println(peak_resist_Ace, 6); Serial.print("Baseline Resistance (Ohm): "); Serial.println(baseline_resist_Ace, 6); Serial.print("Ratio_Acetone: "); Serial.println(ratio_Ace, 6);
  draw_result(conc_Ace,conc_CO2);
  cloud_upload();
}// not in percentage






