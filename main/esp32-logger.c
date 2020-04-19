#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"

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

#define EXAMPLE_WIFI_SSID WIFI_SSID
#define EXAMPLE_WIFI_PASS WIFI_PSK

#define WEB_SERVER ""
#define WEB_PORT "1000"
#define WEB_URL ""
#define WEB_AUTH ""

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

static esp_ip4_addr_t s_ip_addr;

const int READY_BIT = BIT0;

static const char *TAG = "[NODE]";
static const char *REQUEST_HEADER =
    "POST " WEB_URL " HTTP/1.0\r\n"
    "Host: " WEB_SERVER "\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n" ;

static EventGroupHandle_t wifi_event_group;

/* LECAGY, FOR REFERENCE
static esp_err_t event_handler(void *ctx, system_event_t *event) {
  switch (event->event_id) {
  case WIFI_EVENT_WIFI_READY:
  //case IP_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "WIFI_EVENT_WIFI_READY / IP_EVENT_STA_GOT_IP");
    ESP_LOGI(TAG, "Got ip...");
    xEventGroupSetBits(wifi_event_group, READY_BIT);
    break;
  case WIFI_EVENT_STA_START:
    ESP_LOGI(TAG, "WIFI_EVENT_STA_START, connecting");
    esp_wifi_connect();
    break;
  case WIFI_EVENT_STA_STOP:
    ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP");
    break;
    break;
  default:
    ESP_LOGI(TAG, "unknown event %d", event->event_id);
    break;
  }
  return ESP_OK;
} */

static void all_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s: all_event_handler, id: %d", base, id);
    /* ESP_LOGI(TAG, event_data); */
}

static void on_any_wifi(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s: on_any_wifi, id: %d", base, id);
    /* ESP_LOGI(TAG, "event_id: $d", event_data.event_id); */
}

static void on_any_ip(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s: on_any_ip, id: %d", base, id);
    /* ESP_LOGI(TAG, event_data); */
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "STA_DISCONNECTED Wi-Fi disconnected, trying to reconnect...");
    xEventGroupClearBits(wifi_event_group, READY_BIT);
    esp_err_t err = esp_wifi_connect();
    ESP_ERROR_CHECK(err);
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        ESP_LOGI(TAG, "WIFI NOT STARTED");
        return;
    }
}

static void on_wifi_stop(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP ?");
    xEventGroupClearBits(wifi_event_group, READY_BIT);
    esp_err_t err = esp_wifi_connect();
    ESP_ERROR_CHECK(err);
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        ESP_LOGI(TAG, "WIFI NOT STARTED");
        return;
    }
}

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Got IP event!");
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
    xEventGroupSetBits(wifi_event_group, READY_BIT);
}

static void wifi_init(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_wifi_set_default_wifi_sta_handlers();
  // TODO
  wifi_event_group = xEventGroupCreate();
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
      WIFI_EVENT,
      WIFI_EVENT_WIFI_READY,
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
  /* ESP_ERROR_CHECK(esp_wifi_connect()); */
}

static void communicate(struct esp_tls *tls, char *request_body) {
  char buf[1024];
  int ret, len;
  size_t dummy;

  /* sprintf(buf, "%sContent-Length:%d\r\nAuthorization:Basic %s\r\n%s", */
    /* REQUEST_HEADER, */
    /* strlen(request_body), */
    /* base64_encode((const unsigned char*)WEB_AUTH, */
        /* strlen(WEB_AUTH), &dummy), */
    /* request_body); */

  // ESP_LOGI(TAG, "request=%s", buf);
  ESP_LOGI(TAG, "request_body=%s", request_body);
  /* TLS write */
  size_t written_bytes = 0;
  do {
    ret = esp_tls_conn_write(tls, buf + written_bytes,
                             strlen(buf) - written_bytes);
    if (ret >= 0) {
      ESP_LOGI(TAG, "%d bytes written", ret);
      written_bytes += ret;
    } else if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
               ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
      return;
    }
  } while (written_bytes < strlen(buf));

  /* TLS read */
  ESP_LOGI(TAG, "Reading HTTP response...");

  do {
    len = sizeof(buf) - 1;
    bzero(buf, sizeof(buf));
    ret = esp_tls_conn_read(tls, (char *)buf, len);

    if (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ) {
      continue;
    }

    if (ret < 0) {
      ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
      break;
    }

    if (ret == 0) {
      ESP_LOGI(TAG, "connection closed");
      break;
    }

    len = ret;
    ESP_LOGD(TAG, "%d bytes read", len);

    for (int i = 0; i < len; i++) {
      putchar(buf[i]);
    }
  } while (1);
}

static double measure_chip_temp() {
  return ((double)temprature_sens_read() - 32.0) / 1.8;
}

static void build_request_body(char *buf) {
  sprintf(buf, "{\"esp32\":{\"temperature\":%f}}", measure_chip_temp());
}

static void https_request(void *ignore) {

  char request_body[256];

  /* Wait for the callback to set the READY_BIT in the
     event group.
   */
  ESP_LOGI(TAG, "https_request waitBits");
  xEventGroupWaitBits(
    wifi_event_group,
    READY_BIT,
    false,
    true,
    portMAX_DELAY
  );
  ESP_LOGI(TAG, "Connected to AP");

  /* TLS connection */
  return;
    /*
  struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);

  if (tls != NULL) {
    ESP_LOGI(TAG, "Connection established...");
    build_request_body(request_body);
    communicate(tls, request_body);
  } else {
    ESP_LOGE(TAG, "Connection failed...");
  }
  esp_tls_conn_delete(tls);
  */
}

static void measure_loop(void *pvParameters) {
  ESP_LOGI(TAG, "measure_loop");
  https_request(pvParameters);
  led_blink();
  vTaskDelay(60000 / portTICK_PERIOD_MS);
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
  xEventGroupWaitBits(wifi_event_group, READY_BIT, true, true, portMAX_DELAY);
  /* ESP_LOGI(TAG, "Connected to %s", s_connection_name); */
  ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&s_ip_addr));

  ESP_LOGI(TAG, "\n\ninitialise_wifi done!!!, looping!!!\n\n\n");

  xTaskCreate(&task_measure_loop, "measure_loop", 1024 * 16, NULL, 6, NULL);
}
