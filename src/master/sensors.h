/**
 * @file sensors.h
 * @brief Чтение датчиков: 2× MPU6050, датчик Холла KY-024, джойстик KY-023.
 */

#ifndef SMLS_MASTER_SENSORS_H
#define SMLS_MASTER_SENSORS_H

#include "common.h"
#include <cstdint>

/** Данные с одного MPU6050 (углы в градусах, угловая скорость град/с). */
struct MpuData {
    float roll;   ///< Наклон по оси X (град)
    float yaw;    ///< Поворот по оси Z (град)
    float gyroX;  ///< Угловая скорость X (град/с)
    float gyroZ;  ///< Угловая скорость Z (град/с)
};

/** Состояние джойстика (по ТЗ: вперёд=GO, назад=STOP, влево=LEFT, вправо=RIGHT, нажатие=SOS). */
enum class JoystickDir : uint8_t {
    None, Forward, Back, Left, Right, Press
};

/** Инициализация всех датчиков. Возвращает true при успехе. */
bool sensorsInit();

/** Калибровка обоих MPU6050 (офсеты сохраняются в LittleFS). */
void sensorsCalibrateMpu();

/** Чтение двух MPU6050. out1, out2 — указатели на структуры (могут быть nullptr). */
void sensorsReadMpu(MpuData* out1, MpuData* out2);

/** Чтение датчика Холла: время между импульсами в мс (0 = нет импульса). Возвращает true при новом импульсе. */
bool sensorsReadHall(uint32_t* deltaMs);

/** Линейная скорость по Холлу (м/с), расчёт из deltaMs. */
float sensorsHallToSpeedMs(uint32_t deltaMs);

/** Текущее направление джойстика. */
JoystickDir sensorsReadJoystick();

/** Чтение кнопок: [0]=BTN_1, [1]=BTN_2. true = нажата. */
void sensorsReadButtons(bool* btn1, bool* btn2);

#endif /* SMLS_MASTER_SENSORS_H */
