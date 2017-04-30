#include "mbed.h"
#include "sensors.h"

AnalogIn range(P1_31);
DigitalOut trigPin(P1_30);

float voltage = 0;
int cm = 0;

void setup_range()
{
    trigPin = 1;
}

int read_range()
{
        voltage = range.read();
        cm = (int) 5.99615*pow(voltage, -1.206609f);
        if(cm>150)
            return 150; //out of detecting range.
        else
            return cm;
}
