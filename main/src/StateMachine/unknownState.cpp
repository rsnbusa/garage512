
#define GLOBAL
#include "globals.h"
#include "forwards.h"


void unknown_state(void* pArg) //activated by interrupt on Pin from Hal Sensor
{

    lqueuecmd lcmd;

    delay(NEWTASKDELAY);

#ifdef DEBB
    if (theConf.traceTask)
        printf("\n%sUnknown Task started\n",OFFLINESTATE);
#endif

#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
    char tmp[30];
    sprintf(tmp,"UNK %2d-%2d",gpio_get_level((gpio_num_t)HALCL),gpio_get_level((gpio_num_t)HALOP));
    u8g2_ClearBuffer(&u8g2);
    ssdString(stx,sty,tmp,true); 
    // ssdString(stx,sty,doorState[5],true); 
    xSemaphoreGive(dispSem);
}
#endif
 
    esp_rmaker_param_update_and_report(status_param, esp_rmaker_str("Unknown"));   
    DELTASK(blinker);
    setIFR(OFF);                            // set laser off if active
    // setPower(POWER_ON);                     // Power ON due to our HW failure

    while(true)                             //forever check events queue
    {
        if(xQueueReceive(lqueue, &lcmd, portMAX_DELAY))
        {
#ifdef DEBB            
            if (theConf.traceCmdq)
                printf("%sUnknown Cmd [%d] State [%d] [CloseH %d OpenH %d]\n",OFFLINESTATE,lcmd.cmd,lcmd.state,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif            
            switch (lcmd.cmd)
            {
                case OPENHAL:   // Open Hal activated, Door was opened
                    delay(theConf.waithal);                     //debouncing
                    if (confirm_hal(NOCONTACT,CONTACT))         //  door is open
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sUNKNOWN TASK is now OPEN [CloseH %d OpenH %d]\n",OFFLINESTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        setIFR(ON);                            // set Guard acive irrelevant of being used or not by Opened task
                        xTaskCreate(&opened_state,"open",STATEHEAP,NULL, 5, &openedHandle); 	//opening state manager will be in charge now
                        if (!xTimerIsTimerActive( timers[TCLOSE]))  // Start close timer
                            xTimerStart(timers[TCLOSE],0);               // start Close Timer started no delay
                        unknownHandle=NULL;
                        vTaskDelete(unknownHandle);  //we are dead, Closing_state is managing the system
                    }
                    else
                    {
#ifdef DEBB                        
                        printf("%sUnknown OPENHAL sensors do not match [CloseH %d OpenH %d]\n",OFFLINESTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                    
                    }
                    break;
                case CLOSEHAL:  // door was closed 
                    delay(theConf.waithal);
                    
                    if (confirm_hal(CONTACT,NOCONTACT))         //  door closed
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sUNKNOWN TASK is now CLOSED [CloseH %d OpenH %d]\n",OFFLINESTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                       
                        xTaskCreate(&closed_state,"closed",STATEHEAP,NULL, 5, &closedHandle); 	//opening state manager will be in charge now
                        if (xTimerIsTimerActive( timers[TCLOSE]))  
                            xTimerStop(timers[TCLOSE],0);
                        unknownHandle=NULL;
                        vTaskDelete(unknownHandle);  //we are dead, Closed_state is managing the system
                    }
                    else
                    {
#ifdef DEBB                        
                        printf("%sUnknown CLOSEHAL sensors do not match [CloseH %d OpenH %d]\n",OFFLINESTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                    
                    }
                    break;
                default:
                {
#ifdef DEBB                    
                    if (theConf.traceCmdq)
                        printf("%sUnknown Task out of sync cmd %d\n",OFFLINESTATE,lcmd.cmd);
#endif                        
                }
            }
        }
    }
}
