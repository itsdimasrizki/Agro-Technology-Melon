#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>

#include "SoilData.h"

class ESPNowManager {
public:
    bool begin();

    bool hasNewData() const;

    SoilData getData() const;

    void clearFlag();

private:
    static void onDataRecv(
        const uint8_t *mac,
        const uint8_t *incomingData,
        int len
    );

    static SoilData receivedData;

    static volatile bool newData;
};

#endif