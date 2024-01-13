#include "globals.h"
#include "forwards.h"           // LOTS of forwards to have external cpp files!!!!!!


// DO NOT use INLINE functions since its the same as STATIC which does not allow other individual cpp files to use it as an extrernal reference when linking

void pwm_timer_callback(void* pArg)    
{
    if(laserState)      //crucial to be able to stop the PWM
    {
        if(pwmtimer)        //it is active. 
        {
            pwmtimer=OFF;
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));     //stop pwm 0% duty
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
           esp_timer_stop(sample_timer);                                   // stop sample timer
            if (millis()-loopcount>theConf.timers[TBREATH])  //breathing time logic
            {
                xTimerStartFromISR(timers[TCOOL],0);       //this is an ISR cannot/should not delay(). Call timer which after done will restart pwm cycle
            }                                              // TCOOL has coolmgr which is different than other timers callback
            else
                esp_timer_start_once(pwm_timer, theConf.timers[TPWMOFF]);   //start the pwmoff timer now, will call this same callback
        }
        else
        {
            pwmtimer=ON;
        // start pwm according to vdtuy calculated when LEDC initialized, Rainmaker command or KBD new duty set
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL,vduty));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));  
            esp_timer_start_once(pwm_timer, theConf.timers[TPWMON]);        //start the PWM timer to turn PWM off after this time, this callback routine again
            esp_timer_start_once(sample_timer, minSample);                  // start the Sample timer to take a sample of IFRRX line for block/noblock status
        }
    }
}

void sample_timer_callback(void* pArg)      // Sample timer logic
{
    lqueuecmd lcmd;

    if(pwmtimer)            //only when PWM is active
    {
        //for the Logic Analyzer use, make a short pulse meaning a sample was taken at this point
        // Direct Port manipulation for speed in Read and Write ports

        sample=(REG_GET_BIT(GPIO_IN_REG,BIT23)>>IFR);
        REG_WRITE(GPIO_OUT1_W1TS_REG, BIT0);//GPIO32  = HIGH => second port first pos 
        REG_WRITE(GPIO_OUT1_W1TC_REG, BIT0);//GPIO32 = LOW      

        if(blockrequest)        //set by getIFR to get current state of IFR receiver during PWM phase, use another Queue from normal
        {
            bzero((void*)&lcmd,sizeof(lcmd));
            lcmd.cmd=LASERBREAK;
            lcmd.state=sample;
            xQueueSendFromISR(blockqueue, &lcmd, NULL);
            blockrequest=false;
        }
        uint32_t timedif =millisFromISR()-lastTick;

        if(sample !=oldsample && timedif>theConf.timers[TIFR])
        {
            // Again for the Logic Analyzer make a short Pulse when a Break is detected
            REG_WRITE(GPIO_OUT_W1TS_REG, BIT25);
            REG_WRITE(GPIO_OUT_W1TC_REG, BIT25);
            
            //inform a Break was detected
            lastTick=millisFromISR();      //now
            lcmd.cmd=LASERBREAK;
            lcmd.state=oldsample=sample;
            xQueueSendFromISR(lqueue, &lcmd, NULL);
        }
    }
}


int8_t getIfrSensor()
{
    lqueuecmd lcmd;

    blockrequest=true;                          //raise flag requesting IFR status during ON pulse cycle
    xQueueReset( blockqueue);                   //empty queue

    if(xQueueReceive(blockqueue,&lcmd,400 / portTICK_PERIOD_MS)==pdPASS)
    {
#ifdef DEBB 
        if (theConf.traceISR)
            printf("%sBlock answer %d\n",SYSTEM,lcmd.state);
#endif
        return lcmd.state;
    }
#ifdef DEBB                        
    if (theConf.traceISR)
        printf("%sNo block answer\n",SYSTEM);
#endif

    return -1;
}

