/**
 * This is the main class for the sensor box to collect data from different sensor modules.
 */

#include <ELClient.h>
#include <ELClientRest.h>
#include <ELClientCmd.h>
#include <string.h>
#include <stdint.h>
#include <queue>

#include "mbed.h"
#include "rtos.h"
#include "sensors.h"
#include "n25q.h"
#include "aes.h"
#include "base64.h"

#define BOX_ID 108                         //sensor box ID
#define SERVER_IP "YOUR_SERVER_IP"        //data server IP
#define PRIVACY_MODE 1                    //privacy mode gesture
#define RECORDSTR_LEN 720
#define HTTP_LEN 1024
#define BUFLEN 266
#define PACKET_LEN 6

#define DEBUG
#ifdef DEBUG
#define DBG(...) printf (__VA_ARGS__)
#else
#define DBG(...) (void)0
#endif

//LED lights and vibration motor
DigitalOut blueLed(P2_3);
DigitalOut greenLed(P2_4);
DigitalOut redLed(P2_5);
DigitalOut vibr(P2_12);

//Gesture interrupt
InterruptIn gesture_INT(P2_13);

//synchronize the execution of threads
Mutex flash_mutex;

//Global readings
int gesture_read = 0;
int gesture_flag = 0;
int gesture_counter = 0;
float sound_read_sq = 0;
float sound_read = 0;
float sound_counter = 0;
float range_read = 0;

// 128bit key
char key[] = "YOUR_ENCRYPTION_KEY";
//128bit initialization vector
char iv[]  = "YOUR_ENCRYPTION_IV";

//different kinds of sensor module data
//corresponds to one page
union sensor_data{
    struct modules{
//        int box_id;
        int seconds;
        int sample_ctemp;
        int sample_humid;
        int sample_gesture;
        int sample_light;
        int sample_range;
        int sample_sound;
        int sample_particulate;
    }entries[8];
    uint8_t bytes[256];
}recordsW, recordsR;

//flash write and read pointers
int write_pointer = 0;
int read_pointer = 0;

//size of written but not read records
int queue_size = 0;

//Initialize a client to read and write data from N25Q
N25Q *flash;

//Initialize a connection to esp-link using the normal hardware serial port
//both for SLIP and for debug messages
RawSerial serial(P4_28,P4_29);
ELClient esp(&serial, &serial);

// Initialize a REST client on the connection to esp-link
ELClientRest rest(&esp);

// Initialize CMD client (for GetTime)
ELClientCmd cmd(&esp);

bool wifiConnected = false;

/*turn on the blue LED to indicate that the sensor box is powered*/
void power_on(){
    blueLed = 1;
    greenLed = 0;
    redLed = 0;
}

/*turn on the red LED to indicate that the sensor box is in privacy mode*/
void privacy_on(){
    blueLed = 0;
    greenLed = 0;
    redLed = 1;
}

/*turn on the green LED to indicate that the sensor box is connected to wifi*/
void wifi_on(){
    blueLed = 0;
    greenLed = 1;
    redLed = 0;
}

/*turn on the red LED to indicate that wifi is lost*/
void wifi_off(){
    blueLed = 0;
    greenLed = 0;
    redLed = 1;
}

/*turn off the green LED to indicate that the sensor box is off*/
void LED_off(){
    blueLed = 0;
    greenLed = 0;
    redLed = 0;
}

/* Callback made from esp-link to notify of wifi status changes
Here we print something out and set a global flag*/
void wifiCb(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      DBG("WIFI CONNECTED\n\r");
      wifiConnected = true;
    } else {
      DBG("WIFI NOT READY: ");
      DBG("%d\n\r", status);
      wifiConnected = false;
    }
  }
}

/*get wifi status*/
int get_wifiStatus(){
    esp.GetWifiStatus();
    ELClientPacket *packet;
    if ((packet=esp.WaitReturn()) != NULL)
        return packet->value;
    return 0;
}

