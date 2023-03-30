#include "History_Data.h"
#include "SPIFFS.h"
extern String profileNumber;
double recorded_gas_sample[10][2] = {{0}};
String file_dir = "0";

void retrieve_record();                                     //  retreieve data array 
void store_result(float ratio_co2,float ratio_acetone);     //  store maximum 10 gas sample results

void store_result(float ratio_co2,float ratio_acetone){
    if(profileNumber == NULL){
        Serial.println("Not stored");
    }else{
        file_dir = "/History_data_";
        file_dir.concat(profileNumber);
        File file = SPIFFS.open(file_dir,FILE_APPEND);
        Serial.print("Saved directory");Serial.println(file_dir);
        file.print(ratio_co2);file.write(',');
        file.print(ratio_acetone);file.write('\n');
        file.close();   

        // file = SPIFFS.open(file_dir,FILE_READ);
        // while(file.available()){
        // Serial.write(file.read());
        // }
        // file.close();
    }
}

void retrieve_record(){
    file_dir = "/History_data_";
    file_dir.concat(profileNumber);
    // SPIFFS.remove(file_dir);
    if(SPIFFS.exists(file_dir.c_str())){
        File file = SPIFFS.open(file_dir,FILE_READ);
        String buffer_ratio="";
        int length = file.size()/10;
        Serial.print(file.size());Serial.print(",");Serial.println(length);
        double recorded_sample_buffer[length][2] ={{0}};
        for(int i = 0 ; i < length; i++){
            if(file.available()> 0){
                Serial.println("File available");
                buffer_ratio = file.readStringUntil(',');
                // Serial.print("buffer_ratio");Serial.println(buffer_ratio);
                recorded_sample_buffer[i][0] = buffer_ratio.toDouble();
                buffer_ratio = file.readStringUntil('\n');
                // Serial.print("buffer_ratio");Serial.println(buffer_ratio);
                recorded_sample_buffer[i][1] = buffer_ratio.toDouble();
            }
        }
        for(int q =0; q< 10;q ++){
            if((length-1) - q <0){break;}
            else{
                for (int p = 0; p <2; p++)
                {
                    recorded_gas_sample[q][p] = recorded_sample_buffer[(length-1) - q][p];
                    Serial.print("recorded_gas_sample");Serial.println(recorded_gas_sample[q][p]);
                }
            }
        }
        file.close();
    }
    return; 
}

