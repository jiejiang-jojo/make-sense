#include "wifi.h"
#include "led.h"
#include "util.h"
#include "config.h"


/* Callback made from esp-link to notify of wifi status changes
Here we print something out and set a global flag*/
void Wifi::wifiCb(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      DBG("WIFI CONNECTED\n\r");
      connected = true;
    } else {
      DBG("WIFI NOT READY: ");
      DBG("%d\n\r", status);
      connected = false;
    }
  }
}

/*get wifi status*/
int Wifi::get_wifiStatus(){
    _esp.GetWifiStatus();
    ELClientPacket *packet;
    if ((packet=_esp.WaitReturn()) != NULL)
        return packet->value;
    return 0;
}

/*set up wifi connection to data server*/
int Wifi::setup() {
    _serial.baud(ESP_LINK_SERIAL_BAUD);   // the baud rate here needs to match the esp-link config
    DBG("EL-Client starting!\n\r");

    // Sync-up with esp-link, this is required at the start of any sketch and initializes the
    // callbacks to the wifi status change callback. The callback gets called with the initial
    // status right after Sync() below completes.
    _esp.wifiCb.attach(this, &Wifi::wifiCb); // wifi status change callback, optional (delete if not desired)
    bool ok;
    do {
        ok = _esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
        if (!ok) DBG("EL-Client sync failed!\n\r");
    } while(!ok);
    DBG("EL-Client synced!\n\r");

    DBG("Wifi status: %d\n\r", get_wifiStatus());

    // Set up the REST client to talk to "host", this doesn't connect to that server,
    // it just sets-up stuff on the esp-link side
    int err = rest.begin(SERVER_IP);
    if (err != 0) {
        DBG("REST begin failed: ");
        DBG("%d\n\r", err);
        this->setup();
    }
    DBG("EL-REST ready\n\r");
    return 1;

}

///*when there is no connection to the data server, try to reconnect*/
//void reconnect_wifi(){
//    DBG("reconnecting...\n\r");
//    //wifi status list:
//    //enum {wifiIsDisconnected, wifiIsConnected, wifiGotIP}
//    do{
//        setup_wifi();
//        esp.Process();
//        DBG("wifiConnected:%d\n\r", wifiConnected);
////        wait(5);
//    }while(!wifiConnected || get_wifiStatus()!=2);
//    wifiConnected = true;
//    wifi_on();
//}

/*set the starting time of data collecction*/
void Wifi::setup_time() {
    int currenttime;

    _esp.Process();

    DBG("wifiConnected:%d\n\r", connected);

    if(connected){
        // Set RTC time
        currenttime = cmd.GetTime();
    } else{
        this->setup();
        setup_time();
    }

    DBG("current time int: %d\n\r", currenttime);
    //re-get time in case that the timestamp is not valid (e.g., 1970-1-1)
    if (currenttime<1471651200){
        setup_time();
    }

    set_time(currenttime);
    time_t seconds = time(NULL);
    DBG("current time str: %s\n\r", ctime(&seconds));
    _led.wifi_on();
}

void Wifi::process() {
  _esp.Process();
}
