#include "Calibration.h"
#include "Simple_ACE.h"
#include <EEPROM.h>
#include <Adafruit_ADS1X15.h>
#include "TFT_eSPI.h"
#include "Loading.h"
#include "PID.h"

extern TFT_eSPI tft;
extern Adafruit_ADS1115 ads;
const int waittime =1000;
const int caltime =50000;


extern short Sensor_arr[store_size];

int entry_counter = 0;
int peak_counter = 0;

int peak_buffer[peak_candidate];
int previous_buf = peak_buffer[0];
int peak_count = 0;
int position[10] = {0};
int position_average = 0;
int position_counter = 1;
int sample_size=0;
float ref_position[2];

int finding_baseline();
void process_data();
void find_peak();

void EEPROM_setup(){
  EEPROM.begin(20);
  printf("EEPROM begin\n");
}

void check_sample(){
  for(int i = 0; i < sizeof(Sensor_arr)/sizeof(Sensor_arr[0])-10; i ++){
    if(Sensor_arr[i]!=0){
      sample_size +=1;
    }
    else{
      break;
    }
  }
  // printf("Sample size: %d\n",sample_size);
}
void process_data(){
  for(int i = 0; i < sample_size-50; i ++){
      int sum = 0;
      double mean;
      for( int j = 0; j <50 ; j++){
        sum = sum + Sensor_arr[i+j];
      } 
      mean = sum/50.00;
      delay(1);
      printf("%.2f\n",mean);
     Sensor_arr[i] = mean;
  }
}
double get_std(double mean){
  float sum_1 = 0;
  double sample = 1;
  double std = 0;
  printf("imported mean: %.2f\n",mean);
  for(int i =0; i < sample_size-10;i++){
    if (Sensor_arr[i] != 0){
      sum_1 += sq((double)(Sensor_arr[i]-mean));

      std = sqrt(sum_1/sample);
      // printf("sample: %d, sum: %.2f, std:%.2f\n",Sensor_arr[i], sum_1, std);
      
      sample += 1;
    }
  }
  std = sqrt(sum_1/sample);
  printf("std:%.2f,sample size; %f\n",std,sample);
  return std;
}

double get_mean(){
  long sum=0;
  double sample =1;
  double mean=0;
  for(int i =0; i < sample_size-10;i++){
    if (Sensor_arr[i] != 0){
      sum += Sensor_arr[i];
      // printf("sample: %d, sum: %.2d\n",Sensor_arr[i], sum);
      sample +=1;
    }
  }
  mean = sum/sample;
  printf("mean:%.2f,sample size:%f\n",mean,sample);
  return mean;
}

// void find_peak(){
//   printf("average data");
//   double mean = get_mean();
//   double std = get_std(mean);
//   double threshold = mean + 1* std;
//   printf("threshold: %.2f\n", threshold);  
//   bool isIncreasingslope = false;
//   bool isSlowinggradient = false;
//   bool isPlateau = false;
//   int prevSlope;
//   int Slope;
//   int nextSlope;    
//   int checkPlateau = 0;
//   int checkNotplateau = 0;
//   int checkSlope = 0;
//   int checkNotSlope = 0;
//   for (int q = 9; q < sample_size-50 ; ){
//     if(isIncreasingslope == false){
//       prevSlope = Sensor_arr[q] - Sensor_arr[q- 9];
//       Slope = Sensor_arr[q + 9]-Sensor_arr[q];
//       nextSlope = Sensor_arr[q + 19] -Sensor_arr[q+9];
//       // printf("Position: %d: prevSlope: %d, Slope: %d, nextSlope: %d\n", q, prevSlope,Slope, nextSlope);
//       if( Slope -prevSlope >-2 && nextSlope- Slope> -2){
//         checkSlope += 1;
//       }
//       else{
//         checkNotSlope +=1;
//       }
//       if(checkSlope > 5){
//         isIncreasingslope = true;
//         printf("Position: %d , potential peak..checking\n", q);
//         checkSlope = 0;
//         checkNotSlope = 0;
//       }
//       if(checkNotSlope > 2){
//         isIncreasingslope = false;
//         checkSlope = 0;
//         checkNotSlope = 0;
//       }
//       q+=5;
//     }

//     if(isIncreasingslope == true && isSlowinggradient == false){
//       prevSlope = Sensor_arr[q] - Sensor_arr[q-5];
//       Slope = Sensor_arr[q + 4]-Sensor_arr[q];
//       nextSlope = Sensor_arr[q + 10] -Sensor_arr[q+5];
//       printf("Position: %d: prevSlope: %d, Slope: %d, nextSlope: %d\n", q, prevSlope,Slope, nextSlope);
//       if(prevSlope > Slope && Slope > nextSlope && nextSlope >=0 ){
//         isSlowinggradient = true;
//         printf("Position: %d, gradient slowing down\n", q);
//       }
//       q+=1;
//     }
 
