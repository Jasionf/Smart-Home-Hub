#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

void wifi_init_sta(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#endif