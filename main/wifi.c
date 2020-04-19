#include "wifi.h"
#include "../definitions.h"
#include "string.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

const char *TAG = "[WIFI]";

void wifi_connect() {
  /* led_blink(); */
  esp_err_t err = esp_wifi_connect();
  if (err == ESP_ERR_WIFI_NOT_STARTED) {
    ESP_LOGI(TAG, "WIFI NOT STARTED");
    return;
  }
  /* led_blink(); */
  ESP_ERROR_CHECK(err);
}

void on_any_wifi(void* arg, esp_event_base_t base, int32_t id, void* event_data) {
  ESP_LOGI(TAG, "%s: on_any_wifi, id: %d", base, id);
}

void on_any_ip(void* arg, esp_event_base_t base, int32_t id, void* event_data) {
  ESP_LOGI(TAG, "%s: on_any_ip, id: %d", base, id);
}

void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "STA_DISCONNECTED Wi-Fi disconnected, trying to reconnect...");
  xEventGroupClearBits(wifi_event_group, READY_BIT);
  wifi_connect();
}

void on_wifi_start(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
  wifi_connect();
}

void on_wifi_stop(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP ?");
  xEventGroupClearBits(wifi_event_group, READY_BIT);
  wifi_connect();
}

void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Got IP event!");
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  memcpy(&ip_addr, &event->ip_info.ip, sizeof(ip_addr));
  xEventGroupSetBits(wifi_event_group, READY_BIT);
}

void wifi_init(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  netif = esp_netif_create_default_wifi_sta();
  assert(netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_wifi_set_default_wifi_sta_handlers();

  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(
    esp_event_handler_register(
      WIFI_EVENT,
      WIFI_EVENT_STA_START,
      &on_wifi_start,
      NULL
    )
  );

  ESP_ERROR_CHECK(
    esp_event_handler_register(
      WIFI_EVENT,
      WIFI_EVENT_STA_CONNECTED,
      &on_wifi_start,
      NULL
    )
  );

  ESP_ERROR_CHECK(
    esp_event_handler_register(
      WIFI_EVENT,
      WIFI_EVENT_STA_DISCONNECTED,
      &on_wifi_disconnect,
      NULL
    )
  );

  ESP_ERROR_CHECK(
    esp_event_handler_register(
      IP_EVENT,
      IP_EVENT_STA_GOT_IP,
      &on_got_ip,
      NULL
    )
  );

  ESP_ERROR_CHECK(
    esp_event_handler_register(
      WIFI_EVENT,
      WIFI_EVENT_STA_STOP,
      &on_wifi_stop,
      NULL
    )
  );
  ESP_ERROR_CHECK(
    esp_event_handler_register(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      &on_any_wifi,
      NULL
    )
  );

  ESP_ERROR_CHECK(
    esp_event_handler_register(
      IP_EVENT,
      ESP_EVENT_ANY_ID,
      &on_any_ip,
      NULL
    )
  );
  ESP_ERROR_CHECK(
    esp_wifi_set_storage(WIFI_STORAGE_FLASH)
  );

  wifi_config_t wifi_config = {
    .sta =
      {
        .ssid = WIFI_SSID,
        .password = WIFI_PSK,
      }
  };
  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s:%s", wifi_config.sta.ssid, wifi_config.sta.password);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "WiFi started");
}

void wifi_stop() {
  esp_err_t err = esp_wifi_stop();
  if (err == ESP_ERR_WIFI_NOT_INIT) {
      return;
  }
  ESP_ERROR_CHECK(err);
  ESP_ERROR_CHECK(esp_wifi_deinit());
  ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(netif));
  esp_netif_destroy(netif);
}
