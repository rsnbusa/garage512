
#define GLOBAL

#include "includes.h"
#include "defines.h"
#include "globals.h"
#include "forwards.h"
extern void write_to_flash();
extern void erase_config();
extern void setIFR(uint8_t q);
extern void start_ota();
extern void delay(uint32_t cuanto);



int cmdStatusPWM(int argc, char **argv)
{
        printf("%sStatus Duty [%d] Cycle [%d]\n",KBD,vduty,perc);
        return 0;
}

int cmdOnoff(int argc, char **argv)
{
     int nerrors = arg_parse(argc, argv, (void **)&onoffArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, onoffArgs.end, argv[0]);
        return 0;
    }

 if (onoffArgs.on_off->count) {
    int cuanto=onoffArgs.on_off->ival[0];
    printf("%sSystem is now %s\n",KBD,cuanto?"On":"Off");
    theConf.onoff=cuanto;
    write_to_flash();
}
return  0;
}

int cmdTriggerPWM(int argc, char **argv)
{
    // gpio_set_level((gpio_num_t)IFRTESTSAMPLE,1);
    // gpio_set_level((gpio_num_t)IFRTESTSAMPLE,0);    
    setIFR(true);
    delay(10000);
    setIFR(OFF);
    printf("%sTrigger sent for PWM\n",KBD);
    return 0;
}

int cmdStatusDoor(int argc, char **argv)
{
        printf("%sDoor name [%s] RelayWait [%ld], DutyCycle [%d] Reboots [%d] ",KBD,theConf.name,theConf.timers[TRELAY],theConf.ciclo,theConf.reboots);
         uint8_t cl=gpio_get_level((gpio_num_t)HALCL);
         uint8_t op=gpio_get_level((gpio_num_t)HALOP);
        printf("%s HalClose [%d] HalOpen [%d]\n",KBD,cl,op);
        return 0;
}

int cmdStartPWM(int argc, char **argv)
{
        printf("%sPWM started Duty %d Cycle %d%%\n",KBD,vduty,theConf.ciclo);
        setIFR(ON);
        return 0;
}

int cmdStopPWM(int argc, char **argv)
{
        printf("%sPWM is off\n",KBD);
        setIFR(OFF);
        return 0;
}

int cmdReset(int argc, char **argv)
{
        printf("%sFactory Reset performed\n",KBD);
        esp_rmaker_factory_reset(1,3);
        return 0;
}

int cmdConfig(int argc, char **argv)
{
  wifi_config_t wconfig;
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);
  char strftime_buf[100];

        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        esp_wifi_get_config(WIFI_IF_STA,&wconfig);
        printf("%sSystem is %s Date:%s IDF-Version %s Garage-Version:%s\n",KBD,theConf.onoff?"ON":"OFF",strftime_buf,esp_get_idf_version(),app_version);
        printf("SSID %s  password %s\n",wconfig.sta.ssid,wconfig.sta.password);
        printf("Saved Configuration Reboots %d\n",theConf.reboots);
        printf("General\n [Doorclose %ldsec] [Guard %s] [Doorname %s]\n",theConf.timers[TRELAY],theConf.guard?"ON":"OFF",theConf.name);
        printf("PWM\n [Duty %d%%] [Cycle %d]\n",theConf.ciclo,vduty);
        printf("Trace\n [Task %s] [Boot %s] [Cmd %s] [Isr %s] [RMAKER %s] [Timers %s]\n",theConf.traceTask?"ON":"OFF",theConf.traceBoot?"ON":"OFF",theConf.traceCmdq?"ON":"OFF",theConf.traceISR?"ON":"OFF",theConf.traceRmaker?"ON":"OFF",theConf.traceTimers?"ON":"OFF");
        printf("Timers-Timeouts\n [Close %lds] [Opening %lds] [Repeat %lds] [RepeatW %lds] [Display Timer %lds] [WaitHal %ldms] [Break %lds]\n",theConf.timers[TCLOSE],theConf.timers[TOPEN],theConf.timers[TREPEAT],theConf.timers[TREPEATW],
        theConf.timers[TDISPLAY],theConf.waithal,theConf.timers[TBREAK]);
        printf(" [PWMON %ldus] [PWMOFF %ldus] [PWMBreath %ldms] [PWM Cool %ldms] [IFRBreak %ldms] [Frequency %ldhz][SampleTimer %ldus]\n",theConf.timers[TPWMON],theConf.timers[TPWMOFF],theConf.timers[TBREATH],theConf.timers[TCOOL],theConf.timers[TIFR],theConf.timers[TFREQ],minSample);
        printf("Power\n [Off Start %d] [On resume %d] [State %s]\n",theConf.hstart,theConf.htime,gOnOff?"OFF":"ON");
        return 0;
}

