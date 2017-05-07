/**
 * This is the main class for the sensor box to collect data from different sensor modules.
 */

#include <mbed.h>
#include <rtos.h>
#include <string.h>
#include <stdint.h>
#include <queue>
#include <ELClient.h>
#include <ELClientRest.h>
#include <ELClientCmd.h>
#include "IAP.h"

#include "n25q.h"
#include "aes.h"
#include "base64.h"
#include "led.h"
#include "wifi.h"
#include "glibr.h"
#include "sensors.h"

#include "util.h"
#include "config.h"

//Serial port to PC debugging
Serial serial(USBTX, USBRX);
char device_id_str[20] = {0};

//LED lights
LED led(P2_5, P2_4, P2_3);

BoxState box_state(led);

Wifi wifi(P4_28, P4_29, box_state);

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
int sound_counter = 0;
int range_read = 150;
int light_read = 0;

// 128bit key
char key[] = AES_KEY;
//128bit initialization vector
char iv[]  = AES_IV;

//different kinds of sensor module data
//corresponds to one page
struct entry_t {
  int seconds;
  int ctemp;
  int humid;
  int gesture;
  int light;
  int range;
  int sound;
  int dust;
  int bluetooth_0;
  int bluetooth_1;
  int bluetooth_2;
  int bluetooth_3;
  int bluetooth_4;
};
union sensor_data_t {
  entry_t entries[PACKET_LEN];
  uint8_t bytes[256];
};
union sensor_data_t recordsW, recordsR;

//flash write and read pointers
int write_pointer = 0;
int read_pointer = 0;

//size of written but not read records
int queue_size = 0;

//Initialize a client to read and write data from N25Q
N25Q flash(P0_9, P0_8, P0_7, P0_6);

//Initialize a connection to esp-link using the normal hardware serial port
//both for SLIP and for debug messages

/*gesture interruption*/
void detect_gesture() {
    gesture_flag = 1;
    gesture_counter++;
}

/*read sensor data from all sensor modules*/
void read_sensors(int num){
    // BoxID will be appended when the records are sent to server
    recordsW.entries[num].seconds = time(NULL);
    recordsW.entries[num].ctemp = read_ctemp();
    recordsW.entries[num].humid = read_humid();

    int try_light_read = read_light();
    if (try_light_read > 0)
      recordsW.entries[num].light = try_light_read;
    else
      recordsW.entries[num].light = light_read;
    light_read = -1;

    //try to use integer to represent float.
    recordsW.entries[num].dust = (int) (read_particulate()*10000);

    recordsW.entries[num].sound = (int) (cal_sound(sound_read, sound_read_sq, sound_counter)*1000);
    sound_read = 0;
    sound_read_sq = 0;
    sound_counter = 0;

    recordsW.entries[num].range = range_read;
    range_read = 150;

    recordsW.entries[num].bluetooth_0 = (int) read_bluetooth_signal(0);
    recordsW.entries[num].bluetooth_1 = (int) read_bluetooth_signal(1);
    recordsW.entries[num].bluetooth_2 = (int) read_bluetooth_signal(2);
    recordsW.entries[num].bluetooth_3 = (int) read_bluetooth_signal(3);
    recordsW.entries[num].bluetooth_4 = (int) read_bluetooth_signal(4);


    recordsW.entries[num].gesture = gesture_read;
    gesture_counter = 0;
}

void clear_flash_subsector(){
  flash_mutex.lock();
  flash.SubSectorErase(write_pointer);
  flash_mutex.unlock();
}

void write_rec_packet(){
  //program into a page
  flash_mutex.lock();
  flash.ProgramFromAddress(recordsW.bytes, write_pointer, N25Q::PAGE_SIZE);
  flash_mutex.unlock();
}

void push_to_flash(){
  //erase subsector from flash before writing
  if(write_pointer % N25Q::SUBSECTOR_SIZE==0){
    clear_flash_subsector();
  }
  write_rec_packet();
  //when the flash is fully flashed start from 0 again
  write_pointer = (write_pointer + N25Q::PAGE_SIZE) % N25Q::FLASH_SIZE;
  queue_size++;
}

/*Sensor Data Get Thread*/
void get_allData(){
    int counter = 0;
    while(1){
        if(box_state.IsPrivacyOn()) {
            // stop collecting data for some period
            wait(PRIVACY_PERIOD);
            box_state.PrivacyOff();
        }
        DBG("SndCnt:%d GstCnt:%d\n", sound_counter, gesture_counter);
        read_sensors(counter);
        DBG("Reading(%d)[%d]: %4dT %4dH %4dL %4dR %4dD %4dS %4d %4d %4d %4d %4d\n",
          write_pointer,
          recordsW.entries[counter].seconds,
          recordsW.entries[counter].ctemp,
          recordsW.entries[counter].humid,
          recordsW.entries[counter].light,
          recordsW.entries[counter].range,
          recordsW.entries[counter].dust,
          recordsW.entries[counter].sound,
          recordsW.entries[counter].bluetooth_0,
          recordsW.entries[counter].bluetooth_1,
          recordsW.entries[counter].bluetooth_2,
          recordsW.entries[counter].bluetooth_3,
          recordsW.entries[counter].bluetooth_4
        );
        counter++;

        //when there are PACKET_LEN records, pack them and program into a page
        if(counter==PACKET_LEN){
          push_to_flash();
          counter = 0;
        }


        Thread::wait(SAMPLE_RATE * 1000);
    }
}

