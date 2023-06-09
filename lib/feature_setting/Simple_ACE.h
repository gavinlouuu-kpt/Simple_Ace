#ifndef Simple_ACE_h 
#define Simple_ACE_h

#define colPin_1            27
#define pumpPin_1           26
#define solenoidPin          23

#define buz                 13
#define btn_rst             39
#define btn_1               36
#define BOOT                0
#define sensor_heater       14
#define sensor_heater_pulsing   12
#define SDA_pin             32
#define SCL_pin             33

#define baseline_window     50

#define Offset_channel  	2 //raw adc
#define Sensor_channel  	0 //raw adc
#define Heater_channel  	1 //raw adc
#define NTCC_channel      3 //raw adc

#define pumpChannel_1 		0
#define pumpChannel_2 		1
#define colChannel_1 		2
#define colChannel_2 		3

#define EEP_add 			0
#define EEP_add_1           1
#define store_size 			2400 //Number of data collect within sample time //
#define wait_time           10000   // Time for the sensor take reading (seconds)//

const int freq = 40000;
const int resolution = 8;
const double LSB = 0.125 / 1000;
const int sampletime = 80000;       //60000

double baselineRead(int channel);
void restore_baseline();
void checkSetup(void);
void output_result();
void pinSetup(void);
void pump_control(bool control);
void sensor_heater_control(bool control);
void sample_collection();
void storing_data();
#endif 