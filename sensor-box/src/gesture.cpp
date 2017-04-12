#include "mbed.h"
#include "glibr.h"
#include "sensors.h"
#include "util.h"

#define PRV_CALIBRATION            23.60
#define PRV_SENS_UNIT              "uW/cm2"

//gesture sensor
glibr GSensor(P0_27, P0_28);

//values corresponding to different gestures
//int gesture;

void setup_gesture()
{
  GSensor.ginit();
  GSensor.setGestureLEDDrive(LED_DRIVE_12_5MA);
  GSensor.enableGestureSensor(true);
  GSensor.enableLightSensor();
}

bool gesture_available(){
  return GSensor.isGestureAvailable();
}

int read_gesture()
{
    int gesture = GSensor.readGesture();
    DBG("guesture ------ %d", gesture);
    GSensor.disableGestureSensor();
    return gesture;
}

int read_light()
{
    uint16_t light;
    if(GSensor.readAmbientLight(light)){
      GSensor.enableGestureSensor(true);
      return light;
    }
    else{
      GSensor.enableGestureSensor(true);
      return -1;
    }
}
