#include "oled.h"
#include <WiFi.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "definitions.h"
#include "font.h"
#include "images.h"
// #include "utils.h"
// #include "network.h"

SSD1306Wire display(0x3c, 5, 4);
OLEDDisplayUi ui(&display);

void timeOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(Monospaced_plain_10);
  display->drawString(128, 0, timeClient.getFormattedTime());
}

void titleOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Monospaced_plain_10);
  display->drawString(0, 0, (char*)state->userData);
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  state->userData = (void *) "Sensors";
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(Monospaced_plain_10);
  display->drawString(0 + x, 10 + y, "Temp:      xx.xx °c ");
  display->drawString(0 + x, 20 + y, "Humidity:     xx %rH");
  display->drawString(0 + x, 30 + y, "Pressure:   xxxx hPa");
  display->drawString(0 + x, 40 + y, "Light:    45xxxx lux");
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
OverlayCallback overlays[] = { timeOverlay, titleOverlay };
int OVERLAY_COUNT = sizeof(overlays) / sizeof(OverlayCallback);


void initOled () {
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  ui.setIndicatorPosition(BOTTOM);

  ui.setIndicatorDirection(LEFT_RIGHT);

  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, FRAME_COUNT);

  ui.setOverlays(overlays, OVERLAY_COUNT);

  ui.init();

  // If needed
  display.flipScreenVertically();

  ui.runLoadingProcess(loadingStages, LOADING_STAGES_COUNT);
}

void updateOled() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}
