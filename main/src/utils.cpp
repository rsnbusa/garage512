
#define GLOBAL
#include "globals.h"
#include "forwards.h"


void delay(uint32_t cuanto)
{
    vTaskDelay(cuanto / portTICK_PERIOD_MS);
}

uint32_t millis()
{
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

uint32_t millisFromISR()
{
	return xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
}

void coolMgr(TimerHandle_t xTimer)      //used for cooling phase
{
    loopcount=millis();
    esp_timer_start_once(pwm_timer, theConf.timers[TPWMOFF]);   //start off timer
}

void startStopTimer(bool action, TimerHandle_t timer)
{
    if(timer)
    {
#ifdef DEBB
        uint32_t cual=(uint32_t) pvTimerGetTimerID(timer);

        if(theConf.traceTimers)
            printf("%s%s Timer[%ld]=[%s]\n",SYSTEM,action?"Start":"Stop",cual,timer_names[cual]);
#endif

        if(action)  //start
        {
            if (xTimerIsTimerActive(timer))
                xTimerStop(timer,NODELAY);  //reset it first
            xTimerStart(timer,NODELAY);
        }
        else //STOP
        {
            if (xTimerIsTimerActive(timer))
                xTimerStop(timer,NODELAY);
        }
    }
    else
    {
#ifdef DEBB
        if(theConf.traceTimers)
            printf("%sStartStop invalid timer\n",SYSTEM);
#endif

    }
}

void  setPower(uint8_t how)
{
    gpio_set_level((gpio_num_t)PWRPIN,how);
}


#ifdef DISPLAY
void ssdString(int x, int y, char * que,bool centerf)
{
    u8g2_DrawFrame(&u8g2,0,0,127,63);
    if (!centerf)
        u8g2_DrawStr(&u8g2, x,y,que);
    else
    {
        int  w = u8g2_GetStrWidth(&u8g2,que);
        int h = u8g2_GetMaxCharHeight(&u8g2);
        int sw=u8g2_GetDisplayWidth(&u8g2);
        int lstx=(sw-w)/2;
            if (lstx<0)
                lstx=0;
        int lsty=(u8g2_GetDisplayHeight(&u8g2)+h/2)/2;
        u8g2_DrawStr(&u8g2, lstx,lsty,que);
    }
	u8g2_SendBuffer(&u8g2);
    
}
#endif

bool confirm_hal(bool closeh, bool openh)
{
    if(gpio_get_level((gpio_num_t)HALCL)!=closeh)
        return false;
    if(gpio_get_level((gpio_num_t)HALOP)!=openh)
        return false;
    return true;
}


void relayPulse(uint32_t cuanto)
{
#ifdef DEBB
    if(theConf.traceBoot)
        printf("%sRelay pulse %ld\n",SYSTEM,cuanto);
#endif

#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
    u8g2_ClearBuffer(&u8g2);
    // ssdString(stx,sty,(char*)"Relay",true);
    ssdString(stx,sty,doorState[7],true);
    xSemaphoreGive(dispSem);
}
#endif

    gpio_set_level((gpio_num_t)RELAY, true);    // on
    delay(cuanto);                              // seconds delay
    gpio_set_level((gpio_num_t)RELAY, false);   // off

#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
    u8g2_ClearBuffer(&u8g2);
    xSemaphoreGive(dispSem);
}
#endif

}