//format sensor records into json before sending
char* format_data(int i, char* buf){
sprintf(buf,"{\"B\":%d,\"Q\":\"%s\",\"T\":%d,\"P\":%d,\"H\":%d,\"G\":%d,\"L\":%d,\"D\":%.4f,\"S\":%.3f,\"R\":%d,\"M0\":%d,\"M1\":%d,\"M2\":%d,\"M3\":%d,\"M4\":%d}",
            BOX_ID,
            device_id_str,
            recordsR.entries[i].seconds,
            recordsR.entries[i].ctemp,
            recordsR.entries[i].humid,
            recordsR.entries[i].gesture,
            recordsR.entries[i].light,
            recordsR.entries[i].dust/10000.0,
            recordsR.entries[i].sound/1000.0,
            recordsR.entries[i].range,
            recordsR.entries[i].bluetooth_0,
            recordsR.entries[i].bluetooth_1,
            recordsR.entries[i].bluetooth_2,
            recordsR.entries[i].bluetooth_3,
            recordsR.entries[i].bluetooth_4
          );
        // DBG("Json: %s\n", buf);
        return buf;
}

const char *
packet_format(char * buf, char jsons[PACKET_LEN][256], int n) {
  buf[0] = 0;
  strcat(buf, "[");
  strcat(buf, jsons[0]);

  for (int i=1; i<n; i++){
    strcat(buf, ",");
    strcat(buf, jsons[i]);
  }
  strcat(buf, "]");
  return buf;
}


/*Sensor Data Send Thread*/
void send_data(const char* path){


    //read a page of data from the flash
    flash_mutex.lock();
    flash.ReadDataFromAddress(recordsR.bytes,read_pointer,N25Q::PAGE_SIZE);
    flash_mutex.unlock();

    //format the page of data into a json object
    static char format_single[PACKET_LEN][256];
    for(int i=0; i<PACKET_LEN; i++){
      format_data(i, format_single[i]);
    }
    static char format_list[RECORDSTR_LEN];
    packet_format(format_list, format_single, PACKET_LEN);
    // DBG("Packet: %s\n", format_list);

    //data encryption using AES with mode CBC
    static uint8_t cipher_list[RECORDSTR_LEN];
    AES128_CBC_encrypt_buffer(cipher_list, (uint8_t*)format_list, RECORDSTR_LEN, (uint8_t*)key, (uint8_t*)iv);

    //encode the encrypted data bytes using base64
    static char http_body[HTTP_LEN];
    base64_encode(http_body, (char *) cipher_list, RECORDSTR_LEN);

    static char response[BUFLEN];
    memset(response, 0, BUFLEN);
    DBG("Post len=%d ID=%d REMOTE=%s\n", strlen(http_body), BOX_ID, SERVER_IP);
    wifi.rest.post(path, http_body);
    //get post response from the server
    uint16_t code = wifi.rest.waitResponse(response, BUFLEN);
    if(code == HTTP_STATUS_OK){
        DBG("POST: successful -> %s\n", response);
        //only move the read pointer to next page when post is successful
        read_pointer = read_pointer + N25Q::PAGE_SIZE;
        queue_size--;
        //when the flash is fully read start from 0 again
        if(read_pointer==N25Q::FLASH_SIZE)
            read_pointer = 0;
    } else {
        DBG("POST: failed -> %d\n", code);
//            reconnect_wifi(); //when server restarted, sensor box keeps getting post failures
    }
}

/*Thread for checking whether privacy mode is triggered via gesture interruption*/
void check_gesture(){
    while(true){
      if(gesture_flag==1 && gesture_available() ){
        gesture_INT.disable_irq();
        gesture_read = read_gesture();
        light_read = read_light();
        if(!box_state.IsPrivacyOn()){
            if(gesture_read==DIR_UP||gesture_read==DIR_DOWN){
                box_state.PrivacyOn();
            }
        }
        gesture_flag = 0;
        gesture_INT.enable_irq();
      } else
        Thread::wait(GESTURE_SAMPLE_RATE * 1000);
    }
}

/*Thread for getting sensor data that needs high frequency sampling*/
void get_highFrequencyData(){
    float temp_sound;
    int temp_range;
    while(true){
        if(!box_state.IsPrivacyOn()){
            temp_range = read_range();
            temp_sound = read_sound();
            sound_counter++;
            sound_read += temp_sound;
            sound_read_sq += temp_sound*temp_sound;
            if(range_read>temp_range)
                range_read = temp_range;
        }
        Thread::wait(HIGHFREQ_SAMPLE_RATE * 1000);
    }
}

void get_serial_number(){
    IAP iap;
    int *block = iap.read_serial();
    uint32_t device_id[2] = {0};
    device_id[0] = *block;
    device_id[1] = *(block + 1);
    sprintf(device_id_str, "%02X%02X", device_id[0], device_id[1]);
}

int main(void){

    serial.baud(DBG_SERIAL_BAUD);
    DBG("Started!");

    get_serial_number();
    DBG("Device ID: %s\n", device_id_str);

    setup_range();
    setup_gesture();
    //attach the address of the detect_gesture function to the rising edge
    gesture_INT.fall(&detect_gesture);
    //time setup is after wifi setup (sntp)
    wifi.Setup();

    Thread thread[4];
    thread[0].start(get_allData);
    thread[1].start(get_highFrequencyData);
    thread[2].start(check_gesture);
    thread[3].start(bluetooth_scan_loop);

    Timer t_clock_resync;
    t_clock_resync.start();
    while(1){
        //process any callbacks coming from esp_link
        wifi.Process();
        //if wifi is connected and there is data packet in the queue
        if (!box_state.IsWifiOn()) {
          DBG("no wifi on!");
          wifi.Sync();
        }
        else if(queue_size>0)
            send_data("/box-record");

        if (t_clock_resync.read_ms() < CLOCK_RESYNC_CYCLE * 1000)
          Thread::wait(POST_RATE * 1000);
        else{
          wifi.SyncClock();
          t_clock_resync.reset();
        }
    }
}