int cmdErase(int argc, char **argv)
{
    printf("%sResetting configuration.",KBD);
    erase_config();
    return 0;
}

int cmdDuty(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&dutyArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, dutyArgs.end, argv[0]);
        return 0;
    }

 if (dutyArgs.paso->count) {
    int cuanto=dutyArgs.paso->ival[0];
    vduty+=cuanto;
    if (vduty<1) 
        vduty=maxDuty;
    if (vduty>maxDuty)
        vduty=0;
    float perc;
    perc=(float)vduty/(float)maxDuty*100;
    theConf.ciclo=(int)perc;
    printf("%sDuty step changed by %d to Duty %d = %.02f%%\n",KBD,cuanto,vduty,perc);
    write_to_flash();
 }
  if (dutyArgs.porcentaje->count) 
  {
    perc=dutyArgs.porcentaje->ival[0];
        theConf.ciclo=(int)perc;
    vduty=maxDuty*perc/100;
    printf("%sDuty changed to %d%% for value %d\n",KBD,perc,vduty);

    write_to_flash();
  }
    return 0;
}

int cmdTimers(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&timerArg);
    if (nerrors != 0) {
        arg_print_errors(stderr, timerArg.end, argv[0]);
        return 0;
    }

 if (timerArg.close->count) {
    int cuanto=timerArg.close->ival[0];
    if(cuanto<20 || cuanto>400)
        cuanto=KCLOSET;
    theConf.timers[TCLOSE]=cuanto;
    printf("%sClose Timeout set to %d \n",KBD,cuanto);
    xTimerChangePeriod( timers[TCLOSE], cuanto*WORKTIME/ portTICK_PERIOD_MS, 0 );
 }
  if (timerArg.cancel->count) 
  {
    int cuanto=timerArg.cancel->ival[0];
    if(cuanto<20 || cuanto>400)
        cuanto=KOPENINGT;
    theConf.timers[TOPEN]=cuanto;
    printf("%sOpening Timeout set to %d \n",KBD,cuanto);
    xTimerChangePeriod( timers[TOPEN], cuanto*WORKTIME/ portTICK_PERIOD_MS, 0 );
  }
  if (timerArg.repeat->count) 
  {
    int cuanto=timerArg.repeat->ival[0];
    if(cuanto<20 || cuanto>400)
        cuanto=KREPEATT;
    theConf.timers[TREPEAT]=cuanto;
    printf("%sRepeat Timer set to %d \n",KBD,cuanto);
    xTimerChangePeriod( timers[TREPEAT], cuanto*WORKTIME/ portTICK_PERIOD_MS, 0 );
  }
  if (timerArg.relay->count) 
  {
    int cuanto=timerArg.relay->ival[0];
    if(cuanto<10 || cuanto>400)
        cuanto=KESPERA;
    theConf.timers[TRELAY]=cuanto;
    printf("%sRelay set to %d \n",KBD,cuanto);
    xTimerChangePeriod( timers[TRELAY], cuanto*WORKTIME/ portTICK_PERIOD_MS, 0 );    
  }
  if (timerArg.relayw->count) 
  {
    int cuanto=timerArg.relayw->ival[0];
    if(cuanto<1 || cuanto>20)
        cuanto=KREPEATW;
    theConf.timers[TREPEATW]=cuanto;
    printf("%sRelay Repeat set to %d \n",KBD,cuanto);
    xTimerChangePeriod( timers[TREPEATW], cuanto*WORKTIME/ portTICK_PERIOD_MS, 0 );    
  }
    if (timerArg.waithal->count) 
  {
    int cuanto=timerArg.waithal->ival[0];
    if(cuanto<100 )
        cuanto=KWAITHAL;
    theConf.waithal=cuanto;
    printf("%sWait Hal set to %d \n",KBD,cuanto);
  }
    if (timerArg.breaktimer->count) 
  {
    int cuanto=timerArg.breaktimer->ival[0];
    if(cuanto<1 )
        cuanto=KBREAKTIMER;
    theConf.timers[TBREAK]=cuanto;
    printf("%sBreakTimer set to %d \n",KBD,cuanto);
    xTimerChangePeriod( timers[TBREAK], cuanto*WORKTIME*60/ portTICK_PERIOD_MS, 0 );    
  }
    if (timerArg.pwmontimer->count) 
  {
    int cuanto=timerArg.pwmontimer->ival[0];
    if(cuanto<600 )
        cuanto=KPWMTIMER;
    theConf.timers[TPWMON]=cuanto;
    printf("%sPWM ON Timer set to %d \n",KBD,cuanto);
  }
    if (timerArg.pwmofftimer->count) 
  {
    int cuanto=timerArg.pwmofftimer->ival[0];
    // if(cuanto<theConf.timers[TPWMON] )
    //     cuanto=theConf.timers[TPWMON];
    theConf.timers[TPWMOFF]=cuanto;
    printf("%sPWM OFFTimer set to %d \n",KBD,cuanto);
  }
    if (timerArg.pwmbreathtimer->count) 
  {
    int cuanto=timerArg.pwmbreathtimer->ival[0];
    if(cuanto<KPWMBREATH )
        cuanto=KPWMBREATH;
    theConf.timers[TBREATH]=cuanto;
    printf("%sPWM Breath Timer set to %d \n",KBD,cuanto);
  }
    if (timerArg.pwmcooltimer->count) 
  {
    int cuanto=timerArg.pwmcooltimer->ival[0];
    if(cuanto<KPWMCOOL )
        cuanto=KPWMCOOL;
    theConf.timers[TCOOL]=cuanto;
    xTimerChangePeriod( timers[TCOOL], cuanto/ portTICK_PERIOD_MS, 0 );
    printf("%sPWM Cool Timer set to %d\n",KBD,cuanto);
  }

