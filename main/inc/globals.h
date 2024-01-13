#ifndef MAIN_GLOBALS_H
#define MAIN_GLOBALS_H

#ifdef GLOBAL
    #define EXTERN extern
#else
    #define EXTERN
#endif

#include "typedef.h"    
#include "defines.h"

EXTERN SemaphoreHandle_t 		dispSem;
EXTERN uint32_t                 WDOOR,startTime,loopcount,minSample;
EXTERN bool                     activo,laserState,blockrequest,CONTACT,NOCONTACT;
EXTERN TimerHandle_t             timers[13];
EXTERN gpio_config_t 	        io_conf;
EXTERN char                    *TAG ;
EXTERN config_flash             theConf;
EXTERN nvs_handle 		        nvshandle;
EXTERN esp_rmaker_param_val_t   mival;
EXTERN QueueHandle_t            lqueue,blockqueue;
EXTERN esp_rmaker_param_t       *pwr_param,*status_param,*timers_param,*wait_param,*ssid_param,*onoff_param,*hstart,*htime,*guard_param;
EXTERN TaskHandle_t             powerHandle,counterHandle,blinker,closedHandle,openingHandle,openedHandle,closingHandle,unknownHandle,fadeHandle,farrowHandle,obstructHandle,ifrtesthandle;
EXTERN EventGroupHandle_t       doorBits;
EXTERN uint8_t                  countBreak,sample,oldsample,gOnOff,countBlk;
EXTERN int                      maxDuty,vduty,perc,pwmtimer,KMINPULSE;
EXTERN dutyArgs_t               dutyArgs;
EXTERN traceargs_t              traceArgs;
EXTERN logargs_t                logArgs;
EXTERN onoff_t                  onoffArgs;
EXTERN FILE*                    myFile;
EXTERN TickType_t               lasttick;
EXTERN char                     temp[100];
EXTERN uint16_t                 stx,sty;
EXTERN timercon_t               timerArg;
EXTERN char                     timer_names[13][25];  
EXTERN char                     timer_names_abr[13][10];  
EXTERN char                     doorState[10][12],app_version[40];
EXTERN esp_timer_handle_t       pwm_timer,sample_timer;      
EXTERN TickType_t               lastTick;   
#ifdef DISPLAY  
EXTERN u8g2_t                   u8g2; // a structure which will contain all the data for one display
#endif
EXTERN cmdRecord                cmds[NUM_CMDS];
#endif
