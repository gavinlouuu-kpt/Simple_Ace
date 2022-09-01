#include "Simple_ACE.h"
#include <Adafruit_ADS1X15.h>
#include <SHT2x.h>
// #include <SPIFFS.h>
// #include <BlynkSimpleEsp32.h>
// #include <PID_v1.h>

Adafruit_ADS1115 ads;
SHT20 sht;
// double Kp=2, Ki=5, Kd=1;
// double Setpoint = 1550;
// double Input, Output;
// PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);
// BlynkTimer timer;

const char* ntpServer = "pool.ntp.org";
// char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = SSID;
char password[] = PASSWORD;

double upload_buffer;
double upload_buffer_1;
double upload_buffer_2;
double upload_buffer_3; 

// BLYNK_CONNECTED()
// {
//   // Change Web Link Button message to "Congratulations!"
//   Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
//   Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
//   Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
// }
// void myTimerEvent()
// {
//  //not mroe than than 10 samples per seconds
//  Blynk.virtualWrite(V1, upload_buffer);
//  Blynk.virtualWrite(V2, upload_buffer_1);
//  Blynk.virtualWrite(V0, upload_buffer_2);
//  Blynk.virtualWrite(V4, upload_buffer_3);
// }

// void blynk_upload(double v1, double v2, double v3, double v4) {
//   upload_buffer = v1;
//   upload_buffer_1 = v2;
//   upload_buffer_2 = v3;
//   upload_buffer_3 = v4;
//   delay(1000);
//   Blynk.run();
//   timer.run();
// }

void pinSetup(){
  pinMode(pumpPin, OUTPUT);
  pinMode(solPin, OUTPUT);
  // pinMode(fanPin, OUTPUT);
  pinMode(btn_rst, INPUT);
}

void analogSetup(){
  ledcSetup(colChannel, freq, resolution);
  ledcSetup(pumpChannel, freq, resolution);
  ledcSetup(solChannel, freq, resolution);
  ledcAttachPin(colPin, colChannel);
  ledcAttachPin(pumpPin, pumpChannel);
  ledcAttachPin(solPin, solChannel);
  ledcWrite(colChannel, dutyCycle_col);
  ledcWrite(pumpChannel, 255);
  delay(100);
  ledcWrite(pumpChannel, dutyCycle_pump);
}

void checkSetup(){
  WiFi.begin(ssid,password);
  configTime(0, 0, ntpServer);
  unsigned long clk = getTime();
  // while (1) {
  //   if (clk - getTime() < 10) {
  //     Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  //     break;
  //   }
  // }

  // if (Blynk.connect() == false) {
  //   ESP.restart();        //custom function I wrote to check wifi connection
  // }

  // timer.setInterval(1000L, myTimerEvent);
  if (!Wire.begin(21,22)) {
  Serial.println("Failed to initialize wire library");
  while (1);
  }

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

  sht.begin(21,22);
  uint8_t stat = sht.getStatus();
  Serial.println(stat, HEX);

  if (!ads.begin()) {
  Serial.println("Failed to initialize ADS.");
  while (1);
  }
  
  // myPID.SetMode(AUTOMATIC);
  Serial.println("Setup Complete."); 
}


static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void lvgl_Setup(){
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println( LVGL_Arduino );
  Serial.println( "I am LVGL_Arduino" );

  lv_init();

  #if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
  #endif

  tft.begin();          /* TFT init */
  tft.setRotation(0); /* Landscape orientation, flipped */
  tft.fillScreen(TFT_BLACK);

  /*Set the touchscreen calibration data,
    the actual data for your display can be acquired using
    the Generic -> Touch_calibrate example from the TFT_eSPI library*/
  uint16_t calData[5] = { 485, 2909, 352, 3279, 0 };

  tft.setTouch( calData );

  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );//initialize draw buff

  //  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;       // set height of the screen
  disp_drv.ver_res = screenHeight;      // set height of the screen
  disp_drv.flush_cb = my_disp_flush;    // flush screen
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );      //driver initialize
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;   // read touch pad
  lv_indev_drv_register( &indev_drv );

}

