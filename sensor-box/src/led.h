#ifndef _LED_H_
#define _LED_H_

#include "mbed.h"
class LED {

  DigitalOut _redLed;
  DigitalOut _greenLed;
  DigitalOut _blueLed;

public:
  LED(PinName red, PinName green, PinName blue): _redLed(red), _greenLed(green), _blueLed(blue){};
  /*turn on the blue LED to indicate that the sensor box is powered*/
  void power_on();
  /*turn on the red LED to indicate that the sensor box is in privacy mode*/
  void privacy_on();
  /*turn on the green LED to indicate that the sensor box is connected to wifi*/
  void wifi_on();
  /*turn on the red LED to indicate that wifi is lost*/
  void wifi_off();
  /*turn off the green LED to indicate that the sensor box is off*/
  void off();
};

#endif
