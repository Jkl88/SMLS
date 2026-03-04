/**
 * @file led_effects.h
 * @brief Эффекты ленты WS2812B для всех 11 режимов (по ТЗ п.12).
 */

#ifndef SMLS_LED_EFFECTS_H
#define SMLS_LED_EFFECTS_H

#include "common.h"

/** Инициализация ленты (пин и количество из config). */
void ledEffectsInit();

/** Отрисовать текущий кадр для заданного режима. Вызывать ~60 FPS. */
void ledEffectsTick(LightMode mode);

#endif /* SMLS_LED_EFFECTS_H */
