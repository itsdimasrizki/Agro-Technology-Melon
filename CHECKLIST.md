# Checklist Implementasi — Auto-Fill Air Fertigasi

> Cara pakai: AI yang ngerjain isi kolom **Status** (`[x]` kalau selesai/sesuai, `[ ]` kalau belum/skip) dan **Catatan** (jelasin singkat apa yang diubah/kenapa skip). Jangan hapus baris walau dianggap "tidak relevan" — tetap isi Catatan dengan alasan kenapa tidak relevan, biar aku bisa review semuanya.

## 1. Boot & Inisialisasi
- [x] `RelayManager` init tanpa error, semua channel (termasuk channel solenoid air yang baru) ke kondisi awal aman (OFF / tertutup)
  - Catatan: `begin()` memanggil `allOff()`, termasuk `RELAY_SOLENOID_WATER`; build Master berhasil. Boot pada hardware belum diuji.
- [x] Tidak ada lagi reference ke `RELAY_BUZZER` di seluruh source Master (grep bersih)
  - Catatan: Pencarian seluruh `Master/src` tidak menemukan referensi lama.
- [ ] `WaterLevel` (dan sensor lain yang dipakai state ini) berhasil baca nilai awal yang masuk akal saat boot, bukan error/-1 terus-terusan
  - Catatan: Tidak dapat diverifikasi tanpa sensor ultrasonic pada perangkat fisik.

## 2. Relay & Wiring Logic
- [x] Channel relay ke-5 sekarang mengontrol solenoid air (`RELAY_SOLENOID_WATER`), bukan lagi buzzer
  - Catatan: Enum dan `RelayManager::getPin()` memetakan solenoid ke `RELAY_5_PIN`.
- [x] Tidak ada GPIO/relay tambahan yang dipakai khusus buzzer (karena buzzer numpang wiring paralel di channel yang sama)
  - Catatan: Tidak ada channel atau pemanggilan software buzzer terpisah; buzzer diasumsikan paralel secara wiring.
- [x] Ada komentar di kode yang menjelaskan solenoid tipe Normally Closed (fail-safe saat mati daya)
  - Catatan: Komentar NC/fail-safe ada di `RelayManager.h` dan helper FSM.
- [x] Fungsi `startWaterFillSolenoid()` / `stopWaterFillSolenoid()` ada dan dipanggil dengan benar (bukan lagi `startBuzzer()`/`stopBuzzer()`)
  - Catatan: Helper memakai `RELAY_SOLENOID_WATER`; pencarian helper buzzer lama bersih.

## 3. Konstanta (`Constants.h`)
- [x] `FILL_STOP_MARGIN_LITER` ditambahkan dengan nilai awal `0.3f` + komentar bahwa ini perlu dikalibrasi lapangan
  - Catatan: Nilai `0.3f` dan komentar kalibrasi lapangan ada di `Constants.h`.
- [x] `WATER_FILL_TIMEOUT` diubah dari 30 menit → 5 menit
  - Catatan: Nilai sekarang `300000UL`.
- [x] `WATER_LEVEL_STABLE_TIMEOUT` TIDAK berubah (tetap 1 menit)
  - Catatan: Tetap `60000UL`.

## 4. State: `FILL_WATER` (Logic Utama)
- [x] Saat masuk state, kalau `tankVolume` sudah cukup, langsung skip fill (tidak buka solenoid sama sekali) lalu lanjut ke `PRE_MIX_A`
  - Catatan: Kondisi awal menutup relay, memanggil `prepareDailyRecipe()`, lalu berpindah ke `PRE_MIX_A`.
- [x] Kalau belum cukup, solenoid dibuka otomatis + stirrer isi tetap nyala seperti sebelumnya
  - Catatan: FSM memanggil helper solenoid dan `startFillStirrer()` pada inisialisasi state.
- [x] Solenoid ditutup otomatis saat `tankVolume >= target - FILL_STOP_MARGIN_LITER` (bukan nunggu manusia)
  - Catatan: Kondisi memakai `getDailyTargetVolume() - FILL_STOP_MARGIN_LITER`.
- [x] Setelah solenoid tertutup, tetap ada fase tunggu level stabil (pakai `WATER_LEVEL_STABLE_TIMEOUT` & noise threshold yang sudah ada) sebelum lanjut state
  - Catatan: `lastLevelChangeTime` diperbarui hanya di atas `WATER_LEVEL_NOISE_THRESHOLD`, lalu ditunggu 60 detik.
- [x] Overflow (`WATER_OVERFLOW`) tetap berstatus **warning** (bukan hard error) — cek ini TIDAK berubah dari behavior lama
  - Catatan: Tetap memakai `setWarning()`/`clearWarning()`, tanpa `gotoError()` pada cabang overflow.
- [x] Saat overflow terdeteksi, solenoid tetap ikut ditutup paksa (safety tambahan), meski error-nya non-fatal
  - Catatan: Cabang overflow memanggil `stopWaterFillSolenoid()` bila solenoid masih terbuka.
- [x] Timeout 5 menit berjalan benar: kalau solenoid gagal capai target dalam waktu itu, tutup solenoid dulu baru masuk `gotoError(WATER_TIMEOUT)`
  - Catatan: Urutannya adalah stop relay, reset flag, lalu `gotoError()`.
- [x] Log/`Serial.print` di state ini sudah diperbarui teksnya (tidak lagi menyebut proses manual)
  - Catatan: Log sekarang menyebut auto-fill, batas tercapai, dan overflow.