static void draw_event_cb(lv_event_t * e)
{
  lv_obj_t * obj = lv_event_get_target(e);

  /*Add the faded area before the lines are drawn*/
  lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
  if (dsc->part == LV_PART_ITEMS) {
    if (!dsc->p1 || !dsc->p2) return;

    /*Add a line mask that keeps the area below the line*/
    lv_draw_mask_line_param_t line_mask_param;
    lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y,
                                  LV_DRAW_MASK_LINE_SIDE_BOTTOM);
    int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

    /*Add a fade effect: transparent bottom covering top*/
    lv_coord_t h = lv_obj_get_height(obj);
    lv_draw_mask_fade_param_t fade_mask_param;
    lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP,
                           obj->coords.y2);
    int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

    /*Draw a rectangle that will be affected by the mask*/
    lv_draw_rect_dsc_t draw_rect_dsc;
    lv_draw_rect_dsc_init(&draw_rect_dsc);
    draw_rect_dsc.bg_opa = LV_OPA_20;
    draw_rect_dsc.bg_color = dsc->line_dsc->color;

    lv_area_t a;
    a.x1 = dsc->p1->x;
    a.x2 = dsc->p2->x - 1;
    a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
    a.y2 = obj->coords.y2;
    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

    /*Remove the masks*/
    lv_draw_mask_free_param(&line_mask_param);
    lv_draw_mask_free_param(&fade_mask_param);
    lv_draw_mask_remove_id(line_mask_id);
    lv_draw_mask_remove_id(fade_mask_id);
  }
  /*Hook the division lines too*/
  else if (dsc->part == LV_PART_MAIN) {
    if (dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL) return;

    /*Vertical line*/
    if (dsc->p1->x == dsc->p2->x) {
      dsc->line_dsc->color  = lv_palette_lighten(LV_PALETTE_GREY, 1);
      if (dsc->id == 3) {
        dsc->line_dsc->width  = 2;
        dsc->line_dsc->dash_gap  = 0;
        dsc->line_dsc->dash_width  = 0;
      }
      //            else {
      //                dsc->line_dsc->width = 1;
      //                dsc->line_dsc->dash_gap  = 6;
      //                dsc->line_dsc->dash_width  = 6;
      //            }
    }
    /*Horizontal line*/
    else {
      if (dsc->id == 2) {
        dsc->line_dsc->width  = 2;
        dsc->line_dsc->dash_gap  = 0;
        dsc->line_dsc->dash_width  = 0;
      }
      //            else {
      //                dsc->line_dsc->width = 2;
      //                dsc->line_dsc->dash_gap  = 6;
      //                dsc->line_dsc->dash_width  = 6;
      //            }

      if (dsc->id == 1  || dsc->id == 3) {
        dsc->line_dsc->color  = lv_palette_main(LV_PALETTE_GREEN);
      }
      else {
        dsc->line_dsc->color  = lv_palette_lighten(LV_PALETTE_GREY, 2);
      }
    }
  }
}

int val;
static void add_data(lv_timer_t * timer)
{
  LV_UNUSED(timer);
  // val  = readAds(ASD1115, CO2_channel);
  val  = ads.readADC_SingleEnded(CO2_channel);
  delay(1);
  lv_chart_set_next_value(chart1, ser1, val);
  Serial.println(val);
}

void lv_example_chart_2(void)
{
  /*Create a chart1*/
  chart1 = lv_chart_create(lv_scr_act());
  lv_obj_set_size(chart1, 210, 150);
  lv_obj_align(chart1, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_bg_color(chart1, LV_COLOR_MAKE(255, 255, 255), LV_STATE_DEFAULT);
  lv_chart_set_type(chart1, LV_CHART_TYPE_LINE);   /*Show lines and points too*/

  lv_chart_set_div_line_count(chart1, 5, 7);

  lv_obj_add_event_cb(chart1, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
  lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);

  /*Add two data series*/
  ser1 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_PINK), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_point_count(chart1, 500);
  lv_obj_set_style_line_width(chart1, 1 , LV_PART_ITEMS);
  lv_chart_set_range(chart1,LV_CHART_AXIS_PRIMARY_Y,14500,16000);

  lv_obj_t * Graph_title = lv_label_create(lv_scr_act());
  lv_label_set_recolor(Graph_title, true);
  lv_obj_align(Graph_title, LV_ALIGN_TOP_MID, 0, 0);
  lv_label_set_text(Graph_title, "#FFFFFF Live Chart#");

  uint32_t i;
  lv_timer_create(add_data, 1, NULL);
}

