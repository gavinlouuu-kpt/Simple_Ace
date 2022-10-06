#ifndef Calibration_h
#define Calibration_h

#define SAMPLE_LENGTH 10000
#define show_peak 3
#define peak_candidate 100

extern double slope;
extern double constant;
const double ref_1_conc = 1;
const double ref_2_conc = 2;

<<<<<<< HEAD
=======
float ref_position[2];
>>>>>>> 963a53c8f5f10b636a206ae60af293f4fc83487f


void calibration_setup(int value);
void calibration();

#endif 