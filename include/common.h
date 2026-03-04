/**
 * @file common.h
 * @brief Общие типы и константы для SMLS (Master и LED Controller).
 * Режимы световой индикации и структура пакета ESP-NOW.
 */

#ifndef SMLS_COMMON_H
#define SMLS_COMMON_H

#include <stdint.h>

/** Режимы светодиодной ленты (совпадают с командами ESP-NOW). */
enum class LightMode : uint8_t {
    HALT,       ///< Остановка (LS <= 2.0)
    GO,         ///< Движение вперёд
    STOP,       ///< Торможение
    LEFT,       ///< Влево
    RIGHT,      ///< Вправо
    GO_LEFT,    ///< Движение влево
    GO_RIGHT,   ///< Движение вправо
    STOP_LEFT,  ///< Торможение с поворотом влево
    STOP_RIGHT, ///< Торможение с поворотом вправо
    SOS,        ///< Режим SOS (джойстик/кнопка)
    TEST        ///< Тестовый режим (мигание при старте)
};

/** Пакет для передачи по ESP-NOW от Master к LED Controller. */
struct EspNowPacket {
    LightMode mode;
};

/** Количество светодиодов на ленте WS2812B (по ТЗ). */
constexpr uint16_t LED_STRIP_COUNT = 212;

/** Диаметр колеса в метрах (для расчёта линейной скорости по датчику Холла). */
constexpr float WHEEL_DIAMETER_M = 0.65f;

/** Порог скорости (м/с) ниже которого считается HALT. */
constexpr float SPEED_HALT_THRESHOLD = 2.0f;

/** Порог ускорения (м/с²) для определения торможения. */
constexpr float ACCEL_STOP_THRESHOLD = -0.5f;

/** Порог угловой скорости (град/с) для определения поворота. */
constexpr float GYRO_TURN_THRESHOLD = 2.0f;

/** Длительность TEST режима при старте (секунды). */
constexpr uint32_t TEST_MODE_DURATION_SEC = 45;

/** Цвет в TEST режиме (RGB). */
constexpr uint8_t TEST_COLOR_R = 202;
constexpr uint8_t TEST_COLOR_G = 82;
constexpr uint8_t TEST_COLOR_B = 242;

/** Время приоритета джойстика над авто-режимом (секунды). */
constexpr uint32_t JOYSTICK_PRIORITY_SEC = 30;

/** Время удержания джойстика для SOS (мс). */
constexpr uint32_t JOYSTICK_SOS_HOLD_MS = 200;

#endif /* SMLS_COMMON_H */
