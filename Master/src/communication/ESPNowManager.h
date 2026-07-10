#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>

#include "SoilData.h"

class ESPNowManager {
public:
    bool begin();

    bool hasNewData() const;

    // Waktu terakhir paket ESP-NOW diterima (millis())
    // Digunakan oleh SoilHealthMonitor untuk heartbeat timeout check.
    // Mengembalikan 0 jika belum pernah terima paket sejak boot.
    unsigned long getLastReceiveTime() const;

    SoilData getData() const;

    void clearFlag();

private:
    static void onDataRecv(
        const uint8_t *mac,
        const uint8_t *incomingData,
        int len
    );

    static SoilData receivedData;

    static volatile bool        newData;
    static volatile unsigned long _lastReceiveMs;
};

#endif