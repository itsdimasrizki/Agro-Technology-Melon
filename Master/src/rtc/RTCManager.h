#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class RTCManager {
public:
    void begin();

    // Baca satu kali dari RTC, simpan sebagai snapshot.
    void refresh();

    bool isOk() const;

    DateTime now();

    uint16_t getPlantAgeDays();

    uint8_t getHour();
    uint8_t getMinute();

    uint8_t getDay();
    uint8_t getMonth();
    uint16_t getYear();

private:

    RTC_PCF8563 rtc;

    DateTime plantingDate = DateTime(2026, 6, 1, 0, 0, 0);

    // Snapshot hasil refresh()
    DateTime _dt;

    bool _rtcOk = false;
};

#endif