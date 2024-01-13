#define GLOBAL
#include "globals.h"
#include "forwards.h"

void closing_state(void* arg) //activated by interrupt on Pin from Hal Sensor
{
    lqueuecmd lcmd;
    bool previousbreak;

    delay(NEWTASKDELAY);
    // printf("Closing heap %d\n",esp_get_free_heap_size());

if (!theConf.onoff)
{
        xTaskCreate(&offstate_state,"off",STATEHEAP,NULL, 5, NULL); 	//unknown state manager
        //cannot use deleteTask since it will be a local call to another sub and hence crash
        closingHandle=NULL;
        vTaskDelete(closingHandle);  //we are dead, unknown_state is managing the system
}

#ifdef DEBB
    if (theConf.traceTask)
        printf("\n%sClosing Task started\n",CLOSINGSTATE);
#endif

    writeLog((char*)"Door Closing cycle started");

#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
    u8g2_ClearBuffer(&u8g2);
    // ssdString(stx,sty,(char*)"Closing",true);
    ssdString(stx,sty,doorState[3],true);
    xSemaphoreGive(dispSem);
}
#endif

    DELTASK(blinker)
    launchBlink(BLINKCLOSING);

    esp_rmaker_param_update_and_report(status_param, esp_rmaker_str("Closing"));   

    if(!confirm_hal(NOCONTACT,NOCONTACT))               // if false that close sensor is NOCONTACT and Open sensor is NOCONTACT door is CLOSING
    {
#ifdef DEBB        
        if (theConf.traceBoot)
            printf("%sError Hal Sensors not Ok Closing State [CloseH %d OpenH %d]\n",CLOSINGSTATE, gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif        
        xTaskCreate(&unknown_state,"unknown",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager
        startStopTimer(STOP,timers[TCLOSE]);
        closingHandle=NULL;
        vTaskDelete(closingHandle);  //we are dead, unknown_state is managing the system
    }

    xQueueReset( lqueue);                   //empty queue
    previousbreak=false;
    startStopTimer(START,timers[TCLOSE]);

    while(true)                             //forever
    {
        if(xQueueReceive(lqueue, &lcmd, portMAX_DELAY))
        {
#ifdef DEBB        
         if (theConf.traceTask)
            printf("%sClosing Task loop [cmd %d]\n",CLOSINGSTATE,lcmd.cmd);
#endif            
            switch (lcmd.cmd)
            {
                case OPENHAL:   // Open Hal activated, door has opened by some means
                    if (confirm_hal(NOCONTACT,CONTACT))         //  door is open
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sClosing is now OPENED via break or reopen [CloseH %d OpenH %d]\n",CLOSINGSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        xTaskCreate(&opened_state,"open",STATEHEAP,NULL, 5, &openedHandle); 	//opening state manager will be in charge now
                        startStopTimer(STOP,timers[TCLOSE]);
                        closingHandle=NULL;
                        vTaskDelete(closingHandle);  //we are dead, Closing_state is managing the system
                    }
                    break;
                case CLOSETIMER:                                    // Close timer guard fired. Relay should be activated to close the door
                    if (confirm_hal(NOCONTACT,NOCONTACT))         // close hal sensor now false and open hal false, door closing
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sClosing Task Wait Timer fired [CloseH %d OpenH %d]\n",CLOSINGSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                       
                        relayPulse(RELAYTRIGGER);
                    }
                    break;
                case CLOSEHAL:
                    if (confirm_hal(CONTACT,NOCONTACT))         // door closed
                    {
#ifdef DEBB                        
                         if (theConf.traceCmdq)
                            printf("%sClosing Task Done. [CloseH %d OpenH %d]\n",CLOSINGSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        xTaskCreate(&closed_state,"closed",STATEHEAP,NULL, 5, &closedHandle); 	//opening state manager will be in charge now
                        startStopTimer(STOP,timers[TCLOSE]);
                        closingHandle=NULL;
                        vTaskDelete(closingHandle);  //we are dead, Closed_state is managing the system
                    }
                    break;
                case LASERBREAK:   // line break, something in the way. Reopend. But just once, not every break
                    if(theConf.guard)
                    {
                        printf("%sLaserbreak Prev %d laser %d pwmtimer %d\n",CLOSINGSTATE,previousbreak, lcmd.state, pwmtimer);
                        if (confirm_hal(NOCONTACT,NOCONTACT) && pwmtimer)         // door closing
                        {
    #ifdef DEBB
                            if (theConf.traceCmdq)
                                printf("%sBreak in Closing state %d pwmtimer %d cmd %d [CloseH %d OpenH %d]\n",CLOSINGSTATE,lcmd.state,lcmd.reserved,lcmd.cmd, gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
    #endif                       
                            // cmd logic must be saved somewhere to know that this was a Break Cmd vs a Open Cmd vs Remote Control
                            if(!previousbreak && lcmd.state)        //just once
                            {
                                relayPulse(RELAYTRIGGER);
                                previousbreak=true;
                                startStopTimer(STOP,timers[TCLOSE]);
                                #ifdef DISPLAY
                                if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
                                { 
                                    u8g2_ClearBuffer(&u8g2);
                                    ssdString(stx,sty,(char*)"Guard",true);
                                    ssdString(stx,sty,doorState[8],true);
                                    xSemaphoreGive(dispSem);
                                }
                                #endif

                            }
                        }
                    }
                    break;
                default:
                {
#ifdef DEBB                    
                    if (theConf.traceCmdq)
                        printf("%sClosing Task out of sync cmd %d\n",CLOSINGSTATE,lcmd.cmd);
#endif                        
                }
            }
        }
    }
}
