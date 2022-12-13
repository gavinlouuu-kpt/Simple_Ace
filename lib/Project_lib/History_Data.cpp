#include "History_Data.h"
#include "SPIFFS.h"
extern String profileNumber;
float previous_data[10] = {0};
String file_dir = "0";

void store_result(float ratio_co2,float ratio_acetone){
    file_dir = "/History_data_";
    file_dir.concat(profileNumber);
    File file = SPIFFS.open(file_dir,FILE_APPEND);
    Serial.print("Saved directory");Serial.println(file_dir);
    // file_print(ratio_co2);file.print(",");  //store CO2
    file.print(ratio_acetone);file.write('\n');
    file.close();   

    file = SPIFFS.open(file_dir,FILE_READ);
    while(file.available()){
      Serial.write(file.read());
    }
    file.close();
}

void retrieve_result(){
    file_dir = "/History_data_";
    file_dir.concat(profileNumber);
    Serial.print("Retrieved directory");Serial.println(file_dir);
    File file = SPIFFS.open(file_dir);
    String buffer;
    for(int i = 0; i<10;i++){
        buffer = file.readStringUntil('\n');
        previous_data[i] = buffer.toDouble();
        Serial.print("Entry_"); Serial.print(i);Serial.print(": ");Serial.println(previous_data[i]);
    }
    return; //the array
}

