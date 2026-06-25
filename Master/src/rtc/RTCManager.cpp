#include "RTCManager.h"

void RTCManager::begin() {
    if (!rtc.begin()) {
        Serial.println("[RTC] Inisialisasi gagal!");
        _rtcOk = false;
        return;
    }

    _rtcOk = true;
    _dt = rtc.now();
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