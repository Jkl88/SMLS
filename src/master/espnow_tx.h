/**
 * @file espnow_tx.h
 * @brief Отправка команд по ESP-NOW на LED Controller (по ТЗ п.11).
 */

#ifndef SMLS_MASTER_ESPNOW_TX_H
#define SMLS_MASTER_ESPNOW_TX_H

#include "common.h"

/** Инициализация ESP-NOW (WiFi в режиме STA). Возвращает true при успехе. */
bool espnowTxInit();

/** Отправить команду режима на LED Controller. Передаётся только при смене режима. */
void espnowTxSend(LightMode mode);

#endif /* SMLS_MASTER_ESPNOW_TX_H */
