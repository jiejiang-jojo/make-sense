#ifndef __WIFI_H__
#define __WIFI_H__

#include "mbed.h"
#include "led.h"
#include "ELClient.h"
#include "ELClientRest.h"
#include "ELClientCmd.h"

class Wifi {

  Serial _serial;
  ELClient _esp;
  LED _led;


public:

  bool connected = false;

  // Initialize a REST client on the connection to esp-link
  ELClientRest rest;

  // Initialize CMD client (for GetTime)
  ELClientCmd cmd;

  Wifi(PinName tx, PinName rx, LED led): _serial(tx, rx), _esp(&_serial, &_serial), _led(led), rest(&_esp), cmd(&_esp){}
  void wifiCb(void *response);
  int get_wifiStatus();
  int setup();
  void setup_time();
  void process();
};

#endif
