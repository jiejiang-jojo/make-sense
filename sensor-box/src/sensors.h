#ifndef SENSORS_H
#define SENSORS_H
#include <string>
#include <stdint.h>


void setup_gesture();

bool gesture_available();

void setup_range();

int read_ctemp();

int read_humid();

int read_gesture();

int read_light();

float read_particulate();

float read_sound();

float cal_sound(float value, float value_sq, float counter);

int read_range();

int read_bluetooth_signal(int device);

void bluetooth_scan_loop();

void bleinit();

#endif
