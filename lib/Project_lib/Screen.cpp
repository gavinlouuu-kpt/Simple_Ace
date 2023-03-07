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
#include "PID.h"
#include "History_Data.h"
#include "Cloud_storage.h"
#include "WiFi.h"
#include "Free_Fonts.h"

#include "Image_assets/Wifi_connect.h"
#include "Image_assets/Wifi_not_connect.h"
#include "Image_assets/Wifi_Off.h"
#include "Image_assets/Loading.h"

#include "Image_assets/Homepage2.h"
#include "Image_assets/FullBattary.h"
#include "Image_assets/Setting2.h"
#include "Image_assets/WifiButtonOn.h"
#include "Image_assets/WifiButtonOff.h"
#include "Image_assets/BreatheHere.h"
// #include "Image_assets/Analyzing.h"
#include "Image_assets/Bar.h"
#include "Image_assets/Breathe.h"
#include "Image_assets/Beagle.h"
#include "Image_assets/Started.h"
#include "Image_assets/SettingBar.h"
#include "Image_assets/DeveloperMode.h"
#include "Image_assets/LivePlot.h"
#include "Image_assets/DefaultSetting.h"
#include "Image_assets/Spiffs.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph1 = TFT_eSprite(&tft);
extern Adafruit_ADS1115 ads;
extern uFire_SHT20 sht20;
extern float ref_position[2];
// float previous_data[10] = {0.9,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2};
extern double previous_data[10];
String profileNumber = "1";

extern const char *ntpServer;

int rangeL = 0;
int rangeH = 8000;
int stage = 0;
uint16_t beige = tft.color565(239, 227, 214);
uint16_t t_x = 0, t_y = 0;

int SetupNumber = 1;
extern int dutyCycle_pump;
extern double Setpoint;
extern bool control;
extern bool isWifi;
extern bool isConnect;
bool isSensor = true;
const int offset = 10;
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
int WifiButton = 0;

void tft_setup()
{
  tft.init();
  tft.fillScreen(TFT_MilkWhite);
  tft.setSwapBytes(true);
  tft.setRotation(2);

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
  tft.fillScreen(TFT_MilkWhite);
  tft.pushImage(0, 280, BarWidth, BarHeight, Bar);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  draw_Wifi();
}

void draw_Settingframework()
{
  tft.fillScreen(TFT_MilkWhite);
  tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  draw_Wifi();
}

void draw_loading(int count)
{
  tft.fillRect(90, 200, 70, 70, TFT_MilkWhite); //  cover loading logo
  tft.pushImage(90, 200, LoadingWidth, LoadingHeight, Loading[count % 11]);
  delay(100);
}

void draw_progress(float bar_length, float bar_percentage)
{
  tft.setTextDatum(1);
  tft.fillRoundRect(15, 210, 200 * (bar_length / 45000), 5, 2, TFT_NEIGHBOUR_BEIGE); // bar
  if ((int)(bar_percentage * 10) % 10 == 0)
  {
    tft.fillRect(75, 230, 60, 25, TFT_NEIGHBOUR_GREEN); // cover previous_data number
    tft.drawFloat(bar_percentage, 0, 120, 230, 4);
  }
  tft.drawString("%", 155, 230, 4);
}

void draw_wait(void)
{
  tft.fillRect(60, 210, 119, 62, TFT_NEIGHBOUR_GREEN); // cover huff now
  tft.setTextDatum(3);
  tft.setTextColor(beige);
  tft.fillRect(10, 260, 70, 50, TFT_NEIGHBOUR_GREEN); // cover button
  tft.drawString("Analyzing...", 10, 280, 4);
  tft.fillRect(0, 200, 240, 33, TFT_NEIGHBOUR_GREEN); // cover bubbles
}

