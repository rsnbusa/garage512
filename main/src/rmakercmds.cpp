
#define GLOBAL
#include "globals.h"
#include "forwards.h"


//RMaker commands
int remote_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)      //Door Remote Button
{
    relayPulse(RELAYTRIGGER);
    esp_rmaker_param_update_and_report(param, val);
    writeLog((char*)"Door Activated by RMaker");  

#ifdef DEBB
        if(theConf.traceRmaker)
            printf("%sRemote activated\n",WEB);
#endif

    return ESP_OK;
}

int wait_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)        // Door wait duration slider
{
    char *tmp=(char*)malloc(100);
    if(tmp)
    {
        bzero(tmp,100);
        sprintf(tmp,"Door delay set to %d",val.val.i);
        writeLog(tmp);

#ifdef DEBB        
    if(theConf.traceRmaker)
        printf("%s%s\n",WEB,tmp);
#endif
        free(tmp);
    }
    theConf.timers[TRELAY]=val.val.i;
    if(theConf.timers[TRELAY]<1)
        theConf.timers[TRELAY]=1;
    esp_rmaker_param_update_and_report(param, esp_rmaker_int(theConf.timers[TRELAY]));   
    xTimerChangePeriod( timers[TRELAY], theConf.timers[TRELAY]*WORKTIME/ portTICK_PERIOD_MS, 0 );    
    write_to_flash();
    return ESP_OK;
}


int reset_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)           // reset button
{
#ifdef DEBB
    if(theConf.traceRmaker)
        printf("%sReset was received. Factory settings to default\n",WEB);
#endif
    writeLog((char*)"Factory reset received");
    esp_rmaker_factory_reset(1,3);
    return ESP_OK;
}

int guard_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)       // guard button
{
#ifdef DEBB
    if(theConf.traceRmaker)
        printf("%sGuard %s\n",WEB,val.val.b?"ON":"OFF");
#endif
    esp_rmaker_param_update_and_report(param, val);
    theConf.guard=val.val.b;
    write_to_flash();
    return ESP_OK;
}


int closebreak_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)      //close break button
{
    EventBits_t uxBits =xEventGroupGetBits(doorBits); 

#ifdef DEBB
    if(theConf.traceRmaker)
        printf("%sBreak\n",WEB);
#endif
    relayPulse(RELAYTRIGGER);
    esp_rmaker_param_update_and_report(param, val);

if(theConf.guard)
{
    writeLog((char*)"Break command");
    if( ( uxBits & ( CLOSEBREAK_BIT) ) != ( CLOSEBREAK_BIT ) ) // not set
        xEventGroupSetBits(doorBits, CLOSEBREAK_BIT);  
    else
        xEventGroupClearBits(doorBits, CLOSEBREAK_BIT);  
}   //else like a normal remote command
    return ESP_OK;
}


int keepopen_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)        //keep open button
{
    EventBits_t uxBits =xEventGroupGetBits(doorBits); 

#ifdef DEBB
    if(theConf.traceRmaker)
        printf("%sKeepOpen\n",WEB);
#endif
    writeLog((char*)"KeepOpen Command received");
    relayPulse(RELAYTRIGGER);
    esp_rmaker_param_update_and_report(param, val);

    if( ( uxBits & ( KOPEN_BIT) ) == ( KOPEN_BIT ) )  //already set   clear the bit     
        xEventGroupClearBits(doorBits, KOPEN_BIT); 
    else    
        xEventGroupSetBits(doorBits, KOPEN_BIT);     //first time, set the bit for opened task
    return ESP_OK;
}

int dutycycle_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)           //duty slider
{
    theConf.ciclo=val.val.i;
#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%sDuty Cycle %d%%\n",WEB,theConf.ciclo);
#endif
    vduty=(pow(2,int(LEDC_DUTY_RES))-1)*(float)(theConf.ciclo/100.0);
    esp_rmaker_param_update_and_report(param, esp_rmaker_int(theConf.ciclo));   
    write_to_flash();    
    return ESP_OK;
}


int name_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)            //name label
{
#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%sName %s\n",WEB,val.val.s);
#endif
    strcpy(theConf.name,val.val.s);
    write_to_flash();    
    return ESP_OK;
}


int align_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)           //align button
{
    int what=0;

#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%sAlign IFR %d\n",WEB,val.val.i);
#endif
    what=val.val.i;
    if(what && !ifrtesthandle)
        {
            xTaskCreate(&ifrTest,"align",STATEHEAP,NULL, 5, &ifrtesthandle);           
            if(theConf.traceRmaker)
                printf("%sAlign IFR Task started\n",WEB);
#ifdef DISPLAY
        if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
        { 
            u8g2_ClearBuffer(&u8g2);
            ssdString(stx,sty,doorState[9],true);
            xSemaphoreGive(dispSem);
        }
#endif
        }
    else
        if(!what && ifrtesthandle)
        {
#ifdef DISPLAY
        if(xSemaphoreTake(dispSem, portMAX_DELAY/  portTICK_PERIOD_MS))		
        { 
            u8g2_ClearBuffer(&u8g2);
            ssdString(stx,sty,(char*)"",true);
            xSemaphoreGive(dispSem);
        }
#endif
            setIFR(OFF);
            vTaskDelete(ifrtesthandle);
            ifrtesthandle=NULL;
        }
    return ESP_OK;
}


int onoff_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)           // on off button
{
#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%sOnOff %d\n",WEB,val.val.i);
#endif
    theConf.onoff=val.val.i;
    write_to_flash();    
    return ESP_OK;
}

int hstart_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)      //power off slider
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%s%s  %d\n",WEB,CMD11,val.val.i);
#endif
    theConf.hstart=val.val.i;
    write_to_flash();    
    esp_rmaker_param_update_and_report(hstart, esp_rmaker_int(theConf.hstart));     //echo confirmation

// reset state to ON in case change results in Start=End condition
    gOnOff=POWER_ON;
    setPower(POWER_ON);

    return ESP_OK;
}


int htime_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)           // power on slider
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%s%s  %d\n",WEB,CMD12,val.val.i);
#endif
    theConf.htime=val.val.i;
    esp_rmaker_param_update_and_report(htime, esp_rmaker_int(theConf.htime));       //echo confirmation
    write_to_flash();    

// reset state to ON in case change results in Start=End condition
    gOnOff=POWER_ON;
    setPower(POWER_ON);
    
    return ESP_OK;
}

int sampler_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val)     //sampler slider
{
    float freq_width=1.0/theConf.timers[TFREQ]*1000000;

    theConf.sampler=val.val.i-1;
#ifdef DEBB
    if(theConf.traceRmaker)
            printf("%sSample %d\n",WEB,theConf.sampler);
#endif
    float mins=freq_width*theConf.sampler;
    minSample=(int)(mins+freq_width*theConf.ciclo/100.0/2.0);
    write_to_flash();    
    return ESP_OK;
}


