/**
 * @file espnow_rx.h
 * @brief Приём команд по ESP-NOW от Master (по ТЗ п.11).
 */

#ifndef SMLS_LED_ESPNOW_RX_H
#define SMLS_LED_ESPNOW_RX_H

#include "common.h"

/** Инициализация ESP-NOW приёмника. Возвращает true при успехе. */
bool espnowRxInit();

/** Текущий принятый режим (последняя полученная команда). */
LightMode espnowRxGetMode();

#endif /* SMLS_LED_ESPNOW_RX_H */
