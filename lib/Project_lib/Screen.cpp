#include "Screen.h"
#include "SPIFFS.h"
#include "Calibration.h"
#include "uFire_SHT20.h"
#include "Wifi_connection.h"
#include "Simple_ACE.h"
#include "PID.h"
#include "History_Data.h"
#include "Cloud_storage.h"
#include "WiFi.h"
#include "Neighbour_color.h"
#include <TFT_eSPI.h>
#include <Simple_ACE.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_ADS1X15.h>

#include "Image_assets/Bubble_1.h"
#include "Image_assets/Bubble_2.h"
#include "Image_assets/Bubble_3.h"
#include "Image_assets/Bubble_4.h"
#include "Image_assets/Bubble_5.h"
#include "Image_assets/Bubble_6.h"
#include "Image_assets/setting.h"
#include "Image_assets/Beagle.h"
#include "Image_assets/Wifi_connect.h"
#include "Image_assets/Wifi_not_connect.h"
#include "Image_assets/Wifi_Off.h"
#include "Image_assets/Loading.h"

#define setting_x 180
#define setting_y 260

// stages reference:
// stage 0:   HomeScreen
// stage 1:   Menu
// stage 2:   Gas Sampling
// stage 3:   Calibration
// stage 4:   Wifi and Bluetooth setting
// stage 5:   Enter developer mode
// stage 6:   live Plot control
// stage 7:   device setting
// stage 9:   Wifi control
// stage 10:  Print stored data
// stage 11:  Select user profile
// stage 13:  Live Plot
// stage 15:  Pump setting
// stage 16:  PID setting
// stage 17:  print Gas sample
// stage 99:  chagne sensor

void tft_setup();                         //initialize TFT screen
void draw_result(double co2, double ace);
void draw_result_bar(double bar_1, double bar_2);
void draw_sample_progress(float bar_length, float bar_percentage);
void draw_sensor(double sensor_value);
void display_assets();
void display_bluetooth();
void display_control_wifi();
void display_calibration();
void display_developer_menu();
void display_device_setting();
void display_enable_sampling();
void display_live_plot();
void display_load_SPIFFS();
void display_load_data();
void display_loading(int count);
void display_menu();
void display_OTA_control();
void display_PID_selectSetpoint(); 
void display_profile_filenumber();
void display_pump_selectDutycycle();
void display_start_button();
void display_setup_profile_select();
void display_setup_pump();
void display_setup_PID();
void display_Wifi();
void HomeScreen();                        // display brand, assets and setting figure
void Navigation();                        // touch screen navigation logic
void Reset_coordinate();
void Warmup_Screen();
void write_analyzing(void);

extern Adafruit_ADS1115 ads;
extern uFire_SHT20 sht20;
extern float ref_position[2];
extern double recorded_gas_sample[10];
extern int dutyCycle_pump;
extern double PID_Setpoint;
extern bool isWifi;
extern bool isConnect;
extern const char* ntpServer;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph1 = TFT_eSprite(&tft);

bool isSensor =true;
bool isPlotrangeChange = false;
int stage = 0;
int SetupNumber = 1;
String profileNumber = "1";
uint16_t touch_x = 0, touch_y = 0;

void tft_setup(){
  tft.init();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setSwapBytes(true);
  tft.setRotation(2);

  graph1.setColorDepth(16);
  graph1.createSprite(200, 150);
  graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
  graph1.setScrollRect(0, 0, 200, 150, TFT_NEIGHBOUR_GREEN);
}

void Reset_coordinate(){
  touch_x = 0;
  touch_y = 0;
}

void display_assets(){
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE);
  tft.pushImage(10, 200, Bubble1_w, Bubble1_h, Bubble_1, 0x0000);
  tft.pushImage(50, 200, Bubble2_w, Bubble2_h, Bubble_2, 0x0000);
  tft.pushImage(90, 200, Bubble3_w, Bubble3_h, Bubble_3, 0x0000);
  tft.pushImage(130, 200, Bubble4_w, Bubble4_h, Bubble_4, 0x0000);
  tft.pushImage(165, 200, Bubble5_w, Bubble5_h, Bubble_5, 0x0000);
  tft.pushImage(201, 200, Bubble6_w, Bubble6_h, Bubble_6, 0x0000);
  tft.setTextDatum(TC_DATUM);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
}

