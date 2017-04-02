#include "mbed.h"
#include "rtos.h"
#include "sensors.h"
#include <stdlib.h>
#include <cmath>  

//microphone sensor
AnalogIn mic(P0_23);

//sound pressure value
float measure_sound;

//decibels values of sound
float db_sound;

float read_sound()
{
    measure_sound = mic.read()*3.3;
    return measure_sound;
}

float cal_sound(float value, float value_sq, float counter)
{
    db_sound = abs(sqrt(value_sq/counter) - value/counter);
    db_sound = 20 * log10(db_sound) - 36.478;
    db_sound = db_sound + 136;
    return db_sound;
}
