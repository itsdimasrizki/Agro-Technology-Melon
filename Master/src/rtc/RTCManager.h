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

    // [RUNTIME] Set hari pertama tanam via MQTT (greenhouse/config/set)
    // Field JSON: "planting_date": "YYYY-MM-DD"
    void setPlantingDate(uint16_t year, uint8_t month, uint8_t day);

private:

    RTC_PCF8563 rtc;

    // [HARDCODED LAMA] Uncomment untuk hardcoded tanpa MQTT (testing):
    // DateTime plantingDate = DateTime(2026, 6, 1, 0, 0, 0);

    // [RUNTIME] Diisi di begin() dari RuntimeConfig, atau diupdate via setPlantingDate()
    DateTime plantingDate;

    DateTime _dt;

    bool _rtcOk = false;
};

#endif