/**
 * @file storage.cpp
 * @brief Реализация работы с LittleFS для SMLS Master.
 */

#include "storage.h"
#include <LittleFS.h>
#include <FS.h>

static bool s_loggingEnabled = true;

bool storageInit() {
    if (!LittleFS.begin(true)) {
        return false;
    }
    return true;
}

bool storageReady() {
    return LittleFS.exists("/");
}

void storageLogTrack(uint32_t timeSec, float ls, float accel, uint32_t cnt, LightMode mode) {
    if (!storageReady() || !s_loggingEnabled) return;
    File f = LittleFS.open(TRACK_FILE, "a");
    if (!f) return;
    char buf[TRACK_LINE_MAX];
    snprintf(buf, sizeof(buf), "%lu,%.2f,%.2f,%lu,%u\n",
             (unsigned long)timeSec, (double)ls, (double)accel,
             (unsigned long)cnt, (unsigned)static_cast<uint8_t>(mode));
    f.print(buf);
    f.close();
}

#define CALIB_FILE_0 "/calib0.bin"
#define CALIB_FILE_1 "/calib1.bin"

void storageSaveCalibration(int mpuIndex, const int16_t* offsets) {
    if (!storageReady() || !offsets) return;
    const char* path = (mpuIndex == 0) ? CALIB_FILE_0 : CALIB_FILE_1;
    File f = LittleFS.open(path, "w");
    if (!f) return;
    f.write((const uint8_t*)offsets, 6 * sizeof(int16_t));
    f.close();
}

bool storageLoadCalibration(int mpuIndex, int16_t* offsets) {
    if (!storageReady() || !offsets) return false;
    const char* path = (mpuIndex == 0) ? CALIB_FILE_0 : CALIB_FILE_1;
    if (!LittleFS.exists(path)) return false;
    File f = LittleFS.open(path, "r");
    if (!f || f.size() < 6 * (int)sizeof(int16_t)) {
        if (f) f.close();
        return false;
    }
    f.read((uint8_t*)offsets, 6 * sizeof(int16_t));
    f.close();
    return true;
}

void storageSetLogging(bool enable) {
    s_loggingEnabled = enable;
}

bool storageIsLogging() {
    return s_loggingEnabled;
}
