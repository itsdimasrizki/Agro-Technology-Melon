#include "RTCManager.h"

void RTCManager::begin() {
    if (!rtc.begin()) {
        _rtcOk = false;
        return;
    }

    _rtcOk = true;
    _dt = rtc.now();
}

void RTCManager::refresh() {
    if (!_rtcOk) return;
    _dt = rtc.now();
}

bool RTCManager::isOk() const {
    return _rtcOk;
}

DateTime RTCManager::now() const {
    return _dt;
}

void RTCManager::setDateTime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
) {
    if (!_rtcOk) return;

    _dt = DateTime(year, month, day, hour, minute, second);
    rtc.adjust(_dt);
}

uint16_t RTCManager::minuteOfDay() const {
    return static_cast<uint16_t>(_dt.hour() * 60U + _dt.minute());
}

uint8_t RTCManager::getHour() const {
    return _dt.hour();
}

uint8_t RTCManager::getMinute() const {
    return _dt.minute();
}

uint8_t RTCManager::getDay() const {
    return _dt.day();
}

uint8_t RTCManager::getMonth() const {
    return _dt.month();
}

uint16_t RTCManager::getYear() const {
    return _dt.year();
}
