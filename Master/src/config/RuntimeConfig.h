#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <stdint.h>

// Topic subscribe : greenhouse/config/set
// Topic publish   : greenhouse/config/status
//
// Format JSON (kirim ke greenhouse/config/set):
// {
//   "planting_date"       : "2026-06-01",
//   "mix_hour"            : 5,
//   "mix_minute"          : 0,
//   "tank_height_cm"      : 45.0,
//   "tank_capacity_liter" : 15.0
// }
//

struct RuntimeConfig {

    // --- Input: Jam Mixing Harian ---
    // Nilai default = sama dengan Constants.h (DAILY_MIX_HOUR / DAILY_MIX_MINUTE)
    uint8_t mixHour   = 5;
    uint8_t mixMinute = 0;

    // --- Input: Hari Pertama Penanaman ---
    // Nilai default = sama dengan RTCManager.h (plantingDate hardcoded)
    uint16_t plantingYear  = 2026;
    uint8_t  plantingMonth = 6;
    uint8_t  plantingDay   = 1;

    // --- Input: Ukuran Toren ---
    // Nilai default = sama dengan WaterLevel.h (tankHeightCM / tankCapacityLiter)
    float tankHeightCM      = 45.0f;
    float tankCapacityLiter = 15.0f;
};

extern RuntimeConfig gConfig;

#endif