void sntpget()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char strftime_buf[100];
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) 
    {
        // printf("%sTime is not set yet. Connecting to WiFi and getting time over NTP.");
        esp_sntp_setoperatingmode((esp_sntp_operatingmode_t)SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_init();

        memset(&timeinfo,0,sizeof(timeinfo));
        int retry = 0;
        const int retry_count = 30;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        if(retry>retry_count)
        {
            printf("%sSNTP failed\n",SYSTEM);
            return;
        }
        time(&now);
        setenv("TZ", LOCALTIME, 1);
        tzset();
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
#ifdef DEBB
        if (theConf.traceBoot)   
            printf("%s[CMD]The current date/time in %s is: %s day of Year %d\n",SYSTEM, LOCALTIME,strftime_buf,timeinfo.tm_yday);
#endif
    }
}

//when a timer is fired, this routine will dispatch to our lqueue a Timer event with the Timer number 
void timermgr(TimerHandle_t xTimer)
{
    lqueuecmd lcmd;

	uint32_t cual=(uint32_t) pvTimerGetTimerID( xTimer );

#ifdef DEBB
    if(theConf.traceTimers)
        printf("%sTimer[%ld = %s] called\n",SYSTEM,cual,timer_names[cual]);
#endif

    lcmd.cmd=cual;      //timer number is event also
    lcmd.state=1;   //activated
    xQueueSend(lqueue, &lcmd,0);
}

static void blinkit(void* arg) //activated by interrupt on Pin from Hal Sensor
{
    uint32_t dur=(uint32_t)arg;     // blink rate

    while(true)
    {
        gpio_set_level((gpio_num_t)SYSLED, true);    //on
        delay(dur);
        gpio_set_level((gpio_num_t)SYSLED, false);   //off
        delay(dur);
    }
}

void launchBlink(int interval)
{
    xTaskCreate(&blinkit,"blink",STATEHEAP,(uint32_t*)interval, 20, &blinker);     
}

void turnLight(bool como)
{
    gpio_set_level((gpio_num_t)DLED, como);    
}

int find_cmd(char * cmdname)    //find the app command and execute its code
{
    for(int a=0;a<NUM_CMDS;a++)
    {
        if (strcmp(cmdname,cmds[a].comando)==0)
            return a;
    }
    return -1;
}

/* Callback to handle commands received from the RainMaker cloud */
esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{

#ifdef DEBB
    if (theConf.traceCmdq)   
        printf("%sIncoming Device %s Param %s\n",SYSTEM,esp_rmaker_device_get_name(device),esp_rmaker_param_get_name(param));
#endif
    int cual=find_cmd(esp_rmaker_param_get_name(param));
    if(cual>=0)
    {

#ifdef DISPLAY
        if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
        { 
            char tmp[30];
            u8g2_SetPowerSave(&u8g2, 0); // wake up display
            sprintf(tmp,"%s", cmds[cual].comando);
            u8g2_ClearBuffer(&u8g2);
            ssdString(stx,sty,tmp,true); 
            xSemaphoreGive(dispSem);
        }
#endif
        if(gOnOff==POWER_OFF && (strcmp(cmds[cual].comando,CMD0)==0 || strcmp(cmds[cual].comando,CMD4)==0))
        {
            vTaskDelete(powerHandle);
            powerHandle=NULL;           //stop tracking cutoff time
            setPower(POWER_ON);
            delay(RESTARTMOTOR);       //let the motor unit catch up, DO NOT change gOnOff variable
        }

        cmds[cual].code(param,val);         //execute the code for this command
    }
    return ESP_OK;
}



void IRAM_ATTR halISR(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    lqueuecmd lcmd;
    uint8_t st=gpio_get_level((gpio_num_t)gpio_num);

        switch(gpio_num)
        {
            case HALCL:
                lcmd.cmd=CLOSEHAL;
                lcmd.state=st;
                break;
            case HALOP:
                lcmd.cmd=OPENHAL;
                lcmd.state=st;
                break;
            case IFR:               // pass the signal as is to the general logic. 
                lcmd.cmd=LASERBREAK;
                lcmd.state=st;
                break;
            case EXTP:               // pass the signal as is to the general logic. 
                lcmd.cmd=EXTSENSOR;
                lcmd.state=st;
                break;
            default:
                lcmd.cmd=UNK;
                lcmd.state=st;
        }
            xQueueSendFromISR(lqueue, &lcmd, NULL);
// #ifdef DEBB
//             if (theConf.traceISR)
//                 ets_printf("%sint %d state %d\n",gpio_num,st);
// #endif                    
}

