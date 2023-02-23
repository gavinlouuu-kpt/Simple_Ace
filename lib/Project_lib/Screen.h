#ifndef Screen_h
#define Screen_h

void display_assets();
void tft_setup();
void draw_sensor(double value);
void draw_time(int time);
void draw_result(double ace,double co2);
void write_analyzing(void);
void Navigation();
void display_Wifi();
void draw_sample_progress(float bar_time, float bar_percentage);
void display_loading(int count);
void Warmup_Screen();
void HomeScreen();
#endif

