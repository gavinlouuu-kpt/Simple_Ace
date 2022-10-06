#include "Calibration.h"
#include "Simple_ACE.h"
#include "Screen.h"
#include <Adafruit_ADS1X15.h>


Adafruit_ADS1115 ads;
// TFT_eSPI tft = TFT_eSPI();

// short array[store_size] = {0};
// void calibration_setup(int temperature){
//     //convert temperature to pwm signal cycle//
//     ledcWrite(colPin,temperature);
// }

// int acetone_i,acetone_o;
// void array_locate(double temperature){
//     //convert temperature to the array entry 
//     const double std_temp = 55;
//     const int std_acetone_i = 500;
//     const int std_elution_size = 500;
//     double factor = std_temp/temperature;

//     int elution_size = std_elution_size * factor;
//     acetone_i = std_acetone_i * factor;
//     acetone_o = acetone_i + elution_size;
// }

// double ratio[2];
// double update_parameters(){
//     double ref_1[2]={ref_1_conc,ratio[0]};
//     double ref_2[2]= {ref_2_conc,ratio[1]};
//     double slope = (ref_2[1] - ref_1[1])/(ref_2[0]-ref_1[0]);
//     double constant = ref_1[1]/(slope*ref_1[0]);
//     draw_result();
// }

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

short array[SAMPLE_LENGTH] = {0};
double filtered_array[SAMPLE_LENGTH]={0};
int entry_counter = 0;
int peak_counter = 0;
int peak_buffer[100];

int previous_buf = peak_buffer[0];
int peak_count = 1;
int position[10] = {0};
int position_average = 0;
int position_counter = 0;



int finding_baseline();
void process_data();
void find_peak();


// int finding_baseline(){
//   int toSort[baseSample];
//   float mean = 0;
//   for (int i = 0; i < baseSample; ++i ) {
//     toSort[i] = ads.readADC_SingleEnded(1);
//     delay(5);
//   }
//   for (int i = 0; i < baseSample; ++i) {
//     mean += toSort[i];
//   }
//   mean /= baseSample;
//   return int(mean);
// }


void process_data(){
  for(int i = 0; i < sizeof(array)/sizeof(array[0])-10; i ++){
    if (array[i] != 0){
      int sum = 0;
      double mean;
      for( int j = 0; j <10 ; j++){
        sum = sum + array[i+j];
      } 
      mean = sum/10.00;
      delay(1);
    //   printf("%.2f\n",mean);
      filtered_array[i] = mean;
    }
  }
}

void find_peak(){
  for (int q = 0; q < sizeof(array)/sizeof(array[0])-10 -200; q ++){
    if (filtered_array[q] != 0){
      printf("%d\n",q);
      int diff = filtered_array[q+99] -filtered_array[q];
      int diff2= filtered_array[q+199] - filtered_array[q+99];
      int diff3 = filtered_array[q+99-5] -filtered_array[q+99+5];
      printf("Previous : %d, Difference:  %d\n",diff,diff2);
    // if(diff >15 && diff2< -15 && (fabsf(filtered_array[i]-filtered_array[i+199])<10||fabsf(filtered_array[i]-filtered_array[i+199]) > 50)){
      if(diff >15 && diff2< -15 && fabsf(diff3)<10){
        peak_buffer[peak_counter] = q+99;
        peak_counter += 1;
        } 
      delay(1);
    }
  }

  for (int j = 0; j < peak_candidate; j++){
    if(peak_buffer[j] != 0){
      printf("Grabbed point %d : %d\n",j+1,peak_buffer[j]);
      if(peak_buffer[j]-previous_buf >100){
        position_average = position_average/position_counter;
        position[peak_count-1]= position_average;
        
        peak_count +=1;

        position_average = 0;
        position_counter = 0;
      }
      position_average += peak_buffer[j];
      printf("position sum: %d\n",position_average);
      position_counter += 1;
      printf("position counte: %d\n",position_counter);
      previous_buf = peak_buffer[j];
    }
  }

  position_average = position_average/position_counter;
  position[peak_count-1]= position_average;
  printf("Peak Total: %d\n", peak_count);
  for (int i =0; i< show_peak; i++){
    printf("Peak position: %d\n", position[i]);
  }
}

void update_parameters(){
    int ref_position[2]={position[0],position[1]};
    // draw_result();
}

void calibration() { //put your main code here, to run repeatedly:
  delay(5000);
  long previous = millis(); 
  int count = 3;
  while(millis()-previous < 4000){// countdown for start calibration
    long prev;
    if((millis()-prev)>1000){
      printf("%d\n", count);
      count -= 1;
      prev =millis();
    }
  }
  previous = millis();

  while(millis() - previous < 30000){
    array[entry_counter] = ads.readADC_SingleEnded(1);
    printf(" %d\n", array[entry_counter]);
    entry_counter += 1;
    delay(5);
    printf("Counter 1: %d\n", entry_counter);
  }

  process_data();
  find_peak();
  update_parameters();
}
