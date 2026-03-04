/**
 * @file motion.cpp
 * @brief Реализация логики режимов: HALT/GO/STOP + повороты по gyroX.
 */

#include "motion.h"
#include "config.h"
#include <Arduino.h>

static MotionParams s_params = {0.f, 0.f, 0, 0.f};
static float s_prevLs = 0.f;
static uint32_t s_prevDeltaMs = 0;

void motionInit() {
    s_params = {0.f, 0.f, 0, 0.f};
    s_prevLs = 0.f;
    s_prevDeltaMs = 0;
}

void motionUpdate(uint32_t deltaMs) {
    if (deltaMs > 0) {
        float circum = WHEEL_DIAMETER_M * 3.14159f;
        s_params.ls = circum / (deltaMs / 1000.f);
        s_params.accel = (s_params.ls - s_prevLs) / (deltaMs / 1000.f);
        s_prevLs = s_params.ls;
        s_prevDeltaMs = deltaMs;
        s_params.cnt++;
        s_params.odom += circum;
    } else {
        s_params.accel = 0.f;
    }
}

void motionGetParams(MotionParams* out) {
    if (out) *out = s_params;
}

static LightMode modeFromSensors(float ls, float accel, float gyroX) {
    if (ls <= SPEED_HALT_THRESHOLD)
        return LightMode::HALT;
    if (accel < ACCEL_STOP_THRESHOLD) {
        if (gyroX <= -GYRO_TURN_THRESHOLD) return LightMode::STOP_LEFT;
        if (gyroX >= GYRO_TURN_THRESHOLD)  return LightMode::STOP_RIGHT;
        return LightMode::STOP;
    }
    if (gyroX <= -GYRO_TURN_THRESHOLD) return LightMode::GO_LEFT;
    if (gyroX >= GYRO_TURN_THRESHOLD)  return LightMode::GO_RIGHT;
    return LightMode::GO;
}

LightMode motionGetMode(const MpuData* mpu, JoystickDir joy, bool joystickActive) {
    if (joystickActive) {
        switch (joy) {
            case JoystickDir::Forward: return LightMode::GO;
            case JoystickDir::Back:    return LightMode::STOP;
            case JoystickDir::Left:   return LightMode::LEFT;
            case JoystickDir::Right:  return LightMode::RIGHT;
            case JoystickDir::Press:  return LightMode::SOS;
            default: break;
        }
    }
    float gyroX = mpu ? mpu->gyroX : 0.f;
    return modeFromSensors(s_params.ls, s_params.accel, gyroX);
}
