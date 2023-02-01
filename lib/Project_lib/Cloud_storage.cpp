#include "Cloud_storage.h"
#include "time.h"
#include "Wifi_connection.h"
#include "SPIFFS.h"
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Simple_ACE.h>
#include<TimeLib.h>

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
#define filesize 256
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJsonArray array;
FirebaseJson jj;

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
unsigned long sensor_life = 605000;
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
// NTP server to request epoch time
const char *ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime;

extern bool isConnect;
int counter = 0;
extern bool isWifi;

// int filenumber = 8;
// int filesize =store_size/filenumber;

String macadddress = WiFi.macAddress();
String name = "Francis";
String sex = "M";
extern String profileNumber;

int h = 178;
int w = 75;

unsigned long getTime()
{
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

String UnixConvert (unsigned long t){
  //time_t t = 1675827391; //unix timestamp
  setTime(t);
  String sampletime = "";
  sampletime.concat(day());
  sampletime.concat("-");
  sampletime.concat(month());
  sampletime.concat("-");
  sampletime.concat(year());
  sampletime.concat(" ");
  sampletime.concat(hour());
  sampletime.concat(":");
  sampletime.concat(minute());
  sampletime.concat(":");
  sampletime.concat(second());
  Serial.print("Date: ");
  Serial.print(day());
  Serial.print("/");
  Serial.print(month());
  Serial.print("/");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.println(second());
  return sampletime;
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

void storeinfo(String namee, String sx, int height, int weight)
{
  jj.set("/Name", namee);
  jj.set("/Sex", sx);
  jj.set("/Height", String(height));
  jj.set("/Weight", String(weight));
  String info_dir = "/Simple_Ace/";
  info_dir.concat(macadddress);
  info_dir.concat("/");
  info_dir.concat(profileNumber);
  info_dir.concat("/info");
  Serial.print("Directory:");
  Serial.println(info_dir);
  const char *filename = info_dir.c_str();
  Firebase.RTDB.setJSON(&fbdo, F((filename)), &jj);
  // delay(1000);
}

void storedata(String namee, unsigned long tim, int number)
{
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
  Firebase.RTDB.setArray(&fbdo, F((filename)), &array);
  delay(800);
  // array.toString(Serial, true);
  array.clear();
  delay(10);
}

unsigned long unixtime = 0;
void cloud_upload()
{
  if (isWifi == true)
  {
    checkstatus();
  }

  extern short Sensor_arr[store_size];
  if (isConnect)
  {
    if ((millis() - sendDataPrevMillis) > 100 || sendDataPrevMillis == 0)
    {
      storeinfo(name, sex, h, w);
      sendDataPrevMillis = millis();
      // Check first file
      for (int i = 0; i < 20; i++)
      {
        String upload_file_dir = "/Dataset_";
        upload_file_dir.concat((String)(i % 20 + 1));
        if (Firebase.ready() && SPIFFS.exists(upload_file_dir.c_str()))
        {
          File file = SPIFFS.open(upload_file_dir.c_str());
          String data = "0";
          unixtime = getTime();
          printf("%d\n", unixtime);
          for (int j = 0; j < 8; j++)
          {
            for (int i = 0; i < 256; i++)
            {
              if (file.read() != 0)
              {
                data = file.readStringUntil(',');
                array.add(data);
              }
            }
            storedata(name, unixtime, j);
            delay(10);
          }
          Serial.print("Stored from previous ");
          Serial.println(upload_file_dir.c_str());
          file.close();
          SPIFFS.remove(upload_file_dir.c_str()); // deleted Spiffs file
          delay(1000);
        }

        // if(Firebase.ready() && SPIFFS.exists("/Dataset_1")){
        //   File file = SPIFFS.open("/Dataset_1");
        //   String data = "0";
        //   unixtime= getTime();
        //   printf("%d\n",unixtime);
        //     for (int j = 0; j < 8; j++){
        //       for (int i = 0; i <256; i++){
        //         if(file.read() != 0){
        //           data = file.readStringUntil(',');
        //           array.add(data);
        //         }
        //       }
        //       storedata(name,unixtime,j);
        //       delay(10);
        //     }
        //   Serial.println("Stored from previous /Dataset_1");
        //   file.close();
        //   SPIFFS.remove("/Dataset_1");//deleted Spiffs file
        //   delay(1000);
        // }
        // // Check Second file
        // if (Firebase.ready() && SPIFFS.exists("/Dataset_2")){
        //   File file = SPIFFS.open("/Dataset_2");
        //   String data = "0";
        //   unixtime= getTime();
        //   for (int j = 0; j < 8; j++){
        //     for (int i = 0; i <256; i++){
        //       if(file.read() != 0){
        //         data = file.readStringUntil(',');
        //         array.add(data);
        //       }
        //     }
        //     storedata(name,unixtime,j);
        //     delay(10);
        //   }
        //   Serial.println("Stored from previous /Dataset_2");
        //   file.close();
        //   SPIFFS.remove("/Dataset_2"); //deleted Spiffs file
        //   delay(1000);
        // }
      }
      // Sample realtime
      if (Firebase.ready())
      {
        unixtime = getTime();
        printf("%d\n", unixtime);
        int value = 0.00;
        for (int j = 0; j < 8; j++)
        {
          for (int i = 0; i < 256; i++)
          {
            if (Sensor_arr[j * 256 + i] != 0)
            {
              value = Sensor_arr[j * 256 + i];
              array.add(value);
            }
          }
          storedata(name, unixtime, j);
          delay(10);
        }
        Serial.println("Storing Directly");
      }
    }
    else
    {
      Serial.println("get array failed");
      Serial.println(fbdo.errorReason());
    }
  }
  else
  {
    // Wifi_disable();
    String file_dir = "/Dataset_";
    file_dir.concat((String)(counter % 20 + 1));
    counter++;
    if (SPIFFS.exists(file_dir.c_str()))
    {
      SPIFFS.remove(file_dir.c_str());
      printf("removed file: %s\n", file_dir.c_str());
    }
    printf("Storing into %s\n", file_dir.c_str());
    unsigned long save_time = millis();
    File file = SPIFFS.open(file_dir.c_str(), FILE_WRITE);
    file.print(',');
    file.write('\n');
    for (int i = 0; i < 2048; i++)
    {
      if (Sensor_arr[i] != 0)
      {
        file.print(Sensor_arr[i]);
        file.print(',');
        file.write('\n');
      }
    }
    Serial.print("Saved time in millis: ");
    Serial.println(millis() - save_time);
    Serial.print("File size: ");
    Serial.println(file.size());
    file.close();

    // Read
    // file = SPIFFS.open(file_dir.c_str(),FILE_READ);
    // while(file.available()){
    //   Serial.write(file.read());
    // }
    // file.close();
  }
  WiFi.disconnect();
}

String sensor_dir = "/Sensor_update";
void update_sensor()
{
  Wifi_able();
  configTime(0, 0, ntpServer);
  unsigned long update_time = getTime();
  Serial.println(update_time);
  File file = SPIFFS.open(sensor_dir.c_str(), FILE_WRITE);
  file.print(update_time);
  file.write('\n');
  file.close();
  delay(500);
  WiFi.disconnect();
}
unsigned long previous_sensor_time = 0;

void update_check_time()
{
  File file = SPIFFS.open(sensor_dir, FILE_READ);
  previous_sensor_time = strtoul(file.readStringUntil('\n').c_str(), NULL, 10);
  file.close();
  Serial.print("Updated previous time:");
  Serial.println(previous_sensor_time);
}

void check_sensor_life()
{
  Serial.print("Previous time:");
  Serial.println(previous_sensor_time);
  if (isConnect == true && previous_sensor_time != 0)
  {
    // Wifi_able();
    // configTime(0, 0, ntpServer);
    Serial.println();
    Serial.println("Checking sensor life...");
    Serial.println(getTime());
    if (getTime() - previous_sensor_time > sensor_life)
    { // sensor
      Serial.println("Change sensor!");
      extern bool isSensor;
      isSensor = false;
    }
  }
}