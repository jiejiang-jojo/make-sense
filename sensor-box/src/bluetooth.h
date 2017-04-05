#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "mbed.h"
#include "BGLib.h"
#include "util.h"

class BluetoothReading {
  BGLib _bg;

public:
  bd_addr_t * macs;
  int *rssi;
  time_t *lastseen;
  const int number_of_devices;
  BluetoothReading(const char * bt_macs[], BGLib bg):  _bg(bg), number_of_devices(sizeof(bt_macs)){
    macs = new bd_addr_t[number_of_devices];
    for (uint8_t i=0; i<number_of_devices; i++){
      hex2bin(bt_macs[i], macs[i].addr);
    }
    rssi = new int[number_of_devices];
    lastseen = new time_t[number_of_devices];
  };

};

#endif
