#ifndef __WIFI_H__
#define __WIFI_H__

#include "mbed.h"
#include "led.h"
#include "BoxState.h"
#include "ELClient.h"
#include "ELClientRest.h"
#include "ELClientCmd.h"
#include "config.h"

class Wifi {

  Serial serial_;
  ELClient esp_;
  BoxState &box_state_;


public:

  // Initialize a REST client on the connection to esp-link
  ELClientRest rest;

  // Initialize CMD client (for GetTime)
  ELClientCmd cmd;

  Wifi(PinName tx, PinName rx, BoxState &box_state): serial_(tx, rx), esp_(&serial_, &serial_), box_state_(box_state), rest(&esp_), cmd(&esp_){
    serial_.baud(ESP_LINK_SERIAL_BAUD);   // the baud rate here needs to match the esp-link config
  }
  void CallbackHandler(void *response);
  int GetStatus();
  void Sync();
  void Setup();
  time_t GetTime();
  void Process();
  void Connect();
};

#endif
