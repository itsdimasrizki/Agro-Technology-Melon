#ifndef ESPNOW_SENDER_H
#define ESPNOW_SENDER_H

#include <esp_now.h>
#include <WiFi.h>

#include "../communication/SoilData.h"

static const uint8_t MASTER_MAC[6] = {
    0xAC, 0xA7, 0x04, 0x13, 0x6E, 0x8C   // <Konfigurasi : Ubah dengan MAC Address asli dari ESP32 Master>
// AC:A7:04:13:6E:8C
};

class ESPNowSender {
public:
    bool begin();

    bool send(const SoilData& data);

private:
    static void onSendCallback(
        const uint8_t* mac,
        esp_now_send_status_t status
    );

    static bool _lastSendOk;
};

#endif
