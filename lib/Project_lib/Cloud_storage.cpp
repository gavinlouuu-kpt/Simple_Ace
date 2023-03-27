#include "Cloud_storage.h"
#include "time.h"
#include "Wifi_connection.h"
#include "SPIFFS.h"
#include <Firebase_ESP_Client.h>
#include <EEPROM.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Simple_ACE.h>
#include <TimeLib.h>
#include "Adafruit_ADS1X15.h"

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
#define filenumber 8
#define filesize  256
#define GMT_offset 8

extern Adafruit_ADS1115 ads;
unsigned long getTime();                      //get unix time on wifi
String UnixConvert(unsigned long t);          //convert unix time into readable time
void check_sensor_life();                     // check if current time exceed sensor life
void firebase_setup();                        //initialize firebase 
void store_personalinfo(String namee, String sx, int height, int weight);     //store profile information
void store_default(unsigned long tim);        //store device default setting
void store_data();                            //store gas data, either SPIFFS or Cloud
void upload_data(String namee,unsigned long tim ,int number);                 // upload array data to firebase
void update_sensor();                         //restart sensor life count
void update_check_time();
byte index_address =  8;                   //print unix time of sensor change

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJsonArray data_array;
FirebaseJson personal_information;
FirebaseJson default_array;

extern bool isConnect;
extern bool isWifi;
extern String profileNumber;

unsigned long millisPreviousTime = 0;
unsigned long millisSensorLife = 605000;
const long gmtOffset_sec = 28800;
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* ntpServer = "pool.ntp.org";

int file_index = 0;
int h =178;
int w = 75;
String macadddress = WiFi.macAddress();
String name = "Francis";
String sex = "M";
String sensor_dir = "/Sensor_update";
unsigned long millisPreviousUpdate = 0;

void setLocalTime(){
  setenv("TZ","WET0WEST,M3.5.0/1,M10.5.0",1);
  tzset();
}
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  delay(500);
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}


// void UnixToTime(int UnixNum)                                                           self-calculated unix converter
// {
//   int years = UnixNum / 31556926 + 1970;
//   int months = UnixNum / 2629743 - (years-1970)*12;
//   int days = UnixNum / 86400 -(53*365+12+(months-1)*30);
//   int hours = UnixNum / 3600 % 24;
//   int minutes = UnixNum / 60 % 60;
//   int seconds = UnixNum % 60;
//   Serial.println(hours);
// }

void firebase_setup()
{
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

/* Assign the callback function for the long running token generation task */
// config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
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
}

String UnixConvert(unsigned long t){
  int time_hour = 0;
  // time_t t = 1675827391; //unix timestamp
  setTime(t);
  String sampletime ="";
  sampletime.concat(day());
  sampletime.concat("-");
  sampletime.concat(month());
  sampletime.concat("-");
  sampletime.concat(year());
  sampletime.concat(" ");
  if(hour() + GMT_offset >= 24){
     time_hour = hour() +8 -24;
  }
  else{
    time_hour = hour() + GMT_offset;
  }
  sampletime.concat(time_hour);
  sampletime.concat(":");
  sampletime.concat(minute());
  sampletime.concat(":");
  sampletime.concat(second());
  Serial.print("Date: ");Serial.print(day());Serial.print("/");Serial.print(month());Serial.print("/");Serial.print(year());Serial.print(" ");Serial.print(hour());Serial.print(":");Serial.print(minute());Serial.print(":");Serial.println(second());
  return sampletime;
}

void store_personalinfo(String namee, String sx, int height, int weight){
  personal_information.set("/Name", namee);
  personal_information.set("/Sex", sx);
  personal_information.set("/Height", String(height));
  personal_information.set("/Weight", String(weight));
  String info_dir = "/Simple_Ace/";
  info_dir.concat(macadddress);
  info_dir.concat("/");
  info_dir.concat(profileNumber);
  info_dir.concat("/info");
  Serial.print("Directory:");
  Serial.println(info_dir);
  const char *filename = info_dir.c_str();
  Firebase.RTDB.setJSON(&fbdo, F((filename)), &personal_information);
}

void upload_data(String namee,unsigned long tim ,int number){
  String data_dir = "/Simple_Ace/";
  data_dir.concat(macadddress);
  data_dir.concat("/");
  data_dir.concat(profileNumber);
  data_dir.concat("/");
  data_dir.concat((String)UnixConvert(tim));
  data_dir.concat("/File");
  data_dir.concat((String)number);
  Serial.print("Directory:");
  Serial.println(data_dir);
  const char *filename = data_dir.c_str();
  Firebase.RTDB.setArray(&fbdo, F((filename)), &data_array);delay(800);
  // data_array.toString(Serial, true);
  data_array.clear();
  delay(10);
}

void store_default(unsigned long tim){
  extern int dutyCycle_pump;
  extern const int sampletime;
  extern double PID_Setpoint;
  default_array.set("/Pump power", dutyCycle_pump);
  default_array.set("/Column setpoint", PID_Setpoint);
  default_array.set("/Sample Time", sampletime);
  // default_array.set("/Sensor life", );

  String default_dir = "/Simple_Ace/";
  default_dir.concat(macadddress);
  default_dir.concat("/");
  default_dir.concat(profileNumber);
  default_dir.concat("/");
  default_dir.concat((String)UnixConvert(tim));
  default_dir.concat("/Default");

  const char *setting = default_dir.c_str();
  Firebase.RTDB.setJSON(&fbdo, F((setting)), &default_array);
}

