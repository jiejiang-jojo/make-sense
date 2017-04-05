#include "led.h"
/*turn on the blue LED to indicate that the sensor box is powered*/
void LED::power_on(){
    _blueLed = 1;
    _greenLed = 0;
    _redLed = 0;
}

/*turn on the red LED to indicate that the sensor box is in privacy mode*/
void LED::privacy_on(){
    _blueLed = 0;
    _greenLed = 0;
    _redLed = 1;
}

/*turn on the green LED to indicate that the sensor box is connected to wifi*/
void LED::wifi_on(){
    _blueLed = 0;
    _greenLed = 1;
    _redLed = 0;
}

/*turn on the red LED to indicate that wifi is lost*/
void LED::wifi_off(){
    _blueLed = 0;
    _greenLed = 0;
    _redLed = 1;
}

/*turn off the green LED to indicate that the sensor box is off*/
void LED::off(){
    _blueLed = 0;
    _greenLed = 0;
    _redLed = 0;
}
