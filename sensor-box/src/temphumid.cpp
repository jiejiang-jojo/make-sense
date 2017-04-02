#include "mbed.h"
#include "HTU21D.h"
#include "sensors.h"

//temperature and humidity sensor
HTU21D temphumid(P0_27, P0_28);

int ctemp;
int humid;

int read_ctemp()
{
    ctemp = temphumid.sample_ctemp();
    
    return ctemp;
}

int read_humid()
{
    humid = temphumid.sample_humid();
    
    return humid;
}