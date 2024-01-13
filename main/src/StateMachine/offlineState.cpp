#define GLOBAL
#include "globals.h"
#include "forwards.h"


void offstate_state(void* arg) //activated by interrupt on Pin from Hal Sensor
{

#ifdef DEBB
    if (theConf.traceTask)
        printf("\n%sONOFF Task started\n",ONOFFSTATE);
#endif

#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
    u8g2_ClearBuffer(&u8g2);
    ssdString(stx,sty,doorState[4],true); 
    xSemaphoreGive(dispSem);
}
#endif
    while(true)
    {
        delay(10000);
        {
            if(theConf.onoff)
            {
                esp_restart();

            }
        }
    }
}