void setIFR(uint8_t how)
{
    if(how)
    {
        pwmtimer=OFF;
        loopcount=millis();
        lastTick=millis();
        oldsample=0;
        if(!laserState)
            esp_timer_start_once(pwm_timer, theConf.timers[TPWMON]);
        laserState=ON;

#ifdef DEBB        
        if (theConf.traceTask)
            printf("%sIFR PWM starting PWM ON Time %ldus OFF Time %ld Cycle %d%%\n",SYSTEM,theConf.timers[TPWMON],theConf.timers[TPWMOFF],theConf.ciclo);
#endif            
    }
    else            //turn off the Guard Controller
    {
        esp_timer_stop(pwm_timer);
        esp_timer_stop(sample_timer);
        laserState=OFF;     
        pwmtimer=OFF; 
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
#ifdef DEBB        
        if (theConf.traceTask)
            printf("%sIFR PWM Stopping\n",SYSTEM);
#endif      
    }
}

//for the display to show secs remaining before relay activated
void countDown(void *pArg)
{
    int32_t count=(int32_t)pArg;
    char tmp[12];

    while(count>0)
    {
        sprintf(tmp,"%ld",count);
        if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
        { 
            u8g2_ClearBuffer(&u8g2);
            ssdString(stx,sty,tmp,true);
            xSemaphoreGive(dispSem);
        }
        delay(1000);
        count--;
    }
    counterHandle=NULL;
    vTaskDelete(counterHandle);
}


void power_task(void *pArg)
{
    time_t now;
    struct tm timeinfo;

    while(true)
    {
        time(&now);
        setenv("TZ", LOCALTIME, 1);
        tzset();
        localtime_r(&now, &timeinfo);
        // printf("%sLoop Power\n");

        if(theConf.hstart!=theConf.htime)       //if start and stop the same, no power mode enabled
        {
            //test for start of power off schedule MUST be ON
            if(theConf.hstart==timeinfo.tm_hour )
            {
#ifdef DEBB
                if (theConf.traceCmdq)   
                    printf("%sPower cutoff start %d now %d\n",SYSTEM,theConf.hstart,timeinfo.tm_hour);
#endif                
                writeLog((char*)"PowerOff cycle started");
                gOnOff=POWER_OFF;
                setPower(POWER_OFF);
#ifdef DISPLAY
                if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
                { 
                    u8g2_ClearBuffer(&u8g2);
                    ssdString(stx,sty,(char*)"SPWOFF",true); 
                    xSemaphoreGive(dispSem);
                }
#endif
            esp_rmaker_param_update_and_report(status_param, esp_rmaker_str("SLEEP"));   

            }
//test for end of power off schedule, MUST be OFF
            if(theConf.htime==timeinfo.tm_hour)
            {
#ifdef DEBB
                if (theConf.traceCmdq)   
                    printf("%sPower cutoff stop %d now%d\n",SYSTEM,theConf.hstart,timeinfo.tm_hour);
#endif                   
                writeLog((char*)"PowerOff cycle ended");
                gOnOff=POWER_ON;
                setPower(POWER_ON);
#ifdef DISPLAY
                if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
                { 
                    u8g2_ClearBuffer(&u8g2);
                    ssdString(stx,sty,(char*)"SPWON",true); 
                    xSemaphoreGive(dispSem);
                }
#endif
             }
        }

        delay(POWERDELAY);    }
}

