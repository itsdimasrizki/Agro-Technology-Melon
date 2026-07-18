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

    bool hasReceivedData() const;
    uint32_t getPacketCount() const;
    int getLastPacketSize() const;
    void getLastSenderMac(char* buffer, size_t size) const;

    SoilData getData() const;

    void clearFlag();

private:
    static void onDataRecv(
        const uint8_t *mac,
        const uint8_t *incomingData,
        int len
    );

    static SoilData receivedData;

    static volatile bool          newData;
    static volatile unsigned long _lastReceiveMs;
    static volatile uint32_t      _packetCount;
    static volatile int           _lastPacketSize;
    static uint8_t                _lastSenderMac[6];
};

#endif
