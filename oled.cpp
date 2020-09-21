#include "oled.h"
#include <WiFi.h>
#include <Wire.h>
#include <WString.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "definitions.h"
#include "font.h"
#include "images.h"
// #include "utils.h"
// #include "network.h"

#ifdef TOUCH_R_PIN
volatile unsigned long touchRTimer;
volatile int touchR;
volatile unsigned long touchLTimer;
volatile int touchL;
volatile unsigned int touchDelayTime = 100;
int touchThreshold = 50;
#endif

SSD1306Wire display(0x3c, 5, 4);
OLEDDisplayUi ui(&display);

void titleOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Monospaced_plain_10);
  display->drawString(0, 0, (char*)state->userData);
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  state->userData = (void *) "Sensors";
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Monospaced_plain_10);
  // char * tempStr[6];
  // sprintf(tempStr, "%2.2f", temperature);
  int line = y;
#ifdef BME680_I2C
  display->drawString(0 + x, line += 10, "Temp:      " + String(temperature, 2) + " °c ");
  display->drawString(0 + x, line += 10, "Humidity:  " + String(humidity, 2)    + " %rH");
  display->drawString(0 + x, line += 10, "Pressure:  " + String(pressure, 0)    + " hPa");
  display->drawString(0 + x, line += 10, "Air Qual:  " + String(airQuality, 2)  + " %  ");
  display->drawString(0 + x, line += 10, "VPD:       " + String(vpd, 2)         + " kPa");
#endif
#ifdef BME280_I2C
  display->drawString(0 + x, line += 10, "Temp:      " + String(temperature, 2) + " °c ");
  display->drawString(0 + x, line += 10, "Humidity:  " + String(humidity, 2)    + " %rH");
  display->drawString(0 + x, line += 10, "Pressure:  " + String(pressure, 0)    + " hPa");
  display->drawString(0 + x, line += 10, "VPD:       " + String(vpd, 2)         + " kPa");
#endif
  // display->drawString(0 + x, line += 10, "Light:    45xxxx lux");
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  state->userData = (void *) "Something";
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Monospaced_plain_10);
  int line = y;
  display->drawString(0 + x, line += 10, "hello, world °c ");
}

LoadingStage loadingStages[] = {
  {
    .process = "Connecting to WiFi",
    .callback = []() {
      Serial.println("[ OLED ] wifi wait");
      delay(500);
    }
  }
};

int LOADING_STAGES_COUNT = sizeof(loadingStages) / sizeof(LoadingStage);

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1 };

// how many frames are there?
int FRAME_COUNT = sizeof(frames) / sizeof(FrameCallback);

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { titleOverlay };
int OVERLAY_COUNT = sizeof(overlays) / sizeof(OverlayCallback);

#ifdef TOUCH_R_PIN
void IRAM_ATTR handleTouchR(){
  cli();
  int reading = touchRead(TOUCH_R_PIN);

  if (reading > touchThreshold) return;

  if (touchR += (millis() - touchRTimer) >= (touchDelayTime)) {
    touchRTimer = millis();
    Serial.println("counting touch?");
  }
  sei();
}

void IRAM_ATTR handleTouchL(){
  int reading = touchRead(TOUCH_L_PIN);

  if (reading > touchThreshold) return;

  if (touchL += (millis() - touchLTimer) >= (touchDelayTime)) {
    touchLTimer = millis();
    Serial.println("L");
  }
}

void IRAM_ATTR attachTouchInterrupts() {
  touchAttachInterrupt(TOUCH_R_PIN, handleTouchR, touchThreshold);
  touchAttachInterrupt(TOUCH_L_PIN, handleTouchL, touchThreshold);
}
#endif

void initOled () {
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);
  if (FRAME_COUNT == 1) {
    ui.disableAutoTransition();
    ui.disableAllIndicators();
  } else {
    ui.setActiveSymbol(activeSymbol);
    ui.setInactiveSymbol(inactiveSymbol);

    ui.setIndicatorPosition(BOTTOM);

    ui.setIndicatorDirection(LEFT_RIGHT);

    ui.setFrameAnimation(SLIDE_LEFT);
    ui.disableAutoTransition();

#ifdef TOUCH_R_PIN
    attachTouchInterrupts();
#endif
  }

  ui.setFrames(frames, FRAME_COUNT);

  ui.setOverlays(overlays, OVERLAY_COUNT);

  ui.init();

#ifdef OLED_FLIP_V
  display.flipScreenVertically();
#endif

  ui.runLoadingProcess(loadingStages, LOADING_STAGES_COUNT);
}

void updateOled() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    // doscreentick

#ifdef TOUCH_R_PIN
    if (touchR > 0) {
      Serial.println("\nTouched R\n");
      touchR = 0;
      ui.nextFrame();
    }

    if (touchL > 0) {
      Serial.println("\nTouched L\n");
      touchL = 0;
      ui.previousFrame();
    }
#endif

    delay(remainingTimeBudget);
  }
}

void clearOled() {
  display.displayOff();
}
