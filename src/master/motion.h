/**
 * @file motion.h
 * @brief Логика определения режима движения: LS, ACCEL, gyro → LightMode (по ТЗ п.8).
 */

#ifndef SMLS_MASTER_MOTION_H
#define SMLS_MASTER_MOTION_H

#include "common.h"
#include "sensors.h"

/** Параметры движения (линейная скорость, ускорение, счётчик оборотов). */
struct MotionParams {
    float ls;      ///< Линейная скорость (м/с)
    float accel;   ///< Линейное ускорение (м/с²)
    uint32_t cnt;  ///< Количество оборотов (импульсов Холла)
    float odom;    ///< Суммарный пробег (м)
};

/** Инициализация (обнуление счётчиков). */
void motionInit();

/** Обновить параметры: deltaMs — интервал между импульсами Холла (мс), 0 = нет импульса. */
void motionUpdate(uint32_t deltaMs);

/** Получить текущие параметры движения. */
void motionGetParams(MotionParams* out);

/** По данным датчиков и джойстика вернуть текущий режим ленты (джойстик имеет приоритет 30 сек). */
LightMode motionGetMode(const MpuData* mpu, JoystickDir joy, bool joystickActive);

#endif /* SMLS_MASTER_MOTION_H */