void draw_sensor(double value)
{
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  graph1.pushSprite(20, 40);

  if (i < 201)
  {
    // H[i] = ads.readADC_SingleEnded(0);
    H[i] = (int)value;
    if (numMax < 0)
    { // relocate maximum point
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
    { // update maximum bound
      HighY = H[i] + 200;
      numMax = i;
      Change = 1;
    }

    if (H[i] < LowY)
    { // update minimum bound
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
        graph1.drawLine(199 - (i - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)), 199 - (i - 1 - c), 150 - 150 * ((H[c + 1] - LowY) / (HighY - LowY)), TFT_YELLOW);
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

void draw_bar(double bar_1, double bar_2)
{
  const int top_y = 60;
  const int base_y = 190;
  int y_length = base_y - top_y;
  int start_y_ace;
  int end_y_ace;
  int start_y_co2;
  int end_y_co2;
  start_y_co2 = top_y + (int)(y_length * (1 - (bar_1 / 2)));
  end_y_co2 = base_y - start_y_co2;
  start_y_ace = top_y + (int)(y_length * (1 - bar_2 / 2));
  end_y_ace = base_y - start_y_ace;
  // Serial.print("start_y_co2:");Serial.println(start_y_co2);
  // Serial.print("end_y_co2:");Serial.println(end_y_co2);
  // Serial.print("start_y_ace:");Serial.println(start_y_ace);
  // Serial.print("end_y_ace:");Serial.println(end_y_ace);

  tft.drawFastHLine(20, 190, 200, TFT_NEIGHBOUR_BEIGE);
  int slice = 10;
  unsigned long previous_data = 0;
  tft.drawRect(50, top_y, 20, y_length, TFT_NEIGHBOUR_BEIGE);
  tft.drawRect(160, top_y, 20, y_length, TFT_NEIGHBOUR_BEIGE);
  for (int i = 0; i < slice; i++)
  {
    while (millis() - previous_data < 150)
    {
    }
    previous_data = millis();
    // Serial.println("extending");
    tft.fillRect(50, 190 - (int)((i + 1) * (end_y_co2 / slice)), 20, (int)(end_y_co2 / slice), TFT_NEIGHBOUR_BEIGE);
    tft.fillRect(160, 190 - (int)((i + 1) * (end_y_ace / slice)), 20, (int)(end_y_ace / slice), TFT_NEIGHBOUR_BEIGE);
  }
}

void draw_start_button()
{
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
  tft.drawString("START",97,235,2);
}
extern bool isStore;
extern int fail_count;

void draw_result(double co2, double ace)
{
  tft.fillRect(0, 20, 240, 170, TFT_NEIGHBOUR_GREEN);  // cover graph
  tft.fillRect(10, 260, 150, 50, TFT_NEIGHBOUR_GREEN); // cover analyzing
  tft.fillRect(0, 200, 240, 50, TFT_NEIGHBOUR_GREEN);
  // draw_framework();
  tft.fillRect(0, 200, 240, 33, TFT_NEIGHBOUR_GREEN); // cover logo

  tft.setTextDatum(4);
  draw_start_button();
  if (fail_count != 50)
  {
    draw_bar(co2, ace);
    delay(500);
    tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
    tft.drawString("Acetone", 170, 205, 2);
    tft.drawFloat((float)ace, 2, 170, 220, 2);
    tft.drawString("Metabolic rate", 60, 205, 2);
    tft.drawFloat((float)co2, 2, 60, 220, 2);
  }
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);

  if (ace < 1 || co2 < 1 || isStore == false)
  {
    tft.drawString("Try Again", 120, 40, 4);
  }
  else if ((ace >= 1 && ace < 1.2) && (co2 >= 1 && co2 < 1.3))
  {
    tft.drawString("Inactive workout", 120, 40, 4);
  }
  else if ((ace >= 1 && ace < 1.2) && (co2 >= 1.3 && co2 < 1.5))
  {
    tft.drawString("Moderate burn", 120, 40, 4);
  }
  else if ((ace >= 1.2 && ace < 1.3) && (co2 >= 1.3 && co2 < 1.5))
  {
    tft.drawString("Effective training", 120, 40, 4);
  }
  else if ((ace >= 1.2 && ace < 1.3) && (co2 >= 1.5))
  {
    tft.drawString("Intensive training", 120, 40, 4);
  }

  else if ((ace >= 1.2 && ace < 1.3) && (co2 >= 1 && co2 < 1.3))
  {
    tft.drawString("Moderate Ketosis", 120, 40, 4);
  }
  else if (ace >= 1.3 && co2 >= 1)
  {
    tft.drawString("Deep Ketosis", 120, 40, 4);
  }
}
extern unsigned long previous_sensor_time;
void HomeScreen()
{
  //tft.setFreeFont(FMB9);
  tft.setTextColor(TFT_WHITE,TFT_NEIGHBOUR_GREEN);
  tft.fillScreen(TFT_MilkWhite);
  // tft.pushImage(0, 0, Homepage2Width, Homepage2Height, Homepage2);
  tft.pushImage(0, 280, BarWidth, BarHeight, Bar);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(20, 230, BreatheWidth, BreatheHeight, Breathe);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
  tft.drawString("BREATH",97,235,2);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);

  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_MilkWhite);
  tft.drawString("Lets Get Started", 15, 50, 4);
  // tft.pushImage(15, 50, StartedWidth, StartedHeight, Started);

  // tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_MilkWhite);
  // tft.drawString("Let's get started",10,60,4);

  // tft.pushImage(20, 80, BeagleWidth, BeagleHeight, Beagle);
  // tft.setTextDatum(ML_DATUM);
  // tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  // tft.drawString("King's Phase ", 10, 270, 2);
  // tft.drawString("Technologies", 10, 290, 1);
  // tft.drawString("Phase", 10, 280, 2);
  // draw_framework();
  draw_Wifi();
  // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  // tft.setTextDatum(4);
  // tft.fillRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Start", 125, 280, 2);

  // timeot
  // press button
  // write new store time into spiff

  // delay(300);
}

