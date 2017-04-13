#include "mbed.h"
#include "rtos.h"
#include "sensors.h"
#include "config.h"

//particulate sensor
DigitalOut ledPower(P0_24);
AnalogIn analog_value(P0_25);

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float read_particulate()
{
  float dustDensityCN = 0, dustDensitySharp = 0, voMeasured = 0, voCalc = 0;
  for(int i=0; i<PARTICULATE_SAMPLE_SIZE; i++){
    ledPower = 0; //power on the LED. Pull-down to activate
    wait_us(samplingTime);
    voMeasured = analog_value.read();
    Thread::wait(deltaTime/1000.0);
    ledPower = 1; //turn the LED off. Pull up to turn off
    Thread::wait(sleepTime/1000.0);
    voCalc += voMeasured*3.3;
  }
  return voCalc / PARTICULATE_SAMPLE_SIZE;
}
