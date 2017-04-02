#ifndef SENSORS_H
#define SENSORS_H
#include <string>

    void setup_gesture();
    
    void setup_range();
    
    int read_ctemp();
    
    int read_humid();

    int read_gesture();
      
    uint16_t read_light();       
    
    float read_particulate();
    
    float read_sound();
    
    float cal_sound(float value, float value_sq, float counter);
    
    float read_range();

#endif