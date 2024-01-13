#ifndef TYPES_H_
#include "includes.h"



typedef struct consoltrace{
    struct arg_int *task;
    struct arg_int *boot;
    struct arg_int *queue;
    struct arg_int *all;
    struct arg_int *isr;
    struct arg_int *status;
    struct arg_int *rmaker;
    struct arg_int *timers;
    struct arg_int *show;
    struct arg_int *hal;
    struct  arg_end *end;
} traceargs_t;

typedef struct logop{
    struct arg_int *show;
    struct arg_int *erase;
    struct  arg_end *end;
} logargs_t;

enum timerNames{TOPEN,TCLOSE,TREPEAT,TRELAY,TREPEATW,TDISPLAY,TBREAK,TCOOL,TPWMON,TPWMOFF,TBREATH,TIFR,TFREQ,TLAST};

typedef struct config {
    uint32_t    centinel;
    uint8_t     rel;
    uint32_t    waithal;
    bool        free0,guard;
    int         ciclo;
    char        name[20];
    bool        traceTask,traceBoot,traceCmdq,traceISR,traceRmaker,traceTimers;
    int         reboots;
    uint32_t    timers[16];
    uint8_t     free1,free2,free3,free4,free5;
    int         onoff,hstart,htime;
    int         sampler;
} config_flash;

typedef struct laserq {
    uint8_t cmd,state,reserved;
} lqueuecmd;

typedef struct consol{
    struct arg_int *paso;
    struct arg_int *porcentaje;
    struct  arg_end *end;
} dutyArgs_t;

typedef struct onff{
    struct arg_int *on_off;
    struct  arg_end *end;
} onoff_t;

typedef struct timercon{
    struct arg_int *close;
    struct arg_int *cancel;
    struct arg_int *repeat;
    struct arg_int *relay;
    struct arg_int *relayw;
    struct arg_int *waithal;
    struct arg_int *breaktimer;
    struct arg_int *pwmontimer;
    struct arg_int *pwmofftimer;
    struct arg_int *pwmbreathtimer;
    struct arg_int *pwmcooltimer;
    struct arg_int *ifrbreak;
    struct arg_int *frequency;
    struct arg_end *end;
} timercon_t;

typedef int (*functcmd)(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);

typedef struct cmdRecord{
    char 		comando[20];
    functcmd 	code;
}cmdRecord;

#endif