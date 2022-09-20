#include "Screen.h"
#include <TFT_eSPI.h>
#include <Simple_ACE.h>

#include "Asset_2.h"
#include "Asset_7.h"
#include "Asset_8.h"
#include "Asset_10.h"
#include "Asset_13.h"
#include "Asset_14.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph1 = TFT_eSprite(&tft);

int rangeL = 0;
int rangeH = 8000;
uint16_t beige =    tft.color565(239, 227, 214);
// uint32_t beige =    tft.color565(255, 244, 225);


void tft_setup(){
  tft.init();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setSwapBytes(true);
  
  graph1.setColorDepth(16);
  graph1.createSprite(200, 150);
  graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
  graph1.setScrollRect(0, 0, 200,150, TFT_NEIGHBOUR_GREEN); 
}

void draw_framework(){
  tft.setTextColor(beige);
  tft.pushImage(5,200,A2_w,A2_h,Asset_2,0x0000);
  tft.pushImage(45,200,A7_w,A7_h,Asset_7,0x0000);
  tft.pushImage(85,200,A8_w,A8_h,Asset_8,0x0000);
  tft.pushImage(127,200,A10_w,A10_h,Asset_10,0x0000);
  tft.pushImage(167,200,A13_w,A13_h,Asset_13,0x0000);
  tft.pushImage(206,200,A14_w,A14_h,Asset_14,0x0000);
  tft.setTextDatum(TC_DATUM); 
  tft.drawString("Acetone Level",120, 10,4);
  tft.setTextDatum(TL_DATUM); 
  tft.drawString("King's ",8, 250,2);
  tft.drawString("Phase",8,270,2);
  tft.drawString("Technologies",8,290,2);
}

void draw_time(int time){
    tft.fillRect(45,200,148,50,TFT_NEIGHBOUR_GREEN); 
    tft.setTextColor(beige);
    tft.setTextDatum(TC_DATUM); 
    tft.drawFloat(time, 0, 120, 200, 6);
}
void draw_wait(void){
  tft.setTextDatum(TC_DATUM); 
  tft.setTextColor(beige);
  tft.drawString("Analyzing...",120, 250,4);
}

void set_range(int value){
  rangeL = value -1000;
  rangeH = (value +3000)*2;
}

void draw_sensor(double value){
    graph1.scroll(-1); 
    // Move sprite content 1 pixel left. Default dy is 0
    // value = map(value,rangeL,rangeH,0,100);
    value =  (value-rangeL)*(150*3.14-0)/(rangeH-rangeL)+0;
    // printf("%f\n",value); 
    graph1.drawFastVLine(199, 150 - (int)(150*(sin((value/150.0)-(3.14/2.0))+1.0)),5, beige);
    graph1.pushSprite(20, 40);
}
// bool store;
void draw_result(double value){
  tft.fillRect(70,200,100,50,TFT_NEIGHBOUR_GREEN); 
  tft.fillRect(50,250,140,30,TFT_NEIGHBOUR_GREEN); 
  tft.pushImage(45,200,A7_w,A7_h,Asset_7,0x0000);
  tft.pushImage(85,200,A8_w,A8_h,Asset_8,0x0000);
  tft.pushImage(127,200,A10_w,A10_h,Asset_10,0x0000);
  tft.pushImage(167,200,A13_w,A13_h,Asset_13,0x0000);
  tft.setTextDatum(TC_DATUM); 
  tft.drawString("Acetone",180, 270,2);
  tft.drawFloat((float)value,2,180,290,2);

  if(value > 1 || value <= 0||store == false){
  tft.drawString("Error!",120,120,4); 
  }
  else if (value > 0.7 && value <= 1){
  tft.drawString("Workout More!",120,120,4);
  }
  else if(value > 0.5 && value <= 0.7){
  tft.drawString("Pretty Good!",120,120,4);
  }
  else if(value <= 0.5 && value > 0){
    tft.drawString("Excellent Fat Burn!",120,120,4);
  } 
  delay(5000);
  tft.fillRect(140,270,80,40,TFT_NEIGHBOUR_GREEN);
  tft.fillRect(10,120,240,30,TFT_NEIGHBOUR_GREEN);
}