#include "PID.h"
#include "Simple_ACE.h"
#include <PID_v1.h>
#include <SimpleKalmanFilter.h>

SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);

//Define the aggressive and conservative Tuning Parameters
// double aggKp=4, aggKi=0.2, aggKd=1;
// double consKp=6, consKi=0.125, consKd=0.25;
// double consKp=4, consKi=0.3, consKd=0.6;
double consKp=4, consKi=0.08, consKd=0.015;
// double consKp=40, consKi=5, consKd=5;


double Setpoint = 800;
double Input, Output;
int pid_counter =0;
int buffer_input= 0;
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, REVERSE);

void PID_setup(){
    myPID.SetMode(AUTOMATIC);
}

void PID_control(){
    // Input = simpleKalmanFilter.updateEstimate(analogRead(NTCC));
    // if(pid_counter%10 !=0){
    //    buffer_input += analogRead(NTCC);
    //    pid_counter ++;
    //    Serial.println(buffer_input);
    // }
    // else{
    //     pid_counter = 0;
    //     buffer_input /= 10;
    //     // Input = analogRead(NTCC);
    //     Serial.println(buffer_input);
    // }
    // Input = (double)buffer_input;
    Input = analogRead(NTCC);
    //   double gap = abs(Setpoint-Input); //distance away from setpoint
//   if (gap < 100)
//   {  //we're close to setpoint, use conservative tuning parameters
    myPID.SetTunings(consKp, consKi, consKd);
//   }
//   else
//   {
//      //we're far from setpoint, use aggressive tuning parameters
//      myPID.SetTunings(aggKp, aggKi, aggKd);
//   }

    myPID.Compute();
    long map_Output = (long)Output;
    map_Output = map(map_Output,0,255,0,1024);
    ledcWrite(colChannel,map_Output); //220
    
    // Serial.print(map_Output);Serial.print(",");Serial.println(analogRead(NTCC));
    // buffer_input = 0; // Serial.print("Column temp:"); 
}