void show_menu()
{
  ResetXY();
  tft.fillScreen(TFT_MilkWhite);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  draw_Wifi();
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_MilkWhite);
  tft.drawString("Setting", 15, 50, 4);
  //tft.pushImage(0, 0, Setting2Width, Setting2Height, Setting2);
  tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
  tft.fillRoundRect(15,100,210,30,3,TFT_PaleYellow);
  tft.drawString("Wifi",30,107,2);

  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  tft.fillRoundRect(15,140,210,30,3,TFT_DARKGREY);
  tft.drawString("Calibration",30,147,2);
  tft.fillRoundRect(15,170,210,30,3,TFT_DARKGREY);
  tft.drawString("User ID",30,177,2);
  tft.fillRoundRect(15,200,210,30,3,TFT_DARKGREY);
  tft.drawString("Language",30,207,2);
  tft.setTextColor(TFT_BLACK,TFT_PaleYellow);

  tft.fillRoundRect(15,240,210,30,3,TFT_PaleYellow);
  tft.drawString("Developer Mode",30,247,2);

  tft.drawRoundRect(15,100,210,30,3,TFT_WHITE);
  tft.drawRoundRect(15,140,210,30,3,TFT_WHITE);
  tft.drawRoundRect(15,170,210,30,3,TFT_WHITE);
  tft.drawRoundRect(15,200,210,30,3,TFT_WHITE);
  tft.drawRoundRect(15,240,210,30,3,TFT_WHITE);
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  // tft.setTextDatum(4);

  // tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("OTA Setting", 120, 35, 4);

  // tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Calibration", 120, 100, 4);

  // tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("User Setup", 120, 160, 4);

  // tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Developer Mode", 120, 220, 4);

  // tft.fillRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Return", 120, 280, 4);

  delay(300);
}

void draw_Wifi()
{ // draw wifi logo
  if (isWifi == true && isConnect == true)
  {
    tft.pushImage(170, 8, Wifi_true_w, Wifi_true_h, Wifi_true);
  }
  else if (isWifi == true && isConnect == false)
  {
    tft.pushImage(170, 8, Wifi_nc_w, Wifi_nc_h, Wifi_nc);
  }
  else
  {
    tft.pushImage(170, 8, Wifi_false_w, Wifi_false_h, Wifi_false);
  }
}
void sampling_display()
{
  ResetXY();
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // delay(200);
  // draw_framework();
  // draw_start_button();

  // tft.fillRoundRect(90, 270, 60, 46,23 ,TFT_NEIGHBOUR_BLUE);
  // tft.drawRoundRect(90, 270, 60, 46,23 ,TFT_NEIGHBOUR_BEIGE);
  // tft.drawString("END", 110,285,2);
  // drawKeypad();

  //tft.pushImage(0, 0, BreatheHereWidth, BreatheHereHeight, BreatheHere);
  draw_framework();
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_MilkWhite);
  tft.drawString("Breathe Here", 15, 50, 4);
  tft.setTextColor(TFT_WHITE,TFT_NEIGHBOUR_GREEN);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
  tft.drawString("START",100,235,2);

  delay(300);

  printf("stage2 \n");
}

void calibration_display()
{
  ResetXY();
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  draw_Settingframework();
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Calibration", 15, 50, 4);
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
  tft.drawString("START",97,235,2);
  //draw_start_button();
}

void OTA_display()
{
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("OTA Setting", 120, 30, 4);

  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Bluetooth", 120, 100, 4);

  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("WiFi", 120, 160, 4);

  printf("stage4 \n");
}

void developer_display()
{
  ResetXY();
  draw_Settingframework();
  tft.pushImage(0, 100, DeveloperModeWidth, DeveloperModeHeight, DeveloperMode);

  tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
  tft.fillRoundRect(15,105,210,30,3,TFT_PaleYellow);
  tft.drawString("Live Plot",30,112,2);
  
  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  tft.fillRoundRect(15,145,210,30,3,TFT_DARKGREY);
  tft.drawString("Default Setting",30,152,2);

  tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
  tft.fillRoundRect(15,185,210,30,3,TFT_PaleYellow);
  tft.drawString("Print Spiffs",30,192,2);

  tft.fillRoundRect(15,225,210,30,3,TFT_PaleYellow);
  tft.drawString("Previous Value",30,232,2);

  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Developer Mode", 15, 50, 4);
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  // // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  // tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Live Plot", 120, 35, 4);
  // tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Default Setting", 120, 100, 4);
  // tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Print Spiffs", 120, 160, 4);
  // tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Previous Value", 120, 220, 4);
}

void live_plot_display()
{
  draw_Settingframework();
  ResetXY();
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Live Plot", 15, 50, 4);

  tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
  tft.fillRoundRect(15,100,210,30,3,TFT_PaleYellow);
  tft.drawString("ADS0",30,107,2);
  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  tft.fillRoundRect(15,140,210,30,3,TFT_DARKGREY);
  tft.drawString("Humidity",30,147,2);

  // tft.pushImage(0, 100, LivePlotWidth, LivePlotHeight, LivePlot);

  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  //   tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  //   tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  //   tft.drawString("ADS0", 120, 35, 4);

  //   tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  //   tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  //   tft.drawString("Humidity", 120, 100, 4);
}

void bluetooth_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("Bluetooth", 120, 30, 4);
}

