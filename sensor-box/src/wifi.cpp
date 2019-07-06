#include "wifi.h"
#include "rtos/rtos.h"
#include "led.h"
#include "util.h"
#include "config.h"


/* Callback made from esp-link to notify of wifi status changes
Here we print something out and set a global flag*/
void Wifi::CallbackHandler(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      DBG("WIFI CONNECTED\n");
      box_state_.WifiOn();
    } else {
      DBG("WIFI NOT READY: %d\n", status);
      box_state_.WifiOff();
    }
  }
}

/*get wifi status*/
int Wifi::GetStatus(){
    esp_.GetWifiStatus();
    ELClientPacket *packet;
    uint32_t value = 0;
    if ((packet=esp_.WaitReturn(1000)) != NULL)
        value = packet->value;
    return value;
}

/*set up wifi connection to data server*/
void Wifi::Sync() {
    DBG("EL-Client starting!\n");

    // Sync-up with esp-link, this is required at the start of any sketch and initializes the
    // callbacks to the wifi status change callback. The callback gets called with the initial
    // status right after Sync() below completes.
    esp_.wifiCb.attach(this, &Wifi::CallbackHandler); // wifi status change callback, optional (delete if not desired)
    bool ok;
    do {
        ok = esp_.Sync();      // sync up with esp-link, blocks for up to 2 seconds
        if (!ok) DBG("EL-Client sync failed!\n");
    } while(!ok);
    DBG("EL-Client synced!\n");
    DBG("Wifi status: %d\n", GetStatus());

    // Set up the REST client to talk to "host", this doesn't connect to that server,
    // it just sets-up stuff on the esp-link side
    int err;
    while ((err = rest.begin(SERVER_IP)) != 0){
        DBG("REST begin failed: %d\n", err);
    }
    DBG("EL-REST ready\n");
}

/*when there is no connection to the data server, try to reconnect*/
void Wifi::Connect(){
   DBG("reconnecting...\n");
   //wifi status list:
   //enum {wifiIsDisconnected, wifiIsConnected, wifiGotIP}
   do{
       Sync();
       Process();
       DBG("wifiConnected:%d\n", box_state_.IsWifiOn());
       Thread::wait(1000);
   } while(!box_state_.IsWifiOn() || GetStatus()!=2);
}

time_t Wifi::GetTime() {
    Process();
    return cmd.GetTime();
}

void Wifi::SyncClock() {
    //re-get time in case that the timestamp is not valid (e.g., 1970-1-1)
    int currenttime;
    while((currenttime = GetTime()) < 1471651200){
      Thread::wait(1000);
      Process();
      DBG("current time int: %d\n", currenttime);
    }

    set_time(currenttime);
    time_t seconds = time(NULL);
    char * timestr = ctime(&seconds);
    DBG("current time str: %s\n", timestr);
}

/*set the starting time of data collecction*/
void Wifi::Setup() {

    Process();

    DBG("Adjusting time...");
    if (!box_state_.IsWifiOn())
      Connect();
    SyncClock();
}

void Wifi::Process() {
  esp_.Process();
}
