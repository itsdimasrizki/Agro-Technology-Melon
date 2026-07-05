#include "ESPNowManager.h"

SoilData ESPNowManager::receivedData;

volatile bool
ESPNowManager::newData = false;

bool ESPNowManager::begin() {
    WiFi.mode(WIFI_STA);

    if(esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed");
        return false;
    }

    esp_now_register_recv_cb(onDataRecv);

    Serial.println("ESP-NOW Ready");
    return true;
}

void ESPNowManager::onDataRecv(
    const uint8_t *mac,
    const uint8_t *incomingData,
    int len
) {
    memcpy(
        &receivedData,
        incomingData,
        sizeof(receivedData)
    );

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