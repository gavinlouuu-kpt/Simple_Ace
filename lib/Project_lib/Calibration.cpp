#include "Calibration.h"
#include "Simple_ACE.h"
#include "Screen.h"
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>

Adafruit_ADS1115 ads;

// void draw_process(int trial){
//     tft.fillRect(80,140,140,180,TFT_NEIGHBOUR_GREEN);
//     String text = "Calibration: Trial" + (String)trial;
//     text = text.c_str();
//     tft.setTextDatum(CC_DATUM);
//     tft.drawString(text,120,160,2);
// }

// void draw_result(){
//     tft.setTextDatum(TL_DATUM);
//     tft.drawFloat((float)ratio[0],200,160,1);
//     tft.drawFloat((float)ratio[1],200,170,1);
//     tft.drawFloat((float)slope,200,180,1);
//     tft.drawFloat((float)constant,200,190,1);
// }

// int peak_profile[10];
// void calibration(){
//     long previous_time;
//     int counter = 0;
//     double baseline =  baselineRead(CO2_channel);
//     for (int sample=0; sample<2; sample ++){
//         draw_process(sample);
//         while(millis() - previous_time< sampletime -1){
//             array[counter] = ads.readADC_SingleEnded(CO2_channel);
//             counter +=1;
//         }
//         double peak =0; 
//         for (int i = acetone_i; i <acetone_o-4; i ++){
//             // if(array[i]> peak){ //find peak at the peak position
//             //     peak = array[i];
//             // }
//             int previous;
//             int diff = array[i] - array[i+4];
//             if(abs(previous)>5 && abs(diff)>5 && previous >0 && diff< 0){

//             }


//         }
//         ratio[sample] = ads_convert(peak,true)/ads_convert(baseline,true);
//     }
//     update_parameters();
// }

// void calibration_input(){
//     //create a input value
//     calibration_setup(value);
//     array_locate(value);
//     calibration();
// }

