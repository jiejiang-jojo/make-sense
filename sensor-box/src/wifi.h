#ifndef __WIFI_H__
#define __WIFI_H__

#include "mbed.h"
#include "led.h"
#include "ELClient.h"
#include "ELClientRest.h"
#include "ELClientCmd.h"
#include "config.h"

class Wifi {

  Serial m_serial;
  ELClient m_esp;
  LED m_led;


public:

  bool connected = false;

  // Initialize a REST client on the connection to esp-link
  ELClientRest rest;

  // Initialize CMD client (for GetTime)
  ELClientCmd cmd;

  Wifi(PinName tx, PinName rx, LED led): m_serial(tx, rx), m_esp(&m_serial, &m_serial), m_led(led), rest(&m_esp), cmd(&m_esp){
    m_serial.baud(ESP_LINK_SERIAL_BAUD);   // the baud rate here needs to match the esp-link config
  }
  void callback_handler(void *response);
  int get_status();
  int setup();
  void setup_time();
  time_t get_time();
  void process();
  void reconnect();
};

#endif
