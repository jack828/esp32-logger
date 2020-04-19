#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_netif.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "nvs_flash.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#include "../definitions.h"

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

static esp_ip4_addr_t ip_addr;
esp_netif_t *netif = NULL;

const int READY_BIT = BIT0;

static const char *TAG = "[NODE]";

static EventGroupHandle_t wifi_event_group;

static void all_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s: all_event_handler, id: %d", base, id);
}

static void on_any_wifi(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s: on_any_wifi, id: %d", base, id);
}

static void on_any_ip(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s: on_any_ip, id: %d", base, id);
}

static void wifi_connect() {
  esp_err_t err = esp_wifi_connect();
  if (err == ESP_ERR_WIFI_NOT_STARTED) {
      ESP_LOGI(TAG, "WIFI NOT STARTED");
      return;
  }
  ESP_ERROR_CHECK(err);
}
static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  ESP_LOGI(TAG, "STA_DISCONNECTED Wi-Fi disconnected, trying to reconnect...");
  xEventGroupClearBits(wifi_event_group, READY_BIT);
  wifi_connect();
}

static void on_wifi_start(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
  wifi_connect();
}

static void on_wifi_stop(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP ?");
  xEventGroupClearBits(wifi_event_group, READY_BIT);
  wifi_connect();
}

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Got IP event!");
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&ip_addr, &event->ip_info.ip, sizeof(ip_addr));
    xEventGroupSetBits(wifi_event_group, READY_BIT);
}

static void wifi_init(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  netif = esp_netif_create_default_wifi_sta();
  assert(netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_wifi_set_default_wifi_sta_handlers();
  // TODO
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
    esp_wifi_set_storage(WIFI_STORAGE_RAM)
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

static double measure_chip_temp() {
  return ((double)temprature_sens_read() - 32.0) / 1.8;
}

static void build_request_body(char *buf) {
  sprintf(buf, "{\"esp32\":{\"temperature\":%f}}", measure_chip_temp());
}

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
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
                printf("%.*s", evt->data_len, (char*)evt->data);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }

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

  char request_body[256];
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
   .url = "http://wirepusher.com/send?id=SNIPPED&title=test&message=hello_from_esp32",
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
  led_pin_init();
  led_blink();
  ESP_ERROR_CHECK(nvs_flash_init());

  /* if (event_handler != NULL) { */
    /* return ESP_ERR_INVALID_STATE; */
  /* } */
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  led_blink();
  wifi_init();
  led_blink();
  /* ESP_ERROR_CHECK(esp_register_shutdown_handler(&stop)); */
  ESP_LOGI(TAG, "wifi init done, waiting for IP");
  xEventGroupWaitBits(wifi_event_group, READY_BIT, false, true, portMAX_DELAY);
  /* ESP_LOGI(TAG, "Connected to %s", s_connection_name); */
  ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&ip_addr));

  ESP_LOGI(TAG, "\n\ninitialise_wifi done!!!, looping!!!\n\n\n");

  xTaskCreate(&task_measure_loop, "measure_loop", 1024 * 16, NULL, 6, NULL);
}
