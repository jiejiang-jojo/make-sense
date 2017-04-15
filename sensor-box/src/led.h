#ifndef _LED_H_
#define _LED_H_

#include "mbed.h"

class LED {

  DigitalOut red_led_;
  DigitalOut green_led_;
  DigitalOut blue_led_;

public:
  LED(PinName red, PinName green, PinName blue): red_led_(red), green_led_(green), blue_led_(blue){};
  void SetRed();
  void SetGreen();
  void SetBlue();
  void Off();
};

#endif
