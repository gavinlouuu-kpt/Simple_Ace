#include "PID.h"
#include "Simple_ACE.h"
#include <PID_v1.h>

double Kp=2, Ki=5, Kd=1;
double Setpoint = 1550;
double Input, Output;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);

void PID_setup(){
    myPID.SetMode(AUTOMATIC);
}

void PID_control(){
    Input = analogRead(NTCC);
    myPID.Compute();
    ledcWrite(colChannel,Output); //220
    // Serial.println(Output);
    // printf("%.2f\n",Input);
}


