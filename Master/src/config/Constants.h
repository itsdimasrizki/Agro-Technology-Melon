#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

// Dosis per injeksi koreksi PPM nutrisi A/B.
constexpr float CORRECTION_DOSE = 0.05f; // 50mL

// ─── Kalibrasi Flow Meter ────────────────────────────────────────────────────
// Nilai akhir didapat dari pengukuran aktual; 450.0 adalah default pabrik.
constexpr float FLOW_METER_PULSES_PER_LITER = 450.0f;

// ─── Drain Delay setelah Pump OFF ───────────────────────────────────────────
// Setelah flow meter mencapai target, Pump dimatikan terlebih dahulu,
// lalu sistem menunggu NUTRIENT_DRAIN_DELAY_MS sebelum menutup Solenoid.
constexpr uint32_t NUTRIENT_DRAIN_DELAY_MS = 3000UL;

// ─── Mode Dosing Awal Nutrisi A/B ───────────────────────────────────────────
// Definisikan makro ini untuk menggunakan continuous flow (Solenoid+Pump ON
// terus sampai target tercapai). Komentari/undef untuk kembali ke pulsing
// 1s-open/1s-close (perilaku lama sebelum patch ini).
#define NUTRIENT_DOSING_CONTINUOUS

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
