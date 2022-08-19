#include <Arduino.h>

#include "SPIFFS.h"
// #include <lvgl.h>
// #include <BlynkSimpleEsp32.h>
// BlynkWifi Blynk(_blynkTransport);
#include <Simple_ACE.h>

////////////////////////SPIFFS File//////////////////////////////////////
String format_1 = "/";
String format_2 = ".txt";
String file[255];
//////////////////////////Request time//////////////////////////////////////
unsigned long previous_time;
//////////////////////////functions/////////////////////////////////////////
int data_logging(double value, double value_1, double value_2, double value_3,  int storage);
double mapping(double CO2, double O2);
///////////////////////////////////Global Data.//////////////////////////////////////
double avg_ratio_Ace;
double avg_ratio_O2 ;
double rq;
double map_rq;
int file_label;
short CO2_arr[store_size] = {0};
short O2_arr[store_size] = {0};
File dat_file_app;


/*Change to your screen resolution*/
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];


TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
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

/*Read the touchpad*/
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

void setup() {
  Serial.begin(115200);
  //  tftSetup();
  pinSetup();
  analogSetup();
  checkSetup();

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

  Serial.println( "Setup done" );

  // only flush the file when EEPROM is rebooted
  if (EEPROM.read(EEP_add) == 0) {
    printf("Clearing Files");
    for (int i = 0; i < 255 ; i ++) {
      String filename = format_1 + (String)i + format_2;
      SPIFFS.remove(filename);
      if (!filename) {
        File dat_file_w = SPIFFS.open(filename, FILE_WRITE);
        Serial.println("cleared");
        if (!dat_file_w) {
          Serial.println("There was an error opening the file for writing");
          return;
        }
        dat_file_w.close();
      }
    }
  }
  lv_example_chart_2();
}

void loop() {
  // while(1){
  lv_timer_handler();
  delay(5);
  // }
  // for (int i = 0; i < 3; i++) {
  //   sample_collection(i);
  //   if ( store == true) {
  //     file_label = EEPROM.read(EEP_add);
  //     Serial.println(file_label);
  //     String filename = format_1 + (String)file_label + format_2;
  //     Serial.println(filename);
  //     File dat_file_app = SPIFFS.open(filename, FILE_WRITE);
  //     for (int i = 0; i < plot_size; i++) {
  //       dat_file_app.print((unsigned long)millis()); dat_file_app.print(" , "); dat_file_app.print(CO2_arr[i]); dat_file_app.print(" , "); dat_file_app.println(O2_arr[i]);
  //       Serial.println(i);
  //     }
  //     dat_file_app.close();
  //     Serial.println("saved");
  //     Serial.print(EEP_add); Serial.print("\t"); Serial.println(file_label, DEC);
  //     file_label = file_label + 1;
  //     Serial.println(file_label);
  //     EEPROM.write(EEP_add, file_label);
  //     EEPROM.commit();
  //   }
  //   delay(4000);
  // }

  // avg_ratio_Ace =  sort_reject(ratio_Ace, 3);
  // // avg_ratio_O2 = sort_reject(ratio_O2, 3);
  // // mapping(avg_ratio_CO2, avg_ratio_O2);
  // // Blynk.connect();
  // // blynk_upload(avg_ratio_Ace, avg_ratio_O2, rq, map_rq);//editted

  // Serial.print("Acetone Ratio is: "); Serial.println(avg_ratio_Ace, 6); 
  // //    data_logging(avg_ratio_CO2, avg_ratio_O2,rq , 0 , 1 );
  // // Serial.print("Fat Burn effeciency: "); Serial.print(rq); Serial.print (" "); Serial.println(map_rq);
  // data_logging(avg_ratio_Ace, avg_ratio_O2, rq , 0, 5 );
  // tft.fillScreen(BLACK);
  // //  tft.setTextSize(2); tft.setCursor(15, 90); tft.println("Fat Burn");
  // //  tft.setTextSize(3); tft.setCursor(35, 110); tft.print(map_rq, 1);
  // // comment the following two lines if using 2 inch screen
  // tft.setTextSize(4); tft.setCursor(25, 130); tft.println("Fat Burn");
  // tft.setTextSize(5); tft.setCursor(55, 180); tft.print(map_rq, 1);
  // previous_time = getTime();

  // power_saving(previous_time);
}

// int data_logging(double value, double value_1, double value_2, double value_3,  int storage) {
//   if (!dat_file_app) {
//     Serial.println("There was an error opening the file for appending");
//   }
//   switch (storage) {
//     case 1 :
//       dat_file_app.println(value);
//       break;
//     case 2 :
//       dat_file_app.print(value); dat_file_app.print(" , "); dat_file_app.println(value_1);
//       break;
//   }
//   return(0);
// }
