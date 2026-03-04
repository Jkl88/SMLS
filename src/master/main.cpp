/**
 * @file main.cpp
 * @brief Точка входа Master: инициализация, FreeRTOS-задачи (датчики, движение, дисплей, хранилище, WEB).
 */

#include "config.h"
#include "common.h"
#include "storage.h"
#include "sensors.h"
#include "motion.h"
#include "display.h"
#include "espnow_tx.h"
#include "webserver.h"
#include "control.h"
#include <Arduino.h>
#include <WiFi.h>

static MpuData s_mpu1, s_mpu2;
static MotionParams s_params;
static LightMode s_currentMode = LightMode::TEST;
static volatile bool s_calibrating = false;

/** Задача: опрос датчиков ~200 Гц (ядро 1). */
void taskSensors(void*) {
    const TickType_t period = pdMS_TO_TICKS(5);
    uint32_t lastHall = 0;
    for (;;) {
        sensorsReadMpu(&s_mpu1, &s_mpu2);
        uint32_t deltaMs;
        if (sensorsReadHall(&deltaMs))
            motionUpdate(deltaMs);
        else
            motionUpdate(0);
        motionGetParams(&s_params);
        vTaskDelay(period);
    }
}

/** Задача: расчёт режима и отправка ESP-NOW при смене (ядро 1). */
void taskMotion(void*) {
    const TickType_t period = pdMS_TO_TICKS(10);
    for (;;) {
        if (s_calibrating) {
            vTaskDelay(period);
            continue;
        }
        JoystickDir joy = sensorsReadJoystick();
        bool manual = controlIsManualActive();
        LightMode mode = manual ? controlGetManualMode() : motionGetMode(&s_mpu1, joy, false);
        if (!manual && joy != JoystickDir::None) {
            if (joy == JoystickDir::Forward) controlSetManualMode(LightMode::GO);
            else if (joy == JoystickDir::Back) controlSetManualMode(LightMode::STOP);
            else if (joy == JoystickDir::Left) controlSetManualMode(LightMode::LEFT);
            else if (joy == JoystickDir::Right) controlSetManualMode(LightMode::RIGHT);
            else if (joy == JoystickDir::Press) controlSetManualMode(LightMode::SOS);
            mode = controlGetManualMode();
        }
        s_currentMode = mode;
        controlUpdateStatus(&s_params, mode, storageReady());
        espnowTxSend(mode);
        vTaskDelay(period);
    }
}

/** Задача: обновление OLED ~15 Гц (ядро 0). */
void taskDisplay(void*) {
    const TickType_t period = pdMS_TO_TICKS(66);
    for (;;) {
        if (s_calibrating)
            displayShowCalibrating();
        else
            displayUpdate(&s_params, s_currentMode, controlIsManualActive());
        vTaskDelay(period);
    }
}

/** Задача: запись логов в LittleFS ~5 Гц (ядро 0). */
void taskStorage(void*) {
    const TickType_t period = pdMS_TO_TICKS(200);
    uint32_t lastLog = 0;
    for (;;) {
        if (storageReady() && storageIsLogging() && !s_calibrating)
            storageLogTrack(millis() / 1000, s_params.ls, s_params.accel, s_params.cnt, s_currentMode);
        vTaskDelay(period);
    }
}

/** Обработка кнопок: калибровка по BTN_CALIB. */
void taskButtons(void*) {
    const TickType_t period = pdMS_TO_TICKS(50);
    for (;;) {
        bool btn1 = false, btn2 = false;
        sensorsReadButtons(&btn1, &btn2);
        if (btn1) {
            s_calibrating = true;
            displayShowCalibrating();
            digitalWrite(PIN_LED_1, HIGH);
            sensorsCalibrateMpu();
            delay(500);
            digitalWrite(PIN_LED_1, LOW);
            s_calibrating = false;
        }
        vTaskDelay(period);
    }
}

void setup() {
    Serial.begin(115200);
    if (!storageInit()) {
        if (displayInit())
            displayShowFsStatus(false);
        for (;;) delay(1000);
    }
    if (!sensorsInit()) {
        for (;;) delay(1000);
    }
    displayInit();
    displayShowFsStatus(storageReady());
    delay(1000);
    motionInit();
    espnowTxInit();
#if defined(USE_TKWIFIMANAGER)
    /* TKWifiManager: подключение к WiFi (если библиотека подключена). */
    /* WiFi подключение здесь или через портал — затем webserverStart(). */
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    delay(2000);
    webserverStart();
    xTaskCreatePinnedToCore(taskSensors, "sensors", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(taskMotion, "motion", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(taskDisplay, "display", 4096, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(taskStorage, "storage", 2048, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(taskButtons, "buttons", 2048, nullptr, 1, nullptr, 0);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
