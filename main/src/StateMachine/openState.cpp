#define GLOBAL
#include "globals.h"
#include "forwards.h"

void opened_state(void* arg) 
{

    lqueuecmd           lcmd;
    bool                breakin=false;
    int8_t              ifrSensor;
    char                opencase[3][10]={"Open","KOpen","OpenBr",};
    char                tmp[20];

if (!theConf.onoff)
{
        xTaskCreate(&offstate_state,"off",STATEHEAP,NULL, 5, NULL); 	//ONOFF state manager
        openedHandle=NULL;
        vTaskDelete(openedHandle);  
}

    delay(NEWTASKDELAY);                                        //alow time for preivous task to die

    strcpy(tmp,opencase[0]);                                    //by default its a normal Open

    EventBits_t uxBits =xEventGroupGetBits(doorBits);           //get the door bits to see which cmd received Open/Break/Keep cmd

//for rainmaker status display
    if( ( uxBits & ( CLOSEBREAK_BIT) ) == ( CLOSEBREAK_BIT ) ) 
        strcpy(tmp,opencase[2]);
    if( ( uxBits & ( KOPEN_BIT) ) == ( KOPEN_BIT ) ) 
        strcpy(tmp,opencase[1]);

    countBreak=0;
#ifdef DEBB    
    if (theConf.traceTask)
        printf("\n%sOpened Task started %ld\n",OPENEDSTATE,uxBits);
#endif
#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
    u8g2_ClearBuffer(&u8g2);
    ssdString(stx,sty,tmp,true);
    xSemaphoreGive(dispSem);
}
#endif

    DELTASK(blinker);
    launchBlink(BLINKOPEN);             //pcb led

    esp_rmaker_param_update_and_report(status_param, esp_rmaker_str(tmp));   

    if(!confirm_hal(NOCONTACT,CONTACT))        // not correct sensor positions
    {
#ifdef DEBB        
        if (theConf.traceBoot)
            printf("%sError Hal Sensors not Ok Opened State [CloseH %d OpenH %d]\n",OPENEDSTATE, gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif       
        xTaskCreate(&unknown_state,"unknown",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager
        openedHandle=NULL;
        vTaskDelete(openedHandle);  //we are dead, unknown_state is managing the system
    }

    xQueueReset( lqueue);                   //empty queue
 
    //printf("open uxbits %x BreakB %x OpenB %x\n",uxBits,CLOSEBREAK_BIT,OPEN_BIT);
    
    // start the RelayTimer
    if(  uxBits <KOPEN_BIT )                                // Start counting if not Keep Open
    {
#ifdef DISPLAY
        xTaskCreate(&countDown,"count",STATEHEAP,(void*)theConf.timers[TRELAY] , 5,&counterHandle); 	//unknown state manager
#endif
        startStopTimer(START,timers[TRELAY]);    // start Wait timer if not a Break Cmd or keepOpen Cmd just Remote cmd
    }
                                                    // if Break while opening we could reeduce waiting time for Close Relay as is same time
    if( ( uxBits & ( CLOSEBREAK_BIT) ) == ( CLOSEBREAK_BIT ) ) 
        startStopTimer(START,timers[TBREAK]);       // break timer will protect from door being open indefinitely due to Break Cmd

    while(true)                             //wait for state machine event or timer
    {
        if(xQueueReceive(lqueue, &lcmd, portMAX_DELAY))
        {
#ifdef DEBB        
         if (theConf.traceTask)
            printf("%sOpened Task loop [cmd %d]\n",OPENEDSTATE,lcmd.cmd);
#endif    
            switch (lcmd.cmd)
            {
                case OPENHAL:                                   // Open Hal activated, Door should be closing
                    delay(theConf.waithal);                     // debouncing
                    DELTASK(counterHandle)
                    startStopTimer(STOP,timers[TRELAY]);        //stop a lot of timers: Relay
                    startStopTimer(STOP,timers[TBREAK]);        // Break if fired
                    startStopTimer(STOP,timers[TREPEAT]);       // Repeat if fired
                    startStopTimer(STOP,timers[TREPEATW]);      // HW controller if fired
                    if (confirm_hal(NOCONTACT,NOCONTACT))       // close hal sensor now false and open hal true, door is closing
                    {
                        xEventGroupClearBits(doorBits,CLOSEBREAK_BIT|KOPEN_BIT);      // just once, so clear them
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("%sOPENED TASK Door is Closing [CloseH %d OpenH %d]\n",OPENEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        xTaskCreate(&closing_state,"closing",STATEHEAP,NULL, 5, &closingHandle); 	//opening state manager will be in charge now
                        openedHandle=NULL;
                        vTaskDelete(openedHandle);  //we are dead, Closing_state is managing the system
                    }
                    else
                    {
#ifdef DEBB                         
                        printf("%sOpen confirm failed %d %d\n",OPENEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                   
                   }
                    break;
                case RELAYTIMER:                                // Relay timer now done. Relay should be activated to close the door
                    DELTASK(counterHandle)                      // kill counting task after countdown
                    if (confirm_hal(NOCONTACT,CONTACT))         // sensors confirm, Close true(1=>NOCONTACT) and open false(0->CONTACT)
                    {
#ifdef DEBB                       
                        if (theConf.traceCmdq)
                            printf("%sOPEN Task Wait Timer fired Opened state [CloseH %d OpenH %d]\n",OPENEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                       
                        countBlk++;
                        printf("%sCount break %d\n",OPENEDSTATE,countBlk);
                        if(countBlk>MAXBLKCOUNT)
                        {
                            if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
                            { 
                                u8g2_ClearBuffer(&u8g2);
                                ssdString(stx,sty,(char*)"BLKMAX",true);
                                xSemaphoreGive(dispSem);
                            }
                                theConf.guard=0;            // deactivate guard w/o saving the configuration
                                esp_rmaker_raise_alert("MaxBlocks reached.Guard disabled.");
                                esp_rmaker_param_update_and_report(guard_param, esp_rmaker_bool(theConf.guard));
                                // relayPulse(RELAYTIMER);  // if we active the door a stuck vehice/object will get runned over
                                xTaskCreate(&unknown_state,"unknown",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager
                                openedHandle=NULL;
                                vTaskDelete(openedHandle);  //we are dead, unknown_state is managing the system

                        }

                        if(theConf.guard)                         //if we are configured to use Hal Sensors or Motor sensors
                        {
                            ifrSensor=getIfrSensor();           //must BE LOW (unobstructed)
#ifdef DEBB                                
                            if (theConf.traceCmdq)
                                printf("%sIFR Sensor %s\n",OPENEDSTATE,ifrSensor?"Blocked":"Unblocked");
#endif
                            if(!ifrSensor) //sensor low is NO BLOCK, high is BLOCKING
                            {
                                relayPulse(RELAYTRIGGER);
                                startStopTimer(START,timers[TREPEATW]); // Start Relay Wait  timer. We must see a change in Sensors or HW Fault probable Motor
                            }
                            else        //IFR is blocked
                            {
#ifdef DEBB                                
                                if (theConf.traceCmdq)
                                    printf("%sOPEN Task Object blocking IFR %d. Retry Relay\n",OPENEDSTATE,ifrSensor);
#endif            
                                if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
                                { 
                                    u8g2_ClearBuffer(&u8g2);
                                    ssdString(stx,sty,(char*)"BLK",true);
                                    xSemaphoreGive(dispSem);
                                }
                                
                                xTimerStart(timers[TRELAY],0);      //repeat until countBlks exceeds limit
                            }
                        }
                        else //just activate relay
                        {
                                relayPulse(RELAYTRIGGER);
                                startStopTimer(START,timers[TREPEATW]); // Start Relay Wait  timer. We must see a change in Sensors or HW Fault probable Motor
                        }
                    }
                    else
                    {
                            //else the door is already closing by some means, order or remote control
#ifdef DEBB                            
                        if (theConf.traceCmdq)
                            printf("%sOPENED TASK Door is already closing [CloseH %d OpenH %d]\n",OPENEDSTATE,gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));                    
#endif                    
                    }
                    break;
                case CLOSEHAL:
                    DELTASK(counterHandle)
                    if (confirm_hal(CONTACT,NOCONTACT))         // close hal sensor now true and open hal false, door closed
                    { 
                        startStopTimer(STOP,timers[TRELAY]);    //stop timers if fired
                        startStopTimer(STOP,timers[TBREAK]);
                        startStopTimer(STOP,timers[TREPEAT]);
                        startStopTimer(STOP,timers[TREPEATW]);
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("OPENED Task HW error on OPEN HAL. Close Hal detected without openhal transition [CloseH %d OpenH %d]\n",gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        xTaskCreate(&closed_state,"closed",STATEHEAP,NULL, 5, &closedHandle); 	//opening state manager will be in charge now
                        openedHandle=NULL;
                        vTaskDelete(openedHandle);  //we are dead, Closed_state is managing the system
                    }
                    break;
                case LASERBREAK:    // if cmd recieved was close on break activate the door... must not fire the timers[TRELAY] if in this command or a different timer
                    if(theConf.guard)
                    {
                        uxBits =xEventGroupGetBits(doorBits);
                        // printf("GuardBreak ubits %x\n",uxBits);

                            if( ( uxBits & ( CLOSEBREAK_BIT) ) == ( CLOSEBREAK_BIT ) )      //in break mode
                            {
                                if(!breakin)        //if not break cmd set therefore its a real break of the guard line
                                {
                                    if (confirm_hal(NOCONTACT,CONTACT))         // close hal sensor now false and open hal false, door opening
                                        {
                                            xEventGroupClearBits(doorBits,CLOSEBREAK_BIT);      // just once
                                            breakin=true;
            #ifdef DEBB                                
                                            if (theConf.traceCmdq)
                                                printf("Break Cmd in timer started Opened state [CloseH %d OpenH %d]\n",gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
            #endif                               
                                            // cmd logic must be saved somewhere to know that this was a Break Cmd vs a Open Cmd vs Remote Control
                                            startStopTimer(START,timers[TRELAY]);
    #ifdef DISPLAY
                                            xTaskCreate(&countDown,"count",STATEHEAP,(void*)theConf.timers[TRELAY] , 5,&counterHandle); 	//unknown state manager
    #endif                                    
                                        }
                                }
                                // else
                                // { 
                                //     if(lcmd.state==OFF)     // a laserbreak cycle has passed so just close the door by activating the Relay
                                //     {
                                //         relayPulse(RELAYTRIGGER);
                                //         startStopTimer(START,timers[TREPEATW]); // Start Relay Repeat timer. We must see a change in Sensors or HW Fault probable Motor                             
                                //     }
                                // }
                        }
                    }
                    break;
                case RELAYWAIT: // Relay was activated and HAL Sensors not changing. RELAY Fault most likely or Motor not responding
                    DELTASK(counterHandle)
                    if (confirm_hal(NOCONTACT,CONTACT))         // close and open hal sensor now false ... closing else error
                    {
                        sprintf(temp,"Door %s stuck. Take Action",theConf.name);
                        esp_rmaker_raise_alert(temp);
#ifdef DEBB                        
                        if (theConf.traceCmdq)
                            printf("RelayWait Activated. Check Relay and/or Main Motor [CloseH %d OpenH %d]\n",gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));
#endif                        
                        xTaskCreate(&unknown_state,"fault",STATEHEAP,NULL, 5, &unknownHandle); 	//unknown state manager will be in charge now
                        openedHandle=NULL;
                        vTaskDelete(openedHandle);          //this task now dies
                    }
                    break;
                case BREAKTIMER:    // timer expired waiting for Break
                                    //simulate a IFRbreak queue event
#ifdef DEBB                                    
                    if (theConf.traceCmdq)
                        printf("Break Timer fired. [CloseH %d OpenH %d]\n",gpio_get_level((gpio_num_t)HALCL), gpio_get_level((gpio_num_t)HALOP));      
#endif                    
                    lcmd.cmd=LASERBREAK;
                    lcmd.state=1;
                    xQueueSend(lqueue, &lcmd, 0);   //simulate an Event, LASERBREAK
                    break;

                default:
                {
#ifdef DEBB                    
                    if (theConf.traceCmdq)
                        printf("Opened Task out of sync cmd %d\n",lcmd.cmd);
#endif                        
                }
            }
        }
        else
        {
#ifdef DEBB            
            if (theConf.traceCmdq)
                printf("Open fail queue rx\n");
#endif                
        }
    }
}