/*
void factory_reset(void *pArg)
{
    //flash button gpio 0
    io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en =GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en =GPIO_PULLDOWN_DISABLE;
    io_conf.pin_bit_mask = (1ULL<<0);     //input pins
	gpio_config(&io_conf);

    while(true)
    {
        delay(1000);
        if(!gpio_get_level((gpio_num_t)0))
        {
            delay(3000);
            if(!gpio_get_level((gpio_num_t)0))
            {          
#ifdef DISPLAY
if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
{ 
        u8g2_ClearBuffer(&u8g2);
        ssdString(stx,sty,doorState[6],true);
        // ssdString(stx,sty,(char*)"PROV",true);
        xSemaphoreGive(dispSem);
}
#endif
                printf("%sreset to Factory button\n",SYSTEM);
                erase_config();
                esp_restart();
            }
        }
    }
}
*/
void app_main()
{
    const esp_partition_t *running = esp_ota_get_running_partition();

    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
#ifdef DEBB
        if (theConf.traceBoot)   
            printf("%sGarage Running firmware version: %s\n",SYSTEM, running_app_info.version);
#endif
        strcpy(app_version,running_app_info.version);
    }

    TAG=(char*)malloc(10);
    strcpy(TAG,"Garage");
    vTaskDelay(1000/portTICK_PERIOD_MS);    // seconds delay for serial etc

    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        printf("%sOpen NVS error %x\n",SYSTEM,err);
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        erase_config();
    }

    read_flash();           //read our configuration
#ifdef DEBB    
    if (theConf.traceBoot)
    {    
        printf("%sRelay Wait %lds WaitHal %ldms Duty: %d%% Name [%s]\n",SYSTEM,theConf.timers[TRELAY],theConf.waithal,theConf.ciclo,theConf.name);
        printf("%sTrace %d Boot %d Cmdq %d ISR %d\n",SYSTEM,theConf.traceTask,theConf.traceBoot,theConf.traceCmdq,theConf.traceISR);
    }
#endif    

    // xTaskCreate(&factory_reset,"freset",2048,NULL, 5, NULL);           //wifi and factory reset due to change of SSID

    init_conf();                            //initialize a lot of stuff

    uint32_t desdet=millis();

    while(millis()-desdet<3000)     //for 3 secs detect FLASH button to erase configuration on demand
    {
        if(!gpio_get_level((gpio_num_t)RESETP))
        {
    #ifdef DEBB        
            printf("%sErasing flash per request\n",SYSTEM);
            delay(1000);
    #endif        
            erase_config();
        }
        delay(200);
    }

    app_wifi_init();            //wifi init

    logFileInit();              //log file init

    pwm_init();                //pwm controller config

    init_rmaker_device();       //rmaker interface start

    /* Enable OTA */
    esp_rmaker_ota_enable_default();    //enable OTA updates

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();         //rmaker start
    
    xTaskCreate(&blinkit,"blink",MINORHEAP,(uint32_t*)50, 10, &blinker); 	        // show booting sequence active

// VIRTUAL MACHINE start of the whole process. It will change accordingly to the Hal sensors position (CLOSED->OPENING->OPENED->CLOSING) etc
    xTaskCreate(&closed_state,"closed",STATEHEAP,NULL, 5, &closedHandle);           //start the Virtual Machine connected or not to wifi
    
#ifdef DEBB
    kbd();      //start console
#endif

    err = app_wifi_start(POP_TYPE_RANDOM,(char*)"PROV_RSN",(char *)"csttpstt");
    if (err != ESP_OK) {
        printf("%sCould not start Wifi. Aborting!!!\n",SYSTEM);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    sntpget();                                  //get internet time now that we are connected

    gpio_set_level((gpio_num_t)SYSLED, true);   //System Active
#ifdef DEBB   
    if (theConf.traceRmaker)
        printf("%sRainmaker Ready\n",SYSTEM);
#endif    

    theConf.reboots++;
    write_to_flash();

    char * tmp=(char*)malloc(200);
    sprintf(tmp,"System Reboot %d and active",theConf.reboots);
    writeLog(tmp);
    free(tmp);

//report several RainMaker variables
    esp_rmaker_param_update_and_report(wait_param, esp_rmaker_int(theConf.timers[TRELAY]));
    // esp_rmaker_param_update_and_report(status_param, esp_rmaker_str("Closed"));
    esp_rmaker_param_update_and_report(hstart, esp_rmaker_int(theConf.hstart));
    esp_rmaker_param_update_and_report(guard_param, esp_rmaker_bool(theConf.guard));
    esp_rmaker_param_update_and_report(htime, esp_rmaker_int(theConf.htime));
    esp_rmaker_param_update_and_report(onoff_param, esp_rmaker_bool(theConf.onoff));


 }


