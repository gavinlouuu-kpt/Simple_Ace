#ifndef Screen_h
#define Screen_h

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
void write_analyzing(void);
#endif



