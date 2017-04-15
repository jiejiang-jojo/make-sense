#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DBG_SERIAL_BAUD 38400
#define ESP_LINK_SERIAL_BAUD 115200
#define BLE_SERIAL_BAUD 38400

#define RECORDSTR_LEN 720
#define HTTP_LEN 1024
#define BUFLEN 266
#define PACKET_LEN 4

#define PRIVACY_PERIOD 6   // seconds
#define SAMPLE_RATE 3    // seconds
#define POST_RATE 2      // seconds
#define HIGHFREQ_SAMPLE_RATE 0.001  //seconds
#define GESTURE_SAMPLE_RATE 0.1  //seconds

#define BLUETOOTH_SCAN_RATE 10   //seconds
#define BLUETOOTH_ACTIVE_PERIOD 10
#define BLUETOOTH_SCAN_REST_PERIOD 0.5
#define BLUETOOTH_TIMEOUT 1
#define BLUETOOTH_RSSI_MIN -104

#define PARTICULATE_SAMPLE_SIZE 3 

#define BOX_ID 108                         //sensor box ID
#define SERVER_IP "YOUR_SERVER_IP"        //data server IP
#define AES_KEY "YOUR_ENCRYPTION_KEY"
#define AES_IV "YOUR_ENCRYPTION_IV"

#define BLUETOOTH_NUM   2
#define BLUETOOTH_MAC_0 "YOUR_BLUETOOTH_MAC_1",
#define BLUETOOTH_MAC_1 "YOUR_BLUETOOTH_MAC_2"
#define BLUETOOTH_MAC_2
#define BLUETOOTH_MAC_3
#define BLUETOOTH_MAC_4

#endif
