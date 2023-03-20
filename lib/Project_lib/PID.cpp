#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <PID_v1.h>
#include "PID.h"
#include "Simple_ACE.h"

void PID_setup();               // initialize PID
void PID_control();             // regualte heating element

extern Adafruit_ADS1115 ads;
double PID_Setpoint = 8000;
double Input, Output;
double consKp=4, consKi=0.08, consKd=0.015;
// double aggKp=4, aggKi=0.2, aggKd=1;
// double consKp=6, consKi=0.125, consKd=0.25;
// double consKp=4, consKi=0.3, consKd=0.6;

PID myPID(&Input, &Output, &PID_Setpoint, consKp, consKi, consKd, REVERSE);

void PID_setup(){
    myPID.SetMode(AUTOMATIC);
}

void PID_control(){
    Input = (double)ads.readADC_SingleEnded(NTCC_channel);
    myPID.SetTunings(consKp, consKi, consKd);
    myPID.Compute();
    ledcWrite(colChannel_1,Output); //220
}


