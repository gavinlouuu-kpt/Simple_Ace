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
#include "Image_assets/Return_arrow_flip.h"

double baselineRead(int channel); //average out baseline candidate
void restore_baseline();           //define sensor baseline for new set of gas data        
int breath_check();              //  check if sensor value exceed threshold
void checkSetup(void);            //  initialize I2C protocol,EEPROM and SPIFFS memory, PID control
void output_result();             //  return sensor response in ratio
void pinSetup(void);              //  define pin cofig for pump, sensor, and sensor heater
void pump_control(bool control);  //  functions conrol high and low of the pump
void sample_collection();         //  integrate function to analysis one gas sample
void storing_data();              // background storage of gas data into Firebase/ local SPIFFS
void sensor_heater_control(bool control); //  control sensor heater power

uint8_t lifecount_address = 10;        

extern TFT_eSPI tft; 
extern bool leave;
Adafruit_ADS1115 ads;
SHTSensor sht(SHTSensor::SHT4X);

short Sensor_arr[store_size]={0};
short temporal_baseline = 0;
bool isStore = false;

uint8_t dutyCycle_pump = 120;         
int baseline = 0;
uint8_t fail_count = 0;
uint8_t millisUnitTime = 0;  
int loading_index=0 ;

int temp_peak_poisition = 0;

void pinSetup(){
  pinMode(pumpPin_1,OUTPUT);
  pinMode(pumpPin_2,OUTPUT);
  pinMode(colPin_1,OUTPUT);
  pinMode(colPin_2,OUTPUT);
  pinMode(sensor_heater,OUTPUT);
  pinMode(battery_EN, OUTPUT);
  pinMode(btn_rst, INPUT);
  pinMode(battery_read,INPUT);

  digitalWrite(battery_EN,1);           //  enable battery monitor
  dacWrite(sensor_heater,0);             //  enable senosr heater
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
  ledcWrite(pumpChannel_1,150);
  delay(200);
  ledcWrite(pumpChannel_1,100);
  delay(200);
  ledcWrite(pumpChannel_1, dutyCycle_pump);
  }
  else{
    ledcWrite(pumpChannel_1, 0);
    delay(100);
  }
}

void sensor_heater_control(bool control){
  if(control == true){
    dacWrite(sensor_heater,255);
    Serial.println("Sensor Heater On");
  }
  else{
    dacWrite(sensor_heater,0);
    Serial.println("Sensor Heater Off");
  }
}

int breath_check(){
  long previoustime = millis();
  uint8_t array_index= 0;
  while (true) {
    leave_sample();
    if(leave ==true){
      Serial.println("leave breath check");
      return 0;
    }
    PID_control();
    if(array_index <100) {
      Sensor_arr[array_index] = ads.readADC_SingleEnded(Sensor_channel);
      array_index++;
    }
    else{
    //shift the value of each entry one position samller, and store the new valuea at the 99th position
      for (int i = 0; i < 99; i++) {
        Sensor_arr[i] = Sensor_arr[i + 1];
      }
      Sensor_arr[99] = ads.readADC_SingleEnded(Sensor_channel);
    }

    if(millis() - previoustime > 500){
      if(Sensor_arr[99] - Sensor_arr[0] > 200){
        Serial.print("Baseline: ");Serial.println(Sensor_arr[0]);
        tft.fillRect(0, 50, 240, 95, TFT_NEIGHBOUR_BEIGE );  //cover loading
        return Sensor_arr[0];
      }
      temporal_baseline = Sensor_arr[0];
      previoustime =millis();
    }
    draw_sensor(ads.readADC_SingleEnded(Sensor_channel));
  }
}

double baselineRead(int channel) {
  int toSort[baseline_window];
  float mean = 0;
  for (int i = 0; i < baseline_window; ++i ) {
    toSort[i] = ads.readADC_SingleEnded(channel);
  }
  for (int i = 0; i < baseline_window; ++i) {
    mean += toSort[i];
  }
  mean /= baseline_window;
  return mean;
}

