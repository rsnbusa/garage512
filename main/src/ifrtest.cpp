#define GLOBAL
#include "globals.h"
#include "forwards.h"


void ifrTest(void *pArg)
{
   
lqueuecmd lcmd;

    setIFR(ON);
    while(true)
    {
         if(xQueueReceive(lqueue, &lcmd, portMAX_DELAY))
        {
            switch (lcmd.cmd)
            {
                case LASERBREAK:
#ifdef DISPLAY
                if(xSemaphoreTake(dispSem, portMAX_DELAY/ portTICK_PERIOD_MS ))		
                { 
                    u8g2_ClearBuffer(&u8g2);
                    ssdString(stx,sty,(char*)"ABREAK",true);
                    xSemaphoreGive(dispSem);
                }
                delay(3000);
                if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
                { 
                    u8g2_ClearBuffer(&u8g2);
                    ssdString(stx,sty,(char*)"ALIGN",true);
                    xSemaphoreGive(dispSem);
                }           
#endif
            }

        }
        delay(1000);
    }
}
