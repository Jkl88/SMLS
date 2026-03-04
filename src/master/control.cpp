/**
 * @file control.cpp
 * @brief Реализация единых функций управления (кнопки, джойстик, WEB).
 */

#include "control.h"
#include "storage.h"
#include "sensors.h"
#include "display.h"
#include "espnow_tx.h"
#include "config.h"
#include <Arduino.h>

static LightMode s_manualMode = LightMode::HALT;
static uint32_t s_manualUntil = 0;
static MotionParams s_lastParams = {0};
static LightMode s_lastMode = LightMode::HALT;
static bool s_fsReady = false;

void controlSetManualMode(LightMode mode) {
    s_manualMode = mode;
    s_manualUntil = millis() + (JOYSTICK_PRIORITY_SEC * 1000u);
}

void controlTriggerCalibrate() {
    sensorsCalibrateMpu();
}

void controlSetLogging(bool enable) {
    storageSetLogging(enable);
}

void controlGetStatus(MotionParams* params, LightMode* mode, bool* fsReady, bool* logging) {
    if (params) *params = s_lastParams;
    if (mode) *mode = s_lastMode;
    if (fsReady) *fsReady = s_fsReady;
    if (logging) *logging = storageIsLogging();
}

/** Вызывается из main loop: обновить s_lastParams, s_lastMode, s_fsReady. */
void controlUpdateStatus(const MotionParams* params, LightMode mode, bool fsReady) {
    s_lastParams = params ? *params : MotionParams{0};
    s_lastMode = mode;
    s_fsReady = fsReady;
}

/** Проверить, активен ли ручной режим (джойстик/WEB). */
bool controlIsManualActive() {
    return millis() < s_manualUntil;
}

/** Текущий ручной режим. */
LightMode controlGetManualMode() {
    return s_manualMode;
}
