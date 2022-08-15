#include "Simple_COCO.h"
// #include <SPIFFS.h>
#include <BlynkSimpleEsp32.h>

DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);
TFT_eSPI tft= TFT_eSPI();
BlynkTimer timer;

const char* ntpServer = "pool.ntp.org";
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = SSID;
char password[] = PASSWORD;

double upload_buffer;
double upload_buffer_1;
double upload_buffer_2;
double upload_buffer_3;

BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}
void myTimerEvent()
{
 //not mroe than than 10 samples per seconds
 Blynk.virtualWrite(V1, upload_buffer);
 Blynk.virtualWrite(V2, upload_buffer_1);
 Blynk.virtualWrite(V0, upload_buffer_2);
 Blynk.virtualWrite(V4, upload_buffer_3);
}

void analogSetup(){
  ledcSetup(colChannel, freq, resolution);
  ledcSetup(pumpChannel, freq, resolution);
  ledcSetup(solChannel, freq, resolution);
  ledcAttachPin(colPin, colChannel);
  ledcAttachPin(pumpPin, pumpChannel);
  ledcAttachPin(solPin, solChannel);
  ledcWrite(colChannel, 220);
  ledcWrite(pumpChannel, dutyCycle);
}

void blynk_upload(double v1, double v2, double v3, double v4) {
  upload_buffer = v1;
  upload_buffer_1 = v2;
  upload_buffer_2 = v3;
  upload_buffer_3 = v4;
  delay(1000);
  Blynk.run();
  timer.run();
}


void checkSetup(){
  configTime(0, 0, ntpServer);
  unsigned long clk = getTime();
  while (1) {
    if (clk - getTime() < 10) {
      Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
      break;
    }
  }

  if (Blynk.connect() == false) {
    ESP.restart();        //custom function I wrote to check wifi connection
  }

  timer.setInterval(1000L, myTimerEvent);
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
if (!EEPROM.begin(2)) {
    Serial.println("An Error has occurred while mounting EPPROM");
    return;
  }
   //To rewrite each file from the first file
  EEPROM.write(EEP_add_1, 255); EEPROM.commit();

  if(EEPROM.read(EEP_add_1)== 255){
    EEPROM.write(EEP_add, 0);
    EEPROM.write(EEP_add_1,0);
    Serial.println("Rewrite from zero.");
  }
  EEPROM.commit();
  int foo = EEPROM.read(EEP_add);
  Serial.println(foo);
  Serial.println(EEPROM.read(EEP_add_1));
  sht20.initSHT20();
  sht20.checkSHT20();
  Serial.println("Setup Complete."); 
  
  if (!Wire.begin(4,14)) {
    Serial.println("Failed to initialize wire library");
    while (1);
  }
}

void restore_humidity(){
  while(1){
    ledcWrite(pumpChannel, 255);
    int previous = sht20.readHumidity();
    Serial.println(sht20.readHumidity());
    if (sht20.readHumidity() - previous  < 2) {
      ledcWrite(pumpChannel, dutyCycle);
      break;
    }    
  }
}

void power_saving(unsigned long last_time){
  while(1){
    if (digitalRead(btn_rst) == LOW) {
      Serial.println("New loop");
      ledcWrite(pumpChannel, dutyCycle);
      break;
    }
    if (getTime() - last_time > 10) {
      ledcWrite(pumpChannel, 0);
    }
  }
}

void pinSetup(){
  pinMode(pumpPin, OUTPUT);
  pinMode(solPin, OUTPUT);
  // pinMode(fanPin, OUTPUT);
  pinMode(btn_rst, INPUT);
}

void tftSetup(){
  tft.init();
  tft.setRotation(0); delay(100);
  tft.fillScreen(BLACK);
  // tft.setTextSize(3);  tft.setCursor(30, 80);  tft.println("CoCo");
  // tft.setTextSize(2);  tft.setCursor(5, 120);  tft.println("Setting Up");
  // comment the following two lines if using 2 inch screen
  tft.setTextSize(5);  tft.setCursor(60, 100);  tft.println("CoCo");
  tft.setTextSize(3);  tft.setCursor(30,160);  tft.println("Setting Up");
  delay(400);
}

