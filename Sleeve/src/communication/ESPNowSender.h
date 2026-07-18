#ifndef ESPNOW_SENDER_H
#define ESPNOW_SENDER_H

#include <esp_now.h>
#include <WiFi.h>

#include "../communication/SoilData.h"

static const uint8_t ESPNOW_TARGET_MAC[6] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF   // Broadcast: Master menerima paket dari sender mana pun di channel yang sama.
};

class ESPNowSender {
public:
    bool begin();

    bool send(const SoilData& data);
    bool wasLastSendDelivered() const;
    void getTargetMac(char* buffer, size_t size) const;

private:
    static void onSendCallback(
        const uint8_t* mac,
        esp_now_send_status_t status
    );

    static bool _lastSendOk;
};

#endif
