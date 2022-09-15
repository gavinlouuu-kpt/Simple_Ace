#include "Simple_ACE.h"
#include <Adafruit_ADS1X15.h>
#include <SHT2x.h>
#include <Screen.h>
// #include <SPIFFS.h>
// #include <BlynkSimpleEsp32.h>
// #include <PID_v1.h>

Adafruit_ADS1115 ads;
SHT20 sht;
// double Kp=2, Ki=5, Kd=1;
// double Setpoint = 1550;
// double Input, Output;
// PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);
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
  pinMode(solPin, OUTPUT);
  // pinMode(fanPin, OUTPUT);
  pinMode(btn_rst, INPUT);
}

void analogSetup(){
  ledcSetup(colChannel, freq, resolution);
  ledcAttachPin(colPin, colChannel);
  ledcWrite(colChannel, dutyCycle_col);
  dacWrite(pumpChannel, 255);
  delay(100);
  ledcWrite(pumpChannel, dutyCycle_pump);
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

  sht.begin(21,22);
  uint8_t stat = sht.getStatus();
  Serial.println(stat, HEX);

  if (!ads.begin()) {
  Serial.println("Failed to initialize ADS.");
  while (1);
  }
  
  // myPID.SetMode(AUTOMATIC);
  Serial.println("Setup Complete."); 
}

void restore_humidity(){
  while(1){
    // ledcWrite(pumpChannel, 255);
    int previous = sht.getHumidity();
    sht.read();
    Serial.println(sht.getHumidity());
    if (sht.getHumidity() - previous  < 2) {
      printf("Humiditty Restored\n");
      dacWrite(pumpChannel, dutyCycle_pump);
      delay(5);
      break;
    }    
  }
}

double ads_convert(int value, bool resist) {
  double volt;
  const double ref_r = 9990;
  const double V_in = 3.3;
  double sen_r;
  volt = value * LSB;
  switch (resist) {
    case (false):
      Serial.println(volt);
      return volt;
      break;
    case (true):
      sen_r = ref_r * (V_in - volt) / volt;
      Serial.println(sen_r);
      return sen_r;
      break;
  }
}

double sort_reject(double arr[], int arr_size) {
  double buff;
  double buff_1 = 0;
  int len = arr_size;
  for ( int j = 0 ; j < arr_size ; j++) {
    for (int i = 0; i < arr_size - 1 - j; i ++) {
      if (arr[i] > arr[i + 1]) {
        buff = arr[i];
        arr[i] = arr[i + 1];
        arr[i + 1] = buff;
      }
    }
  }
  if (arr[0] < arr[1] * 0.8) {
    arr[0] = 0;
    len = len - 1;
    Serial.println("rejected");
  }
  if (arr[2] > arr[1] * 0.8) {
    arr[2] = 0;
    len = len - 1;
    Serial.println("rejected");
  }

  for (int i = 0; i < arr_size; i++) {
    Serial.println(arr[i], 6);
  }
  for (int i = 0; i < arr_size; i++) {
    buff_1 = buff_1 + arr[i];
  }
  double avg_rat = buff_1 / len;
  return avg_rat;
}

double read_humidity(){
  double value;
  sht.read();
  value = sht.getHumidity();
  return value;
}

void breath_check(){
  while (true) {
    double arr[3];
    float humd;
    double gradient;
    long previous;  
    for (int i = 0; i < 3; i++) {
      sht.read();
      arr[i] = sht.getHumidity();
      printf("%.2f\n",arr[i]);
    }
    short adc_CO2 = ads.readADC_SingleEnded(CO2_channel);
    draw_sensor((double)adc_CO2);
    draw_humid(read_humidity());
    gradient  = (arr[2] - arr[0]) * 7 ;
    printf("Grad: %.3f\n",gradient);
    delay(1);
    if (gradient > 0.6) {
      break;
    }
  }
}