void value_label(void) {
  static lv_obj_t * value = lv_label_create(lv_scr_act());
  lv_label_set_recolor(value, true);
  lv_obj_set_style_bg_color(value, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
  lv_label_set_text(value, "#FFFFFF Value: #");
  lv_obj_align(value, LV_ALIGN_LEFT_MID, 30, 40);
}

void prompt_label(void) {
  prompt = lv_label_create(lv_scr_act());
  lv_label_set_recolor(prompt, true);
  lv_obj_set_style_bg_color(prompt, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
  lv_label_set_text(prompt, "#FFFFFF Please Blow#" LV_SYMBOL_UP LV_SYMBOL_UP LV_SYMBOL_UP);
  lv_obj_align(prompt, LV_ALIGN_BOTTOM_MID, 0, -10);
  Serial.println("Propted");
}

void wait_label(void) {
  wait = lv_label_create(lv_scr_act());
  lv_label_set_recolor(wait, true);
  lv_obj_set_style_bg_color(wait, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
  lv_label_set_text(wait, "#FFFFFF Processing #");
  lv_obj_align(wait, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void hyphen_label(void) {
  hyphen = lv_label_create(lv_scr_act());
  lv_label_set_recolor(hyphen, true);
  lv_obj_set_style_bg_color(hyphen, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
  lv_obj_align(hyphen, LV_ALIGN_LEFT_MID, 150, 40);
  lv_label_set_text(hyphen,LV_SYMBOL_MINUS);
}
void restore_humidity(){
  while(1){
    // ledcWrite(pumpChannel, 255);
    int previous = sht.getHumidity();
    sht.read();
    Serial.println(sht.getHumidity());
    if (sht.getHumidity() - previous  < 2) {
      ledcWrite(pumpChannel, dutyCycle_pump);
      delay(5);
      break;
    }    
  }
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

void breath_check(){
  while (true) {
    double arr[5];
    float humd;
    double gradient;
    long previous;  
    for (int i = 0; i < 5; i++) {
      sht.read();
      arr[i] = sht.getHumidity();
      Serial.println(arr[i]);
      previous = millis();
      Serial.println(previous);
      delay(1);
    }
    gradient  = (arr[4] - arr[0]) * 7 ;
    printf("Grad: %.3f\n",gradient);
    lv_timer_handler();
    delay(5);
    if (gradient > 0.4) {
      break;
    }
  }
}

double read_humidity(){
  double value;
  sht.read();
  value = sht.getHumidity();
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
          slope = 1;
          constant = 0;
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

int baselineRead(int channel) {
  int toSort[baseSample];
  float mean = 0;
  for (int i = 0; i < baseSample; ++i ) {
    toSort[i] = ads.readADC_SingleEnded(channel);
    // toSort[i] = readAds(ASD1115, channel);
    delay(10);
  }
  for (int i = 0; i < baseSample; ++i) {
    mean += toSort[i];
  }
  mean /= baseSample;
  return int(mean);
}

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

static void delete_obj(lv_obj_t* object){
  lv_obj_del_async(object);
  object= NULL;
}

double concentration_ethanol( double temp, int baseline) {
  double acetone_start = 900;
  double acetone_end = 1200;
  double coec  = 55 / temp;
  int peak = 0;
  acetone_start = acetone_start * coec;
  acetone_end = acetone_end * coec;
  printf("%d , %d\n", (int)acetone_start, (int)acetone_end);
  for ( int i = (int)acetone_start - 1 ; i <= (int) acetone_end - 1; i++) {
    printf("%d\n", CO2_arr[i]);
    printf("%d\n", i);
    if ( CO2_arr[i] > peak) {
      peak = CO2_arr[i];
      printf("Replaced\n");
    }
  }
  printf("Peak value is %d.\n", peak);
  printf("Baseline value is %d.\n", baseline);
  // double ratio =  peak / baseline;
  // printf("%d\n", baseline);
  // printf(" Acetone Concentration: %.5f \n", ratio);
  return(peak);
}

void power_saving(unsigned long last_time){
  while(1){
    lv_timer_handler();
    delay(5);
    if (digitalRead(btn_rst) == HIGH) {
      Serial.println("New loop");
      ledcWrite(pumpChannel, dutyCycle_pump);
      break;
    }
    if (getTime() - last_time > 10) {
      ledcWrite(pumpChannel, 0);
    }
  }
}

double ratio_Ace;
bool store;
void sample_collection(){
  int peak = 0;
  int baseline;
  int q = 0;
  unsigned long previous ;
  short adc_CO2;

  restore_humidity();
  baseline = restore_baseline();
  delay(10);
  Serial.println("Blow Now");Serial.println();
  prompt_label();
  breath_check();

  store = false;
  previous = getTime();
  delay(1);
  delete_obj(prompt);
  wait_label();
  lv_timer_handler();
  delay(5);
  int count = 0 ;
  while (getTime() - previous < sampletime + 1) {
    printf("Recording\n");
    lv_timer_handler_run_in_period(1);
    adc_CO2 = ads.readADC_SingleEnded(CO2_channel);
    if (store == false) {
      count = count +1 ;
      Serial.println(read_humidity());
      if (read_humidity() > 70 ) {
        store = true;
        Serial.println("Certain a breathe. Recording...");
      }
      if (count== 100){
        printf("This is a failed breath");
        break;
      }
    }
    // Input = analogRead(NTCC);
    // myPID.Compute();
    // ledcWrite(colChannel,Output); //220
    CO2_arr[q] = adc_CO2;
    Serial.println(q);
    delay(1);
    q = q + 1;
  }
  if(count==100){
    delete_obj(wait);
    lv_timer_handler();
    delay(5);
    return;
  }
  delete_obj(wait);
  lv_timer_handler();
  delay(5);
  printf("Clear ok\n");
  peak = concentration_ethanol(temperate,baseline);delay(1);
  printf("Find peak ok\n");
  double peak_resist_Ace = ads_convert(peak, true);delay(1);
  printf("Find resist ok\n");
  double baseline_resist_Ace = ads_convert(baseline, true);delay(1);
  ratio_Ace =  ratio_calibration(baseline_resist_Ace, peak_resist_Ace, true);delay(1);

//   data_logging(peak, baseline, ratio_CO2[i], 0 , 3 );
//   data_logging(bottom_O2, baseline_O2, ratio_O2[i] , 0  , 4 );
  printf("Breath Analysis Result:\n");
  printf("Peak_Acetone: %.6f\nBaseline Resistance (Ohm): %.6f\n Ratio_Acetone: %.6f\n",peak_resist_Ace, baseline_resist_Ace,ratio_Ace);
  // Serial.print("Peak_Acetone: "); Serial.println(peak_resist_Ace, 6); Serial.print("Baseline Resistance (Ohm): "); Serial.println(baseline_resist_Ace, 6); Serial.print("Ratio_Acetone: "); Serial.println(ratio_Ace, 6);
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

// void command_label(void) {
//   static lv_obj_t * command = lv_label_create(lv_scr_act());
//   lv_label_set_recolor(command, true);
//   lv_obj_set_style_bg_color(command, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
//   lv_label_set_text(command, "#FFFFFF B #");
//   lv_obj_align(command, LV_ALIGN_LEFT_MID, 30, 40);
// }

// void feedback_label(void) {
//   feedback = lv_label_create(lv_scr_act());
//   lv_label_set_recolor(feedback, true);
//   lv_obj_set_style_bg_color(feedback, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
//   if (value < 103) {
//     lv_label_set_text(feedback, "#FFFFFF Too less!!#" );
//   } else if (value > 103 && value< 106) {
//     lv_label_set_text(feedback, "#FFFFFF Too much!!#" );
//   } else if(value >106){
//     lv_label_set_text(feedback, "#FFFFFF Brilliant!!#" );
//   }
//   lv_obj_align(feedback, LV_ALIGN_BOTTOM_MID, 0, -30);
// }
// static void add_value(lv_timer_t * timer)
// {
//   LV_UNUSED(timer);
//   lv_label_set_text_fmt(number, "#FFFFFF %f#",ratio_Ace[0]);
//   Serial.println(ratio_Ace[0]);
// }

// void number_label(void) {
//  number = lv_label_create(lv_scr_act());
//   lv_label_set_recolor(number, true);
//   lv_obj_set_style_bg_color(number, LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
//   lv_obj_align(number, LV_ALIGN_LEFT_MID, 150, 40);
//   lv_timer_create(add_value, 50, NULL);
//   //  lv_label_set_text_fmt(number, "#FFFFFF %d#",15);
// }




