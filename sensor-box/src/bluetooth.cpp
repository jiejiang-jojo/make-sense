#include "mbed.h"
#include "BGLib.h"
#include "util.h"

#define _DEBUG_
#define _BLE_RESET_

#ifdef _BLE_RESET_
DigitalOut RES_PIN(P0_5);
#endif

BufferedSoftSerial eggble(P0_0,P0_1); // tx, rx for Bluetooth
BGLib ble112((BufferedSoftSerial *)&eggble, 0, 1);

// ##### Configuration of bands
bd_addr_t BAND_MAC1 = {0xf7,0x98,0x7e,0x10,0x0f,0xc8};
bd_addr_t BAND_MAC2 = {0xac,0x93,0x7e,0x10,0x0f,0xc8};
int rssi[] = {0, 0};
time_t lastseen[] = {0, 0};

int get_bluetooth_signal(int device){
  time_t current_time = time(NULL);
  if (current_time - lastseen[device] > 10)
    return 0;
  else
    return rssi[device];

}

void my_ble_evt_system_boot(const ble_msg_system_boot_evt_t *msg) {
    #ifdef _DEBUG_
        DBG("###\tsystem_boot: { ");
        DBG("major: "); DBG("%X",msg -> major);
        DBG(", minor: "); DBG("%X",msg -> minor);
        DBG(", patch: "); DBG("%X",msg -> patch);
        DBG(", build: "); DBG("%X",msg -> build);
        DBG(", ll_version: "); DBG("%X",msg -> ll_version);
        DBG(", protocol_version: "); DBG("%X",msg -> protocol_version);
        DBG(", hw: "); DBG("%X",msg -> hw);
        DBG(" }\r\n");
    #endif
}

void my_evt_gap_scan_response(const ble_msg_gap_scan_response_evt_t *msg) {
    #ifdef _DEBUG_
    DBG("###\tgap_scan_response: { ");
    DBG("rssi: "); DBG("%d",msg -> rssi);
    if (memcmp(BAND_MAC1.addr, msg->sender.addr, 6) == 0){
      rssi[0] = msg->rssi;
      lastseen[0] = time(NULL);
    } else {
      rssi[1] = msg->rssi;
      lastseen[1] = time(NULL);
    }
    DBG(", packet_type: "); DBG("%X",(uint8_t)msg -> packet_type);
    DBG(", sender: ");
    // this is a "bd_addr" data type, which is a 6-byte uint8_t array
    for (uint8_t i = 0; i < 6; i++) {
        // if (msg -> sender.addr[i] < 16) putc('0');
        DBG("%X",msg -> sender.addr[i]);
    }
//    DBG(", address_type: "); DBG("%X",msg -> address_type);
//    DBG(", bond: "); DBG("%X",msg -> bond);
//    DBG(", data: ");
//    // this is a "uint8array" data type, which is a length byte and a uint8_t* pointer
//    for (uint8_t i = 0; i < msg -> data.len; i++) {
//        if (msg -> data.data[i] < 16) putc('0');
//        DBG("%X",msg -> data.data[i]);
//    }
    DBG(" }\r\n");
    #endif
}

void my_rsp_gap_set_scan_parameters(const ble_msg_gap_set_scan_parameters_rsp_t *msg) {
    #ifdef _DEBUG_
    DBG("<--\tgap_set_scan_parameters: { ");
    DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    DBG(" }\r\n");
    #endif
}

void my_rsp_gap_discover(const ble_msg_gap_discover_rsp_t *msg) {
    #ifdef _DEBUG_
    DBG("<--\tgap_discover: { ");
    DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    DBG(" }\r\n");
    #endif
}

void my_rsp_gap_set_filtering(const ble_msg_gap_set_filtering_rsp_t *msg) {
    #ifdef _DEBUG_
    DBG("<--\tgap_filtering: { ");
    DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    DBG(" }\r\n");
    #endif
}

void my_rsp_system_whitelist_append(const ble_msg_system_whitelist_append_rsp_t *msg) {
    #ifdef _DEBUG_
    DBG("<--\tgap_white list append: { ");
    DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    DBG(" }\r\n");
    #endif
}

void my_rsp_system_whitelist_clear(const ble_msg_system_whitelist_clear_rsp_t *msg) {
    #ifdef _DEBUG_
    DBG("<--\tgap_white list clear: { ");
    DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    DBG(" }\r\n");
    #endif
}

void my_rsp_gap_end_procedure(const ble_msg_gap_end_procedure_rsp_t *msg) {
    DBG("<--\tgap_end_procedure: { ");
    DBG("result: "); DBG("%X",(uint16_t)msg -> result);
    DBG(" }\r\n");
}

void my_rsp_system_hello(const ble_msg_system_hello_rsp_t *msg) {
    DBG("<--\tsystem_hello\r\n");
}

void bleinit(){
    // set up internal status handlers (these are technically optional)
    #ifdef _BLE_RESET_
    RES_PIN=1;
    #endif
    eggble.baud(38400);

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
    wait(0.05);
    RES_PIN=1;
    #endif
}

void bluetooth_scan_loop() {
    DBG("start...\n\r");
    bleinit();

    // clear whitelist
    ble112.ble_cmd_system_whitelist_clear();
    while (ble112.checkActivity(1000));

    // for(int i=0; i<2; i++) {
      // append a mac adress to the whitelist
      ble112.ble_cmd_system_whitelist_append(BAND_MAC1,0);
      while (ble112.checkActivity(1000));
      ble112.ble_cmd_system_whitelist_append(BAND_MAC2,0);
      while (ble112.checkActivity(1000));
    // }

    // set scan policy for only scan devices in the whitelist
    ble112.ble_cmd_gap_set_filtering(BGLIB_GAP_SCAN_POLICY_WHITELIST,BGLIB_GAP_ADV_POLICY_ALL,1); while (ble112.checkActivity(1000));
    // set scan interval and window
    ble112.ble_cmd_gap_set_scan_parameters(0xC8, 0xC8, 0); while (ble112.checkActivity(1000));


    Timer scan_timer;
    while(1){
      time_t seconds = time(NULL);
      DBG("scanning... %ld\n\r", seconds);
      ble112.ble_cmd_gap_discover(BGLIB_GAP_DISCOVER_GENERIC); while (ble112.checkActivity(1000));
      scan_timer.reset();
      scan_timer.start();
      while(scan_timer.read()<10){
        ble112.checkActivity(1000);
      }
      ble112.ble_cmd_gap_end_procedure(); while (ble112.checkActivity(1000));
    }
    DBG("END");
}
