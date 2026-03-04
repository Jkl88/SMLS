/**
 * @file main.cpp
 * @brief Точка входа LED Controller: приём ESP-NOW, отрисовка ленты WS2812B ~60 FPS.
 */

#include "config.h"
#include "common.h"
#include "espnow_rx.h"
#include "led_effects.h"
#include <Arduino.h>

/** Задача: отрисовка ленты по текущему режиму ~60 FPS (ядро 1). */
void taskLED(void* arg) {
    const TickType_t period = pdMS_TO_TICKS(16);
    for (;;) {
        LightMode mode = espnowRxGetMode();
        ledEffectsTick(mode);
        vTaskDelay(period);
    }
}

void setup() {
    Serial.begin(115200);
    espnowRxInit();
    ledEffectsInit();
    xTaskCreatePinnedToCore(taskLED, "led", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