void display_loading(int count){
  tft.fillRect(90,250,70,70,TFT_NEIGHBOUR_GREEN);//  cover loading logo
  tft.pushImage(90, 250, LoadingWidth, LoadingHeight, Loading[count % 11]);
  delay(100); 
}

void draw_sample_progress(float bar_length, float bar_percentage){
  tft.setTextDatum(1);
  tft.fillRoundRect(15, 210, 200 * (bar_length / 45000), 5,2,TFT_NEIGHBOUR_BEIGE); // bar
  if ((int)(bar_percentage * 10) % 10 == 0)
  {
    tft.fillRect(75, 230, 60, 25, TFT_NEIGHBOUR_GREEN); // cover recorded_gas_sample number
    tft.drawFloat(bar_percentage, 0, 120, 230, 4);
  }
  tft.drawString("%", 155, 230, 4);
}

void write_analyzing(void){
  tft.fillRect(60, 210, 119, 62, TFT_NEIGHBOUR_GREEN); // cover huff now
  tft.setTextDatum(3);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE);
  tft.fillRect(10, 260, 70, 50, TFT_NEIGHBOUR_GREEN); // cover button
  tft.drawString("Analyzing...", 10, 280, 4);
  tft.fillRect(0, 200, 240, 33, TFT_NEIGHBOUR_GREEN); // cover bubbles
}

bool isbufferfull = false;
float temporal_maximum;
float temporal_minimum;
float Plot_buffer[210];
float plot_lower_bound;
float plot_upper_bound;
int array_index = 0;
int as_counter = 0;
int position_temp_max = -1;
int position_temp_min = -1;

