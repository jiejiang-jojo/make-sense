#include "mbed.h"
#include "rtos/rtos.h"
#include "sensors.h"
#include <stdlib.h>
#include <cmath>

//microphone sensor
AnalogIn mic(P0_23);

float read_sound()
{
    //sound pressure value
    return mic.read()*3.3;
}

float cal_sound(float value, float value_sq, float counter)
{
    //decibels values of sound
    float db_sound;

    db_sound = abs(sqrt(value_sq/counter) - value/counter);
    db_sound = 20 * log10(db_sound) - 36.478;
    db_sound = db_sound + 136;
    return db_sound;
}
