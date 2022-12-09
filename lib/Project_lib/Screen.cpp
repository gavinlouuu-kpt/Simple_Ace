#include "Screen.h"
#include "SPIFFS.h"
#include <TFT_eSPI.h>
#include <Simple_ACE.h>
#include "Calibration.h"
#include <SPI.h>
#include <EEPROM.h>
#include "uFire_SHT20.h"
#include <Adafruit_ADS1X15.h>
#include "Wifi_connection.h"
#include "Simple_ACE.h"


#include "Asset_2.h"
#include "Asset_7.h"
#include "Asset_8.h"
#include "Asset_10.h"
#include "Asset_13.h"
#include "Asset_14.h"
#include "setting.h"
#include "Beagle.h"
#include "PID.h"

#define setting_x 170
#define setting_y 250

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph1 = TFT_eSprite(&tft);
extern Adafruit_ADS1115 ads;
extern uFire_SHT20 sht20;
extern float ref_position[2];
String profileNumber = "";

int rangeL = 0;
int rangeH = 8000;
int stage = 0;
uint16_t beige = tft.color565(239, 227, 214);
uint16_t t_x = 0, t_y = 0;

int SetupNumber = 1;

const int offset= 10;
int Change = 0;
float max1;
float min1;

int i = 0;
float H[210];
float LowY;
float HighY;
int as_counter = 0;
int as_counter1 = 0;
int numMax = -1;
int numMin = -1;


void tft_setup()
{
  tft.init();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setSwapBytes(true);

  graph1.setColorDepth(16);
  graph1.createSprite(200, 150);
  graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
  graph1.setScrollRect(0, 0, 200, 150, TFT_NEIGHBOUR_GREEN);
}

void ResetXY()
{
  t_x = 0;
  t_y = 0;
}

void draw_framework()
{
  tft.setTextColor(beige);
  tft.pushImage(10,200,A2_w,A2_h,Asset_2,0x0000);
  tft.pushImage(50,200,A7_w,A7_h,Asset_7,0x0000);
  tft.pushImage(90,200,A8_w,A8_h,Asset_8,0x0000);
  tft.pushImage(130,200,A10_w,A10_h,Asset_10,0x0000);
  tft.pushImage(165,200,A13_w,A13_h,Asset_13,0x0000);
  tft.pushImage(201,200,A14_w,A14_h,Asset_14,0x0000);
  tft.setTextDatum(TC_DATUM); 
  tft.pushImage(setting_x, setting_y, settingWidth  ,settingHeight, setting);
}

void draw_time(int time){
    tft.fillRect(0,200,240,30,TFT_NEIGHBOUR_GREEN); 
    tft.setTextColor(beige);
    tft.setTextDatum(TC_DATUM); 
    //tft.drawFloat(time, 0, 120, 200, 6);
}
void draw_wait(void){  
  tft.fillRect(50,210,120,70,TFT_NEIGHBOUR_GREEN); // cover huff now
  tft.setTextDatum(3); 
  tft.setTextColor(beige);
  tft.fillRect(10,setting_y,70,70,TFT_NEIGHBOUR_GREEN); 
  tft.drawString("Analyzing...",10, 290,4);
}

void set_range(int value)
{
  rangeL = value - 1000;
  rangeH = (value + 3000 * 1.3);
  tft.setTextDatum(4);

  tft.setTextColor(TFT_NEIGHBOUR_BEIGE,TFT_NEIGHBOUR_GREEN);
  // tft.fillRect(10,120,240,30,TFT_NEIGHBOUR_GREEN);
  // tft.fillRect(0,40,240,80, TFT_NEIGHBOUR_GREEN);
  tft.drawString("HUFF now",110, setting_y, 4);
}

