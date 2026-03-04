/**
 * @file espnow_rx.cpp
 * @brief Реализация приёма пакетов ESP-NOW.
 */

#include "espnow_rx.h"
#include <esp_now.h>
#include <WiFi.h>

static volatile LightMode s_mode = LightMode::TEST;

static void onRecv(const uint8_t* mac, const uint8_t* data, int len) {
    if (len >= (int)sizeof(EspNowPacket)) {
        const EspNowPacket* p = (const EspNowPacket*)data;
        s_mode = p->mode;
    }
}

bool espnowRxInit() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
        return false;
    esp_now_register_recv_cb(onRecv);
    return true;
}

LightMode espnowRxGetMode() {
    return s_mode;
}
