#include "ESPNowSender.h"

bool ESPNowSender::_lastSendOk = false;

bool ESPNowSender::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        return false;
    }

    esp_now_register_send_cb(onSendCallback);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, MASTER_MAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        return false;
    }

    return true;
}

bool ESPNowSender::send(const SoilData& data) {
    esp_err_t result = esp_now_send(
        MASTER_MAC,
        (const uint8_t*)&data,
        sizeof(SoilData)
    );

    return (result == ESP_OK);
}

void ESPNowSender::onSendCallback(
    const uint8_t* mac,
    esp_now_send_status_t status
) {
    _lastSendOk = (status == ESP_NOW_SEND_SUCCESS);
}
