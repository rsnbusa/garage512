#ifndef INCLUDES_H
#define INCLUDES_H

#include <string.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_scenes.h>
#include <esp_rmaker_console.h>
#include <esp_rmaker_utils.h>
#include <esp_rmaker_common_events.h>
#include "esp_wifi.h"
#include <app_wifi.h>
#include <esp_spiffs.h>
#include "esp_timer.h"
#include "cJSON.h"
#include "driver/ledc.h"
#include "math.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>
#include "esp_idf_version.h"
#include "soc/gpio_reg.h"       //for direct port control

extern "C"{
#include "u8g2.h"
#include "u8g2_esp32_hal.h"
}
#include "esp_ota_ops.h"

#endif
