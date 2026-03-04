/**
 * @file led_effects.cpp
 * @brief Реализация световых эффектов по режимам (Right, Left, Go, Stop и др.), TEST — мигание RGB 202,82,242.
 */

#include "led_effects.h"
#include "config.h"
#include <FastLED.h>

#define NUM_LEDS LED_COUNT
static CRGB leds[NUM_LEDS];
static uint32_t s_phase = 0;

static const CRGB TEST_COLOR(TEST_COLOR_R, TEST_COLOR_G, TEST_COLOR_B);

static void fillAll(CRGB c) {
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = c;
}

static void fillNone() {
    fillAll(CRGB::Black);
}

/** Бегущая полоса вправо (Right / GO_RIGHT). */
static void effectRight() {
    int pos = (s_phase / 20) % (NUM_LEDS + 10);
    fillNone();
    for (int i = 0; i < 15; i++) {
        int idx = pos - i;
        if (idx >= 0 && idx < NUM_LEDS)
            leds[idx] = CRGB::Green;
    }
}

/** Бегущая полоса влево (Left / GO_LEFT). */
static void effectLeft() {
    int pos = (s_phase / 20) % (NUM_LEDS + 10);
    fillNone();
    for (int i = 0; i < 15; i++) {
        int idx = (NUM_LEDS - 1 - pos) + i;
        if (idx >= 0 && idx < NUM_LEDS)
            leds[idx] = CRGB::Green;
    }
}

/** Движение вперёд — центральная полоса (Go). */
static void effectGo() {
    fillNone();
    int center = NUM_LEDS / 2;
    int w = 20 + (s_phase / 5) % 10;
    for (int i = center - w; i <= center + w; i++) {
        if (i >= 0 && i < NUM_LEDS) leds[i] = CRGB::Green;
    }
}

/** Торможение — красный (Stop). */
static void effectStop() {
    int blink = (s_phase / 100) % 2;
    fillAll(blink ? CRGB::Red : CRGB::DarkRed);
}

/** STOP_LEFT — красный с движением влево. */
static void effectStopLeft() {
    effectStop();
    int pos = (s_phase / 30) % NUM_LEDS;
    for (int i = 0; i < 8; i++) {
        int idx = NUM_LEDS - 1 - pos - i;
        if (idx >= 0 && idx < NUM_LEDS) leds[idx] = CRGB::White;
    }
}

/** STOP_RIGHT — красный с движением вправо. */
static void effectStopRight() {
    effectStop();
    int pos = (s_phase / 30) % NUM_LEDS;
    for (int i = 0; i < 8; i++) {
        int idx = pos + i;
        if (idx >= 0 && idx < NUM_LEDS) leds[idx] = CRGB::White;
    }
}

/** HALT — приглушённый или выключен. */
static void effectHalt() {
    fillAll(CRGB(10, 10, 10));
}

/** SOS — морзекод SOS (короткие/длинные вспышки). */
static void effectSos() {
    fillNone();
    const uint8_t pattern[] = { 1,1,1, 0, 1,1,1, 0, 1,1,1, 0, 0, 0 };
    int step = (s_phase / 80) % 14;
    if (pattern[step])
        fillAll(CRGB::White);
}

/** TEST — все мигают цветом 202,82,242 (по ТЗ п.13). */
static void effectTest() {
    int on = (s_phase / 200) % 2;
    fillAll(on ? TEST_COLOR : CRGB::Black);
}

void ledEffectsInit() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(80);
    fillNone();
    FastLED.show();
}

void ledEffectsTick(LightMode mode) {
    s_phase++;
    switch (mode) {
        case LightMode::HALT:       effectHalt(); break;
        case LightMode::GO:         effectGo(); break;
        case LightMode::STOP:       effectStop(); break;
        case LightMode::LEFT:       effectLeft(); break;
        case LightMode::RIGHT:      effectRight(); break;
        case LightMode::GO_LEFT:    effectLeft(); break;
        case LightMode::GO_RIGHT:   effectRight(); break;
        case LightMode::STOP_LEFT:  effectStopLeft(); break;
        case LightMode::STOP_RIGHT: effectStopRight(); break;
        case LightMode::SOS:        effectSos(); break;
        case LightMode::TEST:       effectTest(); break;
        default: effectHalt(); break;
    }
    FastLED.show();
}
