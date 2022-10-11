#include "Calibration.h"
#include "Simple_ACE.h"
#include <EEPROM.h>
#include <Adafruit_ADS1X15.h>
extern Adafruit_ADS1115 ads;

// void draw_process(int trial){
//     tft.fillRect(80,140,140,180,TFT_NEIGHBOUR_GREEN);
//     String text = "Calibration: Trial" + (String)trial;
//     text = text.c_str();
//     tft.setTextDatum(CC_DATUM);
//     tft.drawString(text,120,160,2);
// }

// void draw_result(){
//     tft.setTextDatum(TL_DATUM);
//     tft.drawFloat((float)ratio[0],200,160,1);
//     tft.drawFloat((float)ratio[1],200,170,1);
//     tft.drawFloat((float)slope,200,180,1);
//     tft.drawFloat((float)constant,200,190,1);
// }

// int peak_profile[10];
// void calibration(){
//     long previous_time;
//     int counter = 0;
//     double baseline =  baselineRead(CO2_channel);
//     for (int sample=0; sample<2; sample ++){
//         draw_process(sample);
//         while(millis() - previous_time< sampletime -1){
//             array[counter] = ads.readADC_SingleEnded(CO2_channel);
//             counter +=1;
//         }
//         double peak =0; 
//         for (int i = acetone_i; i <acetone_o-4; i ++){
//             // if(array[i]> peak){ //find peak at the peak position
//             //     peak = array[i];
//             // }
//             int previous;
//             int diff = array[i] - array[i+4];
//             if(abs(previous)>5 && abs(diff)>5 && previous >0 && diff< 0){

//             }


//         }
//         ratio[sample] = ads_convert(peak,true)/ads_convert(baseline,true);
//     }
//     update_parameters();
// }

// void calibration_input(){
//     //create a input value
//     calibration_setup(value);
//     array_locate(value);
//     calibration();
// }

extern short Sensor_arr[store_size];
int entry_counter = 0;
int peak_counter = 0;
int peak_buffer[100];

int previous_buf = peak_buffer[0];
int peak_count = 1;
int position[10] = {0};
int position_average = 0;
int position_counter = 0;
float ref_position[2];

int finding_baseline();
void process_data();
void find_peak();

void EEPROM_setup(){
  EEPROM.begin(20);
  printf("EEPROM begin\n");
}

void process_data(){
  for(int i = 0; i < sizeof(Sensor_arr)/sizeof(Sensor_arr[0])-10; i ++){
    if (Sensor_arr[i] != 0){
      int sum = 0;
      double mean;
      for( int j = 0; j <10 ; j++){
        sum = sum + Sensor_arr[i+j];
      } 
      mean = sum/10.00;
      delay(1);
    //   printf("%.2f\n",mean);
     Sensor_arr[i] = mean;
    }
  }
}

void find_peak(){
  printf("average data");
  for (int q = 0; q < sizeof(Sensor_arr)/sizeof(Sensor_arr[0])-10 -200; q ++){
    if (Sensor_arr[q] != 0){
      // printf("%d\n",q);
      int diff =Sensor_arr[q+99] -Sensor_arr[q];
      int diff2=Sensor_arr[q+199] -Sensor_arr[q+99];
      int diff3 =Sensor_arr[q+99-5] -Sensor_arr[q+99+5];
      // printf("Previous : %d, Difference:  %d\n",diff,diff2);
    // if(diff >15 && diff2< -15 && (fabsf(filtered_array[i]-filtered_array[i+199])<10||fabsf(filtered_array[i]-filtered_array[i+199]) > 50)){
      if(diff >15 && diff2< -15 && fabsf(diff3)<10){
        peak_buffer[peak_counter] = q+99;
        peak_counter += 1;
        } 
      delay(1);
    }
  }

  previous_buf = peak_buffer[0];
  printf("Finding peak...");
  for (int j = 0; j < peak_candidate; j++){
    if(peak_buffer[j] != 0){
      // printf("Grabbed point %d : %d\n",j+1,peak_buffer[j]);
      if(peak_buffer[j]-previous_buf >100){
        if(position_counter==0){
          position_average = position_average/1;
        }
        else{
          position_average = position_average/position_counter;
        }
        position[peak_count-1]= position_average;
        // printf("Peak_count: %d\n",peak_count);
        // printf("Peak position: %d\n", position[peak_count-1]);
        
        peak_count +=1;

        position_average = 0;
        position_counter = 0;
      }
      position_average += peak_buffer[j];
      // printf("position sum: %d\n",position_average);
      position_counter += 1;
      // printf("position counte: %d\n",position_counter);
      previous_buf = peak_buffer[j];
    }
  }
  if(position_counter==0){
      position_average = position_average/1;
    }
    else{
      position_average = position_average/position_counter;
    }
  position[peak_count-1]= position_average;
  // printf("Peak Total: %d\n", peak_count);
  for (int i =0; i< show_peak; i++){
    // printf("Peak position: %d\n", position[i]);
  }
}

void update_parameters(){
  EEPROM.begin(20);
  printf("EEPROM begin\n");
    int past =0; 
    byte address = 0;
    EEPROM.get(0,past);
    printf("EEPROM write value : %d\n", past);

    ref_position[0] = float(position[0]);
    ref_position[1] = float(position[1]);
    printf("updated: %f\n",ref_position[0]);
   
    EEPROM.put(address, position[0]);  
    delay(100);  
    // printf("EEPROM address: %d, value: %d\n", address,position[0]);
    address += sizeof(int);
    EEPROM.put(address, position[1]);  
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
}

void calibration() { //put your main code here, to run repeatedly:
  long previous = millis(); 
  entry_counter = 0;
  while(millis() - previous < 90000){
    Sensor_arr[entry_counter] = ads.readADC_SingleEnded(1);
    printf(" %d\n", Sensor_arr[entry_counter]);
    entry_counter += 1;
    delay(5);
    printf("Counter 1: %d\n", entry_counter);
  }

  process_data();
  find_peak();
  update_parameters();
}
