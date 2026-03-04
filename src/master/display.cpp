/**
 * @file display.cpp
 * @brief Реализация вывода на SSD1306 по I2C.
 */

#include "display.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_RST -1

static Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, OLED_RST);
static bool s_ok = false;

static const char* modeStr(LightMode m) {
    switch (m) {
        case LightMode::HALT:       return "HALT";
        case LightMode::GO:        return "GO";
        case LightMode::STOP:      return "STOP";
        case LightMode::LEFT:      return "LEFT";
        case LightMode::RIGHT:     return "RIGHT";
        case LightMode::GO_LEFT:   return "GO_LEFT";
        case LightMode::GO_RIGHT:  return "GO_RIGHT";
        case LightMode::STOP_LEFT: return "STOP_LEFT";
        case LightMode::STOP_RIGHT: return "STOP_RIGHT";
        case LightMode::SOS:       return "SOS";
        case LightMode::TEST:      return "TEST";
        default: return "?";
    }
}

bool displayInit() {
    s_ok = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    if (s_ok) {
        oled.clearDisplay();
        oled.setTextSize(1);
        oled.setTextColor(SSD1306_WHITE);
        oled.display();
    }
    return s_ok;
}

void displayUpdate(const MotionParams* params, LightMode mode, bool joystickActive) {
    if (!s_ok) return;
    oled.clearDisplay();
    oled.setCursor(0, 0);
    if (params) {
        oled.print("LS:");
        oled.println(params->ls, 2);
        oled.print("ACCEL:");
        oled.println(params->accel, 2);
        oled.print("CNT:");
        oled.println(params->cnt);
    }
    oled.print("Mode: ");
    if (joystickActive)
        oled.print("Dj_");
    oled.println(modeStr(mode));
    oled.display();
}

void displayShowCalibrating() {
    if (!s_ok) return;
    oled.clearDisplay();
    oled.setCursor(0, 24);
    oled.setTextSize(2);
    oled.println("CALIBRATING");
    oled.setTextSize(1);
    oled.display();
}

void displayShowFsStatus(bool ready) {
    if (!s_ok) return;
    oled.clearDisplay();
    oled.setCursor(0, 24);
    oled.setTextSize(2);
    oled.println(ready ? "FS READY" : "FS ERROR");
    oled.setTextSize(1);
    oled.display();
}
