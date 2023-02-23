#ifndef Simple_ACE_h 
#define Simple_ACE_h

#define BLACK				0x0000
#define baseline_window  50


//Wrover 2.8" display///////////////////////////////////////////////      
#define colPin_1            26
#define colPin_2            13
#define pumpPin_1           27
#define pumpPin_2           32
 
#define buz                 13
#define btn_rst             39
#define btn_1               36
#define BOOT                0
#define sensor_h            25

// //Wrover IB///////////////////////////////////////////////////
// #define pumpPin             32  
// #define solPin              33
// #define buz                 13
// #define colPin              27  
// #define btn_rst             39
// #define btn_1               36
// #define NTCC                34
// #define BOOT                0
// #define Off_s               26
// #define senH                25

#define Heater_channel  		2 //raw adc
#define Sensor_channel  		1 //raw adc
#define Offset_channel  		0 //raw adc
#define NTCC_channel  		    3 //raw adc
#define EEP_add 			0
#define EEP_add_1           1

#define store_size 			2048 //Number of data collect within sample time //

#define pumpChannel_1 		0
#define pumpChannel_2 		1
#define colChannel_1 		2
#define colChannel_2 		3

#define wait_time           10000   // Time for the sensor take reading (seconds)//

const int freq = 20000;
const int resolution = 8;
const double LSB = 0.125 / 1000;
// const int temperate = 55; 
const int sampletime = 45000;       //60000

void checkSetup(void);
void pinSetup(void);
void output_result();
void pump_control(bool control);
void sample_collection();
void storing_data();
// void power_saving(unsigned long last_time);

double baselineRead(int channel);
int restore_baseline();
// double ads_convert(int value, bool resist);

#endif 