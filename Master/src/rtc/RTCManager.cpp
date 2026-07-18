#include "RTCManager.h"

void RTCManager::begin() {
    if (!rtc.begin()) {
        _rtcOk = false;
        return;
    }

    _rtcOk = true;
    _lostPower = rtc.lostPower();

#if SYNC_RTC_FROM_BUILD_TIME
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    _syncedFromBuildTime = true;
#endif

    _dt = rtc.now();
}

bool RTCManager::isOk() const {
    return _rtcOk;
}

bool RTCManager::lostPower() const {
    return _lostPower;
}

bool RTCManager::wasSyncedFromBuildTime() const {
    return _syncedFromBuildTime;
}

void RTCManager::refresh() {
    if (!_rtcOk) return;

#if ENABLE_FSM_SIMULATION_TEST
    return;
#endif

    _dt = rtc.now();
}

DateTime RTCManager::now() {
    return _dt;
}

uint16_t RTCManager::getPlantAgeDays() {
    TimeSpan age = _dt - plantingDate;
    int16_t elapsedDays = age.days();

    if (elapsedDays < 0) return 0;

    // SOP memakai hari tanam sebagai Hari 1, bukan Hari 0.
    return static_cast<uint16_t>(elapsedDays + 1);
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

void RTCManager::setPlantingDate(uint16_t year, uint8_t month, uint8_t day) {
    plantingDate = DateTime(year, month, day, 0, 0, 0);
}

void RTCManager::setDailyMixSchedule(uint8_t hour, uint8_t minute) {
    _dailyMixHour   = hour;
    _dailyMixMinute = minute;
}

#if ENABLE_FSM_SIMULATION_TEST
void RTCManager::setTestDateTime(uint16_t year, uint8_t month, uint8_t day,
                                 uint8_t hour, uint8_t minute) {
    _dt = DateTime(year, month, day, hour, minute, 0);
    _rtcOk = true;
}
#endif
