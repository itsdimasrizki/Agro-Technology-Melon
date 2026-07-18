#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>
#include "TestFlags.h"

class RTCManager {
public:
    void begin();

    // Baca satu kali dari RTC, simpan sebagai snapshot.
    void refresh();

    bool isOk() const;
    bool lostPower() const;
    bool wasSyncedFromBuildTime() const;

    DateTime now();

    uint16_t getPlantAgeDays();

    uint8_t getHour();
    uint8_t getMinute();

    uint8_t getDay();
    uint8_t getMonth();
    uint16_t getYear();

    // Dipanggil ConfigManager setelah load/update dari MQTT
    void setPlantingDate(uint16_t year, uint8_t month, uint8_t day);
    void setDailyMixSchedule(uint8_t hour, uint8_t minute);

    uint8_t getDailyMixHour()   const { return _dailyMixHour; }
    uint8_t getDailyMixMinute() const { return _dailyMixMinute; }

#if ENABLE_FSM_SIMULATION_TEST
    void setTestDateTime(uint16_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute);
#endif

private:

    RTC_PCF8563 rtc;

    DateTime plantingDate = DateTime(2026, 6, 1, 0, 0, 0);

    uint8_t _dailyMixHour   = 5;
    uint8_t _dailyMixMinute = 0;

    // Snapshot hasil refresh()
    DateTime _dt;

    bool _rtcOk = false;
    bool _lostPower = false;
    bool _syncedFromBuildTime = false;
};

#endif
