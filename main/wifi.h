#ifndef WIFI_H
#define WIFI_H

#include "esp_log.h"
#include "esp_wifi.h"

void wifi_connect(void);

void on_any_wifi(void* arg, esp_event_base_t base, int32_t id, void* event_data);
void on_any_ip(void* arg, esp_event_base_t base, int32_t id, void* event_data);
void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void on_wifi_start(void* arg, esp_event_base_t base, int32_t id, void* event_data);
void on_wifi_stop(void* arg, esp_event_base_t base, int32_t id, void* event_data);
void on_got_ip(void* arg, esp_event_base_t base, int32_t id, void* event_data);

void wifi_init(void);
void wifi_stop(void);

#endif