## 5. Sensor Ultrasonic — Median Filter
- [x] `MedianFilter<float, 7>` (atau N ganjil lain) berhasil diintegrasikan ke `WaterLevel`
  - Catatan: `WaterLevel` memiliki member `MedianFilter<float, 7> _distanceFilter`.
- [x] `getLevelPercent()` / `getVolumeLiter()` memakai hasil yang sudah difilter, bukan raw ping tunggal
  - Catatan: `getDistanceCM()` memproses jarak valid melalui filter; kedua fungsi level/volume mengambil jarak lewat jalur ini.
- [ ] Sudah dites: pembacaan tetap stabil/wajar walau disimulasikan noise (opsional kalau ada cara simulasi)
  - Catatan: Belum ada simulasi atau pengujian ultrasonic fisik.

## 6. State Lanjutan (Pastikan Tidak Ada yang Rusak)
- [x] `PRE_MIX_A` / `PRE_MIX_B` / `MIX` tetap jalan normal setelah `FILL_WATER` versi baru (tidak ada state yang ke-skip/nyangkut)
  - Catatan: Review alur menunjukkan kedua jalur FILL_WATER menuju `PRE_MIX_A`; handler state lanjutan tidak diubah. Belum diuji pada hardware.
- [x] `VALIDATE` / `CORRECT_PPM` tidak terpengaruh oleh perubahan ini (dosis nutrisi tetap dari config, bukan dari volume auto-fill)
  - Catatan: Tidak ada perubahan pada handler validasi/koreksi; `prepareDailyRecipe()` tetap menghitung rasio dosis dari volume.
- [x] `IRRIGATE` dan `checkMinimumWater()` tetap berjalan seperti semula, tidak ada perubahan logic top-up darurat (sesuai keputusan: tetap nunggu jadwal besok kalau kurang)
  - Catatan: Tidak ada perubahan pada handler irigasi atau `checkMinimumWater()`.

## 7. MQTT / Dashboard
- [x] Payload relay status (`publishRelayStatus()` atau sejenisnya) tidak lagi mengirim field buzzer yang salah/hilang referensinya (disesuaikan seperlunya)
  - Catatan: Field existing `relays.water` kini membaca `RELAY_5_PIN` (solenoid air), bukan GPIO 21/relay 8.
- [x] Struktur topic MQTT existing tidak berubah drastis (dashboard/web tidak perlu update, sesuai keputusan overflow tetap warning)
  - Catatan: Topic dan struktur payload dipertahankan; hanya sumber status field `water` dikoreksi.
- [ ] Alert "butuh isi ulang" (`_needRefillAlertPending` dkk) tetap terkirim ke dashboard walau prosesnya sekarang otomatis
  - Catatan: Variabel bernama `_needRefillAlertPending`, `_lastRefillAlertMs`, dan `_refillDeficit` tidak ada pada baseline ini. Alert yang ada adalah tank-low (`_tankLowAlertPending`) dan tidak diubah, tetapi pengiriman MQTT belum diuji live.

## 8. Compile & Sanity Check
- [x] Project compile bersih tanpa warning terkait rename relay/fungsi
  - Catatan: `platformio run` untuk environment `esp32-s3-devkitm-1` berhasil setelah perubahan.
- [x] Tidak ada dead code sisa (`startBuzzer`/`stopBuzzer`/`RELAY_BUZZER` lama yang lupa dihapus)
  - Catatan: Pencarian seluruh `Master/src` bersih.

## 9. Kalibrasi Lapangan (Belum Bisa Dicentang oleh AI — Diisi Manual oleh Aku)
- [ ] Sudah lakukan 4-5x run test isi ulang, catat volume saat solenoid nutup vs volume final settle
- [ ] `FILL_STOP_MARGIN_LITER` sudah diupdate dari hasil test lapangan (bukan lagi nilai default `0.3f`)
- [ ] Setelah update margin, hasil akhir volume tangki mendekati target tanpa overflow berulang
- [ ] Solenoid dites manual cabut daya (power off paksa) untuk konfirmasi valve NC benar-benar nutup sendiri

---
**Ringkasan akhir dari AI (isi di sini setelah semua poin di atas selesai):**
- File yang diubah:
  - `Master/src/actuators/RelayManager.h`, `Master/src/actuators/RelayManager.cpp`
  - `Master/src/config/Constants.h`
  - `Master/src/fsm/FertigationFSM.h`, `Master/src/fsm/FertigationFSM.cpp`
  - `Master/src/sensors/MedianFilter.h`, `Master/src/sensors/WaterLevel.h`, `Master/src/sensors/WaterLevel.cpp`
  - `Master/src/communication/MQTTManager.cpp`
- Hal yang perlu aku perhatikan/test manual sebelum flash ke device:
  - Konfirmasi wiring solenoid benar-benar NC dan buzzer paralel pada relay 5; lakukan power-off test.
  - Lakukan 4–5 siklus fill untuk kalibrasi `FILL_STOP_MARGIN_LITER`, serta cek stabilitas ultrasonic saat air bergolak.
  - Pastikan dashboard membaca status `relays.water` sebagai status solenoid relay 5 dan alert tank-low masih terkirim.
- Ada penyimpangan dari spek awal? (kalau ada, jelasin kenapa):
  - Tidak ada penyimpangan fungsional. Pemetaan `relays.water` MQTT dikoreksi dari GPIO 21 ke `RELAY_5_PIN` agar status yang dilaporkan sesuai solenoid baru.