void restore_baseline(){
  extern double PID_Setpoint;
  double temporal_read = 0;
  double reference_read = 0;
  int loading_index=0 ;
  unsigned long millisCleanStart = millis();
  double slope = 0 ;
  double flat_slope[5]= {0};
  uint8_t flat_count = 0;

  while (1) {
    leave_sample();
    if(leave == true){
      Serial.println("escape");
      break;
    }
    PID_control();
    display_loading(loading_index);loading_index ++;
    temporal_read = baselineRead(Sensor_channel);
    Serial.print("Temp value:");Serial.println(temporal_read);
    reference_read = baselineRead(Sensor_channel);
    Serial.print("Ref value:");Serial.println(reference_read);
    slope = (temporal_read - reference_read)/0.5; // timelapse of two value retrieved
    Serial.print("Slope:");Serial.println(slope);
    if (abs(slope) < 8) { //wait baseline drop flat, change according to the strigency of your baseline
      flat_slope[flat_count] = slope;
      flat_count ++; 
      if(flat_count > 4){
        temporal_baseline = reference_read; //update sensor_baseline
        Serial.println("Start forecasting...");
        return;
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

float forecast_baseline(int position){
  // find the slope of the first 100 data points of the sensor array, and then halved the slope to find th equation of the slope
  float slope = 0.1*((Sensor_arr[49] - Sensor_arr[0])/50.00);
  Serial.print("Differnce:");Serial.println(Sensor_arr[49] - Sensor_arr[0]);
  Serial.print("Slope:");Serial.println(slope);
  float intercept = Sensor_arr[0];
  Serial.print("Intercept:");Serial.println(intercept);
  Serial.print("position:");Serial.println(position);
  float baseline = slope * Sensor_arr[position] + intercept;
  Serial.print("Baseline:");Serial.println(baseline);
  return baseline;
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
  int data_size = 100;
  short adc_CO2;
  pump_control(true);
  sensor_heater_control(true);

  leave = false;
  restore_baseline();
  if(leave == true){}
  else{
    for(int i =0; i<store_size; i++){Sensor_arr[i]=0;}
    tft.fillRect(0,30,240,60,TFT_NEIGHBOUR_BEIGE);        //cover initlaizing
    tft.fillRect(90, 200, 70, 70, TFT_NEIGHBOUR_BEIGE );  //cover loading
    tft.fillRect(0, 280, 240, 40, TFT_NEIGHBOUR_BEIGE );  //cover bar
    tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);
    tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Huff for 3 seconds", 15,50, 4);
  }
  baseline = breath_check();
  float slope = 0.1*((Sensor_arr[49] - Sensor_arr[0])/50.00);
  if(leave == true){}
  else{
    isStore = true;
    int previousDrawLoad = 0;
    tft.fillRect(0, 200, 240, 70, TFT_NEIGHBOUR_BEIGE );
    long millisStartSample = millis();
    while (millis() - millisStartSample <= sampletime + 1) {
      int time =0 ;
      bar_time = millis() - (float)millisStartSample;
      bar_percentage = (bar_time/sampletime)*100;
      draw_sample_progress(bar_time,bar_percentage);

      if (millis()-previousDrawLoad >10){ 
        Sensor_arr[data_size]= ads.readADC_SingleEnded(Sensor_channel);
        draw_sensor(Sensor_arr[data_size]); 
        // Serial.print((float)Sensor_arr[data_size]);Serial.print(",");
        // Serial.println(slope * (float)data_size + (float)Sensor_arr[0]);    
        data_size ++;
        previousDrawLoad = millis();
      }
      PID_control();
    }
    if(fail_count==50){
      return;
    }

    Serial.print("Number of sample:");Serial.println(data_size);
    int expose = millis() - millisStartSample;
    Serial.print("Exposed time");Serial.println(expose);
    millisUnitTime= expose/data_size;

    pump_control(false);
    sensor_heater_control(false);
  }
}


int find_peak_value(int address, int unittime) {
  int peak_value = 0;
  int peak_position = 0;
  int peak_time = 0;
  EEPROM.begin(20);
  peak_position =  EEPROM.get(address, peak_time)/unittime;
  int check_peak_start = peak_position - 200;
  delay(100); 
  if(check_peak_start < 0){check_peak_start = 0;}
  int check_peak_end = peak_position + 200;
  delay(100);
  EEPROM.end();
  printf("check_peak_start: %d , check_peak_end: %d\n", (int)check_peak_start, (int)check_peak_end);
  for (int i = check_peak_start ; i < check_peak_end; i++){

    if (Sensor_arr[i] > peak_value) {
      peak_value = Sensor_arr[i];
      temp_peak_poisition = i;
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

double ads_convert(int16_t ads_value) {
  double load_resistance = 47000;
  double input_voltage = 3.3;
  double sensor_resistance = 0;
  double sensor_voltage = 0;
      
  sensor_voltage = ads.computeVolts(ads_value);
  sensor_resistance = ((load_resistance * input_voltage)/sensor_voltage) - load_resistance;
  return sensor_resistance;
}

void update_sensor_lifecount(){
  EEPROM.begin(20);
  uint8_t lifecount;
  EEPROM.get(lifecount_address,lifecount);
  Serial.print("previsou: ");Serial.println(lifecount);
  delay(500);
  lifecount--;
  Serial.print("Now: ");Serial.println(lifecount);
  EEPROM.put(lifecount_address,lifecount);
  delay(100);
  EEPROM.commit();
  delay(500);
  EEPROM.end();
  delay(500);
  return;
}

void output_result(){
  if(fail_count==50){
    draw_result(0,0);
    return;
  }
  double conc_Ace = 0;
  double conc_CO2 = 0;
  int CO2_peak = find_peak_value(0,millisUnitTime);
  double drift_baseline_CO2 = forecast_baseline(temp_peak_poisition);

  int ace_peak = find_peak_value(4,millisUnitTime);
  double drift_baseline_Ace = forecast_baseline(temp_peak_poisition);
  
  double baseline_resist_CO2 = ads_convert(drift_baseline_CO2); 
  double baseline_resist_Ace = ads_convert(drift_baseline_Ace); 
  double peak_resist_CO2 = ads_convert(CO2_peak);
  double peak_resist_Ace = ads_convert(ace_peak);
    // conc_CO2 = ratio_calibration(baseline_resist, peak_resist_CO2, 1);
    // conc_Ace = ratio_calibration(baseline_resist, peak_resist_Ace, 2);
  conc_CO2 = baseline_resist_CO2/peak_resist_CO2;
  conc_Ace = baseline_resist_Ace/peak_resist_Ace;
  // Serial.println("Resistance: ");
  // Serial.print(baseline_resist_CO2);Serial.print(",");Serial.print(peak_resist_CO2);Serial.print(",");Serial.print(baseline_resist_Ace);Serial.print(",");Serial.println(peak_resist_Ace);
  // Serial.println("ADC: ");
  // Serial.print(baseline);Serial.print(",");
  // Serial.print(drift_baseline_CO2);Serial.print(",");Serial.print(CO2_peak);Serial.print(","); Serial.print(drift_baseline_Ace);Serial.print(",");Serial.println(ace_peak);
  // Serial.print("Ratio: ");Serial.print(conc_CO2);Serial.print(",");Serial.println(conc_Ace);
  store_result(conc_Ace,conc_CO2);

  draw_result(conc_CO2,conc_Ace);
  store_data();

  update_sensor_lifecount();
}// not in percentage






