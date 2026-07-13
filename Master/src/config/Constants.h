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

constexpr unsigned long NUTRIENT_TIMEOUT    = 300000UL;  // 5 menit
constexpr unsigned long CORRECTION_MIX_TIME = 60000UL;  // 1 menit
constexpr uint32_t      PRE_IRRIGATION_MIX_TIME = 60000UL; // 1 menit
constexpr uint32_t      CORRECTION_DELAY    = 180000UL; // 3 menit

// --- Pengisian air otomatis via solenoid (FILL_WATER) ---
// Timeout pengisian air otomatis — lebih pendek dari mode manual karena debit keran
// bertekanan tetap dan predictable. Jika dalam waktu ini level belum tercapai,
// kemungkinan ada fault hardware (solenoid macet, keran mati, sensor gagal baca).
constexpr unsigned long WATER_FILL_TIMEOUT  = 300000UL;  // 5 menit
// Toleransi noise sensor ultrasonik — perubahan di bawah nilai ini dianggap noise, bukan gerakan level nyata
constexpr float          WATER_LEVEL_NOISE_THRESHOLD  = 0.1f;     // liter
// Durasi level harus stabil (tidak berubah) setelah solenoid TUTUP sebelum FSM lanjut ke PRE_MIX_A
constexpr unsigned long  WATER_LEVEL_STABLE_TIMEOUT   = 60000UL;  // 1 menit
// Volume minimum tangki (safety floor) untuk proteksi pompa dari dry-run (checkMinimumWater Opsi A)
constexpr float          TANK_SAFETY_FLOOR_LITER       = 5.0f;
// Interval pengiriman ulang alert "butuh diisi" ke MQTT selama menunggu di FILL_WATER
constexpr unsigned long  NEED_REFILL_ALERT_INTERVAL_MS = 30000UL; // 30 detik
// Margin penutupan solenoid lebih awal sebelum target volume tercapai.
// Kompensasi delay mekanik penutupan valve + sisa air di pipa yang masih mengalir
// sesaat setelah solenoid menutup (water hammer / trailing flow).
// WAJIB dikalibrasi ulang di lapangan — lihat catatan kalibrasi di FertigationFSM.cpp.
constexpr float          FILL_STOP_MARGIN_LITER        = 0.3f;    // estimasi awal, kalibrasi sebelum produksi

// =========================================
// Soil Health Monitor
// =========================================

// Interval evaluasi health score (ms)
constexpr unsigned long SOIL_HEALTH_EVAL_INTERVAL    = 300000UL;  // 5 menit

// Rule #1: Heartbeat — berapa lama tidak ada paket ESP-NOW baru dianggap timeout
constexpr unsigned long SOIL_HEARTBEAT_TIMEOUT       = 300000UL;  // 5 menit

// Rule #2: Out-of-range — batas ADC wajar hasil kalibrasi kapasitif (kering ~ 3900, basah ~ 3400)
// Probe fisik: di bawah MIN atau di atas MAX menandakan sensor lepas/rusak keras
constexpr uint16_t      SOIL_ADC_MIN_VALID           = 800;       // batas bawah ADC wajar
constexpr uint16_t      SOIL_ADC_MAX_VALID           = 4095;      // batas atas ADC wajar

// Rule #3: Flatline — max delta ADC dalam ring buffer yang masih dianggap "stuck"
constexpr uint16_t      SOIL_FLATLINE_DELTA          = 50;        // delta ADC
// Ukuran ring buffer sampel ADC untuk deteksi flatline
constexpr uint8_t       SOIL_ADC_BUFFER_SIZE          = 8;
// Jumlah minimum siklus irigasi selesai sebelum flatline mulai dievaluasi
constexpr uint8_t       SOIL_MIN_CYCLES_FOR_FLATLINE  = 2;

// Rule #4: No response after watering — delta ADC minimum yang diharapkan turun setelah irigasi
constexpr uint16_t      SOIL_RESPONSE_MIN_DELTA      = 100;

// Health score threshold — jika health <= ini, auto-switch ke mode TIMER
constexpr uint8_t       SOIL_HEALTH_SWITCH_THRESHOLD  = 50;

#endif