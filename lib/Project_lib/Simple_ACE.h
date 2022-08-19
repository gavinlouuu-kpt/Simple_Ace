#ifndef Simple_ACE_h 
#define Simple_ACE_h

#include <time.h>
#include <WiFiClient.h>
#include <SHT2x.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <lvgl.h>

#define PASSWORD            "10200718"
#define SSID                "KPTESP32"
// #define BLYNK_TEMPLATE_ID   "TMPL7mbkwSQ6"
// #define BLYNK_DEVICE_NAME   "SimpleCoCo 1"
// #define BLYNK_AUTH_TOKEN    "pcRimMdKvxJzGShG1ApwLPZzGzt6GKRM"
// #define BLYNK_PRINT         Serial

#define ASD1115 			0x48
#define BLACK				0x0000
#define baseSample 			20

//Lilygo T-Disaply
// #define fanPin              15
// #define pumpPin             25
// #define solPin              26
// #define buz                 27
// #define colPin              32
// #define btn_rst             33
// #define NTCC                39

//Wrover 2.8" display
#define pumpPin             32
#define solPin              33
#define buz                 13
#define colPin              27
#define btn_rst             39
#define btn_1               36
#define NTCC                34
#define BOOT                0

#define CO2_channel  		1
#define EEP_add 			0
#define EEP_add_1           1
#define O2_channel 			3

#define store_size 			10500
#define plot_size           9500

#define pumpChannel 		0
#define solChannel 			1
#define colChannel 			2
#define fanChannel 			3

#define sampletime          120

const int freq = 5000;
const int resolution = 8;
const int zone = 5000;
const int dutyCycle = 220;
const double LSB = 0.125 / 1000;
const int temperate = 55; 

extern bool clean;
extern TFT_eSPI tft;
extern bool store;
extern short CO2_arr[store_size];
extern short O2_arr[store_size];
extern double ratio_Ace [3];
extern double ratio_O2 [3];
static lv_obj_t * chart1;
static lv_chart_series_t * ser1;


int baselineRead(int channel);
int restore_baseline();
void analogSetup();
// void blynk_upload(double v1, double v2, double v3, double v4);
void breath_check();
void checkSetup();
void pinSetup();
void power_saving(unsigned long last_time);
void restore_humidity();
void sample_collection(int i);
// void tftSetup();
double ads_convert(int value, bool resist);
double ratio_calibration(double uncal_base, double uncal_reading, bool formula);
double read_humidity();
double sort_reject(double arr[], int arr_size);
double concentration_ethanol( double temp, int baseline);
unsigned long getTime();
void lv_example_chart_2(void);
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data );
void value_label(void);
void prompt_label(void);

#endif 