if (timerArg.ifrbreak->count) 
  {
    int cuanto=timerArg.ifrbreak->ival[0];
    if(cuanto<KIFRBREAK )
        cuanto=KIFRBREAK;
    theConf.timers[TIFR]=cuanto;
    printf("%sIFRBreak Timer set to %d \n",KBD,cuanto);
  }
if (timerArg.frequency->count) 
  {
    int cuanto=timerArg.frequency->ival[0];
    int err=ledc_set_freq(LEDC_MODE, LEDC_TIMER, cuanto);
    if(err)
      printf("%sError %x assigning new Frequency %d\n",KBD,err,cuanto);
    else    
      theConf.timers[TFREQ]=cuanto;

    printf("%sFrequency set to %ld \n",KBD,theConf.timers[TFREQ]);
  }
    write_to_flash();
    return 0;
}


int cmdTrace(int argc, char **argv)
{
    bool modify=false;
    int nerrors = arg_parse(argc, argv, (void **)&traceArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, traceArgs.end, argv[0]);
        return 0;
    }

 if (traceArgs.show->count) {
    printf("%sTrace Task %d Boot %d Cmd %d ISR %d Rmaker %d Timers %d\n",KBD,theConf.traceTask,theConf.traceBoot,theConf.traceCmdq,theConf.traceISR,theConf.traceRmaker,theConf.traceTimers);   
 }
 if (traceArgs.task->count) {
    int cuanto=traceArgs.task->ival[0];
    printf("%sTask tracing %s\n",KBD,cuanto?"On":"Off");   
    theConf.traceTask=cuanto;
    modify=true;
 }

 if (traceArgs.boot->count) {
    int cuanto=traceArgs.boot->ival[0];
    printf("%sBoot tracing %s\n",KBD,cuanto?"On":"Off");
    theConf.traceBoot=cuanto;
    modify=true;
 }

 if (traceArgs.all->count) {
    int cuanto=traceArgs.all->ival[0];
    printf("%sSetting all Tracing to %s\n",KBD,cuanto?"On":"Off");
    theConf.traceTask=theConf.traceBoot=theConf.traceCmdq=theConf.traceISR=theConf.traceRmaker=theConf.traceTimers=cuanto;
    theConf.traceBoot=cuanto;
    modify=true;
 }
 if (traceArgs.isr->count) {
    int cuanto=traceArgs.isr->ival[0];
    printf("%sSetting ISR to %s\n",KBD,cuanto?"On":"Off");
    theConf.traceISR=cuanto;
    modify=true;
 }
 if (traceArgs.rmaker->count) {
    int cuanto=traceArgs.rmaker->ival[0];
    printf("%sSetting Rmaker to %s\n",KBD,cuanto?"On":"Off");
    theConf.traceRmaker=cuanto;
    modify=true;
 }
 if (traceArgs.timers->count) {
    int cuanto=traceArgs.timers->ival[0];
    printf("%sSetting trace timers to %s\n",KBD,cuanto?"On":"Off");
    theConf.traceTimers=cuanto;
    modify=true;
 }

  if (traceArgs.status->count) {
    printf("%sTracing options Task %d Boot %d Cmdq %d ISR %d Rmaker %d Timers %d\n",KBD, theConf.traceTask,theConf.traceBoot,theConf.traceCmdq,theConf.traceISR,theConf.traceRmaker,theConf.traceTimers);
 }

    if (modify)
        write_to_flash();
    return 0;
}

