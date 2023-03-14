#ifndef Calibration_h
#define Calibration_h

void Calibration();
void EEPROM_setup(bool factory);
#define SAMPLE_LENGTH 10000
#define show_peak 3

#define peak_candidate 100
#define TFT_NEIGHBOUR_GREEN     0x12E8
#define TFT_MilkWhite  0xEF1A
#define TFT_PaleYellow 0xFFBC
#define TFT_PaleWhite  0xFFCC



// float ref_position[2];

void calibration_setup(int value);
void calibration();
void EEPROM_setup();

#endif 