#include "led.h"


void LED::SetRed(){
  red_led_ = 1;
  green_led_ = 0;
  blue_led_ = 0;
}

void LED::SetGreen(){
  red_led_ = 0;
  green_led_ = 1;
  blue_led_ = 0;
}

void LED::SetBlue(){
  red_led_ = 0;
  green_led_ = 0;
  blue_led_ = 1;
}

void LED::Off(){
  red_led_ = 0;
  green_led_ = 0;
  blue_led_ = 0;
}

