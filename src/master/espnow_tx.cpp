/**
 * @file espnow_tx.cpp
 * @brief Реализация отправки пакетов ESP-NOW.
 */

#include "espnow_tx.h"
#include <esp_now.h>
#include <WiFi.h>

#ifndef LED_CONTROLLER_MAC
#define LED_CONTROLLER_MAC { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
#endif

static uint8_t s_peerMac[6] = LED_CONTROLLER_MAC;
static LightMode s_lastSent = (LightMode)0xFF;
static bool s_init = false;

bool espnowTxInit() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
        return false;
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, s_peerMac, 6);
    peer.channel = 0;
    peer.encrypt = false;
    if (esp_now_add_peer(&peer) != ESP_OK)
        return false;
    s_init = true;
    return true;
}

void espnowTxSend(LightMode mode) {
    if (!s_init || mode == s_lastSent) return;
    EspNowPacket pkt = { mode };
    esp_err_t r = esp_now_send(s_peerMac, (const uint8_t*)&pkt, sizeof(pkt));
    if (r == ESP_OK)
        s_lastSent = mode;
}
