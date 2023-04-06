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
#include "SHTSensor.h"

#include "Bar.h"
#include "Beagle_Warmup.h"
#include "Beagle.h"
#include "Bubble_1.h"
#include "Bubble_2.h"
#include "Bubble_3.h"
#include "Bubble_4.h"
#include "Bubble_5.h"
#include "Bubble_6.h"
#include "DefaultSetting.h"
#include "FullBattary.h"
#include "Loading.h"
#include "Pointer.h"
#include "Return_arrow.h"
#include "Return_arrow_flip.h"
#include "SettingBar.h"
#include "Wifi_connect.h"
#include "Wifi_not_connect.h"
#include "Wifi_Off.h"
#include "WifiButtonOn.h"
#include "WifiButtonOff.h"


void tft_setup();                         //initialize TFT screen
void draw_result(double co2, double ace);
void draw_result_bar(double bar_CO2, double bar_Ace);
void draw_sample_progress(float bar_length, float bar_percentage);
void draw_sensor(double sensor_value);
void display_assets();
void display_bluetooth();
void display_control_wifi();
void display_calibration();
void display_developer_menu();
void display_device_setting();
void display_live_plot();
void display_load_data();
void display_loading(int count);
void display_menu();
void display_OTA_control(); 
void display_PID_selectSetpoint(); 
void display_previous_value();
void display_profile_filenumber();
void display_pump_selectDutycycle();
void display_start_button();
void display_setup_profile_select();
void display_setup_pump();
void display_setup_PID();
void display_sampling_init();
void display_Spiffs_data(int page);
void display_Wifi();
void HomeScreen();                        // display brand, assets and setting figure
void Navigation();                        // touch screen navigation logic
void Reset_coordinate();
void Warmup_Screen();
// void write_analyzing(void);

extern Adafruit_ADS1115 ads;
extern SHTSensor sht;
extern float ref_position[2];
extern double recorded_gas_sample[10][2];
extern int dutyCycle_pump;
extern double PID_Setpoint;
extern bool isWifi;
extern bool isConnect;
extern const char* ntpServer;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite graph1 = TFT_eSprite(&tft);
TFT_eSprite Warmup_Graph = TFT_eSprite(&tft);
TFT_eSprite Bubble2 = TFT_eSprite(&tft);
TFT_eSprite Warmup_BG = TFT_eSprite(&tft);

uint8_t page_number = 0;
bool isSensor =true;
bool isPlotrangeChange = false;
bool isCal = false;
uint8_t stage = homescreen;
uint8_t profileNumber_int = 1;
uint8_t lifecount;
unsigned long start_activity_check_millis = 0;
String profileNumber = "1";
uint16_t touch_x = 0, touch_y = 0;

uint8_t blow_address = 12; 
uint8_t sample_address = 14; 
uint8_t plot_address = 16; 

void tft_setup(){
  tft.init();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  tft.setSwapBytes(true);
  tft.setRotation(2);

  graph1.setColorDepth(16);
  graph1.createSprite(200, 150);
  graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
  graph1.setScrollRect(0, 0, 200, 150, TFT_NEIGHBOUR_GREEN);

  
  // Warmup_Graph.createSprite(Bubbles_w,Bubblesh);
}

void Reset_coordinate(){
  touch_x = 0;
  touch_y = 0;
}

void draw_framework()
{
  tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
  tft.pushImage(0, 280, BarWidth, BarHeight, Bar);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  display_Wifi();
}

void draw_Settingframework()
{
  tft.fillScreen(TFT_NEIGHBOUR_BEIGE);
  tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  display_Wifi();
}

void display_loading(int count)
{
  tft.pushImage(90, 200, LoadingWidth, LoadingHeight, Loading[count % 9]);
  delay(100);
}

void draw_sample_progress(float bar_length, float bar_percentage){
  tft.setTextDatum(TR_DATUM);
  tft.fillRoundRect(20, 265, 200 * (bar_length / sampletime), 5, 2, TFT_TextBrown); // bar
  tft.setTextColor(TFT_TextBrown,TFT_NEIGHBOUR_BEIGE);
  if ((int)(bar_percentage * 10) % 10 == 0)
  {
    tft.fillRect(75, 285, 60, 25, TFT_NEIGHBOUR_BEIGE); // cover recorded_gas_sample number
    tft.drawFloat(bar_percentage, 0, 115, 275, 4);
  }
  tft.setTextDatum(TC_DATUM);
  tft.drawString("%", 130, 275, 4);
}

bool leave = false;
void leave_sample(){ 
  if (tft.getTouch(&touch_x, &touch_y))
  {
    printf("%d\n", touch_x);
    printf("%d\n", touch_y);
    if (touch_x > 65 && touch_x < 80 && touch_y > 270 && touch_y < 295){
      stage = homescreen; 
      pump_control(false);
      sensor_heater_control(false);
      stage = homescreen;
      HomeScreen();
      leave = true;
      return;
    }
  }
}


