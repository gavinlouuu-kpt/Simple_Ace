// #include "Cloud_storage.h"
// #include"time.h"
// #include "Wifi_connection.h"
// #include <Firebase_ESP_Client.h>
// #include <addons/TokenHelper.h>
// #include <addons/RTDBHelper.h>


// #if defined(ESP32)
// #elif defined(ESP8266)
// #include <ESP8266WiFi.h>
// #endif

// #define API_KEY "AIzaSyAqzqWNYLsZlMh-qk5VhkAoi87Q13r1oHY"
// /* 3. Define the RTDB URL */
// #define DATABASE_URL "https://beagle-66fe3-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
// /* 4. Define the user Email and password that alreadey registerd or added in your project */
// #define USER_EMAIL "francischan@kingsphase.page"
// #define USER_PASSWORD "francischan0009"

// FirebaseData fbdo;
// FirebaseAuth auth;
// FirebaseConfig config;
// unsigned long sendDataPrevMillis = 0;

// unsigned long count = 0;

// FirebaseJsonArray array;
// FirebaseJson jj;

// const char* ssid = WIFI_SSID;
// const char* password = WIFI_PASSWORD;
// // NTP server to request epoch time
// const char* ntpServer = "pool.ntp.org";
// // Variable to save current epoch time
// unsigned long epochTime; 

// String name = "Francis";
// String sex = "M";
// int h =178;
// int w = 75;
// unsigned long getTime() {
//   time_t now;
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo)) {
//     //Serial.println("Failed to obtain time");
//     return(0);
//   }
//   time(&now);
//   return now;
// }

// void firebasesetup(){
//     Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

//   /* Assign the api key (required) */
//   config.api_key = API_KEY;

//   /* Assign the user sign in credentials */
//   auth.user.email = USER_EMAIL;
//   auth.user.password = USER_PASSWORD;

//   /* Assign the RTDB URL (required) */
//   config.database_url = DATABASE_URL;

//   /* Assign the callback function for the long running token generation task */
//   config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
//   #if defined(ESP8266)
//   // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
//   fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
//   #endif
//     fbdo.setResponseSize(25000);

//   Firebase.begin(&config, &auth);
//   Firebase.reconnectWiFi(true);
//   Firebase.setDoubleDigits(5);
//   config.timeout.serverResponse = 10 * 1000;
//   config.timeout.socketConnection = 10 * 1000;
//   config.timeout.wifiReconnect = 10 * 1000;
//   configTime(0, 0, ntpServer);
// }

// void storeinfo(String namee, String sx, int height, int weight){
//   jj.set("/Name", namee);
//   jj.set("/Sex", sx);
//   jj.set("/Height", String(height));
//   jj.set("/Weight", String(weight));
//   String strr = "/Simple_Ace/Sample/"+ namee+"/info";
//   const char *filename = strr.c_str();
//   Firebase.RTDB.setJSON(&fbdo, F((filename)), &jj);
//   delay(10);
// }

// extern bool isWifi;
// void cloud_upload(){
//   checkstatus();
//   if(isWifi){
//     if(Firebase.ready() && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)){
//       sendDataPrevMillis = millis();
//       unsigned long time= getTime(); 
//       storeinfo(name,sex,h,w);
//       //Check first file
//       if(SPIFFS.exists("/Dataset_1")){
//         Serial.println("Stored from previos stored value");
//         File file = SPIFFS.open("/Dataset_1");
//         String data = "0";

//         while(file.available()){
//           for (int j = 0; j < 4; j++){
//             for (int i = 0; i < 1024; i++){ 
//               if(file.read() != 0){
//                 data = file.readStringUntil(',');
//                 array.add(data);
//               }
//             }
//             storedata(name,time,j);
//           }
//         }
//       file.close();
//       SPIFFS.remove("/Dataset_1");
//       delay(1000); //deleted Spiffs file
//       }
//       // Check Second file
//       else if (SPIFFS.exists("/Dataset_2")){
//         Serial.println("Stored from previos stored value");
//         File file = SPIFFS.open("/Dataset_2");
//         String data = "0";
//         while(file.available()){
//           for (int j = 0; j < 4; j++){
//             for (int i = 0; i < 1024; i++){ 
//               if(file.read() != 0){
//                 data = file.readStringUntil(',');
//                 array.add(data);
//               }
//             }
//             storedata(name,time,j);
//             // String str = "/Simple_Ace/Sample/" +name+  (String)time +"/File" + (String)j ;
//             // const char *filename = str.c_str();
//             // Firebase.RTDB.setArray(&fbdo, F((filename)), &array);
//             // delay(10);
//             // array.toString(Serial, true);
//             // array.clear();
//             // delay(1);
//           }
//         }
//         file.close();
//         SPIFFS.remove("/Dataset_2"); //deleted Spiffs file
//       }
//       //Sample realtime
//       else{
//         Serial.println("Storing Directly");
//         float value = 0.00;
//         for (int j = 0; j < 4; j++){
//           for (int i = 0; i < 1024; i++){ 
//             if(arr[j*1000+i] != 0){
//               value = arr[j*1000+i];
//               array.add(value);
//             } 
//           }
//           storedata(name,time,j);
//           // String str = "/Simple_Ace/Sample"+name + (String)time +"/File" + (String)j ;
//           // const char *filename = str.c_str();
//           // Firebase.RTDB.setArray(&fbdo, F((filename)), &array);
//           // delay(10);
//           // array.toString(Serial, true);
//           // array.clear();
//           // delay(1);
//         }
//       }
//     }
//     else{
//       Serial.println("get array failed");
//       Serial.println(fbdo.errorReason());
//     }
//   }
//   else{
//     String filename = "/Dataset_" + String(counter%2 + 1); //either 1 or 2
//     counter ++;
//     if(SPIFFS.exists(filename.c_str())){
//       SPIFFS.remove(filename.c_str());
//       printf("removed file: %s\n",filename);
//     }
//     printf("Storing into %s\n",filename);

//     File file = SPIFFS.open(filename.c_str(),FILE_WRITE);
//     file.print(',');file.write('\n'); 
//     for(int i =0; i <arr_size; i++){
//       if(arr[i] !=0){
//         file.print(arr[i]);file.print(',');file.write('\n'); 
//       }
//     }
//     file.close();
//     //Read
//     // file = SPIFFS.open(filename.c_str(),FILE_READ);
//     // while(file.available()){
//     //   Serial.write(file.read());
//     // }
//     // file.close();
//   }  
//   // while (1);replaced with ROM
// }

// }
