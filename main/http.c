#include "http.h"
#include "../definitions.h"
#include "string.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "node.h"

static const char *TAG = "[HTTP]";

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

void get_request(node_sensor_t sensor, char* body) {
  ESP_LOGI(TAG, "http_request waitBits");
  xEventGroupWaitBits(
    wifi_event_group,
    READY_BIT,
    false,
    true,
    portMAX_DELAY
  );
  ESP_LOGI(TAG, "Connected to AP");

  char* rooturi=  "http://wirepusher.com/send?id=bdJ5mpnGj";
  char* request_uri[100];
  sprintf(request_uri, "%s&title=%s&message=%s", rooturi, sensor_to_string(sensor), body);
  esp_http_client_config_t config = {
   .url = request_uri,
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
