#include "Cloud_storage.h"
#include "time.h"
#include "Wifi_connection.h"
#include <Firebase_ESP_Client.h>
#include "SPIFFS.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>


#if defined(ESP32)
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#define API_KEY "AIzaSyAqzqWNYLsZlMh-qk5VhkAoi87Q13r1oHY"
/* 3. Define the RTDB URL */
#define DATABASE_URL "https://beagle-66fe3-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "chichungchan91@gmail.com"
#define USER_PASSWORD "121688"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

FirebaseJsonArray array;
FirebaseJson jj;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime; 

extern bool isWifi;
extern short Sensor_arr[4096];
int counter = 0;

String name = "Francis";
String sex = "M";
int h =178;
int w = 75;

String dir1 ="/Simple_Ace/Sample";
String dir2 ="/Dataset_";

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void firebase_setup(){
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  #if defined(ESP8266)
  // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
  #endif
    fbdo.setResponseSize(25000);

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
  config.timeout.socketConnection = 10 * 1000;
  config.timeout.wifiReconnect = 10 * 1000;
  configTime(0, 0, ntpServer);
}

void storeinfo(String namee, String sx, int height, int weight){
  jj.set("/Name", namee);
  jj.set("/Sex", sx);
  jj.set("/Height", String(height));
  jj.set("/Weight", String(weight));
  String info_dir = "/Simple_Ace/Sample/";
  info_dir.concat(name);
  info_dir.concat("/info");
  // String strr = "/Simple_Ace/Sample/" + name +"/info" ;
  Serial.print("Directory:");Serial.println(info_dir);
  const char *filename = info_dir.c_str();
  Firebase.RTDB.setJSON(&fbdo, F((filename)), &jj);
  delay(1000);
}


void storedata(String namee,unsigned long tim ,int number){
  String data_dir = "/Simple_Ace/Sample/";
  data_dir.concat(name);
  data_dir.concat("/");
  data_dir.concat((String)tim);
  data_dir.concat("/File");
  data_dir.concat((String)number);
  // String str = "/Simple_Ace/Sample/" + name+ "/" + (String)tim +"/File" + (String)number ;
  Serial.print("Directory:");Serial.println(data_dir);
  const char *filename = data_dir.c_str();
  delay(1);
  Firebase.RTDB.setArray(&fbdo, F((filename)), &array);
  delay(100);
  // array.toString(Serial, true);
  array.clear();
  delay(100);
}


void cloud_upload(){
  checkstatus();
  if(isWifi){
    if((millis() - sendDataPrevMillis) > 100 || sendDataPrevMillis == 0){
      sendDataPrevMillis = millis();
      unsigned long time= getTime(); 
      printf("%d\n",time);
      storeinfo(name,sex,h,w);
      //Check first file
      if(Firebase.ready() && SPIFFS.exists("/Dataset_1")){
        File file = SPIFFS.open("/Dataset_1");
        String data = "0";
        while(file.available()){
          for (int j = 0; j < 4; j++){
            for (int i = 0; i < 1024; i++){ 
              if(file.read() != 0){
                data = file.readStringUntil(',');
                array.add(data);
              }
            }
            storedata(name,time,j);
            delay(10);
          }        
        }
        Serial.println("Stored from previous /Dataset_1");
        file.close();
        SPIFFS.remove("/Dataset_1");
        delay(1000); //deleted Spiffs file
      }
      // Check Second file
      // else if (SPIFFS.exists("/Dataset_2")){
      //   Serial.println("Stored from previos stored value");
      //   File file = SPIFFS.open("/Dataset_2");
      //   String data = "0";
      //   while(file.available()){
      //     for (int j = 0; j < 4; j++){
      //       for (int i = 0; i < 1024; i++){ 
      //         if(file.read() != 0){
      //           data = file.readStringUntil(',');
      //           array.add(data);
      //         }
      //       }
      //       storedata(name,time,j);
      //       // String str = "/Simple_Ace/Sample/" +name+  (String)time +"/File" + (String)j ;
      //       // const char *filename = str.c_str();
      //       // Firebase.RTDB.setArray(&fbdo, F((filename)), &array);
      //       // delay(10);
      //       // array.toString(Serial, true);
      //       // array.clear();
      //       // delay(1);
      //     }
      //   }
      //   file.close();
      //   SPIFFS.remove("/Dataset_2"); //deleted Spiffs file
      // }
      //Sample realtime
      else if(Firebase.ready()){
        float value = 0.00;
        for (int j = 0; j < 4; j++){
          for (int i = 0; i < 1024; i++){ 
            if(Sensor_arr[j*1024+i] != 0){
              value = Sensor_arr[j*1024+i];
              array.add(value);
            } 
          }
          storedata(name,time,j);
          delay(10);
        }
        Serial.println("Storing Directly");
      }
    }
    else{
      Serial.println("get array failed");
      Serial.println(fbdo.errorReason());
    }
  }
  else{
    String file_dir = "/Dataset_";
    file_dir.concat((String)(counter%2 + 1));
    counter ++;
    if(SPIFFS.exists(file_dir.c_str())){
      SPIFFS.remove(file_dir.c_str());
      delay(500);
      printf("removed file: %s\n",file_dir.c_str());
    }
    printf("Storing into %s\n",file_dir.c_str());

    File file = SPIFFS.open(file_dir.c_str(),FILE_WRITE);
    file.print(',');file.write('\n'); 
    for(int i =0; i <4096; i++){
      if(Sensor_arr[i] !=0){
        file.print(Sensor_arr[i]);file.print(',');file.write('\n'); 
      }
    }
    file.close();
    // Read
    // file = SPIFFS.open(file_dir.c_str(),FILE_READ);
    // while(file.available()){
    //   Serial.write(file.read());
    // }
    // file.close();
  }  
}
