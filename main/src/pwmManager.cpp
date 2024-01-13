#define GLOBAL
#include "globals.h"

extern void pwm_timer_callback(void* pArg);
extern void sample_timer_callback(void* pArg);

void pwm_init(void)
{
  //PWM pulse controller... see documentation regarding timings in .pdf in project
    pwmtimer=0;
     esp_timer_create_args_t onperiod_timer_args = {
            .callback = &pwm_timer_callback,
            .arg = NULL,           
            .name = "pwmpulse"
    };
    ESP_ERROR_CHECK(esp_timer_create(&onperiod_timer_args, &pwm_timer));

     esp_timer_create_args_t sample_timer_args = {
            .callback = &sample_timer_callback,
            .arg = NULL,        
            .name = "samplepwm"
    };
    ESP_ERROR_CHECK(esp_timer_create(&sample_timer_args, &sample_timer));

    maxDuty=(pow(2,int(LEDC_DUTY_RES))-1);
    vduty=(pow(2,int(LEDC_DUTY_RES))-1)*(float)(theConf.ciclo/100.0);
    float freq_width=1.0/theConf.timers[TFREQ]*1000000;
    float mins=freq_width*KMINPULSE;        //base of sample timer, need to add a positive offset to get the sample at on duty cycle to be sure
    minSample=(int)(mins+freq_width*theConf.ciclo/100.0/2.0);  // frequency ON widht /2 is our offset
    // printf("MinSample %ld cicle %d KMIN %d\n",minSample,theConf.ciclo,KMINPULSE);

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = theConf.timers[TFREQ],      //should be 38K or 56K
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = LEDC_OUTPUT_IO,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, 
        .hpoint         = 0
    };

  int ret=ledc_channel_config(&ledc_channel);
  if(ret!=ESP_OK)
  {
#ifdef DEBB
    printf("%serror setting LEDC configuration %x\n",SYSTEM,ret);
#endif
    return;
  };

  ret=ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
  if(ret!=ESP_OK)
  {
#ifdef DEBB
    printf("%serror setting LEDC Mode %x\n",SYSTEM,ret);
#endif
    return;
  };
ret=ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
  if(ret!=ESP_OK)
  {
#ifdef DEBB
    printf("%serror setting LEDC Duty %x\n",SYSTEM,ret);
#endif

    return;
  };

    //PWM active with duty 0%. Change duty to create pulses. 11bit timer REMEMBER so 2**11-1 * duty %
}