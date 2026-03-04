/**
 * @file display.h
 * @brief Вывод на OLED SSD1306 128×64: LS, ACCEL, CNT, режим (по ТЗ п.9).
 */

#ifndef SMLS_MASTER_DISPLAY_H
#define SMLS_MASTER_DISPLAY_H

#include "common.h"
#include "motion.h"

/** Инициализация дисплея. Возвращает true при успехе. */
bool displayInit();

/** Обновить экран: параметры движения, режим, префикс Dj_ при управлении джойстиком. */
void displayUpdate(const MotionParams* params, LightMode mode, bool joystickActive);

/** Показать сообщение при калибровке. */
void displayShowCalibrating();

/** Показать статус ФС: "FS READY" / "FS ERROR". */
void displayShowFsStatus(bool ready);

#endif /* SMLS_MASTER_DISPLAY_H */