void draw_sensor(double value){
    // float ADS0 = ads.readADC_SingleEnded(0)
    graph1.pushSprite(20, 40);

    if (i < 201)
    {
      // H[i] = ads.readADC_SingleEnded(0);
      H[i] = (int)value;
      if (numMax < 0){ //relocate maximum point
        max1 = H[0];
        numMax = 0;
        for (int a = 0; a < i; a++){
          if (H[a] > max1){
            max1 = H[a];
            numMax = a;
          }
        }
        HighY = max1 + 200;
        Change = 1;
      }

      if (numMin < 0){ // relocate minimum point
        numMin = 0;
        min1 = H[0];
        for (int a = 0; a < i; a++){
          if (H[a] < min1){
            min1 = H[a];
            numMin = a;
          }
        }
        LowY = min1 - 200;
        Change = 1;
      }

      if (H[i] > HighY){ //update maximum bound
        HighY = H[i] + 200;
        numMax = i;
        Change = 1;
      }

      if (H[i] < LowY){//update minimum bound
        LowY = H[i] - 200;
        numMin = i;
        Change = 1;
      }
      // printf("%d\n",i);
      // printf("%d\n",H[i]);

      // printf("%d\n", as_counter);
      // printf("%d\n", Change);

      // printf("%f\n", ((H[i] - LowY) / (HighY - LowY)));
      // printf("%d\n", max1);
      // printf("%f\n", HighY);
      // tft.fillRect(0, 25, 50, 10, TFT_NEIGHBOUR_GREEN);
      // tft.fillRect(0, 195, 240, 10, TFT_NEIGHBOUR_GREEN);
      // tft.fillRect(45, 215, 40, 15, TFT_NEIGHBOUR_GREEN);//
      // tft.drawFloat(float(HighY), 0, 15, 30, 1);
      // tft.drawFloat(float(LowY), 0, 15, 200, 1);
      // tft.drawFloat(float(ADS0), 0, 65, 220, 2);

      if (Change == 0 && i > 0) // draw
      {
        graph1.scroll(-1);
        // printf("%f\n",value);
        graph1.drawLine(198, 150 - 150 * ((H[i - 1] - LowY) / (HighY - LowY)), 199, 150 - 150 * ((H[i] - LowY) / (HighY - LowY)), TFT_YELLOW);
        // printf("%d\n",150 - 150 * ((H[i] - LowY) / (HighY - LowY)));
      }
      if (Change == 1 && i > 0) // redraw
      {
        graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
        for (int c = 0; c < i; c++)
        {
          // graph1.drawFastVLine(199 - (i  - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)),1, TFT_YELLOW);
          graph1.drawLine(199 - (i - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)), 199 - (i - 1 - c), 150 - 150 * ((H[c+1] - LowY) / (HighY - LowY)), TFT_YELLOW);
        }
        Change = 0;
      }
      if (i == 199)
      { // When i >200, H[i-1] = H[i]
        for (int j = 1; j <= 199; j++)
        {
          H[j - 1] = H[j];
          // printf("%d\n",H[j]);
          // printf("%d\n",j);
        }
        as_counter = 1;
      }

      // if(i == 199){                                  // When i >200, H[i-1] = H[i]
      //   for(int j= 1;j <= 199 ; j++){
      //     H[j-1] = H[j];
      //     // printf("%d\n",H[j]);
      //     // printf("%d\n",j);
      //   }
      //   as_counter = 1;
      // }
      i++;
      numMax--;
      numMin--;
      if (as_counter == 1)
      {
        i = 199;
      }
    }

    // graph1.scroll(-1);                                                                      //AUTO-SCALE
    // graph1.pushSprite(20, 40);
    // graph1.drawFastVLine(199,100-100*((H[i]-LowY)/(HighY-LowY)),3,TFT_YELLOW);

    // if(H[i]>HighY){
    //   graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
    //   HighY = H[i] +5 ;
    //   if(i<199){
    //     int num = i;
    //     while(num >0){

    //       graph1.drawFastVLine(199-num,100-100*((H[i-num]-LowY)/(HighY-LowY)),3,TFT_YELLOW);  //AUTO-SCALE FAIL
    //       num = num -1;
    //     }

    //   }
    // }
    // if (tft.getTouch(&t_x, &t_y))
    // {
    //   if (t_x > 0 && t_x < 35 && t_y > 245 && t_y < 290)
    //   {
    //     break;
    //   }
    // }
  // }
  
}
extern bool isStore;
extern int fail_count;

