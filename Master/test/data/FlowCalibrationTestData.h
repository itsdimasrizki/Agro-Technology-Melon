#pragma once

// ============================================================
// Konfigurasi test kalibrasi flow sensor A & B
// Ubah nilai di bawah, lalu upload ulang untuk test berbeda.
// ============================================================

// Target volume yang ingin dikeluarkan (liter).
// Contoh: 0.5f = 500ml, 0.05f = 50ml, 1.0f = 1L
static constexpr float FLOW_CAL_TARGET_A = 0.5f;
static constexpr float FLOW_CAL_TARGET_B = 0.5f;

// Durasi pulsing pompa (ms) — sama dengan yang dipakai FSM
static constexpr unsigned long FLOW_CAL_PULSE_ON_MS  = 1000UL;
static constexpr unsigned long FLOW_CAL_PULSE_OFF_MS = 1000UL;