int cmdLog(int argc, char **argv)
{
    char linea[200];

    int nerrors = arg_parse(argc, argv, (void **)&logArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, traceArgs.end, argv[0]);
        return 0;
    }

 if (logArgs.show->count) {
    int cuanto=logArgs.show->ival[0];
    int ccuanto=cuanto;
    printf("%sShow %d log lines\n",KBD,cuanto);
    fclose(myFile);
    myFile= fopen("/spiffs/log.txt", "r");
    while(cuanto>0)
    {
        if( fgets (linea, sizeof(linea), myFile)!=NULL ) 
        {
            printf("[%3d]%s",ccuanto-cuanto+1,linea);
            cuanto--;
        }
        else
        {
            fclose(myFile);
            myFile= fopen("/spiffs/log.txt", "a");
            if (myFile == NULL) 
            {
                printf("%sFailed to open file for append\n",KBD);
            }
            return 0;
        }
     }
 }

 if (logArgs.erase->count) 
 {
    int cuanto=logArgs.erase->ival[0];
    printf("%sErase Log file confimed %d\n",KBD,cuanto);

    fclose(myFile);
    remove("/spiffs/log.txt");
    myFile= fopen("/spiffs/log.txt", "a");
    if (myFile == NULL) 
    {
        printf("%sFailed to open file for append erase\n",KBD);
    }
 }

    return 0;
}

