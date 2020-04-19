#include <stdlib.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#include "../definitions.h"
#include "wifi.h"
#include "http.h"

EventGroupHandle_t wifi_event_group;

esp_ip4_addr_t ip_addr;
esp_netif_t *netif = NULL;

const int8_t READY_BIT = BIT0;

static void led_blink() {
  gpio_set_level(LED_PIN, 1);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  gpio_set_level(LED_PIN, 0);
}

static void led_pin_init() {
  gpio_pad_select_gpio(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

static const char *TAG = "[NODE]";

static void measure_loop(void *pvParameters) {
  ESP_LOGI(TAG, "measure_loop");
  get_request(pvParameters);
  led_blink();
  vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
}

static void task_measure_loop(void *pvParameters) {
  ESP_LOGI(TAG, "task_measure_loop");
  while (true) {
    measure_loop(pvParameters);
  }
  ESP_LOGI(TAG, "deleting task?");
  vTaskDelete(NULL);
}

void app_main(void) {
  /* ESP_LOGI(TAG, "" +PROJECT_VER +""); */
  led_pin_init();
  led_blink();
  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init();

  ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_stop));
  ESP_LOGI(TAG, "wifi init done, waiting for IP");
  xEventGroupWaitBits(wifi_event_group, READY_BIT, false, true, portMAX_DELAY);
  ESP_LOGI(TAG, "Connected WiFi"); // TODO SSID from flash?
  ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&ip_addr));

  xTaskCreate(&task_measure_loop, "measure_loop", 1024 * 16, NULL, 6, NULL);
}
