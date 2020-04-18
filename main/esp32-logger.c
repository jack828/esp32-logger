#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <sdkconfig.h>
#include "../definitions.h"


void app_main(void) {
  /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
     muxed to GPIO on reset already, but some default to other
     functions and need to be switched to GPIO. Consult the
     Technical Reference for a list of pads and their default
     functions.)
  */
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