void User_setup_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(4);
  tft.drawString("UserID", 120, 40, 4);

  tft.fillRect(10, 140, 100, 100, TFT_RED);
  tft.drawRect(10, 140, 100, 100, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_RED);
  tft.drawString("-", 60, 190, 8);

  tft.fillRect(130, 140, 100, 100, TFT_GREEN);
  tft.drawRect(130, 140, 100, 100, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_GREEN);
  tft.drawString("+", 180, 190, 4);

  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Set", 38, 287, 2);
}

void Pump_setup_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(4);
  tft.drawString("Pump Power", 120, 40, 4);

  tft.fillRect(10, 140, 100, 100, TFT_RED);
  tft.drawRect(10, 140, 100, 100, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_RED);
  tft.drawString("-", 60, 190, 8);

  tft.fillRect(130, 140, 100, 100, TFT_GREEN);
  tft.drawRect(130, 140, 100, 100, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_GREEN);
  tft.drawString("+", 180, 190, 4);

  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Set", 38, 287, 2);
}

void PID_setup_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(4);
  tft.drawString("PID Setpoint", 120, 40, 4);

  tft.fillRect(10, 140, 100, 100, TFT_RED);
  tft.drawRect(10, 140, 100, 100, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_RED);
  tft.drawString("-", 60, 190, 8);

  tft.fillRect(130, 140, 100, 100, TFT_GREEN);
  tft.drawRect(130, 140, 100, 100, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_GREEN);
  tft.drawString("+", 180, 190, 4);

  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Set", 38, 287, 2);
}

void wifi_display()
{
  ResetXY();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("WiFi Setting", 120, 30, 4);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("On", 120, 100, 4);
  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Off", 120, 160, 4);
  ResetXY();
  delay(300);
}

void default_setting_display()
{
  // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  ResetXY();
  draw_Settingframework;
  draw_Wifi();
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Default Setting", 15, 50, 4);
  tft.pushImage(0, 100, DefaultSettingWidth, DefaultSettingHeight, DefaultSetting);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  // tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Pump power", 120, 35, 4);

  // tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Column Temp", 120, 100, 4);

  // tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Sample Time", 120, 160, 4);

  // tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  // tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  // tft.drawString("Update sensor", 120, 220, 4);
}

void Spiffs_display()
{
  ResetXY();
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  draw_Settingframework();
  tft.drawString("Spiffs", 15, 50, 4);

  tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
  tft.fillRoundRect(15,105,210,30,3,TFT_PaleYellow);
  tft.drawString("SPIFFS 1",30,112,2);
  
  tft.fillRoundRect(15,145,210,30,3,TFT_PaleYellow);
  tft.drawString("SPIFFS 2",30,152,2);

  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  tft.fillRoundRect(15,185,210,30,3,TFT_DARKGREY);
  tft.drawString("Calibration",30,192,2);

  
  // tft.pushImage(0, 100, SpiffsWidth, SpiffsHeight, Spiffs);
//   tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
//   tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
//   tft.drawString("SPIFFS 1", 80, 35, 4);
//   tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
//   tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
//   tft.drawString("SPIFFS 2", 80, 100, 4);
//   tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
//   tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
//   tft.drawString("Calibration", 80, 160, 4);
}
void selectfilenumber()
{
  if (t_x > 110 && t_x < 190 && t_y > 0 && t_y < 145)
  {
    if (SetupNumber < 10)
    {
      SetupNumber = SetupNumber + 1;
      ResetXY();
      delay(150);
    }
  }
  else if (t_x > 110 && t_x < 190 && t_y > 160 && t_y < 300)
  {
    if (SetupNumber > 1)
    {
      SetupNumber = SetupNumber - 1;
      ResetXY();
      delay(150);
    }
  }
}

void select_pump_dutycycle()
{
  if (t_x > 110 && t_x < 190 && t_y > 0 && t_y < 145)
  {
    if (dutyCycle_pump < 255)
    {
      dutyCycle_pump = dutyCycle_pump + 5;
      ResetXY();
      delay(150);
    }
  }
  else if (t_x > 110 && t_x < 190 && t_y > 160 && t_y < 300)
  {
    if (dutyCycle_pump > 0)
    {
      dutyCycle_pump = dutyCycle_pump - 5;
      ResetXY();
      delay(150);
    }
  }
}

void select_PID_setpoint()
{
  if (t_x > 110 && t_x < 190 && t_y > 0 && t_y < 145)
  {
    if (Setpoint < 1300)
    {
      Setpoint = Setpoint + 50;
      ResetXY();
      delay(150);
    }
  }
  else if (t_x > 110 && t_x < 190 && t_y > 160 && t_y < 300)
  {
    if (Setpoint > 0)
    {
      Setpoint = Setpoint - 50;
      ResetXY();
      delay(150);
    }
  }
}

