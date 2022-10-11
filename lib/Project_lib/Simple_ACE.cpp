#include "Simple_ACE.h"
#include "uFire_SHT20.h"
#include "Screen.h"
#include "Calibration.h"
#include <PID.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>

#include <time.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define PASSWORD            "10200718"
#define SSID                "KPTESP32"

Adafruit_ADS1115 ads;
uFire_SHT20 sht20;
// BlynkTimer timer;

const char* ntpServer = "pool.ntp.org";
// char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = SSID;
char password[] = PASSWORD;


double upload_buffer;
double upload_buffer_1;
double upload_buffer_2;
double upload_buffer_3; 

short Sensor_arr[store_size]={0};
// BLYNK_CONNECTED()
// {
//   // Change Web Link Button message to "Congratulations!"
//   Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
//   Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
//   Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
// }
// void myTimerEvent()
// {
//  //not mroe than than 10 samples per seconds
//  Blynk.virtualWrite(V1, upload_buffer);
//  Blynk.virtualWrite(V2, upload_buffer_1);
//  Blynk.virtualWrite(V0, upload_buffer_2);
//  Blynk.virtualWrite(V4, upload_buffer_3);
// }

// void blynk_upload(double v1, double v2, double v3, double v4) {
//   upload_buffer = v1;
//   upload_buffer_1 = v2;
//   upload_buffer_2 = v3;
//   upload_buffer_3 = v4;
//   delay(1000);
//   Blynk.run();
//   timer.run();
// }

void pinSetup(){
  pinMode(pumpPin, OUTPUT);
  pinMode(colPin,OUTPUT);
  pinMode(NTCC,INPUT);
  // pinMode(fanPin, OUTPUT);
  pinMode(btn_rst, INPUT);
}

void analogSetup(){
  ledcSetup(colChannel, freq, resolution);
  ledcAttachPin(colPin, colChannel);
  ledcWrite(colChannel, dutyCycle_col);
  dacWrite(pumpPin, 128);
  delay(100);
  dacWrite(pumpPin, dutyCycle_pump);
  // EEPROM_setup();
}

void checkSetup(){
  WiFi.begin(ssid,password);
  configTime(0, 0, ntpServer);
  unsigned long clk = getTime();
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

  // if (!SPIFFS.begin(true)) {
  //   Serial.println("An Error has occurred while mounting SPIFFS");
  //   return;
  // }

   //To rewrite each file from the first file
  EEPROM_setup();
  sht20.begin();
  //  sht20.checkSHT20();

  // uint8_t stat = sht20.getStatus();
  // Serial.println(stat, HEX);

  if (!ads.begin()) {
  Serial.println("Failed to initialize ADS.");
  while (1);
  }
  // PID_setup();
  Serial.println("Setup Complete."); 
}

