#include "Simple_ACE.h"
// #include <SHT2x.h>
// #include "DFRobot_SHT20.h"
#include "uFire_SHT20.h"
#include <Screen.h>
#include <PID.h>
#include "Calibration.h"
// #include <SPIFFS.h>
// Adafruit_ADS1115 ads;
// DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);
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

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  if (!EEPROM.begin(2)) {
    Serial.println("An Error has occurred while mounting EPPROM");
    return;
  }
   //To rewrite each file from the first file
  EEPROM.write(EEP_add_1, 255); EEPROM.commit();

  if(EEPROM.read(EEP_add_1)== 255){
    EEPROM.write(EEP_add, 0);
    EEPROM.write(EEP_add_1,0);
    Serial.println("Rewrite from zero.");
  }
  EEPROM.commit();
  int foo = EEPROM.read(EEP_add);
  Serial.println(foo);
  Serial.println(EEPROM.read(EEP_add_1));

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
      if (read_humidity() > 70) {
        store = true;
        Serial.println("Certain a breathe. Recording...");
      }
    }
    CO2_arr[q] = adc_CO2;
    // Serial.println(q);delay(1);
    q = q + 1;
  }
  if(fail_count==100){
    return;
  }
}

int peak_value(int position) {
  int peak = 0;
  int start = ref_position[1]-100;
  int end = ref_position[1] + 100;
  printf("%d , %d\n", (int)start, (int)end);
  for (int i = start ; i < end; i++){
    if ( CO2_arr[i] > peak) {
      peak = CO2_arr[i];
      // printf("Replaced\n");
    }
  }
  printf("Peak value is %d.\n", peak);
return (peak);


  // double acetone_start = 500;
  // double acetone_end = 1000;
  // double coec  = 55.0 / temp;
  // int peak = 0;
  // acetone_start = acetone_start * coec;
  // acetone_end = acetone_end * coec;
  // printf("%d , %d\n", (int)acetone_start, (int)acetone_end);
  //   printf("Find peak ok\n");
  // for ( int i = (int)acetone_start - 1 ; i <= (int) acetone_end - 1; i++) {
  //   // Serial.println(CO2_arr[i]);
  //   // printf("%d\n", i);
  //   if ( CO2_arr[i] > peak) {
  //     peak = CO2_arr[i];
  //     // printf("Replaced\n");
  //   }
  // }
  // // printf("Peak value is %d.\n", peak);
  // // printf("Baseline value is %d.\n", baseline);
  // return(peak);
}

double ratio_calibration(double uncal_base, double uncal_reading, bool formula){
    double cal_ratio;
    double buffer;
    buffer =  uncal_reading / uncal_base;
    switch (formula) { 
        case (true): // acetone _concentration
          slope = 1;
          constant = 0;
          cal_ratio = (buffer - constant) / slope;
          return cal_ratio;
          break;
        case (false): // CO2 concentration
          slope = -0.0809;
          constant = 2.64;
          cal_ratio = (buffer - constant) / slope;
          return cal_ratio;
          break;
  }
}

double ads_convert(int value, bool resist) {
  double volt;
  const double load_r = 300*1000;
  const double V_in = 3.3;
  const double off_volt= 3.3*10/256;
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

// double sort_reject(double arr[], int arr_size) {
//   double buff;
//   double buff_1 = 0;
//   int len = arr_size;
//   for ( int j = 0 ; j < arr_size ; j++) {
//     for (int i = 0; i < arr_size - 1 - j; i ++) {
//       if (arr[i] > arr[i + 1]) {
//         buff = arr[i];
//         arr[i] = arr[i + 1];
//         arr[i + 1] = buff;
//       }
//     }
//   }
//   if (arr[0] < arr[1] * 0.8) {
//     arr[0] = 0;
//     len = len - 1;
//     Serial.println("rejected");
//   }
//   if (arr[2] > arr[1] * 0.8) {
//     arr[2] = 0;
//     len = len - 1;
//     Serial.println("rejected");
//   }

//   for (int i = 0; i < arr_size; i++) {
//     Serial.println(arr[i], 6);
//   }
//   for (int i = 0; i < arr_size; i++) {
//     buff_1 = buff_1 + arr[i];
//   }
//   double avg_rat = buff_1 / len;
//   return avg_rat;
// }

void output_result(){
  int CO2_peak = peak_value(ref_position[0]);
  int ace_peak = peak_value(ref_position[1]);
  double baseline_resist = ads_convert(baseline, true); 
  double peak_resist_CO2 = ads_convert(CO2_peak, true);
  double peak_resist_Ace = ads_convert(ace_peak, true);
  conc_Ace = ratio_calibration(baseline_resist, peak_resist_Ace, true);
  conc_CO2 = ratio_calibration(baseline_resist, peak_resist_CO2, false);
//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  printf("Breath Analysis Result:\n");
  printf("peal_value: %.6f\nBaseline Resistance (Ohm): %.6f\nRatio_Acetone: %.6f\n",peak_resist_Ace, baseline_resist,conc_Ace);
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




