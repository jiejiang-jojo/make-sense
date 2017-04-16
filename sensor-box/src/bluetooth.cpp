#include "mbed.h"
#include "rtos.h"
#include "BGLib.h"
#include "util.h"
#include "bluetooth.h"
#include "config.h"

#define _BLE_RESET_

#ifdef _BLE_RESET_
DigitalOut RES_PIN(P0_5);
#endif

extern Serial serial;

BufferedSoftSerial eggble(P0_0, P0_1); // tx, rx for Bluetooth
BGLib ble112((BufferedSoftSerial *)&eggble, &serial, 1);


// ##### Configuration of bands
const char * bluetooth_macs[] = {
  BLUETOOTH_MAC_0
  BLUETOOTH_MAC_1
  BLUETOOTH_MAC_2
  BLUETOOTH_MAC_3
  BLUETOOTH_MAC_4
};

BluetoothReading bt_reading(BLUETOOTH_NUM, bluetooth_macs, ble112);

int read_bluetooth_signal(int device){
  if (device >= BLUETOOTH_NUM)
    return BLUETOOTH_RSSI_MIN;

  time_t current_time = time(NULL);
  if (current_time - bt_reading.lastseen[device] > BLUETOOTH_ACTIVE_PERIOD)
    return BLUETOOTH_RSSI_MIN;
  else
    return bt_reading.rssi[device];

}

void my_ble_evt_system_boot(const ble_msg_system_boot_evt_t *msg) {
  DBG("BLE: %d.%d.%d-%X LL=%X protocol=%X hw=%X\n",
      msg -> major,
      msg -> minor,
      msg -> patch,
      msg -> build,
      msg -> ll_version,
      msg -> protocol_version,
      msg -> hw);
}

void my_evt_gap_scan_response(const ble_msg_gap_scan_response_evt_t *msg) {
    for (int i=0; i<BLUETOOTH_NUM; i++){
      if (memcmp(bt_reading.macs[i].addr, msg->sender.addr, 6) == 0){
        bt_reading.rssi[i] = msg->rssi;
        bt_reading.lastseen[i] = time(NULL);
      }
    }
    // DBG("###\tgap_scan_response: { ");
    // DBG("rssi: "); DBG("%d",msg -> rssi);
    // DBG(", packet_type: "); DBG("%X",(uint8_t)msg -> packet_type);
    // DBG(", sender: ");
    // // this is a "bd_addr" data type, which is a 6-byte uint8_t array
    // for (uint8_t i = 0; i < 6; i++) {
    //     // if (msg -> sender.addr[i] < 16) putc('0');
    //     DBG("%02X",msg -> sender.addr[i]);
    // }
    // DBG(", address_type: "); DBG("%X",msg -> address_type);
    // DBG(", bond: "); DBG("%X",msg -> bond);
    // DBG(", data: ");
    // // this is a "uint8array" data type, which is a length byte and a uint8_t* pointer
    // for (uint8_t i = 0; i < msg -> data.len; i++) {
    //     if (msg -> data.data[i] < 16) putc('0');
    //     DBG("%X",msg -> data.data[i]);
    // }
    // DBG(" }\r\n");
}

void my_rsp_gap_set_scan_parameters(const ble_msg_gap_set_scan_parameters_rsp_t *msg) {
    // DBG("<--\tgap_set_scan_parameters: { ");
    // DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    // DBG(" }\r\n");
}

void my_rsp_gap_discover(const ble_msg_gap_discover_rsp_t *msg) {
    // DBG("<--\tgap_discover: { ");
    // DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    // DBG(" }\r\n");
}

void my_rsp_gap_set_filtering(const ble_msg_gap_set_filtering_rsp_t *msg) {
    // DBG("<--\tgap_filtering: { ");
    // DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    // DBG(" }\r\n");
}

void my_rsp_system_whitelist_append(const ble_msg_system_whitelist_append_rsp_t *msg) {
    // DBG("<--\tgap_white list append: { ");
    // DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    // DBG(" }\r\n");
}

void my_rsp_system_whitelist_clear(const ble_msg_system_whitelist_clear_rsp_t *msg) {
    // DBG("<--\tgap_white list clear: { ");
    // DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    // DBG(" }\r\n");
}

void my_rsp_gap_end_procedure(const ble_msg_gap_end_procedure_rsp_t *msg) {
    // DBG("<--\tgap_end_procedure: { ");
    // DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    // DBG(" }\r\n");
}

void my_rsp_system_hello(const ble_msg_system_hello_rsp_t *msg) {
    // DBG("<--\tsystem_hello\r\n");
}

void bleinit(){
    // set up internal status handlers (these are technically optional)
    #ifdef _BLE_RESET_
    RES_PIN=1;
    #endif
    eggble.baud(BLE_SERIAL_BAUD);

    // set up BGLib event handlers
    ble112.ble_evt_system_boot = my_ble_evt_system_boot;
    ble112.ble_rsp_gap_set_scan_parameters = my_rsp_gap_set_scan_parameters;
    ble112.ble_rsp_gap_discover = my_rsp_gap_discover;
    ble112.ble_evt_gap_scan_response = my_evt_gap_scan_response;
    ble112.ble_rsp_gap_set_filtering = my_rsp_gap_set_filtering;
    ble112.ble_rsp_system_whitelist_append = my_rsp_system_whitelist_append;
    ble112.ble_rsp_system_whitelist_clear = my_rsp_system_whitelist_clear;
    ble112.ble_rsp_gap_end_procedure = my_rsp_gap_end_procedure;
    ble112.ble_rsp_system_hello = my_rsp_system_hello;

    #ifdef _BLE_RESET_
    RES_PIN=0;
    Thread::wait(50);
    RES_PIN=1;
    #endif
}

void checkActivity(){
    while (ble112.checkActivity(BLUETOOTH_TIMEOUT * 1000));
}

void bluetooth_scan_loop() {
    DBG("BLE scan loop starting...\n");
    bleinit();

    // clear whitelist
    ble112.ble_cmd_system_whitelist_clear();checkActivity();

    for(int i=0; i<BLUETOOTH_NUM; i++) {
      // append a mac adress to the whitelist
      ble112.ble_cmd_system_whitelist_append(bt_reading.macs[i],0);
      checkActivity();
    }

    // set scan policy for only scan devices in the whitelist
    ble112.ble_cmd_gap_set_filtering(BGLIB_GAP_SCAN_POLICY_WHITELIST,BGLIB_GAP_ADV_POLICY_ALL,1); checkActivity();
    // set scan interval and window
    ble112.ble_cmd_gap_set_scan_parameters(0xC0, 0xC0, 0); checkActivity();

    Timer scan_timer;
    while(1){
      time_t seconds = time(NULL);
      // DBG("scanning... %ld\n\r", seconds);
      ble112.ble_cmd_gap_discover(BGLIB_GAP_DISCOVER_GENERIC); checkActivity();
      scan_timer.reset();
      scan_timer.start();
      while(scan_timer.read()<BLUETOOTH_SCAN_RATE){
        ble112.checkActivity(BLUETOOTH_TIMEOUT * 1000);
        Thread::wait(BLUETOOTH_SCAN_REST_PERIOD * 1000);
      }
      ble112.ble_cmd_gap_end_procedure(); while (ble112.checkActivity(BLUETOOTH_TIMEOUT * 1000));
    }
    DBG("Unexpected BLE loop termination");
}