double ads_convert(int value, bool resist) {
  double volt;
  const double ref_r = 9990;
  const double V_in = 3.3;
  double sen_r;
  volt = value * LSB;
  switch (resist) {
    case (false):
      Serial.println(volt);
      return volt;
      break;
    case (true):
      sen_r = ref_r * (V_in - volt) / volt;
      Serial.println(sen_r);
      return sen_r;
      break;
  }
}

double sort_reject(double arr[], int arr_size) {
  double buff;
  double buff_1 = 0;
  int len = arr_size;
  for ( int j = 0 ; j < arr_size ; j++) {
    for (int i = 0; i < arr_size - 1 - j; i ++) {
      if (arr[i] > arr[i + 1]) {
        buff = arr[i];
        arr[i] = arr[i + 1];
        arr[i + 1] = buff;
      }
    }
  }
  if (arr[0] < arr[1] * 0.8) {
    arr[0] = 0;
    len = len - 1;
    Serial.println("rejected");
  }
  if (arr[2] > arr[1] * 0.8) {
    arr[2] = 0;
    len = len - 1;
    Serial.println("rejected");
  }

  for (int i = 0; i < arr_size; i++) {
    Serial.println(arr[i], 6);
  }
  for (int i = 0; i < arr_size; i++) {
    buff_1 = buff_1 + arr[i];
  }
  double avg_rat = buff_1 / len;
  return avg_rat;
}

int readAds(byte asd, int buff) {
  byte setting[3];
  byte channel[4];
  byte buffer[3];
  int val = 0;
  int size  = 2;
  setting[0] = 1; setting[1] = 0; setting[2] = 0b11100101;
  channel[0] = 0b11000010; channel[1] = 0b11010010; channel[2] = 0b11100010; channel[3] = 0b11110010;
  Wire.beginTransmission(asd);
  setting[1] = channel[buff];
  Wire.write(setting[0]); Wire.write(setting[1]); Wire.write(setting[2]);
  Wire.endTransmission();
  delay(5);
  buffer[0] = 0; // pointer
  Wire.beginTransmission(asd);
  Wire.write(buffer[0]);  // pointer
  Wire.endTransmission();

  Wire.requestFrom(asd, size);
  buffer[1] = Wire.read(); buffer[2] = Wire.read();
  Wire.endTransmission();

  val = buffer[1] << 8 | buffer[2];
  return val;
}

void breath_check(){
  while (true) {
    double arr[5];
    float humd;
    double gradient;
    long previous;
    for (int i = 0; i < 5; i++) {
      arr[i] = sht20.readHumidity();
      previous = millis();
    }
    gradient  = (arr[4] - arr[0]) * 7 ;
    Serial.print("Grad :"); Serial.println(gradient);
    Serial.println(readAds(ASD1115,CO2_channel));
    delay(1);
    if (gradient > 0.4) {
      break;
    }
  }
}

double read_humidity(){
  double value;
  value = sht20.readHumidity();
  return value;
}

double ratio_calibration(double uncal_base, double uncal_reading, bool formula){
    double cal_ratio;
    double buffer;
    double slope;
    double constant;
    buffer = uncal_base / uncal_reading;
    switch (formula) {
        case (true):
          slope = 0.0224;
          constant = 1.008;
          cal_ratio = (buffer - constant) / slope;
          return cal_ratio;
          break;
        case (false):
          slope = -0.0809;
          constant = 2.64;
          cal_ratio = (buffer - constant) / slope;
          return cal_ratio;
          break;
  }
}

double ratio_CO2 [3];
double ratio_O2 [3];
bool store;
int restore_baseline(){
  while (1) {
      int temp = baselineRead(CO2_channel );
      delay(100);
      int ref = baselineRead(CO2_channel );
      if (temp + 3 >= ref && temp - 3 <= ref) {
        Serial.println("Found Baseline");
        delay(100);
        return temp;
        break;
      }
    }
}


