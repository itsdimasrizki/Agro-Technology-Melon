# Requirement Input Web ke Firmware ESP32

Dokumen ini adalah kontrak input dari web/backend ke firmware. Semua input dikirim via MQTT dalam format JSON.

Firmware akan tetap berada di state `IDLE` sampai konfigurasi inti lengkap. Config dianggap lengkap jika bagian berikut sudah valid:

- System/tangki
- Jadwal tanam dan jadwal mixing harian
- PPM dan dosis awal nutrisi A/B
- pH default
- Recipe nutrisi
- Threshold irigasi

Timer irrigation dan stir schedule bersifat tambahan.

## 1. System / Tangki

Topic:

```text
greenhouse/config/system
```

Schema:

```json
{
  "total_plants": 80,
  "max_consumption_per_plant": 15,
  "target_fill_volume": 800,
  "tank_capacity_liter": 1000,
  "tank_height_cm": 120,
  "tank_diameter_cm": 103
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `total_plants` | tanaman | Ya | `_totalPlants` | Jumlah tanaman, dipakai untuk timer irrigation volume total. |
| `max_consumption_per_plant` | mL atau L sesuai desain web saat ini | Ya | `_maxConsumptionPerPlant` | Disimpan untuk konfigurasi sistem; saat ini belum dominan dipakai di flow utama. |
| `target_fill_volume` | liter | Ya | `_targetFillVolume` | Target volume air dalam tangki. Solenoid inlet stop saat `tankVolume >= target_fill_volume`. Biasanya 80–90% kapasitas, bukan 100%. |
| `tank_capacity_liter` | liter | Ya | `_tankCapacityLiter` | Kapasitas nominal tangki. Dipakai sebagai safety clamp dan deteksi overflow. |
| `tank_height_cm` | cm | Ya | `_tankHeightCM` | Tinggi efektif tangki dari dasar sampai titik penuh. Dipakai konversi ultrasonic ke liter. |
| `tank_diameter_cm` | cm | Ya | `_tankDiameterCM` | Diameter tangki silinder vertikal. Dipakai konversi ultrasonic ke liter. |

Rumus volume air:

```text
water_height_cm = tank_height_cm - ultrasonic_distance_cm
volume_liter = π × (tank_diameter_cm / 2)^2 × water_height_cm / 1000
```

Catatan:

- `target_fill_volume` tidak boleh lebih besar dari `tank_capacity_liter`.
- Jika target 800L dan pembacaan aktual 850L, solenoid inlet ditutup dan sistem tetap lanjut setelah level stabil.

## 2. Jadwal Tanam dan Mixing Harian

Topic:

```text
greenhouse/config/schedule
```

Schema:

```json
{
  "plant_year": 2026,
  "plant_month": 7,
  "plant_day": 15,
  "daily_mix_hour": 5,
  "daily_mix_minute": 0
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `plant_year` | tahun | Ya | `_plantYear` | Tahun tanggal tanam. |
| `plant_month` | 1–12 | Ya | `_plantMonth` | Bulan tanggal tanam. |
| `plant_day` | 1–31 | Ya | `_plantDay` | Hari tanggal tanam. |
| `daily_mix_hour` | 0–23 | Ya | `_dailyMixHour` | Jam mulai proses mixing harian. |
| `daily_mix_minute` | 0–59 | Ya | `_dailyMixMinute` | Menit mulai proses mixing harian. |

Firmware menghitung umur tanaman (`plantAgeDays`) dari tanggal tanam. Umur ini dipakai memilih recipe stage.

## 3. PPM dan Dosis Awal Nutrisi

Topic:

```text
greenhouse/config/ppm
```

Schema:

```json
{
  "ppm_tolerance": 50,
  "initial_a": 0.8,
  "initial_b": 0.8
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `ppm_tolerance` | ppm | Ya | `_ppmTolerance` | Toleransi batas bawah target PPM. PPM over tetap diterima. |
| `initial_a` | liter | Ya | `_initialNutrientA` | Volume awal nutrisi A yang dimasukkan setelah fill water. |
| `initial_b` | liter | Ya | `_initialNutrientB` | Volume awal nutrisi B yang dimasukkan setelah fill water. |

Perilaku validasi PPM:

- Jika `ppm >= target_ppm - ppm_tolerance`, sistem dianggap cukup dan lanjut `READY`.
- Jika PPM over target, sistem tetap lanjut dan mengirim warning `OVER_PPM`.
- Jika `ppm < target_ppm - ppm_tolerance`, sistem melakukan koreksi A/B per loop.
- Koreksi memakai `CORRECTION_DOSE = 0.05L` untuk A dan B.

## 4. pH Default

Topic:

```text
greenhouse/config/ph
```

Schema:

```json
{
  "min_ph": 5.5,
  "max_ph": 6.5
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `min_ph` | pH | Ya | `_defaultMinPH` | Batas bawah pH default. |
| `max_ph` | pH | Ya | `_defaultMaxPH` | Batas atas pH default. |

Nilai ini dipakai sebagai fallback jika recipe stage tidak mengirim `min_ph` / `max_ph`.

## 5. Recipe Nutrisi Berdasarkan Umur Tanaman

Topic:

```text
greenhouse/config/recipe
```

Schema:

```json
{
  "stages": [
    {
      "max_age_days": 10,
      "target_ppm": 700,
      "min_ph": 5.5,
      "max_ph": 6.5
    },
    {
      "max_age_days": 20,
      "target_ppm": 900,
      "min_ph": 5.6,
      "max_ph": 6.4
    }
  ]
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `stages[].max_age_days` | hari | Ya | `RecipeStageConfig.maxAgeDays` | Batas akhir umur tanaman untuk stage ini. |
| `stages[].target_ppm` | ppm | Ya | `RecipeStageConfig.targetPPM` | Target PPM untuk stage. |
| `stages[].min_ph` | pH | Disarankan | `RecipeStageConfig.minPH` | Batas bawah pH untuk stage. |
| `stages[].max_ph` | pH | Disarankan | `RecipeStageConfig.maxPH` | Batas atas pH untuk stage. |

Cara kerja stage:

- Stage harus dikirim urut naik berdasarkan `max_age_days`.
- Jika stage pertama `max_age_days = 10`, maka recipe itu berlaku untuk day 1–10.
- Stage kedua `max_age_days = 20` berlaku untuk day 11–20.
- Firmware memilih stage pertama yang memenuhi:

```text
plantAgeDays <= max_age_days
```

Batas firmware:

- Maksimal `MAX_RECIPE_STAGES = 7`.
- Jika umur tanaman melewati `max_age_days` stage terakhir, firmware menghapus config dan kembali ke `IDLE`.

Source code utama:

- `RecipeManager::getRecipe()`
- `ConfigManager::_recipeStages`

## 6. Threshold Irigasi Berdasarkan Umur Tanaman

Topic:

```text
greenhouse/config/irrigation
```

Schema:

```json
{
  "stages": [
    {
      "max_age_days": 10,
      "dry_threshold": 3900,
      "wet_threshold": 3650
    },
    {
      "max_age_days": 20,
      "dry_threshold": 3800,
      "wet_threshold": 3500
    }
  ]
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `stages[].max_age_days` | hari | Ya | `IrrigationStageConfig.maxAgeDays` | Batas akhir umur tanaman untuk threshold ini. |
| `stages[].dry_threshold` | ADC | Ya | `IrrigationStageConfig.dryThreshold` | Jika `soilADC >= dry_threshold`, sistem mulai irigasi saat state `READY`. |
| `stages[].wet_threshold` | ADC | Ya | `IrrigationStageConfig.wetThreshold` | Irigasi berhenti saat `soilADC <= wet_threshold`. |

Cara kerja stage sama seperti recipe nutrisi:

- `max_age_days = 10` berlaku untuk day 1–10.
- `max_age_days = 20` berlaku untuk day 11–20.
- Stage harus urut naik.

Batas firmware:

- Maksimal `MAX_IRRIGATION_STAGES = 7`.

## 7. Timer Irrigation Fallback

Topic:

```text
greenhouse/config/timer_irrigation
```

Schema:

```json
{
  "daily_water_volume_ml_per_plant": 200,
  "slots": [
    { "hour": 7, "minute": 0 },
    { "hour": 16, "minute": 0 }
  ]
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `daily_water_volume_ml_per_plant` | mL/tanaman/hari | Opsional | `_dailyWaterVolumeMLPerPlant` | Volume harian per tanaman untuk mode timer fallback. |
| `slots[].hour` | 0–23 | Opsional | `IrrigationSlot.hour` | Jam slot timer irrigation. |
| `slots[].minute` | 0–59 | Opsional | `IrrigationSlot.minute` | Menit slot timer irrigation. |

Perhitungan target per slot:

```text
daily_total_ml = daily_water_volume_ml_per_plant × total_plants
target_per_slot_ml = daily_total_ml / jumlah_slot
```

Batas firmware:

- Maksimal `MAX_IRRIG_SLOTS = 10`.
- Mode timer dipakai jika `SoilHealthMonitor` switch ke `TIMER`.

## 8. Stir Schedule

Topic:

```text
greenhouse/config/mix_schedule_ext
```

Schema:

```json
{
  "stir_evening_hour": 18,
  "stir_evening_minute": 0,
  "stir_duration_seconds": 300
}
```

Keterangan:

| Field | Unit | Wajib | Variable firmware | Fungsi |
|---|---:|---:|---|---|
| `stir_evening_hour` | 0–23 | Opsional | `_stirEveningHour` | Jam stir sore. |
| `stir_evening_minute` | 0–59 | Opsional | `_stirEveningMinute` | Menit stir sore. |
| `stir_duration_seconds` | detik | Opsional | `_stirDurationMs` | Durasi stir. Firmware menyimpan dalam ms. |

Field lama `mix_interval_days` dan `per_plant_need_liter` diabaikan.

## 9. Command Fertigasi / Aktuator

Topic:

```text
greenhouse/actuators/cmd
```

Status saat ini:

- Command hanya diterima jika FSM berada di state `READY`.
- Jika state belum `READY`, firmware mengabaikan command dan publish ACK error `actuators_cmd_not_ready`.

Tujuan:

- Mencegah irigasi/aktuator berjalan ketika air belum terisi, nutrisi belum masuk, mixing belum selesai, atau PPM/pH belum tervalidasi.

## 10. Reset Soil Mode

Topic:

```text
greenhouse/soil/reset_mode
```

Fungsi:

- Mengembalikan mode soil monitor ke `HUMIDITY`.
- Tidak menghapus config NVS.

## 11. Clear Config / Hapus Memori ESP

Status saat ini:

- Firmware sudah punya fungsi internal `ConfigManager::clearConfig()`.
- Fungsi ini menghapus config NVS untuk PPM, pH, recipe, irrigation, system, schedule, timer irrigation, dan stir schedule.
- Belum ada topic MQTT khusus dari web untuk memanggil clear config.

Rencana patch berikutnya:

```text
greenhouse/config/reset
```

Payload awal yang disarankan:

```json
{
  "scope": "all"
}
```

Setelah diterima, firmware akan menjalankan `configManager.clearConfig()` dan FSM kembali ke `IDLE`.

## 12. Minimal Urutan Input dari Web

Agar firmware keluar dari `IDLE`, web/backend harus mengirim minimal:

1. `greenhouse/config/system`
2. `greenhouse/config/schedule`
3. `greenhouse/config/ppm`
4. `greenhouse/config/ph`
5. `greenhouse/config/recipe`
6. `greenhouse/config/irrigation`

Setelah semua valid, FSM akan pindah dari `IDLE` ke `WAIT_DAILY_MIX`.

## 13. Ringkasan Unit Penting

| Data | Unit |
|---|---:|
| `target_fill_volume` | liter |
| `tank_capacity_liter` | liter |
| `tank_height_cm` | cm |
| `tank_diameter_cm` | cm |
| `initial_a` | liter |
| `initial_b` | liter |
| `CORRECTION_DOSE` | liter, fixed 0.05L |
| `daily_water_volume_ml_per_plant` | mL/tanaman/hari |
| `target_ppm` | ppm |
| `ppm_tolerance` | ppm |
| `dry_threshold` / `wet_threshold` | ADC |
| `stir_duration_seconds` | detik |
