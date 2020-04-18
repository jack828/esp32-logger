#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <sdkconfig.h>
#include "../definitions.h"

void app_main(void) {
  printf("LED_PIN %d", LED_PIN);
  gpio_pad_select_gpio(LED_PIN);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  while(1) {
    /* Blink off (output low) */
    printf("Turning off the LED\n");
    gpio_set_level(LED_PIN, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    /* Blink on (output high) */
    printf("Turning on the LED\n");
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