void draw_result(double ace, double co2){
  tft.fillRect(10,260,150,50,TFT_NEIGHBOUR_GREEN);// cover analyzing
  tft.fillRect(70,200,100,50,TFT_NEIGHBOUR_GREEN);//cover timer
  draw_framework();

  tft.setTextDatum(4); 
  tft.fillRoundRect(10, 263, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Start", 40 ,287,2);        
  if(fail_count != 50){
    tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
    tft.drawString("Acetone", 180, 125, 2);
    tft.drawFloat((float)ace, 2, 180, 140, 2);
    tft.drawString("Metabolic rate", 60, 125, 2);
    tft.drawFloat((float)co2 , 2, 60, 140, 2);
  }
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  // if(ace > 1 || ace <= 0||store == false){
  // tft.drawString("Try Again",120,60,4); 
  // }
  // else if (ace > 0.96 && ace <= 1 ){
  // tft.drawString("Workout More!",120,60,4);
  // }
  // else if(ace > 0.9 && ace <= 0.96){
  // tft.drawString("Pretty Good!",120,60,4);
  // }
  // else if(ace <= 0.9 && ace > 0){
  // tft.drawString("Excellent Fat Burn!",120,60,4);
  // } 
  if(ace < 1 || co2 < 1||isStore == false){
  tft.drawString("Try Again",120,60,4); 
  }
  else if((ace >= 1 && ace < 1.1) && (co2 >= 1 && co2 < 1.3)){
  tft.drawString("Inactive workout",120,60,4);
  } 
  else if((ace >= 1 && ace < 1.2) && (co2 >= 1.3 && co2 < 1.5)){
  tft.drawString("Moderate calories burn",120,60,4);
  } 
  else if((ace >= 1 && ace < 1.3) && (co2 >= 1.5)){
  tft.drawString("Effective training",120,60,4);
  } 
  else if((ace >= 1.2 && ace < 1.3) && (co2 >= 1 && co2 < 1.3 )){
  tft.drawString("Moderate Ketosis",120,60,4);
  } 
  else if(ace >= 1.3 && co2 >= 1 ){
  tft.drawString("Deep Ketosis",120,60,4);
  } 

}

void HomeScreen(){
    tft.pushImage(20,80,BeagleWidth, BeagleHeight, Beagle);
    tft.setTextDatum(0);
    tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
    tft.drawString("King's ",10, setting_y,2);
    tft.drawString("Technologies",10,270,1);
    tft.drawString("Phase",10,280,2);
    draw_framework();

    tft.setTextDatum(4);
    tft.fillRoundRect(95, 265, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
    tft.drawRoundRect(95, 265, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
    tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
    tft.drawString("Start", 125, 288, 2);
    delay(300);
}

void show_menu(){
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.setTextDatum(4);

  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("OTA Setting", 120, 35, 4);

  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Calibration", 120, 100, 4);

  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("User Setup", 120, 160, 4);

  tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Developer Mode", 120, 220, 4);

  tft.fillRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Return", 120, 280, 4);

  delay(300);
  stage = 1;
}

void sampling_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  draw_framework();

  tft.setTextDatum(MC_DATUM); 
  tft.drawString("Acetone Level",120, 30, 4);
  tft.fillRoundRect(10, 263, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);

  tft.drawString("Start", 40,287,2);

  // tft.fillRoundRect(90, 270, 60, 46,23 ,TFT_NEIGHBOUR_BLUE);
  // tft.drawRoundRect(90, 270, 60, 46,23 ,TFT_WHITE);
  // tft.drawString("END", 110,285,2);
  // drawKeypad();

  stage = 2;
  printf("stage2 \n");
}

void calibration_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.pushImage(setting_x, setting_y, settingWidth  ,settingHeight, setting);
  // tft.fillRoundRect(10, 10, 220, 30,15 ,TFT_NEIGHBOUR_BEIGE);
  // tft.drawString("Calibration", 120,25,2);
  // tft.fillRoundRect(10, 10, 220, 44,22 ,TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("Calibration",120,30,4);
  // tft.fillRect(10,setting_y,80,40,TFT_NEIGHBOUR_BLUE);     //Start Button
  // tft.drawString("START", 50,270,2);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(8, 264, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(8, 270, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Start", 38,287,2);
  stage = 3;
}

void OTA_display()
{
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);

  tft.pushImage(setting_x, setting_y, settingWidth  ,settingHeight, setting);
  
  tft.setTextDatum(MC_DATUM); 
  // tft.fillRoundRect(10, 10, 220, 30,15 ,TFT_NEIGHBOUR_BEIGE);
  // tft.drawString("OTA Setting", 120,25,2);
  // tft.fillRoundRect(10, 10, 220, 44,22 ,TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 10, 220, 44,22, TFT_NEIGHBOUR_BLUE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("OTA Setting",120,30,4);

   tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 75, 220, 44,22 , TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22,TFT_NEIGHBOUR_BLUE);
  tft.drawString("Bluetooth",120,100,4);
  
  tft.fillRoundRect(10, 135, 220, 44,22 ,TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44,22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("WiFi",120,160,4);
  
  printf("stage4 \n");

  stage = 4;
}

