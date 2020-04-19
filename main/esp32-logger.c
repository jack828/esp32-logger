#include <stdlib.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#include "../definitions.h"
#include "wifi.h"

EventGroupHandle_t wifi_event_group;

esp_ip4_addr_t ip_addr;
esp_netif_t *netif = NULL;

const int8_t READY_BIT = BIT0;

extern uint8_t temprature_sens_read();

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

static double measure_chip_temp() {
  return ((double)temprature_sens_read() - 32.0) / 1.8;
}

static void build_request_body(char *buf) {
  sprintf(buf, "{\"esp32\":{\"temperature\":%f}}", measure_chip_temp());
}

esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
  switch(evt->event_id) {
    case HTTP_EVENT_ERROR:
      ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
      printf("%.*s", evt->data_len, (char*)evt->data);
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      printf("%.*sz\n", evt->data_len, (char*)evt->data);
      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}

static void http_request(void *ignore) {
  ESP_LOGI(TAG, "http_request waitBits");
  xEventGroupWaitBits(
    wifi_event_group,
    READY_BIT,
    false,
    true,
    portMAX_DELAY
  );
  ESP_LOGI(TAG, "Connected to AP");
  esp_http_client_config_t config = {
   .url = "http://wirepusher.com/send?id=bdJ5mpnGj&title=test&message=hello_from_esp32",
   .event_handler = _http_event_handle,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK) {
   ESP_LOGI(TAG, "Status = %d, content_length = %d",
     esp_http_client_get_status_code(client),
     esp_http_client_get_content_length(client)
   );
  } else {
    ESP_LOGI(TAG, "Request NOT ok %d", err);
  }
  esp_http_client_cleanup(client);
}

static void measure_loop(void *pvParameters) {
  ESP_LOGI(TAG, "measure_loop");
  ESP_LOGI(TAG, "temp %f", measure_chip_temp());
  /* http_request(pvParameters); */
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
