/**
 * @file webserver.h
 * @brief WEB-сервер: отображение LS/ACCEL/CNT/режим, кнопки калибровки и режимов (по ТЗ п.10).
 * Вызывает те же функции, что кнопки и джойстик.
 */

#ifndef SMLS_MASTER_WEBSERVER_H
#define SMLS_MASTER_WEBSERVER_H

#include "common.h"

/** Запуск WEB-сервера (файлы из LittleFS). Вызывать после WiFi и LittleFS. */
void webserverStart();

/** Остановка сервера. */
void webserverStop();

#endif /* SMLS_MASTER_WEBSERVER_H */
