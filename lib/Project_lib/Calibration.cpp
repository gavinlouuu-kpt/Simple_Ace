#include "Calibration.h"
#include "Simple_ACE.h"
#include "Screen.h"

Adafruit_ADS1115 ads;
TFT_eSPI tft = TFT_eSPI();

short array[store_size] = {0};
void calibration_setup(int temperature){
    //convert temperature to pwm signal cycle//
    ledcWrite(colPin,temperature);
}

int acetone_i,acetone_o;
void array_locate(double temperature){
    //convert temperature to the array entry 
    const double std_temp = 55;
    const int std_acetone_i = 500;
    const int std_elution_size = 500;
    double factor = std_temp/temperature;

    int elution_size = std_elution_size * factor;
    acetone_i = std_acetone_i * factor;
    acetone_o = acetone_i + elution_size;
}

double ratio[2];
double update_parameters(){
    double ref_1[2]={ref_1_conc,ratio[0]};
    double ref_2[2]= {ref_2_conc,ratio[1]};
    double slope = (ref_2[1] - ref_1[1])/(ref_2[0]-ref_1[0]);
    double constant = ref_1[1]/(slope*ref_1[0]);
    draw_result();
}

void draw_process(int trial){
    tft.fillRect(80,140,140,180,TFT_NEIGHBOUR_GREEN);
    String text = "Calibration: Trial" + (String)trial;
    text = text.c_str();
    tft.setTextDatum(CC_DATUM);
    tft.drawString(text,120,160,2);
}

void draw_result(){
    tft.setTextDatum(TL_DATUM);
    tft.drawFloat((float)ratio[0],200,160,1);
    tft.drawFloat((float)ratio[1],200,170,1);
    tft.drawFloat((float)slope,200,180,1);
    tft.drawFloat((float)constant,200,190,1);
}

int peak_profile[10];
void calibration(){
    long previous_time;
    int counter = 0;
    double baseline =  baselineRead(CO2_channel);
    for (int sample=0; sample<2; sample ++){
        draw_process(sample);
        while(millis() - previous_time< sampletime -1){
            array[counter] = ads.readADC_SingleEnded(CO2_channel);
            counter +=1;
        }
        double peak =0; 
        for (int i = acetone_i; i <acetone_o-4; i ++){
            // if(array[i]> peak){ //find peak at the peak position
            //     peak = array[i];
            // }
            int previous;
            int diff = array[i] - array[i+4];
            if(abs(previous)>5 && abs(diff)>5 && previous >0 && diff< 0){

            }


        }
        ratio[sample] = ads_convert(peak,true)/ads_convert(baseline,true);
    }
    update_parameters();
}

void calibration_input(){
    //create a input value
    calibration_setup(value);
    array_locate(value);
    calibration();
}
