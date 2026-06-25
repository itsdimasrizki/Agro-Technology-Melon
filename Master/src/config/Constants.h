#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

//! GREENHOUSE CONFIGURATION
constexpr uint16_t TOTAL_PLANTS = 80;

constexpr float MAX_CONSUMPTION_PER_PLANT = 2.0f; // Liter/hari

// constexpr float DAILY_TARGET_VOLUME = TOTAL_PLANTS * MAX_CONSUMPTION_PER_PLANT; // 160L
constexpr float DAILY_TARGET_VOLUME = 2.0f; // 2Liter (Uji Coba)

//! BASE MIXING CONFIGURATION
constexpr float BASE_TARGET_PPM = 850.0f;

//! sementara, nanti dikalibrasi lapangan
constexpr float INITIAL_NUTRIENT_A = 0.8f; // Liter

constexpr float INITIAL_NUTRIENT_B = 0.8f; // Liter


//! PPM CORRECTION
constexpr float CORRECTION_DOSE = 0.05f; // 50mL

// batas maksimum iterasi koreksi PPM sebelum masuk ERROR
constexpr uint8_t MAX_CORRECTION_COUNT = 10;

constexpr uint32_t CORRECTION_DELAY = 180000UL; // 3 menit


//! MIXING TIME
// <konfigurasi lapangan> (Waktu pengadukan bisa disesuaikan di lapangan)

// constexpr uint32_t PRE_MIX_TANK_TIME = 60000UL; // 1 menit
// constexpr uint32_t PRE_MIX_CORRECTION_TIME = 60000UL; // 1 menit
// constexpr uint32_t MIX_A_TIME = 300000UL; // 5 menit
// constexpr uint32_t MIX_B_TIME = 300000UL; // 5 menit

constexpr uint32_t PRE_MIX_TANK_TIME = 5000UL; // 5 detik (uji coba)
constexpr uint32_t PRE_MIX_CORRECTION_TIME = 5000UL; // 5 detik (uji coba)
constexpr uint32_t MIX_A_TIME = 10000UL; // 10 detik (uji coba)
constexpr uint32_t MIX_B_TIME = 10000UL; // 10 detik (uji coba)

constexpr unsigned long WATER_FILL_TIMEOUT = 1800000UL; // 30 menit

constexpr unsigned long NUTRIENT_TIMEOUT = 300000UL;  // 5 menit

constexpr unsigned long CORRECTION_MIX_TIME = 60000UL;

constexpr uint32_t PRE_IRRIGATION_MIX_TIME = 60000UL;

//! DAILY MIX SCHEDULE
constexpr uint8_t DAILY_MIX_HOUR = 5;

constexpr uint8_t DAILY_MIX_MINUTE = 0;

//! WATER LEVEL
constexpr float MIN_REMAINING_VOLUME = 2.0f; // Liter

constexpr uint16_t PPM_TOLERANCE = 50; // PPM

constexpr uint16_t MAX_PPM_OVERDOSE = 100; //PPM

#endif