void TouchScreen()
{
  if (isSensor != true)
  {
    tft.fillScreen(TFT_NEIGHBOUR_GREEN);
    tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE);
    tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
    tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
    tft.drawString("Change sensor", 120, 160, 4);
    stage = 99;
  }

  while (stage == 99)
  {
    if (tft.getTouch(&t_x, &t_y))
    {
      if (t_x > 105 && t_x < 145 && t_y > 0 && t_y < 305)
      {
        tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_GREEN);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
        tft.drawString("Change sensor", 120, 160, 4);
        update_sensor();
        update_check_time();
        isSensor = true;
        stage = 0;
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        HomeScreen();
        break;
      }
    }
  }

  if (stage == 0) // press Breathe
  {
    if (t_x > 180 && t_x < 200 && t_y > 5 && t_y < 200)
    {
      // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
      // tft.fillRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
      // tft.drawRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
      // tft.drawString("Start", 125, 280, 2);
      sampling_display();
      stage = 2;
    }
  }

  if (tft.getTouch(&t_x, &t_y))
  {
    printf("%d\n", t_x);
    printf("%d\n", t_y);

    if (stage != 1)
    {
      // if (t_x > 195 && t_x < 235 && t_y > 5 && t_y < 75)
      if (t_x > 220 && t_x < 240 && t_y > 0 && t_y < 85) // press
      {
        pump_control(false);
        Serial.println("OFF");
        show_menu();
        stage = 1;
      }
      if (t_x > 220 && t_x < 240 && t_y > 220 && t_y < 320) // Return
      {
        stage = 0;
        HomeScreen();
        ResetXY();
      }
    }

    if (stage == 1)
    { // Navigation
      if (t_x > 85 && t_x < 105 && t_y > 10 && t_y < 300)                //OTA
      {
        tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_MilkWhite);
        draw_Settingframework();
        // tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Wifi", 15, 50, 4); // OTA Setting
        tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
        tft.fillRoundRect(15,100,210,30,3,TFT_PaleYellow);
        tft.drawString("On",30,107,2);
        tft.setTextColor(TFT_BLACK,TFT_PaleYellow);
        tft.fillRoundRect(15,140,210,30,3,TFT_PaleYellow);
        tft.drawString("OFF",30,147,2);
        

        delay(200);
        ResetXY();
        // OTA_display();
        stage = 9;
        
      }

      // else if (t_x > 60 && t_x < 100 && t_y > 0 && t_y < 305)               //Calibration
      // {
      //   tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
      //   tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   tft.drawString("Calibration", 120, 100, 4); // Calibration
      //   delay(200);

      //   calibration_display();
      //   stage = 3;
      // }
      // else if (t_x > 105 && t_x < 145 && t_y > 0 && t_y < 305)              //User Set up
      // {
      //   tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);                    // User_setup_display
      //   tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   tft.drawString("User Setup", 120, 160, 4);
      //   delay(200);

      //   User_setup_display();
      //   stage = 11;
      // }

      if (t_x > 185 && t_x < 200 && t_y > 10 && t_y < 280)
      {
        // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        // tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawString("Developer Mode", 120, 220, 4); // developer mode
        // delay(200);
        developer_display();
        stage = 5;
        delay(300);
      }
      // else if (t_x > 195 && t_x <235 && t_y > 0 && t_y < 305)

      // if (t_x > 90 && t_x < 100 && t_y > 10 && t_y < 55 && WifiButton == 0) // WIFI ON
      // {
      //   tft.pushImage(178, 99, WifiButtonOnWidth, WifiButtonOnHeight, WifiButtonOn);
      //   WifiButton = 1;
      //   delay(400);
      //   ResetXY();
      // }
      // if (t_x > 90 && t_x < 100 && t_y > 10 && t_y < 55 && WifiButton == 1) // WIFI ON
      // {
      //   tft.pushImage(178, 99, WifiButtonOffWidth, WifiButtonOffHeight, WifiButtonOff);
      //   WifiButton = 0;
      //   delay(400);
      //   ResetXY();
      // }

      // if (t_x > 109 && t_x < 129 && t_y > 10 && t_y < 290) // Calibration
      // {
      //   calibration_display();
      //   stage = 3;
      // }
      if (t_x > 220 && t_x < 240 && t_y > 220 && t_y < 320) // Return
      {
        // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        // tft.fillRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);        //change colour
        // tft.drawString("Return", 120, 280, 4);
        // delay(200); // return button

        stage = 0;
        // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        HomeScreen();
        ResetXY();
      }
    }

    if (stage == 2)
    {                                                     // sample
                                                          // if (t_x > 195 && t_x < 240 && t_y > 220 && t_y < 305)
      if (t_x > 180 && t_x < 200 && t_y > 5 && t_y < 200) // press start after breathe
      {
        // if (isConnect == true)
        // {
        //   WiFi.disconnect(true, true);
        //   delay(500);
        //   isConnect = false;
        //   Serial.println("Disconnected");
        //   Serial.print("Wifi status:");
        //   Serial.println(WiFi.status());
        // }

        // tft.fillRect(0, 20, 240, 210, TFT_NEIGHBOUR_GREEN); // cover previosu result screeen                               //cover result
        // draw_framework();

        // tft.pushImage(0,0,AnalyzingWidth,AnalyzingHeight,Analyzing);
        draw_framework();
        tft.setTextColor(TFT_NEIGHBOUR_GREEN);
        tft.drawString("Analyzing", 15, 50, 4);
        sample_collection();
        output_result();
      }
    }

    if (stage == 3)
    {                                                     // Calibration Start Button
      if (t_x > 180 && t_x < 200 && t_y > 5 && t_y < 200) // press start after breathe
      {
        draw_Settingframework();
        tft.setTextColor(TFT_NEIGHBOUR_GREEN);
        tft.drawString("Analyzing", 15, 50, 4);
        restore_baseline();
        tft.setTextDatum(4);
        tft.fillRect(10, 50, 200, 150, TFT_NEIGHBOUR_GREEN); //
        // tft.fillRect(0, 100, 240, 40, TFT_NEIGHBOUR_GREEN);       //
        tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
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

    if (stage == 4)
    { // OTA setting options
      // if (t_x > 60 && t_x < 100 && t_y > 0 && t_y < 305)
      // { // bluetooth
      //   bluetooth_display();
      //   stage = 8;
      // }
      //  if (t_x > 105 && t_x < 145 && t_y > 0 && t_y < 305)
      // { // wi-fi
      //   wifi_display();
      //   stage = 9;
      // }
    }

    if (stage == 5)
    { // developer mode                                                                     // developer mode choices
      tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
      if (t_x > 85 && t_x < 105 && t_y > 10 && t_y < 285)
      {
        // tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawString("Live Plot", 120, 35, 4);
        // delay(200);
        live_plot_display();
        stage = 6;
        delay(300);
      }
      // else if (t_x > 120 && t_x < 135 && t_y > 10 && t_y < 285)
      // {
      //   // tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   // tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   // tft.drawString("Default Setting", 120, 100, 4);
      //   delay(200);
      //   default_setting_display();
      //   stage = 7;
      //   delay(300);
      // }

      else if (t_x > 150 && t_x < 165 && t_y > 10 && t_y < 285)
      {
        // tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawString("Print Spiffs", 120, 160, 4);
        delay(200);
        tft.fillScreen(TFT_MilkWhite);
        tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
        // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        ResetXY();
        Spiffs_display();
        stage = 10;
      }
      else if (t_x > 180 && t_x < 195 && t_y > 10 && t_y < 285)
      {
        // tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawString("Previous Value", 120, 220, 4);
        delay(200);
        int DataCounter = 0;
        tft.fillScreen(TFT_MilkWhite);
        tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
        tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_MilkWhite);
        tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
        tft.drawString("Previous Value", 15, 50, 4);
        // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        ResetXY();
        // tft.drawFastVLine(20, 60, 120, TFT_NEIGHBOUR_GREEN);
        // tft.drawFastHLine(20, 180, 200, TFT_NEIGHBOUR_GREEN);
        retrieve_result();
        for (int i = 0; i < 10; i++)
        {
          if (previous_data[i] > 0.5) // what is 0.5
          {
            Serial.print("plotting: ");
            Serial.println(previous_data[i]);
            tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_MilkWhite);
            tft.drawNumber(i+1, 20, 80+10 *(i+1));
            tft.drawFloat(previous_data[i],2,40,80+10*(i+1));
            // tft.fillCircle((i + 2) * 20, (120 - 120 * ((previous_data[i] - 0.9) / 1.1)) + 60, 2, TFT_NEIGHBOUR_GREEN);
            // tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_MilkWhite);
            // // if (i == 0 || i == 2 || i == 4 || i == 6 || i == 8)
            // if (i % 2 == 0)
            // {
            //   tft.drawFloat(previous_data[i], 2, (i + 2) * 20, (120 - 120 * ((previous_data[i] - 0.9) / 1.1)) + 40, 1);
            // }
            // // if (i == 1 || i == 3 || i == 5 || i == 7 || i == 9)
            // else
            // {
            //   tft.drawFloat(previous_data[i], 2, (i + 2) * 20, (120 - 120 * ((previous_data[i] - 0.9) / 1.1)) + 20, 1);
            // }
            // DataCounter++;
          }
        }
        // if (DataCounter > 1)
        // {
        //   for (int i = 0; i < DataCounter - 1; i++)
        //   {
        //     tft.drawLine((i + 2) * 20, (120 - 120 * ((previous_data[i] - 0.9) / 1.1)) + 60, (i + 3) * 20, (120 - 120 * ((previous_data[i + 1] - 0.9) / 1.1)) + 60, TFT_NEIGHBOUR_GREEN);
        //   }
        // }

        // display graph
        //  x_increment = 16
        //  y_increment  =12;
        //  map y coord to ratio value;
        //  for(int m=0; m<sizeofarray; m++){
        //    tft.drawLine(20+x*m,map arra[i-m],20+2*m, map y, TFT_NEIGHBOUR_BEIGE);
        //    plot(Array[i-m])
        //  }
      }
      if (t_x > 220 && t_x < 240 && t_y > 220 && t_y < 320) // Return
      {
        // tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        // tft.fillRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        // tft.drawRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);        //change colour
        // tft.drawString("Return", 120, 280, 4);
        // delay(200); // return button

        stage = 0;
        // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        HomeScreen();
        ResetXY();
      }
    }

    if (stage == 10) // print spiffs
    {
      if (t_x > 85 && t_x < 105 && t_y > 10 && t_y < 285)
      {
        if (SPIFFS.exists("/Dataset_1"))
        {
          File file = SPIFFS.open("/Dataset_1", FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
      else if (t_x > 120 && t_x < 135 && t_y > 10 && t_y < 285)
      {
        if (SPIFFS.exists("/Dataset_2"))
        {
          File file = SPIFFS.open("/Dataset_2", FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }

      // else if (t_x > 150 && t_x < 165 && t_y > 10 && t_y < 285)
      // {
      //   if (SPIFFS.exists("/Calibration"))
      //   {
      //     File file = SPIFFS.open("/Calibration", FILE_READ);
      //     while (file.available())
      //     {
      //       Serial.write(file.read());
      //     }
      //     file.close();
      //   }
      // }
    }

    if (stage == 11)
    { // user_setup
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      selectfilenumber();
      tft.fillRect(100, 80, 50, 30, TFT_NEIGHBOUR_GREEN); // cover file number
      tft.drawFloat(SetupNumber, 0, 120, 100, 4);
      printf("%d\n", SetupNumber);

      if (t_x > 195 && t_x < 240 && t_y > 220 && t_y < 305)
      { // define file number
        profileNumber = (String)SetupNumber;
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE); // change colour
        delay(200);
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        show_menu();
        stage = 1;
      }
    }

    if (stage == 6)
    {
      if (t_x > 85 && t_x < 100 && t_y > 10 && t_y < 285)
      {
        draw_Settingframework();
        draw_start_button();
        ResetXY();
        // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        // draw_start_button();
        stage = 13;
      }
      // else if (t_x > 117 && t_x < 132 && t_y > 10 && t_y < 285) // Calibration
      // {
      //   draw_Settingframework();
      //   ResetXY();
      //   // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
      //   // draw_start_button();
      //   stage = 14;
      // }
    }

    if (stage == 7)
    {
      if (t_x > 15 && t_x < 55 && t_y > 0 && t_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE); // Pump dutycycle
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Pump power", 120, 35, 4);
        delay(200);
        Pump_setup_display();
        stage = 15;
      }
      else if (t_x > 60 && t_x < 100 && t_y > 0 && t_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE); // Pump dutycycle
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Columm Temp", 120, 100, 4);
        delay(200);
        PID_setup_display();
        stage = 16;
      }
      else if (t_x > 105 && t_x < 145 && t_y > 0 && t_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE); // Pump dutycycle
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Sampling Time", 120, 160, 4);
        delay(200);
        // Pump_setup_display();
        // stage =15;
      } // User Set up

      else if (t_x > 150 && t_x < 190 && t_y > 0 && t_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Update sensor", 120, 220, 4); // developer mode
        delay(200);
        if (isWifi != true)
        {
          ResetXY();
          Serial.println("Connect Wifi");
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
          tft.drawString("Update senosor", 120, 30, 4);
          tft.drawString("WIFI", 120, 100, 4);
          tft.drawString("Not Connected", 120, 140, 4);
          delay(2000);
          default_setting_display();
        }
        else
        {
          update_sensor();
          update_check_time();
          Serial.println("Timer updated");
        }
      }
      // updat sensor time
      // developer_display();
      // stage = 5;
    }
    if (stage == 16)
    {
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      select_PID_setpoint();
      tft.fillRect(80, 80, 70, 30, TFT_NEIGHBOUR_GREEN); // cover pump cycle
      tft.drawFloat(Setpoint, 0, 120, 100, 4);
      printf("%d\n", Setpoint);

      if (t_x > 195 && t_x < 240 && t_y > 220 && t_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE); // change colour
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        show_menu();
        stage = 1;
      }
    }
    if (stage == 13)
    { // developer mode:ADS0
      tft.setTextColor(TFT_NEIGHBOUR_GREEN);

      if (t_x > 180 && t_x < 200 && t_y > 5 && t_y < 200)
      {
        draw_Settingframework();
        graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
        control = true;
        pump_control(control);

        while (1)
        {
          // Serial.print("DUty Cycle");Serial.println(dutyCycle_pump);
          PID_control();
          float ADS0 = ads.readADC_SingleEnded(0);
          // float ADS1 = ads.readADC_SingleEnded(1);
          // tft.drawString("ADS0:", 25, 220, 2);
          // tft.drawString("ADS1:", 110, 220, 2);
          // tft.drawString("H:", 200, 220, 2);
          // tft.drawFloat(float(ADS1), 0, 150, 220, 2);
          // tft.drawFloat(float(sht20.humidity()), 0, 220, 220, 2);

          graph1.pushSprite(20, 40);

          if (i < 201)
          {

            H[i] = ads.readADC_SingleEnded(0);

            if (numMax < 0)
            { // relocate maximum point
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
            // printf("%f\n", HighY);

            tft.fillRect(0, 28, 50, 10, TFT_MilkWhite);
            tft.fillRect(0, 195, 240, 40, TFT_MilkWhite);
            // tft.fillRect(50, 215, 40, 15, TFT_GREEN);
            tft.drawFloat(float(HighY), 0, 15, 30, 1);
            tft.drawFloat(float(LowY), 0, 15, 200, 1);
            tft.drawFloat(float(ADS0), 0, 65, 220, 2);
            tft.drawString("ADS0:", 25, 220, 2);

            if (Change == 0 && i > 0) // draw
            {
              graph1.scroll(-1);
              // printf("%f\n",value);
              graph1.drawLine(198, 150 - 150 * ((H[i - 1] - LowY) / (HighY - LowY)), 199, 150 - 150 * ((H[i] - LowY) / (HighY - LowY)), TFT_MilkWhite);
              // printf("%d\n",150 - 150 * ((H[i] - LowY) / (HighY - LowY)));
            }
            if (Change == 1 && i > 0) // redraw
            {
              graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
              for (int c = 0; c < i; c++)
              {
                // graph1.drawFastVLine(199 - (i  - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)),1, TFT_YELLOW);
                graph1.drawLine(199 - (i - c), 150 - 150 * ((H[c] - LowY) / (HighY - LowY)), 199 - (i - 1 - c), 150 - 150 * ((H[c + 1] - LowY) / (HighY - LowY)), TFT_MilkWhite);
              }
              Change = 0;
            }
            if (i == 199)
            { // When i >200, H[i-1] = H[i]
              for (int j = 1; j <= 199; j++)
              {
                H[j - 1] = H[j];
                // printf("%d\n",H[j]);mnn
                // printf("%d\n",j);
              }
              as_counter = 1;
            }
            i++;
            numMax--;
            numMin--;
            if (as_counter == 1)
            {
              i = 199;
            }
            // Serial.print(ADS0);Serial.print(",");Serial.print(ads.readADC_SingleEnded(1));Serial.print(",");Serial.print(ads.readADC_SingleEnded(2));Serial.print(",");Serial.print(ads.readADC_SingleEnded(3));Serial.print(",");Serial.print(",");

            extern double Output;
            Serial.print(ADS0);
            Serial.print(",");
            Serial.print(ads.readADC_SingleEnded(2));
            Serial.print(",");
            Serial.print(Output);
            Serial.print(",");
            Serial.println(analogRead(NTCC));
          }
          if (tft.getTouch(&t_x, &t_y))
          {
            if (t_x > 220 && t_x < 240 && t_y > 0 && t_y < 85)
            {
              control = false;
              pump_control(control);
              break;
              show_menu();
              stage = 1;
            }
            if (t_x > 220 && t_x < 240 && t_y > 220 && t_y < 320) // Return
            {
              control = false;
              pump_control(control);
              break;
              stage = 0;
              HomeScreen();
              ResetXY();
            }
          }
        }
      }
    }

    if (stage == 14)
    { // HUmidiity
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      if (t_x > 195 && t_x < 240 && t_y > 220 && t_y < 305)
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

    if (stage == 15)
    {
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      select_pump_dutycycle();
      tft.fillRect(100, 80, 50, 30, TFT_NEIGHBOUR_GREEN); // cover pump cycle
      tft.drawFloat(dutyCycle_pump, 0, 120, 100, 4);

      if (t_x > 195 && t_x < 240 && t_y > 220 && t_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE); // change colour
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        show_menu();
        stage = 1;
      }
    }

    if (stage == 9)
    {
      
      if (t_x > 90 && t_x < 105 && t_y > 0 && t_y < 285) // WIFI on
      {                                                  // WIFI
        draw_Settingframework();
        extern bool isWifi;
        // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_MilkWhite);
        tft.drawString("WiFi Setting",50, 50, 4);
        isWifi = true;

        // tft.drawString("Connected", 120, 100, 4);
        // delay(2000);
        // tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        // stage = 0;
        Wifi_able();
        configTime(0, 0, ntpServer);
        if (isConnect == true)
        {
          tft.drawString("Connected", 50, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_MilkWhite);
          stage = 0;
          HomeScreen();
          draw_Wifi();
        }
        else
        {
          tft.drawString("Not Connected", 50, 100, 4);
          delay(2000);
          // wifi_display();
        }
      }

      else if (t_x > 120 && t_x < 140 && t_y > 10 && t_y < 285) // WIFI off
      {
        draw_Settingframework();
        ResetXY();
        extern bool isWifi;
        // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_MilkWhite);
        tft.drawString("WiFi Setting", 50, 50, 4);
        Wifi_disable();
        if (isWifi == false)
        {
          tft.drawString("OFF", 100, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_MilkWhite);
          HomeScreen();
          // wifi_display();
        }
        else
        {
          tft.drawString("Failed", 50, 100, 4);
          delay(2000);
          // tft.fillScreen(TFT_MilkWhite);
          // wifi_display();
        }
      }
    }
  }
}
