#include "RTCManager.h"
#include "../config/RuntimeConfig.h"

void RTCManager::begin() {
    if (!rtc.begin()) {
        Serial.println("[RTC] Inisialisasi gagal!");
        _rtcOk = false;
        return;
    }

    _rtcOk = true;
    _dt = rtc.now();

    // [HARDCODED LAMA] Uncomment untuk hardcoded tanpa MQTT (testing):
    // plantingDate = DateTime(2026, 6, 1, 0, 0, 0);

    // [RUNTIME] Init plantingDate dari RuntimeConfig
    // Nilai default gConfig = 2026-06-01 (sama dengan hardcoded lama)
    plantingDate = DateTime(
        gConfig.plantingYear,
        gConfig.plantingMonth,
        gConfig.plantingDay,
        0, 0, 0
    );
}

bool RTCManager::isOk() const {
    return _rtcOk;
}

void RTCManager::refresh() {
    if (!_rtcOk) return;

    _dt = rtc.now();
}

DateTime RTCManager::now() {
    return _dt;
}

uint16_t RTCManager::getPlantAgeDays() {
    TimeSpan age = _dt - plantingDate;

    return age.days();
}

uint8_t RTCManager::getDay() {
    return _dt.day();
}

uint8_t RTCManager::getMonth() {
    return _dt.month();
}

uint16_t RTCManager::getYear() {
    return _dt.year();
}

uint8_t RTCManager::getHour() {
    return _dt.hour();
}

uint8_t RTCManager::getMinute() {
    return _dt.minute();
}

// [RUNTIME] Dipanggil oleh MQTTManager saat config "planting_date" diterima
void RTCManager::setPlantingDate(uint16_t year, uint8_t month, uint8_t day) {
    plantingDate = DateTime(year, month, day, 0, 0, 0);

    Serial.print("[RTC] Planting date diupdate: ");
    Serial.print(year);
    Serial.print("-");
    if (month < 10) Serial.print("0");
    Serial.print(month);
    Serial.print("-");
    if (day < 10) Serial.print("0");
    Serial.println(day);
}