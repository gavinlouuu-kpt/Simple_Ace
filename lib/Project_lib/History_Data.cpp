#include "History_Data.h"
#include "SPIFFS.h"
extern String profileNumber;
double recorded_gas_sample[10] = {0};
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
        file.print(ratio_acetone);file.write('\n');
        file.close();   

        file = SPIFFS.open(file_dir,FILE_READ);
        while(file.available()){
        Serial.write(file.read());
        }
        file.close();
    }
}

void retrieve_record(){
    file_dir = "/History_data_";
    file_dir.concat(profileNumber);
    Serial.print("Retrieved directory");Serial.println(file_dir);
    File file = SPIFFS.open(file_dir,FILE_READ);
    String buffer_ratio;
    for(int i = 0; i<10;i++){
        buffer_ratio = file.readStringUntil('\n');
        recorded_gas_sample[i] = buffer_ratio.toDouble();
        Serial.print("Entry_"); Serial.print(i);Serial.print(": ");Serial.println(recorded_gas_sample[i]);
    }
    return; //the array
}