void  developer_display(){
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.pushImage(setting_x, setting_y, settingWidth  ,settingHeight, setting);
  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("ADS0", 120, 35, 4);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Humidity", 120, 100, 4);
  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Print Spiffs", 120, 160, 4);
  stage=5;
  ResetXY();
}

void bluetooth_display(){
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth  ,settingHeight, setting);
  tft.fillRoundRect(10, 10, 220, 24, 12, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 24, 12, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Bluetooth", 120, 22, 2);
  stage = 8;
}

void User_setup(){
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("UserID", 130, 30, 4);

  tft.fillRect(10, 140, 100, 100, TFT_RED);
  tft.drawRect(10, 140, 100, 100, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.drawString("-", 60, 190, 6);

  tft.fillRect(130, 140, 100, 100, TFT_GREEN);
  tft.drawRect(130, 140, 100, 100, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.drawString("+", 180, 190, 4);

  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(8, 264, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Start", 38,287,2);

  stage = 11;
}


void wifi_display(){
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x,setting_y, settingWidth  ,settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("WiFi Setting",120,30,4);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 75, 220, 44,22 , TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22,TFT_NEIGHBOUR_BLUE);
  tft.drawString("On", 120, 100, 4);
  tft.fillRoundRect(10, 135, 220, 44,22 ,TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44,22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Off", 120, 160, 4);
  stage = 9;
}

void selectfilenumber(){
  if (t_x > 80 && t_x < 150 && t_y > 185 && t_y < 300){
    if (SetupNumber < 10){
      SetupNumber = SetupNumber + 1;
      ResetXY();
      delay(100);
    }
  }
  if (t_x > 70 && t_x < 150 && t_y > 30 && t_y < 150){
    if (SetupNumber > 1){
      SetupNumber = SetupNumber - 1;
      ResetXY();
      delay(100);
    }
  }
}
void TouchScreen(){
  if (stage == 0){
    HomeScreen();
    if (t_x > 15 && t_x < 55 && t_y > 130 && t_y < 210){
      tft.setTextColor(TFT_BLACK, TFT_SKYBLUE);
      tft.fillRoundRect(95, 265, 60, 46, 23, TFT_SKYBLUE);
      tft.drawRoundRect(95, 265, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
      tft.drawString("Start", 125, 288, 2);                                         // Sampling start
      delay(200);
      sampling_display();
    }
  }

  if (tft.getTouch(&t_x, &t_y)){
    printf("%d\n", t_x);
    printf("%d\n", t_y);

    if (stage == 0 || stage == 2 || stage == 3 || stage == 4 || stage == 5 || stage == 6 || stage == 8 || stage == 9 || stage == 10 || stage == 11){
      if (t_x > 10 && t_x < 70 && t_y > 210 && t_y < 300){
        show_menu();
      }
    }

    if (stage == 1){                                                                        // Navigation
      if (t_x > 30 && t_x < 65 && t_y > 10 && t_y < 295){
        tft.setTextColor(TFT_BLACK, TFT_SKYBLUE);
        tft.fillRoundRect(10, 255, 220, 44, 22, TFT_SKYBLUE);
        tft.drawRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Return", 120, 280, 4);
        delay(200); // return button

        stage = 0;
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        ResetXY();
      }
      
      else if (t_x > 110 && t_x < 140 && t_y > 10 && t_y < 295){
        tft.setTextColor(TFT_BLACK, TFT_SKYBLUE);                                               // User_setup
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_SKYBLUE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("User Setup", 120, 160, 4);
        delay(200);

        User_setup();
      }
      else if (t_x > 160 && t_x < 190 && t_y > 10 && t_y < 295){
        tft.setTextColor(TFT_BLACK, TFT_SKYBLUE);
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_SKYBLUE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Calibration", 120, 100, 4);                                               // Calibration
        delay(200);

        calibration_display();
      }

      else if (t_x > 210 && t_x < 235 && t_y > 10 && t_y < 295){
        tft.setTextColor(TFT_BLACK, TFT_SKYBLUE);
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_SKYBLUE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("OTA Setting", 120, 35, 4);                                          // OTA Setting
        delay(200);
        OTA_display();
      }
      else if (t_x > 75 && t_x < 105 && t_y > 10 && t_y < 295){
        tft.setTextColor(TFT_BLACK, TFT_SKYBLUE);
        tft.fillRoundRect(10, 195, 220, 44, 22, TFT_SKYBLUE);
        tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Developer Mode", 120, 220, 4);                                    // developer mode
        delay(200);

        developer_display();
      }
    }


    if(stage == 2){                                                                     //sample
      if(t_x > 20 && t_x < 50  && t_y > 10 && t_y < 70){
        tft.fillRect(0,40,240,150,TFT_NEIGHBOUR_GREEN);
        sample_collection();
        output_result();
      }
    }

    if (stage == 3){                                                                // Calibration Start Button
      if (t_x > 20 && t_x < 50  && t_y > 10 && t_y < 70)
      {
        int baseline = restore_baseline();
        tft.setTextDatum(4);
        tft.fillRect(10,50,200,150,TFT_NEIGHBOUR_GREEN);
        tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
        tft.fillRect(0, 100, 240, 40, TFT_NEIGHBOUR_GREEN);
        tft.drawString("Starting in 3", 120, 120, 4);
        delay(1000);
        tft.fillRect(10, 80, 200, 150, TFT_NEIGHBOUR_GREEN);
        tft.drawString("Starting in 2", 120, 120, 4);
        delay(1000);
        tft.fillRect(10, 80, 200, 150, TFT_NEIGHBOUR_GREEN);
        tft.drawString("Starting in 1", 120, 120, 4);
        delay(1000);
        tft.fillRect(10, 80, 200, 150, TFT_NEIGHBOUR_GREEN);

        calibration();
        tft.fillRect(10, 80, 200, 150, TFT_NEIGHBOUR_GREEN);
        EEPROM.begin(20);
        int value, value_1;
        byte address = 0;
        EEPROM.get(address, value);
        delay(100);
        tft.drawFloat(float(value), 0, 80, 120, 2);
        address += sizeof(int);
        EEPROM.get(address, value_1);
        delay(100);
        tft.drawFloat(float(value_1), 0, 160, 120, 2);
        EEPROM.end();
        delay(500);
      }
    }

    if (stage == 4){                                                                                    // OTA setting options
      if(t_x > 160 && t_x < 195  && t_y >10  && t_y < 295){ // bluetooth
        bluetooth_display();
      }
      if(t_x > 120 && t_x < 145  && t_y >10  && t_y < 295){ // wi-fi
        wifi_display();
      }
    }

    if (stage == 5){    //developer mode                                                                     // developer mode choices
      if (t_x > 210 && t_x < 235 && t_y > 10 && t_y < 295){
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        ResetXY();
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.fillRoundRect(10, 263, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
        tft.drawString("Start", 40,287,2);
        stage = 6;
      }
      if(t_x > 160 && t_x < 195  && t_y >10  && t_y < 295){
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        ResetXY();
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);


        tft.fillRect(10, setting_y, 80, 40, TFT_NEIGHBOUR_BLUE);
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE); // Start Button
        tft.drawString("Start", 38,287,2);
        stage = 7;
      }
      if(t_x > 120 && t_x < 145  && t_y >10  && t_y < 295){
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        tft.pushImage(setting_x, setting_y, settingWidth  ,settingHeight, setting);
        ResetXY();
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("SPIFFS 1", 120, 35, 4);
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("SPIFFS 2", 120, 100, 4);
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Calibration", 120, 160, 4);
        stage =10;
      }
    }

    if (stage == 11){// user_setup

      tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
      tft.fillRect(100, 45, 50, 30, TFT_NEIGHBOUR_GREEN);
      tft.drawFloat(SetupNumber, 0, 120, 70, 4);
      printf("%d\n", SetupNumber);
      
      selectfilenumber();

      if(t_x > 20 && t_x < 50  && t_y > 10 && t_y < 70){    //define file number
        profileNumber = (String)SetupNumber;
      }
    }

    if(stage==10){
      if(t_x > 210 && t_x < 235  && t_y >10  && t_y < 295){
        if(SPIFFS.exists("/Dataset_1")){
            File file = SPIFFS.open("/Dataset_1",FILE_READ);
            while(file.available()){
              Serial.write(file.read());
            }
            file.close();
          }
      }
      if(t_x > 160 && t_x < 195  && t_y >10  && t_y < 295){     
        if(SPIFFS.exists("/Dataset_2")){
          File file = SPIFFS.open("/Dataset_2",FILE_READ);
          while(file.available()){
            Serial.write(file.read());
          }
          file.close();
        }  
      }

      if(t_x > 120 && t_x < 145  && t_y >10  && t_y < 295){
        if(SPIFFS.exists("/Calibration")){
          File file = SPIFFS.open("/Calibration",FILE_READ);
          while(file.available()){
            Serial.write(file.read());
          }
          file.close();
        }
      }
    }


    if (stage == 7)
    {
      // int MaxNum, MinNum;
      tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
      if (t_x > 20 && t_x < 50  && t_y > 10 && t_y < 70)
      {
        // int Change = 0;

        // int max1;
        // int min1;

        // int i = 0;
        // int H[210];
        // float LowY;
        // float HighY;
        // int as_counter = 0;
        // graph1.fillSprite(TFT_NEIGHBOUR_GREEN);

        // while (1)
        // {
        //   // if(tft.getTouch(&t_x, &t_y)){
        //   //   printf("%d\n",t_x);
        //   //   printf("%d\n",t_y);
        //   // }
        //   //
        //   // float ADS0 = ads.readADC_SingleEnded(0);
        //   // float ADS1 = ads.readADC_SingleEnded(1);

        //   // tft.drawString("ADS0:", 25, 200, 2);
        //   // tft.drawString("ADS1:", 110, 200, 2);
        //   // tft.drawString("H:", 200, 200, 2);
        //   // tft.drawFloat(float(ADS0), 0, 65, 200, 2);
        //   // tft.drawFloat(float(ADS1), 0, 150, 200, 2);
        //   // tft.drawFloat(float(H[i]), 0, 220, 200, 2);
        //   // printf("%d\n", H[i]);

        //   // AUTO-SCALE
        //   graph1.pushSprite(20, 40);

        //   if (i < 201)
        //   {
        //     H[i] = sht20.humidity();
        //     if (i == 199)
        //     { // When i >200, H[i-1] = H[i]
        //       for (int j = 1; j <= 199; j++)
        //       {
        //         H[j - 1] = H[j];

        //         // printf("%d\n",H[j]);
        //          printf("%d\n",j);
        //       }
        //       as_counter = 1;
        //     }

        //     for (int a = 0; a <= i; a++)
        //     {
        //       max1 = H[0];
        //       min1 = H[0];
        //       HighY = max1 + 5;
        //       LowY = min1 - 5;
        //       if (H[a] > HighY)
        //       {
        //         max1 = H[a];
        //         Change = 1;
        //       }

        //       if (H[a] < LowY)
        //       {
        //         min1 = H[a];
        //         Change = 1;
        //       }
        //     }

        //     // printf("%d\n",i);
        //     printf("%d\n", H[i]);

        //     // printf("%d\n", as_counter);
        //     // printf("%d\n", Change);
        //     HighY = max1 + 5;
        //     LowY = min1 - 5;


        //     //   if(H[a]<min1){
        //     //     min1 = H[a];
        //     //     Change = 1;
        //     //   }
        //     // }
        //     printf("%d\n",i);
        //     // printf("%d\n",H[i]);
        //     printf("%d\n", max1);
        //     printf("%d\n", as_counter);
        //     printf("%d\n", Change);
        //     // HighY = max1 +2;
        //     // LowY = min1 -2;

        //     // tft.drawFloat(float(HighY), 0, 15, 32, 1);
        //     // tft.drawFloat(float(LowY), 0, 10, 132, 1);

        //     if (Change == 0)
        //     {
        //       graph1.scroll(-1);
        //       graph1.drawFastVLine(199, 100 - 100 * ((H[i] - LowY) / (HighY - LowY)), 3, TFT_YELLOW);
        //       // if(i >10){
        //       //   graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
        //       //   i = 0;
        //       // }
        //     }
        //     if (Change == 1)
        //     {
        //       graph1.fillSprite(TFT_NEIGHBOUR_GREEN);

        //       for (int c = 0; c <= i; c++)
        //       {
        //         graph1.drawFastVLine(199 - (i - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)), 3, TFT_YELLOW);
        //       }

        //       Change = 0;
        //     }
        //     i++;

        //     if (as_counter == 1)
        //     {
        //       i = 199;

        //     }
        //   }

        //   // if(MaxNum<0){                                   //find the max when MaxNum <0
        //   //   H[0] = max;
        //   //   for(int a = 0; a++; a == i){
        //   //     if(H[a]> max){
        //   //       max = H[a];
        //   //       MaxNum = a;
        //   //     }
        //   //   }
        //   // }

        //   // if(MinNum<0){                                   //find the min when MinNum <0
        //   //   H[0] = min;
        //   //   for(int b = 0; b++; b == i){
        //   //     if(H[b]< min){
        //   //       min = H[b];
        //   //       MinNum = b;
        //   //     }
        //   //   }

        //   // }

        //   if (tft.getTouch(&t_x, &t_y))
        //   {
        //     if (t_x > 0 && t_x < 35 && t_y > 245 && t_y < 290)
        //     {
        //       break;
        //     }
        //   }
        // }
      }
    }

    if (stage == 6){ // developer mode stage6 = ADS0  Stage7 = Humidity
      PID_control();
      // int Change = 0;
      // float max1;
      // float min1;

      // int i = 0;
      // float H[210];
      // float LowY;
      // float HighY;
      // int as_counter = 0;
      // int as_counter1 = 0;
      // int numMax = -1;
      // int numMin = -1;
      tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);

      if (t_x > 22 && t_x < 47 && t_y > 13 && t_y < 108)
      {
        graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
        // int Time = millis();
        while (1){
          float ADS0 = ads.readADC_SingleEnded(0);
          // float ADS1 = ads.readADC_SingleEnded(1);
          tft.drawString("ADS0:", 25, 220, 2);
          // tft.drawString("ADS1:", 110, 220, 2);
          tft.drawString("H:", 200, 220, 2);
          // tft.drawFloat(float(ADS1), 0, 150, 220, 2);
          tft.drawFloat(float(sht20.humidity()), 0, 220, 220, 2);

          graph1.pushSprite(20, 40);

          if (i < 201)
          {

            H[i] = ads.readADC_SingleEnded(0);

            if (numMax < 0)
            { //relocate maximum point
              max1 = H[0];
              numMax = 0;
              for (int a = 0; a < i; a++)
              {

                if (H[a] > max1)
                {
                  max1 = H[a];
                  numMax = a;
                }
              }
              HighY = max1 + 200;
              Change = 1;
            }

            if (numMin < 0)
            { // relocate minimum point
              numMin = 0;
              min1 = H[0];
              for (int a = 0; a < i; a++)
              {
                if (H[a] < min1)
                {
                  min1 = H[a];
                  numMin = a;
                }
              }
              LowY = min1 - 200;
              Change = 1;
            }

            if (H[i] > HighY)
            {
              HighY = H[i] + 200;
              numMax = i;
              Change = 1;
            }

            if (H[i] < LowY)
            {
              LowY = H[i] - 200;
              numMin = i;
              Change = 1;
            }
            // printf("%d\n",i);
            // printf("%d\n",H[i]);

            // printf("%d\n", as_counter);
            // printf("%d\n", Change);

            // printf("%f\n", ((H[i] - LowY) / (HighY - LowY)));
            // printf("%d\n", max1);
            printf("%f\n", HighY);
            tft.fillRect(0, 25, 50, 10, TFT_NEIGHBOUR_GREEN);
            tft.fillRect(0, 195, 240, 10, TFT_NEIGHBOUR_GREEN);
            tft.fillRect(45, 215, 40, 15, TFT_NEIGHBOUR_GREEN);
            tft.drawFloat(float(HighY), 0, 15, 30, 1);
            tft.drawFloat(float(LowY), 0, 15, 200, 1);
            tft.drawFloat(float(ADS0), 0, 65, 220, 2);

            if (Change == 0 && i > 0) // draw
            {
              graph1.scroll(-1);
              // printf("%f\n",value);
              graph1.drawLine(198, 150 - 150 * ((H[i - 1] - LowY) / (HighY - LowY)), 199, 150 - 150 * ((H[i] - LowY) / (HighY - LowY)), TFT_YELLOW);
              // printf("%d\n",150 - 150 * ((H[i] - LowY) / (HighY - LowY)));
            }
            if (Change == 1 && i > 0) // redraw
            {
              graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
              for (int c = 0; c < i; c++)
              {
                // graph1.drawFastVLine(199 - (i  - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)),1, TFT_YELLOW);
                graph1.drawLine(199 - (i - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)), 199 - (i - 1 - c), 150 - 150 * ((H[c+1] - LowY) / (HighY - LowY)), TFT_YELLOW);
              }
              Change = 0;
            }
            if (i == 199)
            { // When i >200, H[i-1] = H[i]
              for (int j = 1; j <= 199; j++)
              {
                H[j - 1] = H[j];
                // printf("%d\n",H[j]);
                // printf("%d\n",j);
              }
              as_counter = 1;
            }

            // if(i == 199){                                  // When i >200, H[i-1] = H[i]
            //   for(int j= 1;j <= 199 ; j++){
            //     H[j-1] = H[j];
            //     // printf("%d\n",H[j]);
            //     // printf("%d\n",j);
            //   }
            //   as_counter = 1;
            // }
            i++;
            numMax--;
            numMin--;
            if (as_counter == 1)
            {
              i = 199;
            }
          }

          // graph1.scroll(-1);                                                                      //AUTO-SCALE
          // graph1.pushSprite(20, 40);
          // graph1.drawFastVLine(199,100-100*((H[i]-LowY)/(HighY-LowY)),3,TFT_YELLOW);

          // if(H[i]>HighY){
          //   graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
          //   HighY = H[i] +5 ;
          //   if(i<199){
          //     int num = i;
          //     while(num >0){

          //       graph1.drawFastVLine(199-num,100-100*((H[i-num]-LowY)/(HighY-LowY)),3,TFT_YELLOW);  //AUTO-SCALE FAIL
          //       num = num -1;
          //     }

          //   }
          // }
          if (tft.getTouch(&t_x, &t_y))
          {
            if (t_x > 0 && t_x < 35 && t_y > 245 && t_y < 290)
            {
              break;
            }
          }
        }
      }
    }

    if (stage == 9){ 
      if (t_x > 160 && t_x < 195  && t_y >10  && t_y < 295){ // WIFI
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        extern bool isWifi;
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.fillRoundRect(10, 10, 220, 24, 12, TFT_NEIGHBOUR_BEIGE);
        tft.drawRoundRect(10, 10, 220, 24, 12, TFT_NEIGHBOUR_BEIGE);
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
        tft.drawString("Wi-fi", 120, 22, 2);
        Wifi_able();
        if(isWifi == true){
          tft.drawString("ON", 180, 22, 2);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          stage=0;
        }
        else{
          tft.drawString("Failed", 180, 22, 2);
          wifi_display();
        }
      }

      if (t_x > 120 && t_x < 145  && t_y >10  && t_y < 295){
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        ResetXY();
        extern bool isWifi;
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.fillRoundRect(10, 10, 220, 24, 12, TFT_NEIGHBOUR_BEIGE);
        tft.drawRoundRect(10, 10, 220, 24, 12, TFT_NEIGHBOUR_BEIGE);
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
        tft.drawString("Wi-fi", 120, 22, 2);
        Wifi_disable();
        if(isWifi == false){
          tft.drawString("OFF" , 180, 22, 2);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          stage = 0;
        }
        else
        {
          tft.drawString("Failed", 180, 22, 2);
        }
      }
    }
  }
}