void draw_sensor(double sensor_value){
  graph1.pushSprite(20, 40);

  if (array_index < 201){
    Plot_buffer[array_index] = (int)sensor_value;
    if (position_temp_max < 0){ // relocate maximum point
      temporal_maximum = Plot_buffer[0];
      position_temp_max = 0;
      for (int a = 0; a < array_index; a++){
        if (Plot_buffer[a] > temporal_maximum){
          temporal_maximum = Plot_buffer[a];
          position_temp_max = a;
        }
      }
      plot_upper_bound = temporal_maximum + 200;
      isPlotrangeChange = 1;
    }

    if (position_temp_min < 0){ // relocate minimum point
      position_temp_min = 0;
      temporal_minimum = Plot_buffer[0];
      for (int a = 0; a < array_index; a++){
        if (Plot_buffer[a] < temporal_minimum){
          temporal_minimum = Plot_buffer[a];
          position_temp_min = a;
        }
      }
      plot_lower_bound = temporal_minimum - 200;
      isPlotrangeChange = 1;
    }

    if (Plot_buffer[array_index] > plot_upper_bound){ // update maximum bound
      plot_upper_bound = Plot_buffer[array_index] + 200;
      position_temp_max = array_index;
      isPlotrangeChange = 1;
    }

    if (Plot_buffer[array_index] < plot_lower_bound){ // update minimum bound
      plot_lower_bound = Plot_buffer[array_index] - 200;
      position_temp_min = array_index;
      isPlotrangeChange = 1;
    }

    if (isPlotrangeChange == 0 && array_index > 0){      // draw
      graph1.scroll(-1);
      // printf("%f\n",sensor_value);
      graph1.drawLine(198, 150 - 150 * ((Plot_buffer[array_index - 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199, 150 - 150 * ((Plot_buffer[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_YELLOW);
      // printf("%d\n",150 - 150 * ((H[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)));
    }
    if (isPlotrangeChange == 1 && array_index > 0){ // redraw
      graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
      for (int c = 0; c < array_index; c++){
        // graph1.drawFastVLine(199 - (array_index  - c), 150 - 150 * ((H[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)),1, TFT_YELLOW);
        graph1.drawLine(199 - (array_index - c), 150 - 150 * ((Plot_buffer[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199 - (array_index - 1 - c), 150 - 150 * ((Plot_buffer[c + 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_YELLOW);
      }
      isPlotrangeChange = 0;
    }
    if (array_index == 199){ // When array_index >200, H[array_index-1] = H[array_index]
      for (int j = 1; j <= 199; j++){
        Plot_buffer[j - 1] = Plot_buffer[j];
        // printf("%d\n",H[j]);
        // printf("%d\n",j);
      }
      isbufferfull =true;
    }

    array_index++;
    position_temp_max--;
    position_temp_min--;
    if (isbufferfull){
      array_index = 199;
    }
  }
}

void draw_result_bar(double bar_1, double bar_2){
  const int top_y =60;
  const int base_y =190;
  int y_length = base_y - top_y;
  int start_y_ace;
  int end_y_ace;
  int start_y_co2;
  int end_y_co2;
  start_y_co2 = top_y + (int)(y_length*(1-(bar_1/2)));
  end_y_co2 = base_y - start_y_co2;
  start_y_ace = top_y + (int)(y_length*(1-bar_2/2));
  end_y_ace = base_y - start_y_ace;
  // Serial.print("start_y_co2:");Serial.println(start_y_co2);
  // Serial.print("end_y_co2:");Serial.println(end_y_co2);
  // Serial.print("start_y_ace:");Serial.println(start_y_ace);
  // Serial.print("end_y_ace:");Serial.println(end_y_ace);

  tft.drawFastHLine(20,190,200,TFT_NEIGHBOUR_BEIGE);
  int split_portion =10;
  unsigned long previous_data = 0;
  tft.drawRect(50,top_y, 20, y_length,TFT_NEIGHBOUR_BEIGE);
  tft.drawRect(160,top_y, 20, y_length,TFT_NEIGHBOUR_BEIGE);
  for(int i =0; i<split_portion; i++){
    while(millis()-previous_data <150){
    }
    previous_data =millis();
    tft.fillRect(50,190 - (int)((i+1)*(end_y_co2/split_portion)), 20, (int)(end_y_co2/split_portion) ,TFT_NEIGHBOUR_BEIGE);
    tft.fillRect(160,190 - (int)((i+1)*(end_y_ace/split_portion)), 20, (int)(end_y_ace/split_portion) ,TFT_NEIGHBOUR_BEIGE);
  }
}

void display_start_button(){
  tft.setTextDatum(MC_DATUM);
  tft.fillRoundRect(10, 263, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.drawString("Start", 40, 287, 2);
}

void draw_result(double co2, double ace){
  extern bool isStore;
  extern int fail_count;
  tft.fillRect(0,20,240,170,TFT_NEIGHBOUR_GREEN);     // cover graph 
  tft.fillRect(10,260,150,50,TFT_NEIGHBOUR_GREEN);    // cover analyzing
  tft.fillRect(0,200,240,50,TFT_NEIGHBOUR_GREEN);     // ????
  display_assets();
  tft.fillRect(0,200,240,33,TFT_NEIGHBOUR_GREEN);     //cover logo
 
  tft.setTextDatum(4); 
  display_start_button();     
  if(fail_count != 50){ 
    draw_result_bar(co2,ace);
    delay(500);
    tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
    tft.drawString("Acetone", 170, 205, 2);
    tft.drawFloat((float)ace, 2, 170, 220, 2);
    tft.drawString("Metabolic rate", 60, 205, 2);
    tft.drawFloat((float)co2 , 2, 60, 220, 2);
  }
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);

  if(ace < 1 || co2 < 1||isStore == false){
    tft.drawString("Try Again",120,40,4); 
  } else if((ace >= 1 && ace < 1.2) && (co2 >= 1 && co2 < 1.3)){
    tft.drawString("Inactive workout",120,40,4);
  } else if((ace >= 1 && ace < 1.2) && (co2 >= 1.3 && co2 < 1.5)){
    tft.drawString("Moderate burn",120,40,4);
  } else if((ace >= 1.2 && ace < 1.3) && (co2 >= 1.3 && co2 < 1.5)){
    tft.drawString("Effective training",120,40,4);
  } else if((ace >= 1.2 && ace < 1.3) && (co2 >= 1.5)){
    tft.drawString("Intensive training",120,40,4);
  } else if((ace >= 1.2 && ace < 1.3) && (co2 >= 1 && co2 < 1.3 )){
    tft.drawString("Moderate Ketosis",120,40,4);
  } else if(ace >= 1.3 && co2 >= 1 ){
    tft.drawString("Deep Ketosis",120,40,4);
  } 
}

void Warmup_Screen(){
  extern double PID_Setpoint;
  unsigned long counttime = 0;
  double display_warmup = 0;
  double warmup_bar_base  = (double)ads.readADC_SingleEnded(NTCC_channel) - PID_Setpoint;
  int temperature_range = 10;
  tft.pushImage(20, 80, BeagleWidth, BeagleHeight, Beagle);
  tft.drawRoundRect(15,210, 200,15,7,TFT_NEIGHBOUR_BEIGE);
  while(abs(ads.readADC_SingleEnded(NTCC_channel)-(int)PID_Setpoint) > temperature_range){ 
    PID_control();
    if(ads.readADC_SingleEnded(NTCC_channel) < (int)PID_Setpoint){
       tft.fillRoundRect(15, 210, 190, 15, 7, TFT_NEIGHBOUR_BEIGE);
    }else{
      display_warmup = abs ((double)ads.readADC_SingleEnded(NTCC_channel)-PID_Setpoint);
      tft.fillRoundRect(15, 210, (int)(200 * (1-(display_warmup / warmup_bar_base))), 15, 7, TFT_NEIGHBOUR_BEIGE);
    }
    delay(10);
  }
  tft.fillRect(20,200,200,80,TFT_NEIGHBOUR_GREEN);   // cover graph 
}

void HomeScreen()
{
  tft.pushImage(20, 80, BeagleWidth, BeagleHeight, Beagle);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("King's Phase ", 10, 270, 2);
  tft.drawString("Technologies", 10, 290, 1);
  display_assets();
  display_Wifi();
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.setTextDatum(4);
  tft.fillRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Start", 125, 280, 2);
}

void display_menu(){
  Reset_coordinate();
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
}

void display_Wifi(){             // draw wifi logo
  if (isWifi == true && isConnect == true)
  {
    tft.pushImage(200, 10, Wifi_true_w, Wifi_true_h, Wifi_true);
  }
  else if (isWifi == true && isConnect == false)
  {
    tft.pushImage(200, 10, Wifi_nc_w, Wifi_nc_h, Wifi_nc);
  }
  else
  {
    tft.pushImage(200, 10, Wifi_false_w, Wifi_false_h, Wifi_false);
  }
}

void display_enable_sampling(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  delay(200);
  display_assets();
  display_start_button();
}

void display_calibration(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("Calibration", 120, 30, 4);
  display_start_button();
}

void display_OTA_control(){
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);

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

void display_developer_menu(){
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Live Plot", 120, 35, 4);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Default Setting", 120, 100, 4);
  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Print Spiffs", 120, 160, 4);
  tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Previous Value", 120, 220, 4);
  Reset_coordinate();
}

void display_live_plot(){
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  Reset_coordinate();
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("ADS0", 120, 35, 4);

  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Humidity", 120, 100, 4);
}

void display_bluetooth(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("Bluetooth", 120, 30, 4);
}

void display_setup_profile_select(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
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

void display_setup_pump()
{
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
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

void display_setup_PID()
{
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(4);
  tft.drawString("PID PID_Setpoint", 120, 40, 4);

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

void display_control_wifi(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("WiFi Setting", 120, 30, 4);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("On", 120, 100, 4);
  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Off", 120, 160, 4);
  Reset_coordinate();
  delay(300);
}

void display_device_setting(){
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  Reset_coordinate();
  tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Pump power", 120, 35, 4);

  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Column Temp", 120, 100, 4);

  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Sample Time", 120, 160, 4);

  tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Update sensor", 120, 220, 4);
}

void display_load_SPIFFS(){
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Data", 120, 35, 4);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("Calibration", 120, 100, 4);
}

void display_load_data(){
  tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BEIGE);
  tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("SPIFFS 1 - 5", 120, 35, 4);
  tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("SPIFFS 6 - 10", 120, 100, 4);
  tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("SPIFFS 11 - 15", 120, 160, 4);
  tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
  tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
  tft.drawString("SPIFFS 16 - 20", 120, 220, 4);
}

void display_profile_filenumber()
{
  if (touch_x > 110 && touch_x <190 && touch_y > 0 && touch_y <145)
  {
    if (SetupNumber <10)
    {
      SetupNumber = SetupNumber + 1;
      Reset_coordinate();
      delay(150);
    }
  }
  else if (touch_x > 110 && touch_x < 190 && touch_y > 160 && touch_y < 300)
  {
    if (SetupNumber > 1)
    {
      SetupNumber = SetupNumber - 1;
      Reset_coordinate();
      delay(150);
    }
  }
}

void display_pump_selectDutycycle()
{
  if (touch_x > 110 && touch_x <190 && touch_y > 0 && touch_y <145)
  {
    if (dutyCycle_pump <255)
    {
      dutyCycle_pump = dutyCycle_pump + 5;
      Reset_coordinate();
      delay(150);
    }
  }
  else if (touch_x > 110 && touch_x < 190 && touch_y > 160 && touch_y < 300)
  {
    if (dutyCycle_pump > 0)
    {
      dutyCycle_pump = dutyCycle_pump - 5;
      Reset_coordinate();
      delay(150);
    }
  }
}

void display_PID_selectSetpoint()
{
  if (touch_x > 110 && touch_x <190 && touch_y > 0 && touch_y <145)
  {
    if (PID_Setpoint <10000)
    {
      PID_Setpoint = PID_Setpoint + 50;
      Reset_coordinate();
      delay(150);
    }
  }
  else if (touch_x > 110 && touch_x < 190 && touch_y > 160 && touch_y < 300)
  {
    if (PID_Setpoint > 0)
    {
      PID_Setpoint = PID_Setpoint - 50;
      Reset_coordinate();
      delay(150);
    }
  }
}

void Navigation()
{
  if(isSensor!=true){
    tft.fillScreen(TFT_NEIGHBOUR_GREEN);
    tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE);
    tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
    tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
    tft.drawString("Change sensor", 120, 160, 4);
    stage =99;
  }

  while(stage == 99){
    if (tft.getTouch(&touch_x, &touch_y)){
      if(touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305){
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

  if (stage == 0)
  {
    if (touch_x > 195 && touch_x < 240 && touch_y > 100 && touch_y < 195)
    {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(95, 257, 60, 46, 23, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Start", 125, 280, 2);
        display_enable_sampling();
        stage = 2;
    }
  }
  
  if (tft.getTouch(&touch_x, &touch_y))
  {
    printf("%d\n", touch_x);
    printf("%d\n", touch_y);


    if (stage !=1)
    {
      if (touch_x > 195 && touch_x < 235 && touch_y > 5 && touch_y < 75)                 // press
      {
        pump_control(false);
        Serial.println("OFF");
        display_menu();
        stage = 1;
      }
    }

    if (stage == 1)
    {                                                                       // Navigation
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305)                //OTA
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("OTA Setting", 120, 35, 4); // OTA Setting
        delay(200);
        display_OTA_control();
        stage = 4;
      }

      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)               //Calibration
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Calibration", 120, 100, 4); // Calibration
        delay(200);
        display_calibration();
        stage = 3;
      }
      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305)              //User Set up 
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE); // display_setup_profile_select
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("User Setup", 120, 160, 4);
        delay(200);
        display_setup_profile_select();
        stage = 11;
      }

      else if (touch_x > 150 && touch_x < 190 && touch_y > 0 && touch_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Developer Mode", 120, 220, 4); // developer mode
        delay(200);
        display_developer_menu();
        stage = 5;
      }
      else if (touch_x > 195 && touch_x <235 && touch_y > 0 && touch_y < 305)                    //Return
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 255, 220, 44, 22, TFT_NEIGHBOUR_BLUE);        //change colour
        tft.drawString("Return", 120, 280, 4);
        delay(200); // return button

        stage = 0;
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        HomeScreen();
        Reset_coordinate();
      }
    }

    if(stage == 2){                                                          //sample
      if(touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){
        if(isConnect ==true){
          WiFi.disconnect(true,true);
          delay(500);
          isConnect = false;
          Serial.println("Disconnected");
          Serial.print("Wifi status:");Serial.println(WiFi.status());
        }
        tft.fillRect(0,20,240,210,TFT_NEIGHBOUR_GREEN);                    //cover previosu result screeen                               //cover result
        display_assets();
        sample_collection();
        output_result();
        }
        
      }

    if (stage == 3)
    { // Calibration Start Button
      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305)
      {
        restore_baseline();
        tft.setTextDatum(4);
        tft.fillRect(10, 50, 200, 150, TFT_NEIGHBOUR_GREEN);      //
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
      if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)
      { // bluetooth
        display_bluetooth();
        stage = 8;
      }
      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305)
      { // wi-fi
        display_control_wifi();
        stage = 9;
      }
    }

    if (stage == 5)
    { // developer mode                                                                     // developer mode choices
      tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305)
      {
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Live Plot", 120, 35, 4);
        delay(200);
        display_live_plot();
        stage = 6;
        delay(300);
      }
      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)
      {
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Default Setting", 120, 100, 4);
        delay(200);
        display_device_setting();
        stage = 7;
        delay(300);
      }

      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305)
      {
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Print Spiffs", 120, 160, 4);
        delay(200);
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        Reset_coordinate();
        display_load_SPIFFS();
        stage = 10;
      }
      else if (touch_x > 150 && touch_x < 190 && touch_y > 0 && touch_y < 305)
      {
        tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Previous Value", 120, 220, 4);
        delay(200);
        int DataCounter = 0;
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        Reset_coordinate();
        tft.drawFastVLine(20, 60, 120, TFT_NEIGHBOUR_BEIGE);
        tft.drawFastHLine(20, 180, 200, TFT_NEIGHBOUR_BEIGE);
        retrieve_record();
        for (int i = 0; i < 10; i++)
        {
          if (recorded_gas_sample[i] > 0.5) // what is 0.5
          {
            Serial.print("plotting: "); Serial.println(recorded_gas_sample[i]);
            tft.fillCircle((i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 60, 2, TFT_YELLOW);
            tft.setTextColor(TFT_YELLOW, TFT_NEIGHBOUR_GREEN);
            // if (i == 0 || i == 2 || i == 4 || i == 6 || i == 8)
            if (i %2 ==0)
            {
              tft.drawFloat(recorded_gas_sample[i], 2, (i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 40, 1);
            }
            // if (i == 1 || i == 3 || i == 5 || i == 7 || i == 9)
            else
            {
              tft.drawFloat(recorded_gas_sample[i], 2, (i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 20, 1);
            }
            DataCounter++;
          }
        }
        if (DataCounter > 1)
        {
          for (int i = 0; i < DataCounter - 1; i++)
          {
            tft.drawLine((i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 60, (i + 3) * 20, (120 - 120 * ((recorded_gas_sample[i + 1] - 0.9) / 1.1)) + 60, TFT_YELLOW);
          }
        }

        // display graph
        //  x_increment = 16
        //  y_increment  =12;
        //  map y coord to ratio value;
        //  for(int m=0; m<sizeofarray; m++){
        //    tft.drawLine(20+x*m,map arra[i-m],20+2*m, map y, TFT_NEIGHBOUR_BEIGE);
        //    plot(Array[i-m])
        //  }
      }
    }

    if (stage == 10)      //print spiffs
    {
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305){
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        Reset_coordinate();
        display_load_data();
        stage = 17;
      }

      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)
      {
        if (SPIFFS.exists("/Calibration"))
        {
          File file = SPIFFS.open("/Calibration", FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
    }
    if(stage == 17){
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305)
      {
        for(int i = 0; i< 5; i++){
          String data_name = "/Dataset_";
          data_name.concat(i+1);
          Serial.println(data_name);
          if (SPIFFS.exists(data_name))
          {
            File file = SPIFFS.open(data_name, FILE_READ);
            while (file.available())
            {
              Serial.write(file.read());
            }
            file.close();
          }
          Serial.println();
        }
      }
      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)
      {
        // if (SPIFFS.exists("/Dataset_2"))
        // {
        //   File file = SPIFFS.open("/Dataset_2", FILE_READ);
        //   while (file.available())
        //   {
        //     Serial.write(file.read());
        //   }
        //   file.close();
        // }
        for(int i = 0; i< 5; i++){
          String data_name = "/Dataset_";
          data_name.concat(i+6);
          Serial.println(data_name);
          if (SPIFFS.exists(data_name))
          {
            File file = SPIFFS.open(data_name, FILE_READ);
            while (file.available())
            {
              Serial.write(file.read());
            }
            file.close();
          }
          Serial.println();
        }
      }
      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305)
      {
        for(int i = 0; i< 5; i++){
          String data_name = "/Dataset_";
          data_name.concat(i+11);
          Serial.println(data_name);
          if (SPIFFS.exists(data_name))
          {
            File file = SPIFFS.open(data_name, FILE_READ);
            while (file.available())
            {
              Serial.write(file.read());
            }
            file.close();
          }
          Serial.println();
        }
      }
      else if(touch_x > 150 && touch_x < 190 && touch_y > 0 && touch_y < 305)
      {
        for(int i = 0; i< 5; i++){
          String data_name = "/Dataset_";
          data_name.concat(i+16);
          Serial.println(data_name);
          if (SPIFFS.exists(data_name))
          {
            File file = SPIFFS.open(data_name, FILE_READ);
            while (file.available())
            {
              Serial.write(file.read());
            }
            file.close();
          }
          Serial.println();
        }
      }
    }

    if (stage == 11){     // user_setup
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      display_profile_filenumber();
      tft.fillRect(100, 80, 50, 30, TFT_NEIGHBOUR_GREEN);  //cover file number
      tft.drawFloat(SetupNumber, 0, 120, 100, 4);
      printf("%d\n", SetupNumber);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){    // define file number
        profileNumber = (String)SetupNumber;
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE);  //change colour
        delay(200);    
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        display_menu();
        stage = 1;
      }
    }
    
    if (stage == 6){
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305){
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        Reset_coordinate();
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        display_start_button();
        stage =13;
      }
      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)               //Calibration
      {
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        Reset_coordinate();
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        display_start_button();
        stage =14;
      }
    }

    if (stage == 7){
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305){  
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);    //Pump dutycycle
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);   
        tft.drawString("Pump power", 120, 35, 4);
        delay(200);
        display_setup_pump();
        stage =15;
      }
      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305){
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);    //Pump dutycycle
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);   
        tft.drawString("Columm Temp", 120, 100, 4);
        delay(200);
        display_setup_PID();
        stage =16;
      }
      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305){
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);    //Pump dutycycle
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);   
        tft.drawString("Sampling Time", 120, 160, 4);
        delay(200);
        // display_setup_pump();
        // stage =15;
      }         //User Set up 

      else if (touch_x > 150 && touch_x < 190 && touch_y > 0 && touch_y < 305)
      {
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 195, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Update sensor", 120, 220, 4); // developer mode
        delay(200);
        if(isWifi != true){
          Reset_coordinate();
          Serial.println("Connect Wifi");
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
          tft.drawString("Update senosor", 120, 30, 4);
          tft.drawString("WIFI", 120, 100, 4);
          tft.drawString("Not Connected", 120, 140, 4);
          delay(2000);
          display_device_setting();
        }
        else{
          update_sensor();
          update_check_time();
          Serial.println("Timer updated");
        }
      }
        // updat sensor time 
        // developer_display();
        // stage = 5;
    }
    if (stage == 16){
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      display_PID_selectSetpoint();
      tft.fillRect(80, 80, 70, 30, TFT_NEIGHBOUR_GREEN);  //cover pump cycle
      tft.drawFloat(PID_Setpoint, 0, 120, 100, 4);
      printf("%d\n", PID_Setpoint);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){    
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE);       //change colour
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        display_menu();
        stage = 1;
      }
    }
    if (stage == 13)
    { // developer mode:ADS0 
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305)
      {
        graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
        pump_control(true);
        while (1){
          PID_control();  
          float ADS0 = ads.readADC_SingleEnded(Sensor_channel);
          tft.drawString("ADS0:", 25, 220, 2);
          graph1.pushSprite(20, 40);

          if (array_index < 201){
            Plot_buffer[array_index] = ads.readADC_SingleEnded(Sensor_channel);
            if (position_temp_max < 0){ // relocate maximum point
              temporal_maximum = Plot_buffer[0];
              position_temp_max = 0;
              for (int a = 0; a < array_index; a++){
                if (Plot_buffer[a] > temporal_maximum){
                  temporal_maximum = Plot_buffer[a];
                  position_temp_max = a;
                }
              }
              plot_upper_bound = temporal_maximum + 200;
              isPlotrangeChange = true;
            }

            if (position_temp_min < 0){ // relocate minimum point
              position_temp_min = 0;
              temporal_minimum = Plot_buffer[0];
              for (int a = 0; a < array_index; a++){
                if (Plot_buffer[a] < temporal_minimum){
                  temporal_minimum = Plot_buffer[a];
                  position_temp_min = a;
                }
              }
              plot_lower_bound = temporal_minimum - 200;
              isPlotrangeChange = true;
            }

            if (Plot_buffer[array_index] > plot_upper_bound){
              plot_upper_bound = Plot_buffer[array_index] + 200; position_temp_max = array_index; isPlotrangeChange = true;
            }

            if (Plot_buffer[array_index] < plot_lower_bound)
            {
              plot_lower_bound = Plot_buffer[array_index] - 200;
              position_temp_min = array_index;
              isPlotrangeChange = true;
            }
             tft.fillRect(0, 25, 50, 10, TFT_NEIGHBOUR_GREEN);
            tft.fillRect(0, 195, 240, 10, TFT_NEIGHBOUR_GREEN);
            tft.fillRect(45, 215, 40, 15, TFT_NEIGHBOUR_GREEN);
            tft.drawFloat(float(plot_upper_bound), 0, 15, 30, 1);
            tft.drawFloat(float(plot_lower_bound), 0, 15, 200, 1);
            tft.drawFloat(float(ADS0), 0, 65, 220, 2);
            

            if (isPlotrangeChange == false && array_index > 0) // draw
            {
              graph1.scroll(-1);
              // printf("%f\n",value);
              graph1.drawLine(198, 150 - 150 * ((Plot_buffer[array_index - 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199, 150 - 150 * ((Plot_buffer[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_YELLOW);
              // printf("%d\n",150 - 150 * ((H[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)));
            }
            if (isPlotrangeChange == true && array_index > 0) // redraw
            {
              graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
              for (int c = 0; c < array_index; c++)
              {
                // graph1.drawFastVLine(199 - (array_index  - c), 150 - 150 * ((H[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)),1, TFT_YELLOW);
                graph1.drawLine(199 - (array_index - c), 150 - 150 * ((Plot_buffer[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199 - (array_index - 1 - c), 150 - 150 * ((Plot_buffer[c + 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_YELLOW);
              }
              isPlotrangeChange = false;
            }
            if (array_index == 199){ // When array_index >200, H[array_index-1] = H[array_index]
              for (int j = 1; j <= 199; j++)
              {
                Plot_buffer[j - 1] = Plot_buffer[j];
                // printf("%d\n",H[j]);mnn 
                // printf("%d\n",j);
              }
              as_counter = 1;
            }
            array_index++;
            position_temp_max--;
            position_temp_min--;
            if (as_counter == 1)
            {
              array_index = 199;
            }
            // Serial.print(ADS0);Serial.print(",");Serial.print(ads.readADC_SingleEnded(1));Serial.print(",");Serial.print(ads.readADC_SingleEnded(2));Serial.print(",");Serial.print(ads.readADC_SingleEnded(3));Serial.print(",");Serial.print(",");
            
            extern double Output;
            // Serial.print(ADS0);Serial.print(",");Serial.print(ads.readADC_SingleEnded(3));Serial.print(",");Serial.print(Output);Serial.print(",");Serial.println(analogRead(NTCC)); 
            Serial.print(ADS0);Serial.print(",");Serial.print(ads.readADC_SingleEnded(Heater_channel));Serial.print(",");Serial.print(ads.readADC_SingleEnded(Offset_channel));Serial.print(",");Serial.print(Output);Serial.print(",");Serial.println(ads.readADC_SingleEnded(NTCC_channel)); 
          }
          if (tft.getTouch(&touch_x, &touch_y))
          {
            if (touch_x > 195 && touch_x <235 && touch_y > 5 && touch_y < 75)
            {
              // control = false;
              pump_control(false);
              break;
            }
          }
        }
        display_menu();
        stage = 1;
      }
    }


    if (stage == 15){  //select pump duty cycle
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      display_pump_selectDutycycle();
      tft.fillRect(100, 80, 50, 30, TFT_NEIGHBOUR_GREEN);  //cover pump cycle
      tft.drawFloat(dutyCycle_pump, 0, 120, 100, 4);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){    
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE);       //change colour
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        display_menu();
        stage = 1;
      }
    }
    
    if (stage == 9){
      if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)    //WIFI on 
      { // WIFI
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        extern bool isWifi;
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
        tft.drawString("WiFi Setting", 120, 30, 4);
        isWifi = true;

        Wifi_able();
        configTime(0, 0, ntpServer);
        if (isConnect == true)
        {
          tft.drawString("Connected", 120, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          stage = 0;    
          HomeScreen();    
          display_Wifi();
        }
        else
        {
          tft.drawString("Not Connected", 120, 100, 4);
          delay(2000);
          display_control_wifi();
        }
      }

      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305)  //WIFI off
      {
        tft.fillScreen(TFT_NEIGHBOUR_GREEN);
        Reset_coordinate();
        extern bool isWifi;
        tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
        tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
        tft.drawString("WiFi Setting", 120, 30, 4);
        Wifi_disable();
        if (isWifi == false)
        {
          tft.drawString("OFF", 120, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          display_control_wifi();
        }
        else
        {
          tft.drawString("Failed", 120, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_GREEN);
          display_control_wifi();
        }
      }
    }
  }
}