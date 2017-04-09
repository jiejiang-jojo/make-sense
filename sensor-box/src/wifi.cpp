#include "wifi.h"
#include "led.h"
#include "util.h"
#include "config.h"


/* Callback made from esp-link to notify of wifi status changes
Here we print something out and set a global flag*/
void Wifi::callback_handler(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      DBG("WIFI CONNECTED\n");
      connected = true;
    } else {
      DBG("WIFI NOT READY: %d\n", status);
      connected = false;
    }
  }
}

/*get wifi status*/
int Wifi::get_status(){
    m_esp.GetWifiStatus();
    ELClientPacket *packet;
    if ((packet=m_esp.WaitReturn()) != NULL)
        return packet->value;
    return 0;
}

/*set up wifi connection to data server*/
int Wifi::setup() {
    DBG("EL-Client starting!\n");

    // Sync-up with esp-link, this is required at the start of any sketch and initializes the
    // callbacks to the wifi status change callback. The callback gets called with the initial
    // status right after Sync() below completes.
    m_esp.wifiCb.attach(this, &Wifi::callback_handler); // wifi status change callback, optional (delete if not desired)
    bool ok;
    do {
        ok = m_esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
        if (!ok) DBG("EL-Client sync failed!\n");
    } while(!ok);
    DBG("EL-Client synced!\n");
    DBG("Wifi status: %d\n", get_status());

    // Set up the REST client to talk to "host", this doesn't connect to that server,
    // it just sets-up stuff on the esp-link side
    int err;
    while ((err = rest.begin(SERVER_IP)) != 0){
        DBG("REST begin failed: %d\n", err);
    }
    DBG("EL-REST ready\n");
    return 1;
}

/*when there is no connection to the data server, try to reconnect*/
void Wifi::reconnect(){
   DBG("reconnecting...\n");
   //wifi status list:
   //enum {wifiIsDisconnected, wifiIsConnected, wifiGotIP}
   do{
       setup();
       m_esp.Process();
       DBG("wifiConnected:%d\n", connected);
       wait(1);
   }while(!connected || get_status()!=2);
   connected = true;
   m_led.wifi_on();
}

time_t Wifi::get_time() {
    m_esp.Process();
    return cmd.GetTime();
}

/*set the starting time of data collecction*/
void Wifi::setup_time() {

    m_esp.Process();

    DBG("wifiConnected:%d\n", connected);
    while(!connected){
      wait(1);
      process();
      DBG("wifiConnected:%d\n", connected);
    }

    //re-get time in case that the timestamp is not valid (e.g., 1970-1-1)
    int currenttime;
    while((currenttime = get_time()) < 1471651200){
      wait(1);
      process();
      DBG("current time int: %d\n", currenttime);
    }

    set_time(currenttime);
    time_t seconds = time(NULL);
    char * timestr = ctime(&seconds);
    DBG("current time str: %s\n", timestr);
    m_led.wifi_on();
}

void Wifi::process() {
  m_esp.Process();
}
