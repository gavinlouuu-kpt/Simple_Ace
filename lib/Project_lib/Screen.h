#ifndef Screen_h
#define Screen_h


#define TFT_NEIGHBOUR_BEIGE     0xEF1A
#define TFT_NEIGHBOUR_GREEN     0x12E8
#define TFT_NEIGHBOUR_BLUE      0x5EBC

void draw_framework();
void tft_setup();
void draw_sensor(double value);
void draw_time(int time);
void draw_result(double ace,double co2);
void set_range(int value);
void draw_wait(void);
void TouchScreen();
void draw_Wifi();
void draw_progress(float bar_time, float bar_percentage);
void draw_loading(int count);
#endif

