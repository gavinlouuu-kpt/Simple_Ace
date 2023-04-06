#ifndef Screen_h
#define Screen_h

#define homescreen              0
#define setting_menu            1
#define sampling                2
#define calibration             3  
#define OTA_setting             4
#define developer_mode          5
#define liveplot_control        6
#define device_setting          7
#define bluetooth_setting       8
#define wifi_control            9   
#define print_stored_data       10
#define select_user_profile     11
#define previous_value          12
#define live_plot               13
#define pump_setting            15
#define PID_setting             16
#define print_gas_sample        17
#define change_sensor           99

void tft_setup();
void draw_sensor(double value);
void draw_sample_progress(float bar_time, float bar_percentage);
void draw_result(double ace,double co2);
void display_assets();
void display_loading(int count);
void display_Wifi();
void Navigation();
void HomeScreen();
void Warmup_Screen();
// void write_analyzing(void);
void screen_count(int screen_address);

void leave_sample();
#endif



