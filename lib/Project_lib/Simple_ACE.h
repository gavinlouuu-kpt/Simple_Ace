#ifndef Simple_ACE_h 
#define Simple_ACE_h

#include <time.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <EEPROM.h>

#define PASSWORD            "10200718"
#define SSID                "KPTESP32"
// #define BLYNK_TEMPLATE_ID   "TMPL7mbkwSQ6"
// #define BLYNK_DEVICE_NAME   "SimpleCoCo 1"
// #define BLYNK_AUTH_TOKEN    "pcRimMdKvxJzGShG1ApwLPZzGzt6GKRM"
// #define BLYNK_PRINT         Serial

#define ASD1115 			0x48
#define BLACK				0x0000
#define baseSample 			20

//Lilygo T-Disaply//
// #define fanPin              15
// #define pumpPin             25
// #define solPin              26
// #define buz                 27
// #define colPin              32
// #define btn_rst             33
// #define NTCC                39

//Wrover 2.8" display//
#define pumpPin             25
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

#define store_size 			5000 //Number of data collect within sample time //

#define pumpChannel 		0
#define solChannel 			1
#define colChannel 			2
#define fanChannel 			3

#define sampletime          60000//60000
#define wait_time           10000   // Time for the sensor take reading (seconds)//

const int freq = 5000;
const int resolution = 8;
const int zone = 5000;
const int dutyCycle_pump = 65; //to be changed
const int dutyCycle_col = 150;
const double LSB = 0.125 / 1000;
const int temperate = 55; 

extern bool clean;
extern bool store;
extern short CO2_arr[store_size];
extern double ratio_Ace;

void checkSetup(void);
void pinSetup(void);
void analogSetup(void);
void output_result();

void sample_collection();
void power_saving(unsigned long last_time);
unsigned long getTime();


#endif 