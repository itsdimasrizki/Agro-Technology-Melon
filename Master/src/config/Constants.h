#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

//! Dosis per injeksi koreksi PPM (Nutrisi A & B, rasio 1:1)
constexpr float CORRECTION_DOSE = 0.05f; // 50mL

// Produksi: uncomment baris ini, comment baris uji coba di bawah
// constexpr uint32_t PRE_MIX_TANK_TIME       = 60000UL;   // 1 menit
// constexpr uint32_t PRE_MIX_CORRECTION_TIME = 60000UL;   // 1 menit
// constexpr uint32_t MIX_A_TIME              = 300000UL;  // 5 menit
// constexpr uint32_t MIX_B_TIME              = 300000UL;  // 5 menit

constexpr uint32_t PRE_MIX_TANK_TIME       = 5000UL;    // 5 detik (uji coba)
constexpr uint32_t PRE_MIX_CORRECTION_TIME = 5000UL;    // 5 detik (uji coba)
constexpr uint32_t MIX_A_TIME              = 10000UL;   // 10 detik (uji coba)
constexpr uint32_t MIX_B_TIME              = 10000UL;   // 10 detik (uji coba)

constexpr unsigned long WATER_FILL_TIMEOUT  = 1800000UL; // 30 menit
constexpr unsigned long NUTRIENT_TIMEOUT    = 300000UL;  // 5 menit
constexpr unsigned long CORRECTION_MIX_TIME = 60000UL;  // 1 menit
constexpr uint32_t      PRE_IRRIGATION_MIX_TIME = 60000UL; // 1 menit
constexpr uint32_t      CORRECTION_DELAY    = 180000UL; // 3 menit

// --- Pengisian air manual (FILL_WATER) ---
// Toleransi noise sensor ultrasonik — perubahan di bawah nilai ini dianggap noise, bukan gerakan level nyata
constexpr float          WATER_LEVEL_NOISE_THRESHOLD  = 0.1f;     // liter
// Durasi level harus stabil (tidak berubah) setelah buzzer ON sebelum FSM lanjut ke PRE_MIX_A
constexpr unsigned long  WATER_LEVEL_STABLE_TIMEOUT   = 60000UL;  // 1 menit

#endif