double ratio_calibration(double uncal_base, double uncal_reading, bool formula){
    double cal_ratio;
    double buffer;
    double slope;
    double constant;
    buffer = uncal_base / uncal_reading;
    switch (formula) {
        case (true):
          slope = 1;
          constant = 0;
          cal_ratio = (buffer - constant) / slope;
          return cal_ratio;
          break;
        case (false):
          slope = -0.0809;
          constant = 2.64;
          cal_ratio = (buffer - constant) / slope;
          return cal_ratio;
          break;
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

double concentration_ethanol( double temp, int baseline) {
  double acetone_start = 900;
  double acetone_end = 1200;
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
      printf("Replaced\n");
    }
  }
  printf("Peak value is %d.\n", peak);
  printf("Baseline value is %d.\n", baseline);
  // double ratio =  peak / baseline;
  // printf("%d\n", baseline);
  // printf(" Acetone Concentration: %.5f \n", ratio);
  return(peak);
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

double ratio_Ace;
bool store;
int baseline;
void sample_collection(){
  int baseline;
  int q = 0;
  unsigned long previous ;
  short adc_CO2;

  restore_humidity();
  baseline = restore_baseline();

  delay(1);
  printf("Blow Now\n");
  breath_check();

  store = false;
  previous = millis();
  int count = 0 ;
  while (millis() - previous < sampletime + 1) {
    adc_CO2 = ads.readADC_SingleEnded(CO2_channel);
    draw_sensor((double)adc_CO2);
    draw_humid(read_humidity());
    if (store == false) {
      count = count +1 ;
      if (count== 100){
        printf("This is a failed breath");
        break;
      }
      if (read_humidity() > 70 ) {
        store = true;
        Serial.println("Certain a breathe. Recording...");
      }
    }
    // Input = analogRead(NTCC);
    // myPID.Compute();
    // ledcWrite(colChannel,Output); //220
    CO2_arr[q] = adc_CO2;
    Serial.println(q);delay(1);
    q = q + 1;
  }

  if(count==100){
    delay(5);
    return;
  }

}

void output_result(){
  printf("Clear ok\n");
  int peak=0;
  peak = concentration_ethanol(temperate,baseline);delay(1);
  printf("Find peak ok\n");
  double peak_resist_Ace = ads_convert(peak, true);delay(1);
  printf("Find resist ok\n");
  double baseline_resist_Ace = ads_convert(baseline, true);delay(1);
  ratio_Ace = 12.0; //ratio_calibration(baseline_resist_Ace, peak_resist_Ace, true);delay(1);
//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  printf("Breath Analysis Result:\n");
  printf("Peak_Acetone: %.6f\nBaseline Resistance (Ohm): %.6f\n Ratio_Acetone: %.6f\n",peak_resist_Ace, baseline_resist_Ace,ratio_Ace);
  draw_result(ratio_Ace);// Serial.print("Peak_Acetone: "); Serial.println(peak_resist_Ace, 6); Serial.print("Baseline Resistance (Ohm): "); Serial.println(baseline_resist_Ace, 6); Serial.print("Ratio_Acetone: "); Serial.println(ratio_Ace, 6);
}


unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

// void command_label(void) {
//   static lv_obj_t * command = lv_label_create(lv_scr_act());
//   lv_label_set_recolor(command, true);
//   lv_obj_set_style_bg_color(command, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
//   lv_label_set_text(command, "#FFFFFF B #");
//   lv_obj_align(command, LV_ALIGN_LEFT_MID, 30, 40);
// }

// void feedback_label(void) {
//   feedback = lv_label_create(lv_scr_act());
//   lv_label_set_recolor(feedback, true);
//   lv_obj_set_style_bg_color(feedback, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
//   if (value < 103) {
//     lv_label_set_text(feedback, "#FFFFFF Too less!!#" );
//   } else if (value > 103 && value< 106) {
//     lv_label_set_text(feedback, "#FFFFFF Too much!!#" );
//   } else if(value >106){
//     lv_label_set_text(feedback, "#FFFFFF Brilliant!!#" );
//   }
//   lv_obj_align(feedback, LV_ALIGN_BOTTOM_MID, 0, -30);
// }
// static void add_value(lv_timer_t * timer)
// {
//   LV_UNUSED(timer);
//   lv_label_set_text_fmt(number, "#FFFFFF %f#",ratio_Ace[0]);
//   Serial.println(ratio_Ace[0]);
// }

// void number_label(void) {
//  number = lv_label_create(lv_scr_act());
//   lv_label_set_recolor(number, true);
//   lv_obj_set_style_bg_color(number, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
//   lv_obj_align(number, LV_ALIGN_LEFT_MID, 150, 40);
//   lv_timer_create(add_value, 50, NULL);
//   //  lv_label_set_text_fmt(number, "#FFFFFF %d#",15);
// }




