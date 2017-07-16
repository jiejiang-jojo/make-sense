#include "mbed.h"
#include "sensors.h"

AnalogIn range(P1_31);
DigitalOut trigPin(P1_30);

float voltage = 0;
int cm = 0;

void setup_range()
{
    trigPin = 1;
    wait_ms(26);
}

int read_range()
{
        // voltage = range.read() + 1E-5; //add a very small number to prevent zeros
        // cm = (int) 5.99615*pow(voltage, -1.206609f);
        // if(cm>150)
        //     return 150; //out of detecting range.
        // else
        //     return cm;

        float retval = 0;
        float calibration = 0;
        float adc_value = range.read();
        adc_value *= 3.3;

      	adc_value += calibration;

      	if (adc_value >= 1.9){
      		retval = 10;
      	}
        if ((adc_value < 1.9) && (adc_value >= 1.1)){
        	retval = 20 + ((adc_value - 1.1) / (1.9-1.1) * (10-20));
        }
        if ((adc_value < 1.1) && (adc_value >= 0.75)){
        	retval = 30 + ((adc_value - 0.75) / (1.1-0.75) * (20-30));
        }
        if ((adc_value < 0.75) && (adc_value >= 0.6)){
        	retval = 50 + ((adc_value - 0.6) / (0.75-0.6) * (30-50));
        }
        if ((adc_value < 0.6) && (adc_value >= 0.4)){
        	retval = 100 + ((adc_value - 0.4) / (0.6-0.4) * (50-100));
        }
        if ((adc_value < 0.4) && (adc_value > 0.3)){
        	retval = 150 + ((adc_value - 0.3) / (0.4-0.3) * (100-150));
        }
        if (adc_value <= 0.3){
        	retval = 150;
        }
        return (int)retval;
}
