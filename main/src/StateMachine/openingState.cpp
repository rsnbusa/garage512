
#define GLOBAL
#include "globals.h"
#include "forwards.h"


void opening_state(void* arg) //Opening State Task
{
    lqueuecmd lcmd;
        // printf("Opening heap %d\n",esp_get_free_heap_size());

    if (!theConf.onoff)
{
        xTaskCreate(&offstate_state,"off",STATEHEAP,NULL, 5, NULL); 	//unknown state manager
        //cannot use deleteTask since it will be a local call to another sub and hence crash
        openingHandle=NULL;
        vTaskDelete(openingHandle);  //we are dead, unknown_state is managing the system
}

    delay(NEWTASKDELAY);             //delay to allow previous task to die

    countBreak=0;
#ifdef DEBB
    if (theConf.traceTask)
        printf("\n%sOpening Task started\n",OPENINGSTATE);
#endif    
    writeLog((char*)"Door Opening cycle started");


#ifdef DISPLAY
    if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
    {   
        u8g2_ClearBuffer(&u8g2);
        delay(10);
        ssdString(stx,sty,doorState[1],true);
        u8g2_SetPowerSave(&u8g2, 0); // wake up display
        xSemaphoreGive(dispSem);
    }
#endif

    DELTASK(blinker)
    launchBlink(BLINKOPENING);

    turnLight(ON);                      //external light 

    esp_rmaker_param_update_and_report(status_param, esp_rmaker_str("Opening"));   

     if(!confirm_hal(NOCONTACT,NOCONTACT))       // both sensors should be off, else HW error
    {
#ifdef DEBB        
        printf("%sError Hal Sensors not Ok Opening State [CloseH %d OpenH %d]\n",OPENINGSTATE, gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif 
        xTaskCreate(&unknown_state,"unknown",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager
        openingHandle=NULL;
        vTaskDelete(openingHandle);           //we are dead, unknown_state is managing the system
    }

    startStopTimer(START,timers[TOPEN]);    // start the Opening Timer

    xQueueReset( lqueue);                   // empty queue
    setIFR(ON);                             // ifr on, ready to detect break

  while(true)                             //forever, so state machine will launch whatever task the new state requires
    {     
        if(xQueueReceive(lqueue, &lcmd, portMAX_DELAY))
        {
#ifdef DEBB        
         if (theConf.traceTask)
            printf("%sOpening Task loop [cmd %d]\n",OPENINGSTATE,lcmd.cmd);
#endif                   
            switch (lcmd.cmd)
            {
                case OPENHAL:                                       // Open Sensor activated, Door is OPEN
                    if (confirm_hal(NOCONTACT,CONTACT))             // close hal sensor now false and open hal true, door opened
                    { 
                        startStopTimer(STOP,timers[TOPEN]);         //open timer stop
                        startStopTimer(STOP,timers[TREPEAT]);       //repeat timer stop which was /couldhave been activated internally in this loop 
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sDoor is Opened [CloseH %d OpenH %d]\n",OPENINGSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif 
                        xTaskCreate(&opened_state,"opening",STATEHEAP,NULL, 5, &openedHandle); 	//OPENED state manager will be in charge now
                        openingHandle=NULL;
                        vTaskDelete(openingHandle);                  //we are dead, Opened_state is managing the system
                    }
                    break;
                case OPENINGTIMER:                                  // Opening Timer Fired. Close door and fired repeattimer 
                    if (confirm_hal(NOCONTACT,NOCONTACT))           //  door opening
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sOpening Timer fired opening state [CloseH %d OpenH %d]\n",OPENINGSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        relayPulse(RELAYTRIGGER);                     //activate relay
                        xTimerStart(timers[TREPEAT],0);               // timers[TREPEAT] started no delay. To avoid repeating forever. Go to unknown state if repeated
                    }
                    break;
                case CLOSEHAL:
                    if (confirm_hal(CONTACT,NOCONTACT))               // close hal sensor now true and open hal false, door closed
                    { 
                        startStopTimer(STOP,timers[TOPEN]);
                        startStopTimer(STOP,timers[TREPEAT]);
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sDoor was closed by Opening State timeout or Remote [CloseH %d OpenH %d]\n",OPENINGSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        xTaskCreate(&closed_state,"closed",STATEHEAP,NULL, 5, &closedHandle); 	//opening state manager will be in charge now
                        openingHandle=NULL;
                        vTaskDelete(openingHandle);                     //we are dead, Closed_state is managing the system
                    }
                    break;
                case LASERBREAK:    //nbot used now but could be usefull later like detecting a car/body broke the guard
#ifdef DEBB                        
                    if (theConf.traceCmdq)
                        printf("%sGuard Break Openning state %d\n",OPENINGSTATE,lcmd.state);
#endif     
                        // writeLog((char*)"IFR Break opening");
                        xEventGroupSetBits(doorBits, OPENBREAK_BIT);        //set Break Bit ocurred. Can happend many times
                        countBreak++;
                    break;
                case REPEATTIMER:
                     if(confirm_hal(NOCONTACT,NOCONTACT))               // door still has sensors without contact hence Limbo
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sRepeat Timer Closed Fired [CloseH %d OpenH %d]\n",OPENINGSTATE, gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                       
                        xTaskCreate(&unknown_state,"unknown",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager because we already tried to recover now this is fatal, DO NOT use relay againb it will overheat the motor
                        openingHandle=NULL;
                        vTaskDelete(openingHandle);                     //we are dead, unknown_state is managing the system
                    }
                    break;
                default:
                {
#ifdef DEBB                    
                    printf("%sOpeningTask out of sync cmd %d\n",OPENINGSTATE,lcmd.cmd);
#endif                    
                }
            }
        }
    }

}
