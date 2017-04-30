#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "mbed.h"
#include "BGLib.h"
#include "util.h"

class BluetoothReading {
  BGLib _bg;

public:
  bd_addr_t * macs;
  int * rssi;
  time_t * lastseen;

  BluetoothReading(int num, const char * bt_macs[], BGLib bg):  _bg(bg){
    macs = new bd_addr_t[num]();
    rssi = new int[num]();
    lastseen = new time_t[num]();
    for (int i=0; i<num; i++){
      str2mac(bt_macs[i], macs[i].addr);
    }
  };

};

#endif
