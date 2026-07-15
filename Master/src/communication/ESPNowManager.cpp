#include "ESPNowManager.h"

SoilData ESPNowManager::receivedData;

volatile bool         ESPNowManager::newData        = false;
volatile unsigned long ESPNowManager::_lastReceiveMs = 0;

bool ESPNowManager::begin() {
    WiFi.mode(WIFI_STA);

    if(esp_now_init() != ESP_OK) {
        return false;
    }

    esp_now_register_recv_cb(onDataRecv);

    return true;
}

void ESPNowManager::onDataRecv(
    const uint8_t *mac,
    const uint8_t *incomingData,
    int len
) {
    (void)mac;

    // Reject packets from incompatible Sleeve firmware.
    if (len != sizeof(receivedData)) {
        return;
    }

    memcpy(
        &receivedData,
        incomingData,
        sizeof(receivedData)
    );

    _lastReceiveMs = millis();
    newData = true;
}

bool ESPNowManager::hasNewData()
const {
    return newData;
}

SoilData ESPNowManager::getData()
const {
    return receivedData;
}

void ESPNowManager::clearFlag() {
    newData = false;
}

unsigned long ESPNowManager::getLastReceiveTime() const {
    return _lastReceiveMs;
}
