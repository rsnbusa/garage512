#ifndef FORWARDS_H
#define FORWARDS_H

#include "includes.h"         //for this file appearence only
void closed_state(void* arg);
void opening_state(void* arg);
void opened_state(void* arg);
void closing_state(void* arg);
void unknown_state(void* arg);
void offstate_state(void *pArg);
void power_task(void *pArg);
void countDown(void *pArg);
void ifrTest(void *pArg);

void kbd();
void pwm_init();
void writeLog(char * que);
void setIFR(uint8_t como);
void ssdString(int x, int y, char * que,bool centerf);

void launchBlink(int interval);
void delay(uint32_t cuanto);
uint32_t millisFromISR();
void startStopTimer(bool action, TimerHandle_t timer);
int8_t getIfrSensor();
bool confirm_hal(bool closeh, bool openh);
void relayPulse(uint32_t cuanto);
void turnLight(bool como);
uint32_t millis();
void  setPower(uint8_t how);
void write_to_flash(); //save our configuration
void read_flash();
void erase_config();
void init_conf();
void init_rmaker_device();
void halISR(void* arg);
void timermgr(TimerHandle_t xTimer);
esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx);
void coolMgr(TimerHandle_t xTimer);      //used for cooling phase
void startStopTimer(bool action, TimerHandle_t timer);
void logFileInit();
void writeLog(char * que);

int remote_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int wait_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int reset_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int sensor_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int guard_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int closebreak_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int keepopen_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int sampler_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int dutycycle_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int name_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int hstart_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int htime_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int ptest_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int onoff_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int align_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);
int breath_cmd(const esp_rmaker_param_t *param,esp_rmaker_param_val_t val);

extern "C" {
  void app_main();
}
#endif
