#pragma once

// ============================================================
// Konfigurasi test kalibrasi flow sensor A & B
// Ubah nilai di bawah, lalu upload ulang untuk test berbeda.
// A dan B dikonfigurasi terpisah karena kekuatan pompa beda.
// ============================================================

// --- Channel A (pompa lemah / submersible kecil) ---
// Target volume yang ingin dikeluarkan (liter).
static constexpr float         FLOW_CAL_TARGET_A        = 0.5f;
// ON lebih lama agar air punya waktu naik ke sensor.
static constexpr unsigned long FLOW_CAL_PULSE_ON_MS_A   = 10000UL; // 10 detik ON
static constexpr unsigned long FLOW_CAL_PULSE_OFF_MS_A  =   200UL; //  200ms OFF

// --- Channel B (pompa lebih kuat) ---
static constexpr float         FLOW_CAL_TARGET_B        = 0.5f;
static constexpr unsigned long FLOW_CAL_PULSE_ON_MS_B   =  1000UL; // 1 detik ON
static constexpr unsigned long FLOW_CAL_PULSE_OFF_MS_B  =   200UL; // 200ms OFF