/*set up wifi connection to data server*/
int setup_wifi() {
    serial.baud(115200);   // the baud rate here needs to match the esp-link config
    DBG("EL-Client starting!\n\r");

    // Sync-up with esp-link, this is required at the start of any sketch and initializes the
    // callbacks to the wifi status change callback. The callback gets called with the initial
    // status right after Sync() below completes.
    esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
    bool ok;
    do {
        ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
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
        setup_wifi();
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
void setup_time() {
    int currenttime;

    esp.Process();

    DBG("wifiConnected:%d\n\r", wifiConnected);

    if(wifiConnected){
        // Set RTC time
        currenttime = cmd.GetTime();
    } else{
        setup_wifi();
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
    wifi_on();
}

/*gesture interruption*/
void detect_gesture() {
    gesture_flag = 1;
    gesture_counter++;
//    gesture_INT.disable_irq();
}

/*read sensor data from all sensor modules*/
void read_sensors(int num){
//    recordsW.entries[num].box_id = BOX_ID;
    recordsW.entries[num].seconds = time(NULL); //Timestamp
    //check whether it is a valid timestamp
    if (recordsW.entries[num].seconds<1471651200) {
        setup_time();
        recordsW.entries[num].seconds = time(NULL); //Timestamp
    }
    recordsW.entries[num].sample_ctemp = read_ctemp();
    recordsW.entries[num].sample_humid = read_humid();
    recordsW.entries[num].sample_gesture = gesture_read;
    recordsW.entries[num].sample_light = read_light();
    //try to use integer to represent float.
    recordsW.entries[num].sample_particulate = (int) (read_particulate()*10000);
    recordsW.entries[num].sample_sound = (int) (cal_sound(sound_read, sound_read_sq, sound_counter)*1000);
    sound_read = 0;
    sound_read_sq = 0;
    sound_counter = 0;
    gesture_counter = 0;
    recordsW.entries[num].sample_range = (int) range_read;
    range_read = 150;
}

/*Sensor Data Get Thread*/
void get_allData(){
    int counter = 0;
    while(1){
        if(gesture_read==-1)
        {
            //stop collecting data for 1 hour
            for(int j=0; j<600; j++){
                Thread::wait(10); //3600000
            }
            gesture_read = 0;
            wifi_on();
        }
        DBG("%f %d\n\r", sound_counter, gesture_counter);
        read_sensors(counter);
        DBG("%d %d %d %d %d %d %d %d\n\r", write_pointer, recordsW.entries[counter].sample_ctemp, recordsW.entries[counter].sample_humid, recordsW.entries[counter].sample_light, recordsW.entries[counter].seconds, recordsW.entries[counter].sample_range,
                          recordsW.entries[counter].sample_particulate, recordsW.entries[counter].sample_sound);
        counter++;

        //when there are PACKET_LEN records, pack them and program into a page
        if(counter==PACKET_LEN){
            //erase subsector from flash before writing
            if(write_pointer%SUBSECTOR_SIZE==0){
//                flash_mutex.lock();
                flash->SubSectorErase(write_pointer);
//                flash_mutex.unlock();
            }
//            flash_mutex.lock();
            //program into a page
            flash->ProgramFromAddress(recordsW.bytes, write_pointer, PAGE_SIZE);
//            flash_mutex.unlock();
            write_pointer = write_pointer + PAGE_SIZE;
            queue_size++;
            counter = 0;
        }

        //when the flash is fully flashed start from 0 again
        if(write_pointer==FLASH_SIZE)
            write_pointer = 0;

        Thread::wait(3000);
    }
}

//format sensor records into json before sending
char* format_data(int i){
        printf("formatting data ... \r\n");
        char* json_single = new char[256];
        sprintf(json_single, "{\"B\": %d, \"T\": %d, \"P\": %d, \"H\": %d, \"G\": %d, \"L\": %d, \"D\": %.4f, \"S\": %.3f, \"R\": %d}",
            BOX_ID, recordsR.entries[i].seconds, recordsR.entries[i].sample_ctemp,
            recordsR.entries[i].sample_humid, recordsR.entries[i].sample_gesture, recordsR.entries[i].sample_light, recordsR.entries[i].sample_particulate/10000.0,
            recordsR.entries[i].sample_sound/1000.0, recordsR.entries[i].sample_range);
        DBG("%s\n\r", json_single);
        return json_single;
}

 void phex(uint8_t* str)
{
    unsigned char i;
    for(i = 0; i < 16; ++i)
        printf("%.2x", str[i]);
    printf("\n");
}

/*Sensor Data Send Thread*/
void send_data(const char* path){

    // if we're connected make an HTTP request
    if(wifiConnected) {
//        flash_mutex.lock();
        //read a page of data from the flash
        flash->ReadDataFromAddress(recordsR.bytes,read_pointer,PAGE_SIZE);
//        flash_mutex.unlock();

        //format the page of data into a json object
        char* format_single[PACKET_LEN];
        for(int i=0; i<PACKET_LEN; i++){
            format_single[i] = format_data(i);
        }
        char* format_list = new char[RECORDSTR_LEN];
        uint8_t* cipher_list = new uint8_t[RECORDSTR_LEN];
        char* http_body = new char[HTTP_LEN];
        sprintf(format_list, "[%s,%s,%s,%s,%s,%s]", format_single[0], format_single[1], format_single[2],
                format_single[3], format_single[4], format_single[5]);

        //data encryption using AES with mode CBC
        AES128_CBC_encrypt_buffer(cipher_list, (uint8_t*)format_list, RECORDSTR_LEN, (uint8_t*)key, (uint8_t*)iv);

        //encode the encrypted data bytes using base64
        int length = base64_encode(http_body, (char *) cipher_list, RECORDSTR_LEN);

        DBG("posting: %d\r\n", strlen(http_body));
        rest.post(path, http_body);

        //release memory
        for(int i=0; i<PACKET_LEN; i++)
            delete format_single[i];
        delete[] format_list;
        delete[] cipher_list;
        delete[] http_body;

        //get post response from the server
        char response[BUFLEN];
        memset(response, 0, BUFLEN);
//        flash_mutex.lock();
        uint16_t code = rest.waitResponse(response, BUFLEN);
//        flash_mutex.unlock();
        if(code == HTTP_STATUS_OK){
            DBG("POST successful:\n\r");
            DBG("%s\n\r", response);
            //only move the read pointer to next page when post is successful
            read_pointer = read_pointer + PAGE_SIZE;
            queue_size--;
            //when the flash is fully read start from 0 again
            if(read_pointer==FLASH_SIZE)
                read_pointer = 0;
        } else {
            DBG("POST failed: ");
            DBG("%d\n\r", code);
//            reconnect_wifi(); //when server restarted, sensor box keeps getting post failures
        }
    }
}

/*Thread for checking whether privacy mode is triggered via gesture interruption*/
void check_gesture(){
    while(1){
        if(gesture_read!=-1 && gesture_flag==1){
            gesture_read = read_gesture();
            if(gesture_read==PRIVACY_MODE||gesture_read==(PRIVACY_MODE+1)){
                gesture_read = -1;
                LED_off();
            }
            setup_gesture();
            gesture_flag = 0;
//            gesture_INT.enable_irq();
        }
        if(gesture_read==-1 && gesture_flag==1){
            setup_gesture();
            gesture_flag = 0;
        }
        Thread::wait(10);
    }
}

/*Thread for getting sensor data that needs high frequency sampling*/
void get_highFrequencyData(){
    float temp_sound;
    float temp_range;
    while(1){
        if(gesture_read != -1){
            temp_sound = read_sound();
            sound_counter++;
            sound_read += temp_sound;
            sound_read_sq += temp_sound*temp_sound;
            temp_range = read_range();
            if(range_read>temp_range)
                range_read = temp_range;
        }
        Thread::wait(1);
    }
}

main(){

    DBG("\nStarting\n\r");

    flash = new N25Q();

    power_on();

    setup_range();

    setup_gesture();

    //attach the address of the detect_gesture function to the rising edge
    gesture_INT.fall(&detect_gesture);

    //set up wifi connection to the data server
    setup_wifi();

    //time setup is after wifi setup (sntp)
    setup_time();

    Thread thread1(get_allData);

    Thread thread2(get_highFrequencyData);

    Thread thread3(check_gesture, osPriorityHigh);

    while(1){
        //process any callbacks coming from esp_link
        esp.Process();
        //if wifi is connected and there is data packet in the queue
        if(wifiConnected && queue_size>0)
            send_data("/box-record");
        Thread::wait(2000);
    }
}
