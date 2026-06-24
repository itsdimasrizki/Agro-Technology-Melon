#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class RTCManager {
public:
    void begin();

    // Baca satu kali dari RTC, simpan sebagai snapshot.
    // Panggil di awal setiap loop agar semua getter konsisten.
    void refresh();

    // Kembalikan true jika RTC berhasil diinisialisasi
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

    // Snapshot hasil refresh() — semua getter membaca dari sini
    DateTime _dt;

    bool _rtcOk = false;
};

#endif