//     if(isIncreasingslope == true && isSlowinggradient == true){
//       if(abs(Sensor_arr[q]- Sensor_arr[q-10]) <3){
//         checkPlateau +=1;
//         printf("plateauing..waiting\n");
//       }
//       else{
//         checkNotplateau +=1;
//       }
//       if(checkPlateau >10){
//         position[peak_count] = q;
//         peak_count +=1;
//         printf("Peak %d, position : %d\n", peak_count, q);
//         isIncreasingslope = false;
//         isSlowinggradient = false;
//         checkNotplateau =0;
//         checkPlateau =0;
//       }
//       if(checkNotplateau > 5){
//         printf("not plateau.. reset\n");
//         isIncreasingslope = false;
//         isSlowinggradient = false;
//         checkNotplateau =0;
//         checkPlateau =0;
//       }
//       q+=1;
//     }
//     // if (Sensor_arr[q] != 0){
//     //   if(Sensor_arr[q]>(int)(threshold)){
//     //     peak_buffer[peak_counter] = q;
//     //     peak_counter += 1;
//     //   }
//     //   printf("sample%d: %.2d,threshold: %.2f\n",q, Sensor_arr[q],threshold);
//     // }
//   }

//   previous_buf = peak_buffer[0];
//   printf("Finding peak...");
//   // for (int j = 0; j < peak_candidate; j++){
//   //   if(peak_buffer[j] != 0){
//   //     printf("Grabbed point %d : %d\n",j+1,peak_buffer[j]);
//   //     if(peak_buffer[j]-previous_buf >100){
//   //       position_average = position_average/position_counter;
//   //       position[peak_count-1]= position_average;
//   //       printf("Peak_count: %d\n",peak_count);
//   //       printf("Peak position: %d\n", position[peak_count-1]);
        
//   //       peak_count +=1;

//   //       position_average = 0;
//   //       position_counter = 1;
//   //     }

//   //     position_average += peak_buffer[j];
//   //     // printf("position sum: %d\n",position_average);
//   //     position_counter += 1;
//   //     // printf("position counte: %d\n",position_counter);
//   //     previous_buf = peak_buffer[j];
//   //   }
//   // }
//   // if(position_counter !=1){
//   //   position_average = position_average/position_counter;
//   //   position[peak_count-1]= position_average;
//   // }
//   printf("Peak Total: %d\n", peak_count);
  
//   for (int i =0; i< peak_count+1; i++){
//     printf("Peak position: %d\n", position[i]);
//   }

//   // for (int i = 0; i<100; i++){
//   //   peak_buffer[i] =0;
//   // }
// }

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
    delay(500);
}

void  calibration() { //put your main code here, to run repeatedly:
  PID_control();

  long previous = millis(); 
  long previous_2;
  entry_counter = 0;

  int i = 0;
  int time = 0;
  int num = 3;
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  bool fillscreen = true;
  bool istenth=true;
  while(millis() - previous < caltime){
      if(millis() - time > waittime && num >0){
        tft.drawString("Sample in ",110,120,4);
        tft.drawFloat(float(num),0,180,120,4);
        num--;
        time = millis();
      }
    //work simultaneously?
    if(millis()-time >1000 && num == 0 && fillscreen == true){
    tft.fillRect(0,100,240,40,TFT_NEIGHBOUR_GREEN);
    fillscreen = false;
    }
    // if(num == 0 && counter == 0 && millis() - time > waittime){
    // if(counter == 0 && millis() - time > waittime){
    //   tft.fillRect(0,100,240,40,TFT_NEIGHBOUR_GREEN);
    //   counter++;
    // }
    // printf("%d\n",time);
    // printf("%d\n",num);
    if(millis() - time > waittime){
      tft.drawString("Remain ",110,120,4);
      if((caltime-(millis() - previous))/waittime < 10 && istenth ==true){
        tft.fillRect(165,100,40,40,TFT_NEIGHBOUR_GREEN);
        istenth = false;
      }
      tft.drawFloat(float((caltime-(millis() - previous))/waittime),0,180,120,4);
      time= millis();
    }


    tft.pushImage(90, 150, LoadingWidth  ,LoadingHeight, Loading[i]);
    delay(10);
    i++;
    if(i>10){
      i = 0;
    }
    // tft.drawFloat(float((9000-millis())/1000),0,200,120,2);
    
    // printf("%d\n", millis());

    if (millis()-previous_2>10){
      Sensor_arr[entry_counter] = ads.readADC_SingleEnded(0);
      //printf(" %d\n", Sensor_arr[entry_counter]);
      entry_counter += 1;
      //printf("Counter 1: %d\n", entry_counter);
      previous_2 = millis();
      
    }

    
  }
  

  // while(millis() - previous < 90000){
  //   if (millis()-previous_2>10){
  //     Sensor_arr[entry_counter] = ads.readADC_SingleEnded(1);
  //     // printf(" %d\n", Sensor_arr[entry_counter]);
      
  //     printf(" %d, %d\n", entry_counter,Sensor_arr[entry_counter]);
  //     entry_counter += 1;
  //     previous_2 = millis();
  //   }
  // }
  check_sample();

  process_data();
  find_peak();//part to be corrected
  update_parameters();
  
}

void find_peak(){
  int max_1=0;
  // int max_11=0;
  int max_2=0;
  // int max_21 =0;
  for(int i = 50; i <250; i++){
    Serial.println(Sensor_arr[i]);
    if(Sensor_arr[i]>max_1){
      max_1=Sensor_arr[i];
      position[0] = i;
      Serial.println(position[0]);
    }
  }
  Serial.println();
  for(int j = 1000; j <2000; j++){
    Serial.println(Sensor_arr[j]);
    if(Sensor_arr[j]> max_2){
      max_2=Sensor_arr[j];
      position[1] = j;
      Serial.println(position[1]);
    }
  }
}