void sample_collection(int i){
  int peak = 0;
  int bottom_O2 = 100000;
  int baseline_O2 = baselineRead(O2_channel );
  int baseline;
  double max_humd = 0;
  int q = 0;
  unsigned long previous ;
  short adc_CO2;
  short adc_O2;

  restore_humidity();
  baseline = restore_baseline();
  tft.fillScreen(BLACK);
  // tft.setTextSize(3); tft.setCursor(30, 90); tft.println("BLOW");
  // tft.setTextSize(3); tft.setCursor(35, 130); tft.print(i + 1); tft.print("/3");
  // comment the following two lines if using 2 inch screen
  tft.setTextSize(5); tft.setCursor(0, 245); tft.println("Exhale");
  tft.setTextSize(4); tft.setCursor(0, 285); tft.print("1"); tft.print("/3");
  delay(10);
  Serial.println("Blow"); Serial.print(i + 1); Serial.println(" /3");
  breath_check();

  
  store = false;
  previous = getTime();
  tft.fillScreen(BLACK);
  // tft.setTextSize(2); tft.setCursor(5, 110); tft.print("Process...");
  // comment the following line if using 2 inch screen
  tft.setTextSize(3); tft.setCursor(5, 160); tft.print("Processing..");
  delay(1);
  while (getTime() - previous < sampletime + 1) {
    adc_CO2 = readAds(ASD1115, CO2_channel );
    adc_O2 = readAds(ASD1115, O2_channel );
    if (store == false) {
      Serial.println(read_humidity());
      if (read_humidity() > 75 ) {
        store == true;
        Serial.println("Upload");
      }
    }
    CO2_arr[q] = adc_CO2;
    O2_arr[q] = adc_O2;
    if (adc_CO2 > peak) {
      peak = adc_CO2;
    }
    if (adc_O2 < bottom_O2) {
      bottom_O2 = adc_O2;
    }
    delay(1);
    Serial.println(q);
    q = q + 1;
  }

  Serial.print(peak); Serial.print("\t\t"); Serial.println(baseline);
  double bottom_resist_CO2 = ads_convert(peak, true);
  double bottom_volt_O2 = ads_convert(bottom_O2, false);
  double baseline_resist_CO2 = ads_convert(baseline, true);
  double baseline_volt_O2 = ads_convert(baseline_O2, false);
  ratio_CO2[i] =  ratio_calibration(baseline_resist_CO2, bottom_resist_CO2, true);
  ratio_O2[i] = ratio_calibration(baseline_volt_O2, bottom_volt_O2, false);
//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  Serial.print(i + 1); Serial.print(" "); Serial.print("TH "); Serial.println("Breath");
  Serial.print("Bottom_CO2: "); Serial.println(bottom_resist_CO2, 6); Serial.print("baseline: "); Serial.println(baseline_resist_CO2, 6); Serial.print("Ratio_CO2: "); Serial.println(ratio_CO2[i], 6);
  Serial.print("bottom_O2: "); Serial.println(bottom_volt_O2, 6); Serial.print("baseline_O2: "); Serial.println(baseline_volt_O2, 6); Serial.print("Ratio_O2: "); Serial.println(ratio_O2[i], 6);
  Serial.print("Max_humidity :"); Serial.println(max_humd);
  tft.fillScreen(BLACK);
  
  // tft.setTextSize(2); tft.setCursor(0, 60); tft.print(i + 1); tft.setTextSize(2); tft.print("Th "); tft.setTextSize(2); tft.println("Breath:");
  // tft.setCursor(0, 100); tft.print("CO2:"); tft.print(ratio_CO2[i], 3); tft.println("%");
  // tft.setCursor(11, 130); tft.print("O2:"); tft.print(ratio_O2[i], 1); tft.println("%");
  // comment the following two lines if using 2 inch screen
  tft.setTextSize(3); tft.setCursor(0, 230); tft.print("1"); tft.setTextSize(3); tft.print("TH "); tft.setTextSize(3); tft.println("Breath");
  tft.setCursor(0, 265); tft.print("CO");tft.setCursor(35, 272);tft.setTextSize(2);tft.print("2");tft.setTextSize(3);tft.setCursor(50, 265);tft.print(":"); tft.print(ratio_CO2[i], 3); tft.println("%");
  tft.setCursor(18, 293); tft.print("O"); tft.setCursor(35, 300);tft.setTextSize(2);tft.print("2");tft.setTextSize(3);tft.setCursor(50, 293);tft.print(":");tft.print(ratio_O2[i], 1); tft.println("%");
}

int baselineRead(int channel) {
  int toSort[baseSample];
  float mean = 0;
  for (int i = 0; i < baseSample; ++i ) {
    toSort[i] = readAds(ASD1115, channel);
    delay(10);
  }
  for (int i = 0; i < baseSample; ++i) {
    mean += toSort[i];
  }
  mean /= baseSample;
  return int(mean);
}

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}