short array[SAMPLE_LENGTH] = {12389 ,12376 ,12372 ,12374 ,12378 ,12368 ,12388 ,12374 ,12355 ,12377 ,12360 ,12358 ,12360 ,12401 ,12384 ,12363 ,12348 ,12366 ,12386 ,12385 ,12368 ,12344 ,12375 ,12379 ,12367 ,12381 ,12377 ,12388 ,12369 ,12375 ,12376 ,12361 ,12387 ,12381 ,12384 ,12356 ,12342 ,12369 ,12359 ,12364 ,12362 ,12355 ,12347 ,12375 ,12366 ,12353 ,12346 ,12373 ,12372 ,12362 ,12357 ,12358 ,12364 ,12378 ,12371 ,12357 ,12365 ,12384 ,12370 ,12376 ,12396 ,12404 ,12425 ,12428 ,12449 ,12450 ,12449 ,12474 ,12505 ,12509 ,12547 ,12556 ,12582 ,12645 ,12631 ,12671 ,12700 ,12711 ,12768 ,12777 ,12822 ,12846 ,12880 ,12919 ,12941 ,12967 ,13015 ,13029 ,13087 ,13108 ,13142 ,13149 ,13176 ,13218 ,13217 ,13236 ,13270 ,13291 ,13286 ,13305 ,13311 ,13317 ,13315 ,13320 ,13324 ,13321 ,13293 ,13328 ,13303 ,13290 ,13283 ,13284 ,13275 ,13266 ,13245 ,13242 ,13242 ,13233 ,13244 ,13195 ,13171 ,13153 ,13172 ,13144 ,13142 ,13130 ,13113 ,13082 ,13107 ,13056 ,13065 ,13025 ,13040 ,13017 ,12994 ,12980 ,12977 ,12964 ,12951 ,12950 ,12928 ,12916 ,12921 ,12919 ,12895 ,12906 ,12874 ,12849 ,12840 ,12845 ,12842 ,12829 ,12836 ,12809 ,12789 ,12806 ,12795 ,12777 ,12767 ,12778 ,12773 ,12758 ,12741 ,12762 ,12730 ,12707 ,12715 ,12716 ,12702 ,12701 ,12682 ,12681 ,12696 ,12677 ,12674 ,12695 ,12687 ,12667 ,12644 ,12634 ,12655 ,12636 ,12643 ,12623 ,12615 ,12622 ,12620 ,12606 ,12616 ,12602 ,12585 ,12610 ,12574 ,12607 ,12582 ,12586 ,12585 ,12583 ,12574 ,12563 ,12564 ,12579 ,12540 ,12542 ,12565 ,12556 ,12550 ,12545 ,12553 ,12568 ,12550 ,12544 ,12536 ,12535 ,12556 ,12554 ,12526 ,12546 ,12523 ,12505 ,12523 ,12519 ,12518 ,12501 ,12517 ,12502 ,12505 ,12500 ,12514 ,12511 ,12508 ,12494 ,12474 ,12523 ,12489 ,12508 ,12473 ,12484 ,12505 ,12505 ,12494 ,12478 ,12498 ,12478 ,12480 ,12481 ,12500 ,12472 ,12457 ,12469 ,12475 ,12466 ,12478 ,12481 ,12452 ,12463 ,12467 ,12468 ,12459 ,12463 ,12455 ,12445 ,12447 ,12435 ,12425 ,12434 ,12445 ,12433 ,12423 ,12430 ,12430 ,12414 ,12432 ,12431 ,12413 ,12434 ,12426 ,12403 ,12419 ,12444 ,12438 ,12421 ,12439 ,12430 ,12420 ,12403 ,12408 ,12433 ,12439 ,12409 ,12423 ,12430 ,12427 ,12414 ,12445 ,12442 ,12433 ,12450 ,12415 ,12420 ,12425 ,12425 ,12428 ,12435 ,12415 ,12414 ,12401 ,12390 ,12405 ,12411 ,12403 ,12413 ,12411 ,12392 ,12402 ,12403 ,12379 ,12399 ,12392 ,12408 ,12394 ,12390 ,12395 ,12393 ,12399 ,12386 ,12407 ,12397 ,12393 ,12395 ,12390 ,12406 ,12396 ,12394 ,12389 ,12406 ,12377 ,12377 ,12385 ,12389 ,12397 ,12404 ,12405 ,12407 ,12384 ,12387 ,12397 ,12382 ,12386 ,12392 ,12386 ,12385 ,12377 ,12384 ,12360 ,12382 ,12400 ,12381 ,12375 ,12384 ,12391 ,12361 ,12363 ,12382 ,12371 ,12381 ,12388 ,12372 ,12364 ,12382 ,12352 ,12389 ,12378 ,12365 ,12378 ,12376 ,12379 ,12374 ,12372 ,12361 ,12364 ,12372 ,12389 ,12355 ,12379 ,12378 ,12375 ,12363 ,12368 ,12389 ,12386 ,12355 ,12377 ,12387 ,12403 ,12385 ,12384 ,12371 ,12388 ,12374 ,12351 ,12371 ,12382 ,12361 ,12377 ,12378 ,12376 ,12391 ,12360 ,12371 ,12379 ,12383 ,12381 ,12351 ,12381 ,12387 ,12384 ,12382 ,12372 ,12384 ,12386 ,12387 ,12372 ,12376 ,12389 ,12371 ,12368 ,12384 ,12386 ,12376 ,12391 ,12381 ,12373 ,12374 ,12387 ,12376 ,12379 ,12382 ,12375 ,12394 ,12386 ,12379 ,12392 ,12385 ,12379 ,12369 ,12379 ,12380 ,12386 ,12387 ,12365 ,12395 ,12363 ,12364 ,12376 ,12369 ,12394 ,12401 ,12378 ,12399 ,12376 ,12384 ,12375 ,12362 ,12371 ,12371 ,12390 ,12375 ,12391 ,12389 ,12385 ,12378 ,12398 ,12390 ,12389 ,12390 ,12378 ,12402 ,12387 ,12372 ,12394 ,12401 ,12387 ,12384 ,12374 ,12401 ,12396 ,12401 ,12406 ,12386 ,12408 ,12391 ,12382 ,12397 ,12391 ,12398 ,12403 ,12404 ,12410 ,12407 ,12402 ,12421 ,12400 ,12400 ,12403 ,12405 ,12391 ,12396 ,12386 ,12419 ,12404 ,12409 ,12391 ,12400 ,12396 ,12397 ,12399 ,12393 ,12402 ,12400 ,12386 ,12406 ,12400 ,12400 ,12403 ,12425 ,12411 ,12409 ,12405 ,12399 ,12415 ,12424 ,12416 ,12396 ,12405 ,12419 ,12419 ,12406 ,12414 ,12420 ,12402 ,12413 ,12419 ,12421 ,12429 ,12424 ,12409 ,12416 ,12415 ,12416 ,12431 ,12412 ,12420 ,12422 ,12429 ,12427 ,12465 ,12439 ,12442 ,12438 ,12461 ,12442 ,12445 ,12439 ,12448 ,12457 ,12474 ,12465 ,12483 ,12454 ,12443 ,12452 ,12459 ,12475 ,12472 ,12456 ,12465 ,12473 ,12466 ,12459 ,12457 ,12479 ,12492 ,12492 ,12473 ,12494 ,12496 ,12477 ,12479 ,12498 ,12491 ,12473 ,12483 ,12488 ,12508 ,12510 ,12518 ,12512 ,12514 ,12506 ,12504 ,12499 ,12516 ,12524 ,12504 ,12506 ,12518 ,12507 ,12538 ,12523 ,12531 ,12530 ,12519 ,12521 ,12535 ,12545 ,12533 ,12544 ,12548 ,12559 ,12522 ,12534 ,12535 ,12530 ,12538 ,12548 ,12532 ,12542 ,12550 ,12554 ,12537 ,12535 ,12561 ,12542 ,12558 ,12543 ,12549 ,12566 ,12580 ,12572 ,12560 ,12553 ,12580 ,12571 ,12565 ,12578 ,12564 ,12561 ,12562 ,12584 ,12582 ,12605 ,12575 ,12574 ,12588 ,12580 ,12600 ,12587 ,12578 ,12577 ,12576 ,12578 ,12587 ,12591 ,12587 ,12592 ,12591 ,12582 ,12604 ,12604 ,12614 ,12601 ,12615 ,12605 ,12605 ,12634 ,12631 ,12615 ,12615 ,12614 ,12616 ,12627 ,12630 ,12629 ,12642 ,12629 ,12642 ,12637 ,12636 ,12635 ,12666 ,12651 ,12659 ,12640 ,12669 ,12648 ,12655 ,12672 ,12666 ,12669 ,12666 ,12684 ,12652 ,12678 ,12682 ,12687 ,12683 ,12682 ,12692 ,12687 ,12685 ,12685 ,12676 ,12701 ,12702 ,12691 ,12677 ,12692 ,12704 ,12693 ,12694 ,12687 ,12697 ,12701 ,12700 ,12701 ,12704 ,12724 ,12707 ,12706 ,12714 ,12712 ,12729 ,12704 ,12724 ,12717 ,12740 ,12725 ,12755 ,12728 ,12727 ,12749 ,12735 ,12755 ,12746 ,12727 ,12759 ,12746 ,12742 ,12741 ,12774 ,12743 ,12760 ,12765 ,12761 ,12762 ,12778 ,12770 ,12776 ,12775 ,12782 ,12773 ,12771 ,12764 ,12794 ,12787 ,12791 ,12788 ,12798 ,12774 ,12793 ,12809 ,12808 ,12802 ,12808 ,12811 ,12792 ,12818 ,12804 ,12820 ,12808 ,12828 ,12832 ,12818 ,12804 ,12828 ,12853 ,12827 ,12826 ,12843 ,12840 ,12815 ,12819 ,12833 ,12838 ,12858 ,12824 ,12832 ,12841 ,12851 ,12872 ,12861 ,12833 ,12851 ,12854 ,12852};

