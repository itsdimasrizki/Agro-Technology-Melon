#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class RTCManager {
public:
    void begin();
    void refresh();

    bool isOk() const;
    DateTime now() const;
    void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

    uint16_t minuteOfDay() const;
    uint8_t getHour() const;
    uint8_t getMinute() const;
    uint8_t getDay() const;
    uint8_t getMonth() const;
    uint16_t getYear() const;

private:
    RTC_PCF8563 rtc;
    DateTime _dt;
    bool _rtcOk = false;
};

#endif
