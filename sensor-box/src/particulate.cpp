#include "mbed.h"
#include "sensors.h"

//particulate sensor
DigitalOut ledPower(P0_24);
AnalogIn analog_value(P0_25);

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float dustDensityCN = 0, dustDensitySharp = 0, voMeasured = 0, voCalc = 0;

float read_particulate()
{
    ledPower = 0; //power on the LED. Pull-down to activate
    wait_us(samplingTime);
    voMeasured = analog_value.read();
    wait_us(deltaTime);
    ledPower = 1; //turn the LED off. Pull up to turn off
    wait_us(sleepTime);
    voCalc = voMeasured*3.3;
    dustDensitySharp = 0.5/2.8*(float(voCalc)-0.7);
    dustDensityCN = (float(voCalc) - 0.0356)*1.2;

    if(dustDensitySharp<0)
        return 0;
    else
        return dustDensitySharp;
}