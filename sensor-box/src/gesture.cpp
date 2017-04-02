#include "mbed.h"
#include "glibr.h"
#include "sensors.h"

#define PRV_CALIBRATION            23.60
#define PRV_SENS_UNIT              "uW/cm2"

//gesture sensor
glibr GSensor(P0_27, P0_28);

//values corresponding to different gestures
//int gesture;
int gesture = DIR_NONE;

void setup_gesture()
{
    GSensor.ginit();
    GSensor.enableGestureSensor(true);
    GSensor.enableLightSensor();
}

int read_gesture()
{
    switch (GSensor.readGesture()) {
        case DIR_LEFT:
            gesture = DIR_LEFT;
            break;
        case DIR_RIGHT:
            gesture = DIR_RIGHT;
            break;
        case DIR_UP:
            gesture = DIR_UP;
            break;
        case DIR_DOWN:
            gesture = DIR_DOWN;
            break;
        case DIR_NEAR:
            gesture = DIR_NEAR;
            break;
        case DIR_FAR:
            gesture = DIR_FAR;
            break;
        default:
            gesture = DIR_NONE;
    }
    return gesture;
}

uint16_t read_light()
{
    uint16_t light;
    if(GSensor.readAmbientLight(light))
        return light;
    else
        return 0;
}