void restore_humidity(){
  while(1){
    // ledcWrite(pumpChannel, 255);
    float previous = sht20.humidity();
    // sht20.read();
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
    float arr[3];
    float humd;
    double gradient;
    long previous;  
    for (int i = 0; i < 3; i++) {
      // sht20.read();
      arr[i] = sht20.humidity();
      // printf("%.2f\n",arr[i]);
      previous= millis();
      // printf("%d\n",previous);
    }
    short adc_CO2 = ads.readADC_SingleEnded(CO2_channel);
    printf("%d\n",adc_CO2);
    draw_sensor((double)adc_CO2);
    // draw_humid(arr[2]);
    // PID_control();
    gradient  = (arr[2] - arr[0]) * 7 ;
    // printf("Grad: %.3f\n",gradient);
    if (gradient > 0.6) {
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
  while (1) {
      int temp = baselineRead(CO2_channel );
      delay(100);
      int ref = baselineRead(CO2_channel );
      if (temp + 3 >= ref && temp - 3 <= ref) {
        Serial.println("Found Baseline");
        delay(10);
        return temp;
        break;
      }
    }
}
void power_saving(unsigned long last_time){
  while(1){
    delay(5);
    if (digitalRead(btn_rst) == HIGH) {
      Serial.println("New loop");
      dacWrite(pumpChannel, dutyCycle_pump);
      break;
    }
    if (millis() - last_time > wait_time) {
      dacWrite(pumpChannel, 0);
    }
  }
}

double conc_Ace;
double conc_CO2;
bool store;
int baseline;
void sample_collection(){
  int q = 0;
  unsigned long previous ;
  short adc_CO2;

  restore_humidity();
  baseline = restore_baseline();
  set_range(baseline);
  delay(1);
  printf("Blow Now\n");
  breath_check();
  store = false;
  previous = millis();
  int fail_count = 0 ;
  int previous_counter;
  draw_wait();
  while (millis() - previous < sampletime + 1) {
    if (millis() -previous_counter >1000){
      int time;
      time = (60-((millis()-previous))/1000)-1;
      previous_counter= millis();
      draw_time(time);
    }
    adc_CO2 = ads.readADC_SingleEnded(CO2_channel);
    printf("%d\n",adc_CO2);
    draw_sensor((double)adc_CO2); 
    // PID_control();
    if (store == false) {
      fail_count += 1 ;
      if (fail_count== 100){
        printf("This is a failed breath");
        break;
      }
      if (read_humidity() > 60) {
        store = true;
        Serial.println("Certain a breathe. Recording...");
      }
    }
    Sensor_arr[q] = adc_CO2;
    Serial.println(q);delay(1);
    q = q + 1;
  }
  if(fail_count==100){
    return;
  }
}

int peak_value(int address) {
  int peak = 0;
  int position;
  int start = EEPROM.get(address,position)-100;
  int end =  EEPROM.get(address,position) + 100;
  printf("%d , %d\n", (int)start, (int)end);
  for (int i = start ; i < end; i++){
    if ( Sensor_arr[i] > peak) {
      peak = Sensor_arr[i];
      printf("Replaced\n");
    }
  }
  printf("Peak value is %d.\n", peak);
return (peak);
}

double ratio_calibration(double base_resist, double peak_resist, int formula){
  double concentration;
  double buffer;
  buffer =  peak_resist / base_resist;
  switch (formula) { 
    case (1):
      { // CO2 concentration
        float coeff_1 = 0.596;
        float coeff_2 = -1.0194;
        float coeff_3 = 0.4467;
        concentration = coeff_1 * buffer * buffer + coeff_2 * buffer + coeff_3;
        return concentration;
        break;
      }
    case (2): // acetone _concentration
      {
        float slope = 1;
        float constant = 0;
        concentration = (buffer - constant) / slope;
        return concentration;
        break;
      }
  }
}

double ads_convert(int value, bool resist) {
  double volt;
  const double load_r = 300*1000;
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
  int CO2_peak = peak_value(0);
  int ace_peak = peak_value(4);
  double baseline_resist = ads_convert(baseline, true); 
  double peak_resist_CO2 = ads_convert(CO2_peak, true);
  double peak_resist_Ace = ads_convert(ace_peak, true);
    conc_CO2 = ratio_calibration(baseline_resist, peak_resist_CO2, 1);
    conc_Ace = ratio_calibration(baseline_resist, peak_resist_Ace, 2);

//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  printf("Breath Analysis Result:\n");
  printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, CO2(%): %.6f\n", peak_resist_CO2 , baseline_resist , conc_CO2);
  printf("peal_value: %.6f, Baseline Resistance (Ohm): %.6f, Ratio_Acetone: %.6f\n", peak_resist_Ace , baseline_resist , conc_Ace);
  draw_result(conc_Ace);// Serial.print("peal_value: "); Serial.println(peak_resist_Ace, 6); Serial.print("Baseline Resistance (Ohm): "); Serial.println(baseline_resist_Ace, 6); Serial.print("Ratio_Acetone: "); Serial.println(ratio_Ace, 6);
}


unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}




