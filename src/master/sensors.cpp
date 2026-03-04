/**
 * @file sensors.cpp
 * @brief Реализация чтения MPU6050×2, Холла, джойстика и кнопок.
 */

#include "sensors.h"
#include "storage.h"
#include "config.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino.h>

static Adafruit_MPU6050 mpu1;
static Adafruit_MPU6050 mpu2;
static bool mpu1Ok = false;
static bool mpu2Ok = false;
static uint32_t lastHallUs = 0;
static bool hallFirst = true;
static float yawIntegrated = 0.f;
static const float GYRO_DT = 1.f / 200.f;  /* 200 Hz */

bool sensorsInit() {
    Wire.begin(I2C_SDA, I2C_SCL);
    mpu1Ok = mpu1.begin(0x68);
    mpu2Ok = mpu2.begin(0x69);
    if (mpu1Ok) {
        mpu1.setAccelerometerRange(MPU6050_RANGE_4_G);
        mpu1.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu1.setFilterBandwidth(MPU6050_BAND_21_HZ);
    }
    if (mpu2Ok) {
        mpu2.setAccelerometerRange(MPU6050_RANGE_4_G);
        mpu2.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu2.setFilterBandwidth(MPU6050_BAND_21_HZ);
    }
    pinMode(PIN_HALL, INPUT);
    pinMode(PIN_JOY_X, INPUT);
    pinMode(PIN_JOY_Y, INPUT);
    pinMode(PIN_JOY_SW, INPUT_PULLUP);
    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    loadMpuCalibration();
    return mpu1Ok || mpu2Ok;
}

void sensorsCalibrateMpu() {
    /* Калибровка: записываем текущие значения как офсеты в LittleFS (для вывода в Serial и последующей программной коррекции). */
    int16_t off1[6], off2[6];
    for (int i = 0; i < 6; i++) off1[i] = off2[i] = 0;
    if (mpu1Ok) {
        sensors_event_t a, g, t;
        mpu1.getEvent(&a, &g, &t);
        off1[0] = (int16_t)(-g.gyro.x * 4);
        off1[1] = (int16_t)(-g.gyro.y * 4);
        off1[2] = (int16_t)(-g.gyro.z * 4);
        off1[3] = (int16_t)(-a.acceleration.x / 8);
        off1[4] = (int16_t)(-a.acceleration.y / 8);
        off1[5] = (int16_t)(-a.acceleration.z / 8);
        storageSaveCalibration(0, off1);
    }
    if (mpu2Ok) {
        sensors_event_t a, g, t;
        mpu2.getEvent(&a, &g, &t);
        off2[0] = (int16_t)(-g.gyro.x * 4);
        off2[1] = (int16_t)(-g.gyro.y * 4);
        off2[2] = (int16_t)(-g.gyro.z * 4);
        off2[3] = (int16_t)(-a.acceleration.x / 8);
        off2[4] = (int16_t)(-a.acceleration.y / 8);
        off2[5] = (int16_t)(-a.acceleration.z / 8);
        storageSaveCalibration(1, off2);
    }
}

static void loadMpuCalibration() {
    /* Загрузка офсетов из LittleFS (при наличии). Можно использовать для программной коррекции в sensorsReadMpu. */
    int16_t off[6];
    storageLoadCalibration(0, off);
    storageLoadCalibration(1, off);
}

void sensorsReadMpu(MpuData* out1, MpuData* out2) {
    sensors_event_t a, g, t;
    if (mpu1Ok && out1) {
        mpu1.getEvent(&a, &g, &t);
        out1->roll = atan2f(a.acceleration.y, a.acceleration.z) * 57.2958f;
        out1->gyroX = g.gyro.x * 57.2958f;
        out1->gyroZ = g.gyro.z * 57.2958f;
        yawIntegrated += g.gyro.z * 57.2958f * GYRO_DT;
        out1->yaw = yawIntegrated;
    }
    if (mpu2Ok && out2) {
        mpu2.getEvent(&a, &g, &t);
        out2->roll = atan2f(a.acceleration.y, a.acceleration.z) * 57.2958f;
        out2->gyroX = g.gyro.x * 57.2958f;
        out2->gyroZ = g.gyro.z * 57.2958f;
        out2->yaw = 0.f;  /* второй MPU — угол не интегрируем */
    }
}

bool sensorsReadHall(uint32_t* deltaMs) {
    int v = analogRead(PIN_HALL);
    if (v < 512) return false;
    uint32_t now = micros();
    if (hallFirst) {
        hallFirst = false;
        lastHallUs = now;
        return false;
    }
    *deltaMs = (now - lastHallUs) / 1000;
    lastHallUs = now;
    return true;
}

float sensorsHallToSpeedMs(uint32_t deltaMs) {
    if (deltaMs == 0) return 0.f;
    float circum = WHEEL_DIAMETER_M * 3.14159f;
    return circum / (deltaMs / 1000.f);
}

JoystickDir sensorsReadJoystick() {
    int x = analogRead(PIN_JOY_X);
    int y = analogRead(PIN_JOY_Y);
    int sw = digitalRead(PIN_JOY_SW);
    if (sw == LOW) return JoystickDir::Press;
    const int dead = 500;
    const int low = 1500, high = 3500;
    if (y < low - dead) return JoystickDir::Forward;
    if (y > high + dead) return JoystickDir::Back;
    if (x < low - dead) return JoystickDir::Left;
    if (x > high + dead) return JoystickDir::Right;
    return JoystickDir::None;
}

void sensorsReadButtons(bool* btn1, bool* btn2) {
    if (btn1) *btn1 = (digitalRead(PIN_BTN_1) == LOW);
    if (btn2) *btn2 = (digitalRead(PIN_BTN_2) == LOW);
}
