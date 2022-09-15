#ifndef Screen_h
#define Screen_h

#define TFT_BEIGE       0xFFBC
#define TFT_NEIGHBOUR_GREEN 0x12E8

void draw_framework();
void tft_setup();
void draw_sensor(double value);
void draw_humid(double H);
void draw_result(double value);

#endif