double filtered_array[SAMPLE_LENGTH]={0};
int entry_counter = 0;
int peak_counter = 0;
int peak_buffer[100];

int previous_buf = peak_buffer[0];
int peak_count = 1;
int position[10] = {0};
int position_average = 0;
int position_counter = 0;
float ref_position[2];

int finding_baseline();
void process_data();
void find_peak();

void EEPROM_setup(){
  EEPROM.begin(20);
}

void process_data(){
  for(int i = 0; i < sizeof(array)/sizeof(array[0])-10; i ++){
    if (array[i] != 0){
      int sum = 0;
      double mean;
      for( int j = 0; j <10 ; j++){
        sum = sum + array[i+j];
      } 
      mean = sum/10.00;
      delay(1);
    //   printf("%.2f\n",mean);
      filtered_array[i] = mean;
    }
  }
}

void find_peak(){
  for (int q = 0; q < sizeof(array)/sizeof(array[0])-10 -200; q ++){
    if (filtered_array[q] != 0){
      printf("%d\n",q);
      int diff = filtered_array[q+99] -filtered_array[q];
      int diff2= filtered_array[q+199] - filtered_array[q+99];
      int diff3 = filtered_array[q+99-5] -filtered_array[q+99+5];
      printf("Previous : %d, Difference:  %d\n",diff,diff2);
    // if(diff >15 && diff2< -15 && (fabsf(filtered_array[i]-filtered_array[i+199])<10||fabsf(filtered_array[i]-filtered_array[i+199]) > 50)){
      if(diff >15 && diff2< -15 && fabsf(diff3)<10){
        peak_buffer[peak_counter] = q+99;
        peak_counter += 1;
        } 
      delay(1);
    }
  }

  for (int j = 0; j < peak_candidate; j++){
    if(peak_buffer[j] != 0){
      printf("Grabbed point %d : %d\n",j+1,peak_buffer[j]);
      if(peak_buffer[j]-previous_buf >100){
        position_average = position_average/position_counter;
        position[peak_count-1]= position_average;
        
        peak_count +=1;

        position_average = 0;
        position_counter = 0;
      }
      position_average += peak_buffer[j];
      printf("position sum: %d\n",position_average);
      position_counter += 1;
      printf("position counte: %d\n",position_counter);
      previous_buf = peak_buffer[j];
    }
  }

  position_average = position_average/position_counter;
  position[peak_count-1]= position_average;
  printf("Peak Total: %d\n", peak_count);
  for (int i =0; i< show_peak; i++){
    printf("Peak position: %d\n", position[i]);
  }
}

void update_parameters(){
    ref_position[0] = float(position[0]);
    ref_position[1] = float(position[1]);
    printf("updated: %f\n",ref_position[1]);
    int address = 0;
    EEPROM.put(address, position[0]);
    address += sizeof(int);
    EEPROM.put(address, position[1]);
    EEPROM.commit();
    EEPROM.end();
    printf("EEPROM address: %d, value: %d\n", address,position[0]);
    printf("EEPROM address: %d, value: %d\n", address, position[1]);
    
}

void calibration() { //put your main code here, to run repeatedly:
  delay(5000);
  long previous = millis(); 


  while(millis() - previous < 30000){
    array[entry_counter] = ads.readADC_SingleEnded(1);
    printf(" %d\n", array[entry_counter]);
    entry_counter += 1;
    delay(5);
    printf("Counter 1: %d\n", entry_counter);
  }

  process_data();
  find_peak();
  update_parameters();
}
