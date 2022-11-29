#include "PID.h"
#include "Simple_ACE.h"
#include <PID_v1.h>

//Define the aggressive and conservative Tuning Parameters
double aggKp=4, aggKi=0.2, aggKd=1;
double consKp=1, consKi=0.05, consKd=0.25;

double Setpoint = 1300;
double Input, Output;
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, REVERSE);

void PID_setup(){
    myPID.SetMode(AUTOMATIC);
}

void PID_control(){
    Input = analogRead(NTCC);

      double gap = abs(Setpoint-Input); //distance away from setpoint
  if (gap < 100)
  {  //we're close to setpoint, use conservative tuning parameters
    myPID.SetTunings(consKp, consKi, consKd);
  }
  else
  {
     //we're far from setpoint, use aggressive tuning parameters
     myPID.SetTunings(aggKp, aggKi, aggKd);
  }

    myPID.Compute();
    ledcWrite(colChannel,Output); //220
    // Serial.println(Output);
    // printf("%.2f\n",Input);
}


