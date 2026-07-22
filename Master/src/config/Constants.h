#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

// KALIBRASI FLOW METER
// Nilai default 450.0 adalah spesifikasi datasheet generic.
// Setelah field test (ENABLE_NUTRIENT_WIRING_TEST), isi dengan:
//   PULSES_PER_LITER = total_pulses / volume_gelas_takar_liter
constexpr float PULSES_PER_LITER = 450.0f;

// DRAIN DELAY — selang draining setelah pompa dimatikan
constexpr uint32_t NUTRIENT_DRAIN_DELAY_MS = 3000UL; 

// MODE DOSING NUTRISI AWAL (ADD_NUTRIENT_A / ADD_NUTRIENT_B)
// 1 = continuous flow (pompa+solenoid ON terus sampai target tercapai) — DEFAULT
// 0 = pulsing 1s ON / 1s OFF (mode lama, tersedia sebagai fallback)
#define NUTRIENT_DOSING_CONTINUOUS 1

// Dosis per injeksi koreksi PPM nutrisi A/B.
constexpr float CORRECTION_DOSE = 0.05f; // 50mL

// Durasi mixing produksi.
constexpr uint32_t PRE_MIX_TANK_TIME       = 60000UL;   // 1 menit
constexpr uint32_t PRE_MIX_CORRECTION_TIME = 300000UL;   // 5 menit
constexpr uint32_t MIX_A_TIME              = 900000UL;  // 15 menit
constexpr uint32_t MIX_B_TIME              = 900000UL;  // 15 menit

constexpr unsigned long WATER_FILL_TIMEOUT  = 7200000UL; // 2 jam
constexpr unsigned long NUTRIENT_TIMEOUT    = 300000UL;  // 5 menit
constexpr unsigned long CORRECTION_MIX_TIME = 60000UL;  // 1 menit
constexpr uint32_t      PRE_IRRIGATION_MIX_TIME = 60000UL; // 1 menit
constexpr uint32_t      CORRECTION_DELAY    = 180000UL; // 3 menit

// FILL_WATER guards.
constexpr float          WATER_LEVEL_NOISE_THRESHOLD  = 0.1f;     // liter
constexpr unsigned long  WATER_LEVEL_STABLE_TIMEOUT   = 3000UL;   // 3 detik
constexpr float          TANK_SAFETY_FLOOR_LITER       = 5.0f;
constexpr unsigned long  NEED_REFILL_ALERT_INTERVAL_MS = 1000UL;  // 1 detik

// Soil health monitor.
constexpr unsigned long SOIL_HEALTH_EVAL_INTERVAL    = 300000UL;  // 5 menit
constexpr unsigned long SOIL_HEARTBEAT_TIMEOUT       = 300000UL;  // 5 menit
constexpr uint16_t      SOIL_ADC_MIN_VALID           = 800;       // batas bawah ADC wajar
constexpr uint16_t      SOIL_ADC_MAX_VALID           = 4095;      // batas atas ADC wajar
constexpr uint16_t      SOIL_FLATLINE_DELTA          = 50;        // delta ADC
constexpr uint8_t       SOIL_ADC_BUFFER_SIZE          = 8;
constexpr uint8_t       SOIL_MIN_CYCLES_FOR_FLATLINE  = 2;
constexpr uint16_t      SOIL_RESPONSE_MIN_DELTA      = 100;
constexpr uint8_t       SOIL_HEALTH_SWITCH_THRESHOLD  = 50;

#endif
