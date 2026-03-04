/**
 * @file control.h
 * @brief Единые точки управления для кнопок, джойстика и WEB (по ТЗ п.10).
 */

#ifndef SMLS_MASTER_CONTROL_H
#define SMLS_MASTER_CONTROL_H

#include "common.h"
#include "motion.h"

/** Установить ручной режим (приоритет на JOYSTICK_PRIORITY_SEC). */
void controlSetManualMode(LightMode mode);

/** Запустить калибровку MPU (мигание LED, вывод на дисплей). */
void controlTriggerCalibrate();

/** Включить/выключить запись логов. */
void controlSetLogging(bool enable);

/** Получить текущее состояние для WEB API: params, mode, fsReady, logging. */
void controlGetStatus(MotionParams* params, LightMode* mode, bool* fsReady, bool* logging);

/** Обновить кэш состояния (вызывается из main). */
void controlUpdateStatus(const MotionParams* params, LightMode mode, bool fsReady);

/** Активен ли ручной режим (таймаут джойстика/WEB). */
bool controlIsManualActive();

/** Текущий ручной режим при активном ручном управлении. */
LightMode controlGetManualMode();

#endif /* SMLS_MASTER_CONTROL_H */