void screen_count(int screen_address){
  EEPROM.begin(20);
  int8_t press_count;
  EEPROM.get(screen_address,press_count);
  delay(500);
  Serial.println(press_count);
  press_count++;
  Serial.println(press_count);
  EEPROM.put(screen_address,press_count);
  delay(100);
  EEPROM.commit();
  delay(500);
  EEPROM.end();
  delay(500);
  return;
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
  graph1.pushSprite(20, 105);
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
      graph1.drawLine(198, 150 - 150 * ((Plot_buffer[array_index - 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199, 150 - 150 * ((Plot_buffer[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_NEIGHBOUR_BEIGE);
      // printf("%d\n",150 - 150 * ((H[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)));
    }
    if (isPlotrangeChange == 1 && array_index > 0){ // redraw
      graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
      for (int c = 0; c < array_index; c++){
        // graph1.drawFastVLine(199 - (array_index  - c), 150 - 150 * ((H[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)),1, TFT_YELLOW);
        graph1.drawLine(199 - (array_index - c), 150 - 150 * ((Plot_buffer[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199 - (array_index - 1 - c), 150 - 150 * ((Plot_buffer[c + 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_NEIGHBOUR_BEIGE);
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

void draw_result_bar(double bar_CO2, double bar_Ace){
  // const int top_y =60;
  // const int base_y =190;
  double CO2_max = 1.5;double CO2_min = 0.9;
  double ACE_max = 1.4;double ACE_min = 0.6;
  if (bar_CO2 > CO2_max){
    bar_CO2 = CO2_max;
  }
  if (bar_CO2 < CO2_min){
    bar_CO2 = CO2_min;
  }
  if (bar_Ace > ACE_max){
    bar_Ace = ACE_max;
  }
  if (bar_Ace < ACE_min){
    bar_Ace = ACE_min;
  }
  tft.fillRoundRect(20,135,200,10,5,TFT_PaleYellow);
  tft.fillRoundRect(20,200,200,10,5,TFT_PaleYellow);
  
  tft.fillRect(20,135,200.00*(bar_Ace - ACE_min)/(ACE_max- ACE_min),10,TFT_TextBrown);
  tft.fillRect(20,200,200.00*(bar_CO2 - CO2_min)/(CO2_max- CO2_min),10,TFT_TextBrown);
  tft.pushImage(20 + 200.00*(bar_Ace - ACE_min)/(ACE_max- ACE_min) - Pointer_width/2,135-Pointer_height,Pointer_width,Pointer_height,Pointer);
  tft.pushImage(20 + 200.00*(bar_CO2 - CO2_min)/(CO2_max- CO2_min) - Pointer_width/2,200-Pointer_height,Pointer_width,Pointer_height,Pointer);
  tft.drawFastVLine(120,135,10,TFT_PaleYellow);
  tft.drawFastVLine(120,200,10,TFT_PaleYellow);
} 

void display_start_button(){
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(CC_DATUM);
  tft.drawString("BREATHE AGAIN",120,245,2);
}

void update_sensor_lifecount(bool display){
  //retrieve sensor life count from EEPROM address 12 and display at the bottom corner of the screen ,alighned to the top left of the text
  extern uint8_t lifecount_address; 
  EEPROM.begin(512);
  lifecount = 0 ;
  lifecount = EEPROM.get(lifecount_address,lifecount);
  delay(100);
  Serial.print("lifecount:");Serial.println(lifecount);
  EEPROM.end();
  delay(500);
  if(display == false){
    return;
  }else{
    tft.setTextColor(TFT_TextBrown,TFT_NEIGHBOUR_BEIGE );
    tft.setTextDatum(ML_DATUM);
    tft.drawString("Sensor Life :", 20, 215, 2);
    tft.setTextDatum(CC_DATUM);
    tft.drawNumber((int)(lifecount*20/3), 120, 215, 2);
    tft.drawString("%", 140, 215, 2);
  }
}

void draw_result(double co2, double ace){
  // extern bool isStore;
  extern bool fail_count;

  // creat a pointer of array of result according to the ace and co2 concentration
  char *result[]={"Try Again","Inactive workout","Moderate burn","Effective training","Moderate Ketosis","Deep Ketosis", "Ketoacidosis"};
  tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
  draw_framework();
  tft.pushImage(0, 280, BarWidth, BarHeight, Bar);
 
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_NEIGHBOUR_BEIGE );
  tft.drawString("Results", 15, 50, 4);

  tft.setTextColor(TFT_TextBrown, TFT_NEIGHBOUR_BEIGE );
  if(fail_count == true){
    tft.drawString("Result interfered by multiple breath", 15, 80, 2);
  }
  // if(fail_count != 50){ 
    tft.setTextDatum(CC_DATUM);
    tft.drawString("Ketone",85,110,4);tft.drawString(":",130,110,4);
    tft.setTextDatum(ML_DATUM);tft.drawFloat(ace,2,140, 110,4);
    tft.setTextColor(TFT_TextBrown, TFT_NEIGHBOUR_BEIGE);
    tft.setTextDatum(CC_DATUM);
    tft.drawString("CO2",85,175,4);tft.drawString(":",130,175,4);
    tft.setTextDatum(ML_DATUM);tft.drawFloat(co2,2,140, 175,4);
    draw_result_bar(co2,ace);
  // }
}

void Warmup_Screen(){
  extern double PID_Setpoint;
  unsigned long counttime = 0;
  double display_warmup = 0;
  double warmup_bar_base  = (double)ads.readADC_SingleEnded(NTCC_channel) - PID_Setpoint;
  int temperature_range = 10;
  int i = 0;
   tft.fillScreen(TFT_NEIGHBOUR_GREEN);

   tft.pushImage(30, 80, Beagle_WarmupWidth, Beagle_WarmupHeight, Beagle_Warmup);
   extern double Output;
   while(abs(ads.readADC_SingleEnded(NTCC_channel)-(int)PID_Setpoint) > temperature_range){ 
      Serial.print("Warmup:");Serial.println(abs(ads.readADC_SingleEnded(NTCC_channel)-(int)PID_Setpoint));
      Serial.print("Output");Serial.println(Output);
      PID_control();
      if(ads.readADC_SingleEnded(NTCC_channel) < (int)PID_Setpoint){
        tft.pushImage(30, 200, Bubble6_w,Bubble6_h,Bubble_6);
        tft.pushImage(70, 200, Bubble2_w,Bubble2_h,Bubble_2);
        tft.pushImage(110, 200, Bubble3_w,Bubble3_h,Bubble_3);
        tft.pushImage(150, 200, Bubble4_w,Bubble4_h,Bubble_4);
        tft.pushImage(190, 200, Bubble5_w,Bubble5_h,Bubble_5);
     }else{
        // display_warmup = abs ((double)ads.readADC_SingleEnded(NTCC_channel)-PID_Setpoint);
        tft.pushImage(23, 200, Bubble6_w,Bubble6_h,Bubble_6);
        PID_control();
        delay(100);
        tft.pushImage(63, 200, Bubble2_w,Bubble2_h,Bubble_2);
        PID_control();
        delay(100);
        tft.pushImage(103, 200, Bubble3_w,Bubble3_h,Bubble_3);
        PID_control();
        delay(100);
        tft.pushImage(143, 200, Bubble4_w,Bubble4_h,Bubble_4);
        PID_control();
        delay(100);
        tft.pushImage(183, 200, Bubble5_w,Bubble5_h,Bubble_5);
        PID_control();
        delay(100);
        tft.fillRect(0,180,240,100,TFT_NEIGHBOUR_GREEN);
        PID_control();
        delay(100);
     }
   delay(10);
  }
  //tft.fillRect(20,200,200,80,TFT_NEIGHBOUR_GREEN);   // cover graph 
}

void HomeScreen()
{
  tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
  tft.pushImage(0, 280, BarWidth, BarHeight, Bar);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  // tft.pushImage(20, 230, BreatheWidth, BreatheHeight, Breathe);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_NEIGHBOUR_BEIGE );
  tft.drawString("Lets Get Started", 15, 50, 4);

  tft.setTextDatum(CC_DATUM);
  tft.setTextColor(TFT_WHITE,TFT_NEIGHBOUR_GREEN);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
  tft.drawString("BREATHE",120, 245,2);
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);

  display_Wifi();

  update_sensor_lifecount(true);
  delay(150);
}



void display_menu(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  display_Wifi();
  tft.setTextDatum(0);
  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_NEIGHBOUR_BEIGE );
  tft.drawString("Setting", 15, 50, 4);

  tft.setTextColor(TFT_TextBrown ,TFT_PaleYellow);
  tft.fillRoundRect(15,100,210,30,3,TFT_PaleYellow);tft.drawString("Wifi",30,107,2);
  tft.fillRoundRect(15,240,210,30,3,TFT_PaleYellow);tft.drawString("Developer Mode",30,247,2);

  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  tft.fillRoundRect(15,140,210,30,3,TFT_DARKGREY);tft.drawString("Calibration",30,147,2);
  tft.fillRoundRect(15,170,210,30,3,TFT_DARKGREY);tft.drawString("User ID",30,177,2);
  tft.fillRoundRect(15,200,210,30,3,TFT_DARKGREY);tft.drawString("Language",30,207,2);

  delay(300);
}

void display_Wifi(){             // draw wifi logo
  if (isWifi == true && isConnect == true)
  {
    tft.pushImage(180, 8, Wifi_true_w, Wifi_true_h, Wifi_true);
  }
  else if (isWifi == true && isConnect == false)
  {
    tft.pushImage(180, 8, Wifi_nc_w, Wifi_nc_h, Wifi_nc);
  }
  else
  {
    tft.pushImage(180, 8, Wifi_false_w, Wifi_false_h, Wifi_false);
  }
}


void display_calibration(){
  Reset_coordinate();
  draw_Settingframework();
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Calibration", 15, 50, 4);
  tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(CC_DATUM);
  tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);tft.drawString("START",110,235,2);
}

void display_OTA_control(){
  tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE );
  tft.drawString("Wifi", 15, 50, 4); // OTA Setting
  tft.setTextColor(TFT_TextBrown ,TFT_PaleYellow);
  tft.fillRoundRect(15,100,210,30,3,TFT_PaleYellow);tft.drawString("On",30,107,2);
  tft.fillRoundRect(15,140,210,30,3,TFT_PaleYellow);tft.drawString("OFF",30,147,2);
  tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);

}
void display_sampling_init(){
  tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Initializing", 15, 50, 4);
}
void display_developer_menu(){
  Reset_coordinate();
  tft.setTextDatum(TL_DATUM);
  draw_Settingframework();
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Developer Mode", 15, 50, 4);
  tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);

  tft.setTextColor(TFT_TextBrown ,TFT_PaleYellow);
  tft.fillRoundRect(15,105,210,30,3,TFT_PaleYellow);tft.drawString("Live Plot",30,112,2);
  tft.fillRoundRect(15,185,210,30,3,TFT_PaleYellow);tft.drawString("Print Spiffs",30,192,2);
  tft.fillRoundRect(15,225,210,30,3,TFT_PaleYellow);tft.drawString("Previous Value",30,232,2);

  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  tft.fillRoundRect(15,145,210,30,3,TFT_DARKGREY);
  tft.drawString("Default Setting",30,152,2);
}

void display_live_plot(){
  draw_Settingframework();
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_NEIGHBOUR_BEIGE);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Live Plot", 15, 50, 4);
  tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);
}
void display_previous_value(){
  tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
  tft.pushImage(15, 10, BeagleWidth, BeagleHeight, Beagle);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN,TFT_NEIGHBOUR_BEIGE );
  tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
  tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Previous Value", 15, 50, 4);
  int DataCounter = 0;

  tft.setTextDatum(CC_DATUM);
  tft.setTextColor(TFT_TextBrown,TFT_NEIGHBOUR_BEIGE );
  tft.drawString("CO2", 80, 100, 2);
  tft.drawString("Acetone", 160, 100, 2);
  retrieve_record();
  for (int i = 0; i < 10; i++)
  {
    if(recorded_gas_sample[i][0] != 0)
    {
      
  //     Serial.print("plotting: "); Serial.println(recorded_gas_sample[i]);
  //     tft.fillCircle((i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 60, 2, TFtouch_yELLOW);
  //     tft.setTextColor(TFT_YELLOW, TFT_NEIGHBOUR_GREEN);
  //     // if (i == 0 || i == 2 || i == 4 || i == 6 || i == 8)
  //     if (i %2 ==0)
  //     {
  //       tft.drawFloat(recorded_gas_sample[i], 2, (i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 40, 1);
  //     }
  //     // if (i == 1 || i == 3 || i == 5 || i == 7 || i == 9)
  //     else
  //     {
  //       tft.drawFloat(recorded_gas_sample[i], 2, (i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 20, 1);
  //     }
  //     DataCounter++;
  //   }
  // }
  // if (DataCounter > 1)
  // {
  //   for (int i = 0; i < DataCounter - 1; i++)
  //   {
  //     tft.drawLine((i + 2) * 20, (120 - 120 * ((recorded_gas_sample[i] - 0.9) / 1.1)) + 60, (i + 3) * 20, (120 - 120 * ((recorded_gas_sample[i + 1] - 0.9) / 1.1)) + 60, TFT_YELLOW);
  //   }
  // }
      
      Serial.println(recorded_gas_sample[i][0]);Serial.print(","); Serial.println(recorded_gas_sample[i][1]);
      tft.setTextColor(TFT_TextBrown,TFT_NEIGHBOUR_BEIGE );
      tft.drawNumber(i+1, 22, 110+15 *(i+1),2);
      tft.drawFloat(recorded_gas_sample[i][0],2,80,110+15*(i+1),2);
      tft.drawFloat(recorded_gas_sample[i][1],2,160,110+15*(i+1),2);
      // tft.fillCircle((i + 2) * 20, (120 - 120 * ((previous_data[i] - 0.9) / 1.1)) + 60, 2, TFT_NEIGHBOUR_GREEN);
      // tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE );
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
}
void display_bluetooth(){
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
  tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
  tft.drawString("Bluetooth", 120, 30, 4);
}

void display_setup_profile_select(){
  Reset_coordinate();
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

void display_setup_pump()
{
  Reset_coordinate();
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

void display_setup_PID()
{
  Reset_coordinate();
  tft.fillScreen(TFT_NEIGHBOUR_GREEN);
  // tft.pushImage(setting_x, setting_y, settingWidth, settingHeight, setting);
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
  Reset_coordinate();
  delay(300);
}

void display_device_setting(){
  Reset_coordinate();
  draw_Settingframework;
  display_Wifi();

  tft.pushImage(208, 10, FullBattaryWidth, FullBattaryHeight, FullBattary);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  tft.drawString("Default Setting", 15, 50, 4);
  tft.pushImage(0, 100, DefaultSettingWidth, DefaultSettingHeight, DefaultSetting);
}

void display_Spiffs_data(int page)
{
  int buffer_y = 5;
  int buffer_x = 15;
  int rect_length = 210;
  int rect_height = 30;
  int rect_radius = 3;
  String file_name = "Dataset_";
  Reset_coordinate();
  tft.setTextDatum(0);
  tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  draw_Settingframework();
  tft.drawString("Spiffs", 15, 50, 4);
  tft.pushImage(205, 80, Return_arrow_width, Return_arrow_height, Return_arrow);
  tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_TextBrown ,TFT_PaleYellow);
  for(int i = 0; i < 5; i++){
    tft.fillRoundRect(buffer_x,105 +rect_height*i + buffer_y*i,rect_length,rect_height,rect_radius,TFT_PaleYellow);
    tft.drawString(file_name + String(i+1+page*5),30,120 +35*i,2);
  }
  delay(150);
}

void display_profile_filenumber()
{
  if (touch_x > 110 && touch_x <190 && touch_y > 0 && touch_y <145)
  {
    if (profileNumber_int <10)
    {
      profileNumber_int = profileNumber_int + 1;
      Reset_coordinate();
      delay(150);
    }
  }
  else if (touch_x > 110 && touch_x < 190 && touch_y > 160 && touch_y < 300)
  {
    if (profileNumber_int > 1)
    {
      profileNumber_int = profileNumber_int - 1;
      Reset_coordinate();
      delay(150);
    }
  }
}

void display_pump_selectDutycycle()
{
  if (touch_x > 110 && touch_x <190 && touch_y > 0 && touch_y <145)
  {
    if (dutyCycle_pump < 255)
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
  if (isSensor != true)
  {
    tft.fillScreen(TFT_NEIGHBOUR_GREEN);
    tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE);
    tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
    tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BEIGE);
    tft.drawString("Change sensor", 120, 160, 4);
    stage = change_sensor ;
  }

  while(stage == change_sensor ){
    if (tft.getTouch(&touch_x, &touch_y)){
      if(touch_x > 180 && touch_x < 200 && touch_y > 5 && touch_y <200){
        extern uint8_t lifecount_address;
        int lifecount;
        EEPROM.begin(512);
        EEPROM.put(lifecount_address, 15);
        delay(100);
        EEPROM.commit();
        delay(500);
        EEPROM.get(lifecount_address, lifecount);
        Serial.print("lifecount = "); Serial.println(lifecount);
        EEPROM.end();
        delay(500);
        // update_sensor();
        // update_check_time();
        isSensor = true;
        stage = homescreen;
        tft.fillScreen(TFT_NEIGHBOUR_BEIGE);
        HomeScreen();
        delay(150);
        Reset_coordinate();
        break;
      }
    }
  }

  if (tft.getTouch(&touch_x, &touch_y))
  {
    printf("%d\n", touch_x);
    printf("%d\n", touch_y);

    if (stage == homescreen)
    {
      if (touch_x > 180 && touch_x < 200 && touch_y > 5 && touch_y < 200)
      {
        if(lifecount == 0){
          tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
          tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);
          tft.setTextDatum(CC_DATUM);
          tft.drawString("Change sensor to continue",120,245,2);
          delay(150);
          Reset_coordinate();
          stage = change_sensor;
        }
        else{
          delay(150);
          Reset_coordinate();
          stage = sampling;
        }
      }
    }
  

    if (stage !=setting_menu)
    {
      if (touch_x > 220 && touch_x < 240 && touch_y > 0 && touch_y < 85)                 // press
      {
        pump_control(false);
        Serial.println("OFF");
        display_menu();
        stage = setting_menu;
      }
    }
    if(stage != homescreen){
      if (touch_x > 220 && touch_x < 240 && touch_y > 220 && touch_y < 320) // Return
      {
        stage = homescreen;
        HomeScreen();
        Reset_coordinate();
      }
    }

    if (stage == setting_menu)
    { // Navigation
      if (touch_x > 85 && touch_x < 105 && touch_y > 10 && touch_y < 300)                //OTA
      {
        draw_Settingframework();
        display_OTA_control();
        delay(200);
        Reset_coordinate();
        stage = wifi_control;
      }

      if (touch_x > 185 && touch_x < 200 && touch_y > 10 && touch_y < 280)
      {
        display_developer_menu();
        stage = developer_mode ;
        delay(300);
      }

      if (touch_x > 109 && touch_x < 129 && touch_y > 10 && touch_y < 290) // Calibration
      {
        if(isCal == true){
          stage = calibration;
          display_calibration();
        }
      }

      if (touch_x > 220 && touch_x < 240 && touch_y > 220 && touch_y < 320) // Return
      {
        stage = homescreen;
        HomeScreen();
        Reset_coordinate();
      }
    }

    if(stage == sampling){//sample
      if(isConnect == true){
        WiFi.disconnect(true,true);
        delay(500);
        isConnect = false;
        Serial.println("Disconnected");
        Serial.print("Wifi status:");Serial.println(WiFi.status());
      }
      draw_framework();
      screen_count(blow_address);
  //       tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);
  // tft.setTextColor(TFT_NEIGHBOUR_GREEN);
  // tft.setTextDatum(TL_DATUM);
  // tft.drawString("Initializing", 15, 50, 4);
      display_sampling_init();
      sample_collection();
      if(leave != true){
        output_result();
        screen_count(sample_address);
        update_sensor_lifecount(false);
        stage = homescreen;
      }
    }

    if (stage == calibration)
    { // Calibration Start Button
      if (touch_x > 180 && touch_x < 200 && touch_y > 5 && touch_y < 200)
      {
        draw_Settingframework();
        tft.setTextColor(TFT_NEIGHBOUR_GREEN);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("Initializing", 15, 50, 4);
        pump_control(true);
        sensor_heater_control(true);
        restore_baseline();
        tft.setTextDatum(4);
        tft.fillRect(10, 50, 200, 210, TFT_NEIGHBOUR_BEIGE);  
        tft.setTextColor(TFT_TextBrown, TFT_NEIGHBOUR_BEIGE);
        //count down three second and display on screen 
        for (size_t i = 0; i < 3; i++)
        {
          String countdown = "Starting in " + String(3 - i);
          tft.drawString(countdown, 120, 120, 4);
          delay(1000);
          tft.fillRect(10, 80, 200, 150, TFT_NEIGHBOUR_BEIGE);
        }
        
        Calibration();
        pump_control(false);
        sensor_heater_control(false);
        tft.fillRect(0, 80, 240, 180, TFT_NEIGHBOUR_BEIGE);
        EEPROM.begin(512);
        int value, value_1;
        byte address = 0;
        EEPROM.get(address, value);
        delay(100);
        tft.drawNumber(value, 80, 120, 2);
        address += sizeof(int);
        EEPROM.get(address, value_1);
        delay(100);
        tft.drawNumber(value_1, 160, 120, 2);
        EEPROM.end();
        delay(500);
        tft.setTextColor(TFT_NEIGHBOUR_GREEN);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("Calibration", 15, 50, 4);
        tft.setTextColor(TFT_WHITE, TFT_NEIGHBOUR_GREEN);
        tft.setTextDatum(CC_DATUM);
        tft.fillRoundRect(20,230,200,30,3,TFT_NEIGHBOUR_GREEN);tft.drawString("START",120,245,2);
      }
    }


    if (stage == developer_mode )
    { // developer mode                                                                     // developer mode choices
      tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
      if (touch_x > 65 && touch_x < 80 && touch_y > 270 && touch_y < 295){
        display_menu();
        stage = setting_menu;
      }
      if (touch_x > 85 && touch_x < 105 && touch_y > 10 && touch_y < 285)
      {
        display_live_plot();
        stage = live_plot;
        delay(400);
      }
      // else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305)
      // {
      //   tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
      //   tft.drawString("Default Setting", 120, 100, 4);
      //   delay(200);
      //   display_device_setting();
      //   stage = device_setting;
      //   delay(300);
      // }

      else if (touch_x > 150 && touch_x < 165 && touch_y > 10 && touch_y < 285)
      {
        delay(200);
        tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
        tft.pushImage(0, 280, SettingBarWidth, SettingBarHeight, SettingBar);
        Reset_coordinate();
        page_number = 0;
        display_Spiffs_data(page_number);
        stage = print_stored_data ;
      }
      else if (touch_x > 180 && touch_x < 195 && touch_y > 10 && touch_y < 285)
      {
        delay(200);
        Reset_coordinate();
        display_previous_value();
        stage = previous_value;
      }
      if (touch_x > 220 && touch_x < 240 && touch_y > 220 && touch_y < 320) // Return
      {
        stage = 0;
        HomeScreen();
        Reset_coordinate();
      }
    }
    if (stage == previous_value)
    {
      if (touch_x > 65 && touch_x < 80 && touch_y > 270 && touch_y < 295){
          display_developer_menu();
          stage = developer_mode;
          delay(400);
        }
    }
    
    if (stage == print_stored_data )      //print spiffs
    {
      String file_name = "/Dataset_";
      if (touch_x > 65 && touch_x < 80 && touch_y > 10 && touch_y < 40){
        if(page_number >=3){
        }else{
          page_number  += 1;
          display_Spiffs_data(page_number);
        }
      }

      if (touch_x > 65 && touch_x < 80 && touch_y > 270 && touch_y < 295){
        if(page_number <=0){
          display_developer_menu();
          stage = developer_mode;
          delay(400);
        
        }else{
          page_number  -= 1;
          display_Spiffs_data(page_number);
        }
      }

      if (touch_x > 85 && touch_x < 115 && touch_y > 10 && touch_y < 285)
      {
        file_name.concat(page_number*5 +1);
        if (SPIFFS.exists(file_name))
        {
          File file = SPIFFS.open(file_name, FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
      else if (touch_x> 115 && touch_x < 140 && touch_y > 10 && touch_y < 285)
      {
        file_name.concat(page_number*5 +2);
        if (SPIFFS.exists(file_name))
        {
          File file = SPIFFS.open(file_name, FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
       else if (touch_x> 140 && touch_x < 165 && touch_y > 10 && touch_y < 285)
      {
        file_name.concat(page_number*5 +3);
        if (SPIFFS.exists(file_name))
        {
          File file = SPIFFS.open(file_name, FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
       else if (touch_x> 165 && touch_x < 190 && touch_y > 10 && touch_y < 285)
      {
        file_name.concat(page_number*5 +4);
        if (SPIFFS.exists(file_name))
        {
          File file = SPIFFS.open(file_name, FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
       else if (touch_x> 190 && touch_x < 220 && touch_y > 10 && touch_y < 285)
      {
        file_name.concat(page_number*5 +5);
        if (SPIFFS.exists(file_name))
        {
          File file = SPIFFS.open(file_name, FILE_READ);
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
      }
    }

    if (stage == select_user_profile){     // user_setup
      tft.setTextDatum(4);
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      display_profile_filenumber();
      tft.fillRect(100, 80, 50, 30, TFT_NEIGHBOUR_GREEN);  //cover file number
      tft.drawNumber(profileNumber_int, 120, 100, 4);
      printf("%d\n", profileNumber_int);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){    // define file number
        profileNumber = (String)profileNumber_int;
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE); // change colour
        delay(200);
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        display_menu();
        stage = setting_menu;
      }
    }

    if (stage == device_setting){
      if (touch_x > 15 && touch_x < 55 && touch_y > 0 && touch_y < 305){  
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);    //Pump dutycycle
        tft.fillRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 10, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Pump power", 120, 35, 4);
        delay(200);
        display_setup_pump();
        stage = pump_setting;
      }
      else if (touch_x > 60 && touch_x < 100 && touch_y > 0 && touch_y < 305){
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);    //Pump dutycycle
        tft.fillRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 75, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Columm Temp", 120, 100, 4);
        delay(200);
        display_setup_PID();
        stage = PID_setting;
      }
      else if (touch_x > 105 && touch_x < 145 && touch_y > 0 && touch_y < 305){
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);    //Pump dutycycle
        tft.fillRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawRoundRect(10, 135, 220, 44, 22, TFT_NEIGHBOUR_BLUE);
        tft.drawString("Sampling Time", 120, 160, 4);
        delay(200);
        // display_setup_pump();
        // stage =15;
      } // User Set up

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
        else
        {
          update_sensor();
          update_check_time();
          Serial.println("Timer updated");
        }
      }
    }

    if (stage == PID_setting){
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      display_PID_selectSetpoint();
      tft.fillRect(80, 80, 70, 30, TFT_NEIGHBOUR_GREEN);  //cover pump cycle
      tft.drawFloat(PID_Setpoint, 0, 120, 100, 4);
      printf("%d\n", PID_Setpoint);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){    
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE); // change colour
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        display_menu();
        stage = setting_menu;
      }
    }


    if (stage == live_plot){ // developer mode:ADS0 
      graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
      screen_count(plot_address);
      pump_control(true);
      sensor_heater_control(true);
      double load_resistance = 47000;
      double input_voltage = 3.3;
      double sensor_resistance = 0;
      double sensor_voltage = 0;
      tft.setTextColor(TFT_TextBrown, TFT_NEIGHBOUR_BEIGE);
      while (1){
        if (tft.getTouch(&touch_x, &touch_y))
        {
          if (touch_x > 65 && touch_x < 80 && touch_y > 270 && touch_y < 295)
          {
            pump_control(false);
            sensor_heater_control(false);
            break;
          }
        }
        PID_control();  
        int ADS0 = ads.readADC_SingleEnded(Sensor_channel);
        sensor_voltage = ads.computeVolts(ADS0);
        sensor_resistance = ((load_resistance * input_voltage)/sensor_voltage) - load_resistance;
        int heater = ads.readADC_SingleEnded(Heater_channel);
        int offset = ads.readADC_SingleEnded(Offset_channel);
        int ntcc = ads.readADC_SingleEnded(NTCC_channel);

        graph1.pushSprite(20, 105);

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

          tft.fillRect(205, 90, 30, 10, TFT_NEIGHBOUR_BEIGE );
          tft.fillRect(205, 260, 30, 10, TFT_NEIGHBOUR_BEIGE );
          tft.setTextDatum(BC_DATUM);
          tft.drawNumber(plot_upper_bound, 220, 100, 1);
          tft.setTextDatum(TC_DATUM);
          tft.drawNumber(plot_lower_bound, 220, 260, 1);
          tft.setTextDatum(TL_DATUM);
          // tft.drawFloat(float(ADS0), 0, 65, 240, 2);
          // tft.drawString("ADS0:", 25, 240, 2);


          if (isPlotrangeChange == false && array_index > 0) // draw
          {
            graph1.scroll(-1);
            graph1.drawLine(198, 150 - 150 * ((Plot_buffer[array_index - 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199, 150 - 150 * ((Plot_buffer[array_index] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_NEIGHBOUR_BEIGE );
          }
          if (isPlotrangeChange == true && array_index > 0) // redraw
          {
            graph1.fillSprite(TFT_NEIGHBOUR_GREEN);
            for (int c = 0; c < array_index; c++)
            {
              graph1.drawLine(199 - (array_index - c), 150 - 150 * ((Plot_buffer[c] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), 199 - (array_index - 1 - c), 150 - 150 * ((Plot_buffer[c + 1] - plot_lower_bound) / (plot_upper_bound - plot_lower_bound)), TFT_NEIGHBOUR_BEIGE );
            }
            isPlotrangeChange = false;
          }
          if (array_index == 199){ // When array_index >200, H[array_index-1] = H[array_index]
            for (int j = 1; j <= 199; j++)
            {
              Plot_buffer[j - 1] = Plot_buffer[j];
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
          extern double Output;
          
          if (sht.readSample()) {
            Serial.print(sht.getHumidity(), 2);Serial.print(","); 
            Serial.print(sht.getTemperature(), 2); Serial.print(","); 
            Serial.print(ADS0);Serial.print(",");
            Serial.print(sensor_resistance);Serial.print(",");
            Serial.print(heater);Serial.print(",");
            Serial.print(offset);Serial.print(",");
            Serial.print(Output);Serial.print(",");
            Serial.println(ntcc); 
            // Serial.println(analogRead(battery_read));
          }
        }
      }
      display_menu();
      stage = setting_menu;
    }

    if (stage == pump_setting){  //select pump duty cycle
      tft.setTextColor(TFT_NEIGHBOUR_BEIGE, TFT_NEIGHBOUR_GREEN);
      tft.setTextDatum(4);
      display_pump_selectDutycycle();
      tft.fillRect(100, 80, 50, 30, TFT_NEIGHBOUR_GREEN);  //cover pump cycle
      tft.drawNumber(dutyCycle_pump, 120, 100, 4);

      if (touch_x > 195 && touch_x < 240 && touch_y > 220 && touch_y < 305){    
        tft.setTextColor(TFT_BLACK, TFT_NEIGHBOUR_BLUE);
        tft.fillRoundRect(8, 264, 60, 46, 23, TFT_NEIGHBOUR_BLUE); // change colour
        tft.drawString("Set", 38, 287, 2);
        delay(500);
        display_menu();
        stage = setting_menu;
      }
    }
    
    if (stage == wifi_control){
      if (touch_x > 65 && touch_x < 80 && touch_y > 270 && touch_y < 295){
        display_menu();
        stage = setting_menu;
      }

      if (touch_x > 90 && touch_x < 105 && touch_y > 0 && touch_y < 285)    //WIFI on 
      { // WIFI
        draw_Settingframework(); 
        extern bool isWifi;
        tft.setTextDatum(CC_DATUM);
        tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE );
        tft.drawString("WiFi Setting",120, 50, 4);
        isWifi = true;

        Wifi_able();
        configTime(28800, 0, ntpServer);
        if (isConnect == true)
        {
          tft.drawString("Connected", 120, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
          stage = homescreen;    
          HomeScreen();    
          display_Wifi();
        }
        else
        {
          tft.drawString("Not Connected", 120, 100, 4);
          delay(2000);
          tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE );
          draw_Settingframework();
          tft.setTextDatum(0);
          tft.drawString("Wifi", 15, 50, 4); // OTA Setting
          tft.setTextColor(TFT_TextBrown ,TFT_PaleYellow);
          tft.fillRoundRect(15,100,210,30,3,TFT_PaleYellow);tft.drawString("On",30,107,2);
          tft.fillRoundRect(15,140,210,30,3,TFT_PaleYellow);tft.drawString("OFF",30,147,2);
          tft.pushImage(15, 80, Return_arrow_flip_width, Return_arrow_flip_height, Return_arrow_flip);

          delay(200);
          Reset_coordinate();
          stage = wifi_control;
        }
      }

      else if (touch_x > 120 && touch_x < 140 && touch_y > 10 && touch_y < 285)  //WIFI off
      {
        draw_Settingframework();
        Reset_coordinate();
        extern bool isWifi;
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_NEIGHBOUR_GREEN, TFT_NEIGHBOUR_BEIGE);
        tft.drawString("WiFi Setting", 50, 50, 4);
        Wifi_disable();
        if (isWifi == false)
        {
          tft.drawString("OFF", 95, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
          stage = homescreen;  
          HomeScreen();
        }
        else
        {
          tft.drawString("Failed", 95, 100, 4);
          delay(2000);
          tft.fillScreen(TFT_NEIGHBOUR_BEIGE );
          display_control_wifi();
        }
      }
    }
  }
}