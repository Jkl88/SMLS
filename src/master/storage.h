/**
 * @file storage.h
 * @brief Работа с LittleFS: логи движения (track.csv), калибровка MPU6050, настройки.
 */

#ifndef SMLS_MASTER_STORAGE_H
#define SMLS_MASTER_STORAGE_H

#include "common.h"
#include <cstdint>
#include <cstddef>

/** Имя файла лога движения (по ТЗ). */
#define TRACK_FILE "/track.csv"

/** Максимальная длина строки в track.csv. */
#define TRACK_LINE_MAX 128

/** Инициализация LittleFS, при необходимости форматирование. Возвращает true при успехе. */
bool storageInit();

/** Проверка: смонтирована ли ФС. */
bool storageReady();

/** Записать одну строку телеметрии в track.csv (время, LS, ACCEL, CNT, режим). */
void storageLogTrack(uint32_t timeSec, float ls, float accel, uint32_t cnt, LightMode mode);

/** Сохранить офсеты калибровки MPU6050 во Flash (массив 6 int16 для одного датчика). */
void storageSaveCalibration(int mpuIndex, const int16_t* offsets);

/** Загрузить офсеты калибровки MPU6050 из Flash. Возвращает true, если данные есть. */
bool storageLoadCalibration(int mpuIndex, int16_t* offsets);

/** Включить/выключить запись логов (для WEB/кнопок). */
void storageSetLogging(bool enable);
bool storageIsLogging();

#endif /* SMLS_MASTER_STORAGE_H */
