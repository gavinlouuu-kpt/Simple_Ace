#include "Screen.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph1 = TFT_eSprite(&tft);

int rangeL = 2000;
int rangeH = 18000;

void tft_setup(){
  tft.init();
  tft.fillScreen(TFT_BLACK);
  graph1.createSprite(200, 100);
}

void draw_framework(){
  tft.drawString("Humidity",1, 5,2);
  tft.drawString("100",1, 32);
  tft.drawString("0",10, 132);
  tft.drawString("King's ",1, 250,2);
  tft.drawString("Technologies",1,270,1);
  tft.drawString("Phase",1,280,2);
}

void draw_humid(double H){
    tft.drawFloat(float(H), 1, 70, 180, 6);
}

void draw_sensor(int value){
    graph1.scroll(-1); 
    // printf("%lf\n",value);
    // Move sprite content 1 pixel left. Default dy is 0
    // value = map(value,rangeL,rangeH,0,100);
    value = (value-rangeL)*(3.14-0)/(rangeH-rangeL)+0;
    printf("%lf\n",value);
    graph1.drawFastVLine(199, 100 - 100*(sin(value)),2, TFT_YELLOW);
  // graph2.scroll(1); // Move sprite content 1 pixel right. Default dy is 0
  // graph2.drawFastVLine(0,64-graph2Val,graph2Val,TFT_RED);
    graph1.pushSprite(20, 32);
}