void kbd()
{
    esp_console_repl_t       *repl=NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt=(char*)"Garage>";
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    dutyArgs.paso =                 arg_int0(NULL, "step", "+/-", "Inc /Dec Duty counter");
    dutyArgs.porcentaje =           arg_int0(NULL, "cycle", "%%", "Set Duty cycle in %%");
    dutyArgs.end =                  arg_end(2);
 
    onoffArgs.on_off =               arg_int0(NULL, "start", "+On-Off", "Start or stop system");
    onoffArgs.end =                  arg_end(1);

    traceArgs.task =                arg_int0(NULL, "task", "0/1", "Main Task trace");
    traceArgs.boot =                arg_int0(NULL, "boot", "0/1" ,"Boot trace");
    traceArgs.queue =               arg_int0(NULL, "queue", "0/1" ,"Cmd queue trace");
    traceArgs.all =                 arg_int0(NULL, "all", "0/1" ,"Set all on or off");
    traceArgs.isr =                 arg_int0(NULL, "isr", "0/1" ,"ISR  Trace");
    traceArgs.status =              arg_int0(NULL, "status", "0/1" ,"Show Tracing options");
    traceArgs.rmaker =              arg_int0(NULL, "rmaker", "0/1" ,"Rmaker trace");
    traceArgs.timers =              arg_int0(NULL, "timers", "0/1" ,"Timers trace");
    traceArgs.show =                arg_int0(NULL, "show", "0/1" ,"Show traces");
    traceArgs.hal =                 arg_int0(NULL, "hal", "0/1" ,"Set Hal type");
    traceArgs.end =                 arg_end(10);

    logArgs.show =                  arg_int0(NULL, "show", "# of lines", "Show logs");
    logArgs.erase =                 arg_int0(NULL, "erase", "0/1" ,"Erase logs");
    logArgs.end =                   arg_end(2);

    timerArg.close =                arg_int0(NULL, "close", "0-255", "Time(sec) for Close Timeout");
    timerArg.cancel =               arg_int0(NULL, "opening", "0-255" ,"Time (sec) for Opening Timeout");
    timerArg.repeat =               arg_int0(NULL, "repeat", "0-255" ,"Time (sec) for Repeat Time");
    timerArg.relay =                arg_int0(NULL, "relay", "0-255" ,"Timew (sec) to Wait time for Auto Close");
    timerArg.relayw =               arg_int0(NULL, "relayw", "0-255" ,"Time(sec) out for HW failure closing");
    timerArg.waithal =              arg_int0(NULL, "WaitHal", "100-" ,"Time(ms) to wait before checking Hals again");
    timerArg.breaktimer =           arg_int0(NULL, "break", "1-" ,"Time(min) for break timer waiting");
    timerArg.pwmontimer =           arg_int0(NULL, "pwmon", "600-" ,"Time(us) for pwm timer ON interval");
    timerArg.pwmofftimer =          arg_int0(NULL, "pwmoff", "pwmON-" ,"Time(us) for pwm timer OFF interval");
    timerArg.pwmbreathtimer =       arg_int0(NULL, "pwmbreath", "1000-" ,"Time(ms) for pwm BREATH interval of 1 secojnd");
    timerArg.pwmcooltimer =         arg_int0(NULL, "pwmcool", "1000-" ,"Time(ms) for pwm Cool interval of 1 secojnd");
    timerArg.ifrbreak =             arg_int0(NULL, "ifrbreak", "1000-" ,"Time(ms) to consider an IFR Break");
    timerArg.frequency =             arg_int0(NULL, "frequency", "?" ,"Emitter Frequency");
    timerArg.end =                  arg_end(1);

    const esp_console_cmd_t duty_cmd = {
        .command = "dutypwm",
        .help = "Manage PWM Duty",
        .hint = NULL,
        .func = &cmdDuty,
        .argtable = &dutyArgs
    };

    const esp_console_cmd_t status_cmd = {
        .command = "statuspwm",
        .help = "PWM configuration and status",
        .hint = NULL,
        .func = &cmdStatusPWM,
        .argtable = NULL
    };

    const esp_console_cmd_t start_cmd = {
        .command = "startpwm",
        .help = "Start PWM",
        .hint = NULL,
        .func = &cmdStartPWM,
        .argtable = NULL
    };

    const esp_console_cmd_t trigger_cmd = {
        .command = "triggerpwm",
        .help = "Trigger Logizer PWM",
        .hint = NULL,
        .func = &cmdTriggerPWM,
        .argtable = NULL
    };

    const esp_console_cmd_t stop_cmd = {
        .command = "stoppwm",
        .help = "Stop PWM",
        .hint = NULL,
        .func = &cmdStopPWM,
        .argtable = NULL
    };

    const esp_console_cmd_t statusGarage_cmd = {
        .command = "statusdoor",
        .help = "Show door configuration and status",
        .hint = NULL,
        .func = &cmdStatusDoor,
        .argtable = NULL
    };

    const esp_console_cmd_t resetFactory_cmd = {
        .command = "factory",
        .help = "Reset to factory defaults",
        .hint = NULL,
        .func = &cmdReset,
        .argtable = NULL
    };

    const esp_console_cmd_t trace_cmd = {
        .command = "trace",
        .help = "Set tracing options",
        .hint = NULL,
        .func = &cmdTrace,
        .argtable = &traceArgs
    };

    const esp_console_cmd_t log_cmd = {
        .command = "log",
        .help = "Log options",
        .hint = NULL,
        .func = &cmdLog,
        .argtable = &logArgs
    };

    const esp_console_cmd_t timers_cmd = {
        .command = "timers",
        .help = "Timers setup",
        .hint = NULL,
        .func = &cmdTimers,
        .argtable = &timerArg
    };

    const esp_console_cmd_t config_cmd = {
        .command = "config",
        .help = "Show saved configuration",
        .hint = NULL,
        .func = &cmdConfig,
        .argtable = NULL
    };

    const esp_console_cmd_t eraseconfig_cmd = {
        .command = "erase",
        .help = "Erase configuration",
        .hint = NULL,
        .func = &cmdErase,
        .argtable = NULL
    };

    const esp_console_cmd_t onoff_cmd = {
        .command = "onoff",
        .help = "Start/Stop System",
        .hint = NULL,
        .func = &cmdOnoff,
        .argtable = &onoffArgs
    };


    ESP_ERROR_CHECK(esp_console_cmd_register(&duty_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&status_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&statusGarage_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&start_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&stop_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&resetFactory_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&config_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&trace_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&log_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&timers_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&eraseconfig_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&onoff_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&trigger_cmd));

   ESP_ERROR_CHECK(esp_console_start_repl(repl));
}