void store_data(){
  unsigned long millisUnixTime =0;  
  extern short Sensor_arr[store_size];
  if(isWifi == true){
    checkWifiStatus();
  }
  if(isConnect){
    if((millis() - millisPreviousTime) > 100 || millisPreviousTime == 0){
      store_personalinfo(name,sex,h,w);        
      millisPreviousTime = millis();
      //Check first file
      configTime(gmtOffset_sec, 0, ntpServer);
      for(int i = 0; i <20 ;i++){
        String upload_file_dir = "/Dataset_";
        upload_file_dir.concat((String)(i % 20 + 1));
        if (Firebase.ready() && SPIFFS.exists(upload_file_dir.c_str()))
        {
          File file = SPIFFS.open(upload_file_dir.c_str());
          String data = "0";
          millisUnixTime= getTime();       
          printf("%d\n",millisUnixTime);
            for (int j = 0; j < 8; j++){
              for (int i = 0; i <256; i++){ 
                if(file.read() != 0){
                  data = file.readStringUntil(',');
                  data_array.add(data);
                }
              }        
              upload_data(name,millisUnixTime,j);
              delay(10);
            }
          Serial.print("Stored from previous ");
          Serial.println(upload_file_dir.c_str());
          file.close();
          SPIFFS.remove(upload_file_dir.c_str()); // deleted Spiffs file
          delay(1000);
        }
      } 
      //Sample realtime
      if(Firebase.ready()){
        millisUnixTime= getTime(); 
        printf("%d\n",millisUnixTime);
        store_default(millisUnixTime);
        int value = 0.00;
        for (int j = 0; j < 8; j++){
          for (int i = 0; i < 256; i++){ 
            if(Sensor_arr[j*256+i] != 0){
              value = Sensor_arr[j*256+i];
              data_array.add(value);
            } 
          }
          upload_data(name,millisUnixTime,j);
          delay(10);
        }
        Serial.println("Storing Directly");
      }
    }
    else{
      Serial.println("get data_array failed");
      Serial.println(fbdo.errorReason());
    }
  }
  else
  {// Wifi_disable();
    Serial.println(SPIFFS.usedBytes());
    String file_dir = "/Dataset_";
    ////Subsequent setting
    EEPROM.begin(20);
    EEPROM.get(index_address, file_index);
    delay(500); 
    file_dir.concat((String)(file_index%20 +1));
    file_index++;
    EEPROM.put(index_address, file_index);
    delay(100); 
    Serial.print("file dir: ");Serial.println(file_dir);
    EEPROM.commit();
    delay(500);
    EEPROM.end();
    delay(500);

    if(SPIFFS.exists(file_dir.c_str())){
      SPIFFS.remove(file_dir.c_str());
      printf("removed file: %s\n", file_dir.c_str());
    }
    printf("Storing into %s\n", file_dir.c_str());
    unsigned long save_time = millis();
    File file = SPIFFS.open(file_dir.c_str(), FILE_WRITE);
    file.print(',');
    file.write('\n');
    for (int i = 0; i < store_size; i++)
    {
      if (Sensor_arr[i] != 0)
      {

        file.print(Sensor_arr[i]);
        file.print(',');
        file.write('\n');
      }
    }
    // Serial.print("Saved time in millis: ");Serial.println(millis()-save_time);
    // Serial.print("File size: ");Serial.println(file.size());
    file.close();

    Serial.println(SPIFFS.usedBytes());

    // Read
    // file = SPIFFS.open(file_dir.c_str(),FILE_READ);
    // while(file.available()){
    //   Serial.write(file.read());
    // }
    // file.close();
  }
  Wifi_disable();
}

void update_sensor(){
  Wifi_able();
  configTime(gmtOffset_sec, 0, ntpServer);
  unsigned long millisUpdateSensorTime = getTime();
  Serial.println(millisUpdateSensorTime);
  File file = SPIFFS.open(sensor_dir.c_str(),FILE_WRITE);
  file.print(millisUpdateSensorTime);file.write('\n'); 
  file.close();
  delay(500);
  Wifi_disable();
}

void update_check_time(){
  File file = SPIFFS.open(sensor_dir,FILE_READ);
  millisPreviousUpdate = strtoul(file.readStringUntil('\n').c_str(),NULL,10);
  file.close();
  Serial.print("Updated previous time:");Serial.println(millisPreviousUpdate);
}

void check_sensor_life(){
  Serial.print("Previous time:"); Serial.println(millisPreviousUpdate);
  if(isConnect == true && millisPreviousUpdate !=0){
    // Wifi_able();
    // configTime(0, 0, ntpServer);
    Serial.println();
    Serial.println("Checking sensor life...");
    Serial.println(getTime());
    if(getTime() - millisPreviousUpdate > millisSensorLife){ // sensor
      Serial.println("Change sensor!");
      extern bool isSensor;
      isSensor = false;
    }
  }
}