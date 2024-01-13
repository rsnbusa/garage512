
#define GLOBAL
#include "globals.h"
#include "forwards.h"


void closed_state(void* arg) //Initial State of Virutal Machine
{
    lqueuecmd lcmd;

    // printf("Closing heap %d\n",esp_get_free_heap_size());

    delay(NEWTASKDELAY);             //delay to allow previous task to die
    setIFR(OFF);                     // laser  off

if (!theConf.onoff)
{
        xTaskCreate(&offstate_state,"off",STATEHEAP,NULL, 5, NULL); 	//unknown state manager
        //cannot use deleteTask since it will be a local call to another sub and hence crash
        closedHandle=NULL;
        vTaskDelete(closedHandle);  //we are dead, unknown_state is managing the system
}

#ifdef DEBB
    if (theConf.traceTask)
        printf("\n%s Closed Task started\n",CLOSEDSTATE);
#endif

#ifdef DISPLAY
    if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
    {  
        u8g2_ClearBuffer(&u8g2);
        ssdString(stx,sty,doorState[0],true);
        xSemaphoreGive(dispSem);
    }
#endif

    writeLog((char*)"Door Closed cycle started");

    DELTASK(blinker);
    gpio_set_level((gpio_num_t)SYSLED, true);    //on
    turnLight(OFF);             // external LIGHT not the strip

    if(!powerHandle)
        xTaskCreate(&power_task,"poweron",STATEHEAP,NULL, 5, &powerHandle);           //start power cutoff task
    else
        setPower(gOnOff);                   //if rmainmaker cmd changed power state, bring it to working state

    xEventGroupClearBits(doorBits, KOPEN_BIT);         // a closed door reset the Open Door states
    xEventGroupClearBits(doorBits, CLOSEBREAK_BIT);  

    esp_rmaker_param_update_and_report(status_param, esp_rmaker_str("Closed"));   

    if(!confirm_hal(CONTACT,NOCONTACT))        // close hal is CONTACT and Open hal is NOCONTACT
    {
#ifdef DEBB
        printf("%sInconsistent Closed State [CloseH %d OpenH %d]\n\n",CLOSEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif

        xTaskCreate(&unknown_state,"unknown",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager
        //cannot use deleteTask since it will be a local call to another sub and hence crash
        closedHandle=NULL;
        vTaskDelete(closedHandle);  //we are dead, unknown_state is managing the system
    }
#ifdef DISPLAY
    startStopTimer(START,timers[TDISPLAY]);     // start display timer
#endif

    xQueueReset( lqueue);                       //clean queue

//reset max breaks for Opened state
countBlk=0;

// Preamble done, close state state machine loop begin

    while(true)                                 //wait for state machine change
    {  
        if (startTime>0 && ((millis()-startTime)/1000)>0)      // if we had a cycle(close-open-close), log time it took
        {            
            char *tmp=(char*)malloc(100);
            if(tmp)
            {
                sprintf(tmp,"%sComplete door cycle in %ld seconds",CLOSEDSTATE,(millis()-startTime)/1000);
                writeLog(tmp);
                startTime=0;
                free(tmp);
            }
        }

        if(xQueueReceive(lqueue, &lcmd, portMAX_DELAY)) //wait for a status change from the Queue
        {
#ifdef DEBB        
        if (theConf.traceTask)
            printf("%sClosed Task loop [cmd %d st %d]\n",CLOSEDSTATE,lcmd.cmd,lcmd.state);
#endif                  
            switch (lcmd.cmd)
            {   
                case CLOSEHAL:   
                    delay(theConf.waithal);                         //some additional debouncing
                    if (confirm_hal(NOCONTACT,NOCONTACT))           // close and open hal sensor now true, door opening
                    {
                        if (gOnOff==POWER_OFF)      // No power and door opened
                        {
                            // door opened manually, presumably, so send a warning
                            esp_rmaker_raise_alert("Door Manually Opened");

                        }
                        startTime=millis();
                        #ifdef DISPLAY
                        startStopTimer(STOP,timers[TDISPLAY]);      //timer now not valid
                        #endif
#ifdef DEBB                                            
                        if (theConf.traceCmdq)
                            printf("%sDoor is opening [CloseH %d OpenH %d]\n\n",CLOSEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif
                        xTaskCreate(&opening_state,"opening",STATEHEAP,NULL, 5, &openingHandle); 	//opening state manager will be in charge now
                        closedHandle=NULL;
                        vTaskDelete(closedHandle);  //we are dead. Opening_state managing system
                    }
                    else
                    {
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sClosed not confirm sensors HW error? %d %d\n",CLOSEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                            
                    }
                    break;
                    case OPENHAL:
                        delay(theConf.waithal);                         //some additional debouncing
                        if (confirm_hal(NOCONTACT,CONTACT))         // close hal sensor now false and open true. THis is HW error since we did not received the CLOSE Hal sensor going low, door opened, Live with it
                        {
                            #ifdef DISPLAY
                            startStopTimer(STOP,timers[TDISPLAY]);   
                            #endif
    #ifdef DEBB                        
                            if (theConf.traceCmdq)
                                printf("%sOpened Hal received CLOSED Task. HW error on Close Hal [CloseH %d OpenH %d]\n\n",CLOSEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
    #endif          
                            xTaskCreate(&opened_state,"opening",STATEHEAP,NULL, 5, &openedHandle); 	//opening state manager will be in charge now
                            closedHandle=NULL;
                            vTaskDelete(closedHandle);  //we are dead. Opening_state managing system
                        }
                    break;
                    case LASERBREAK:        // could be used as a backdoor entrance but not now
#ifdef DEBB                        
                    if (theConf.traceCmdq)
                        printf("%sGuard Break Closed\n",CLOSEDSTATE);
#endif         
                        break;
                    case TDISPLAY:        // could be used as a backdoor entrance but not now
#ifdef DEBB                        
                    if (theConf.traceCmdq)
                        printf("%sDisplay Time out\n",CLOSEDSTATE);
#endif         
#ifdef DISPLAY
                    u8g2_SetPowerSave(&u8g2, 1); // wake up display
#endif
                        break;
                    default:
                        {
#ifdef DEBB                            
                            if (theConf.traceCmdq)
                                printf("%sClosedTask out of sync cmd %d\n",CLOSEDSTATE,lcmd.cmd);
#endif                                
                        }
            }
        }
    }
}