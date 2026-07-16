# FSM Full Loop Test Input

File ini berisi paket input MQTT untuk test 1 loop FSM Master dari `IDLE` sampai selesai daily mix dan siap irigasi timer.

Asumsi:
- MQTT broker/topic mengikuti `Master/src/communication/MQTTManager.h`.
- RTC sudah benar.
- Tanggal tanam diset sama dengan tanggal RTC saat test supaya `plantAgeDays == 0` dan recipe day 1 tetap terpakai.
- Daily mix diset `22:58`; ubah mendekati waktu RTC saat test kalau ingin langsung trigger.
- Timer irrigation diset `23:00`.

## 1. System

Topic:

```text
greenhouse/config/system
```

Payload:

```json
{
  "total_plants": 1,
  "max_consumption_per_plant": 400,
  "target_fill_volume": 400,
  "tank_capacity_liter": 700,
  "tank_height_cm": 136,
  "tank_diameter_cm": 83
}
```

## 2. PPM

Topic:

```text
greenhouse/config/ppm
```

Payload:

```json
{
  "ppm_tolerance": 50,
  "initial_a": 1500,
  "initial_b": 1500
}
```

## 3. pH Default

Topic:

```text
greenhouse/config/ph
```

Payload:

```json
{
  "min_ph": 5,
  "max_ph": 6.5
}
```

## 4. Recipe Day 1

Topic:

```text
greenhouse/config/recipe
```

Payload:

```json
{
  "stages": [
    {
      "max_age_days": 1,
      "target_ppm": 1200,
      "min_ph": 5.5,
      "max_ph": 6.5
    }
  ]
}
```

## 5. Irrigation Threshold Day 1

Topic:

```text
greenhouse/config/irrigation
```

Payload:

```json
{
  "stages": [
    {
      "max_age_days": 1,
      "dry_threshold": 1000,
      "wet_threshold": 800
    }
  ]
}
```

## 6. Schedule

Topic:

```text
greenhouse/config/schedule
```

Payload untuk test pada 2026-07-16:

```json
{
  "plant_year": 2026,
  "plant_month": 7,
  "plant_day": 16,
  "daily_mix_hour": 22,
  "daily_mix_minute": 58
}
```

Ubah `plant_year`, `plant_month`, dan `plant_day` ke tanggal RTC saat test kalau RTC bukan `2026-07-16`.

## 7. Timer Irrigation

Topic:

```text
greenhouse/config/timer_irrigation
```

Payload:

```json
{
  "slots": [
    {
      "hour": 23,
      "minute": 0
    }
  ]
}
```

## 8. Optional Stir Schedule

Topic:

```text
greenhouse/config/mix_schedule_ext
```

Payload:

```json
{
  "stir_evening_hour": 18,
  "stir_evening_minute": 0,
  "stir_duration_seconds": 300
}
```

## Mosquitto Publish Commands

```bash
BROKER="aead004bf5144152b88233f1a1949418.s1.eu.hivemq.cloud"
PORT="8883"
USER="greenhouse_esp32"
PASS="Kebonagungpanenmelon1"

mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/system" -m '{"total_plants":1,"max_consumption_per_plant":400,"target_fill_volume":400,"tank_capacity_liter":700,"tank_height_cm":136,"tank_diameter_cm":83}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/ppm" -m '{"ppm_tolerance":50,"initial_a":1500,"initial_b":1500}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/ph" -m '{"min_ph":5,"max_ph":6.5}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/recipe" -m '{"stages":[{"max_age_days":1,"target_ppm":1200,"min_ph":5.5,"max_ph":6.5}]}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/irrigation" -m '{"stages":[{"max_age_days":1,"dry_threshold":1000,"wet_threshold":800}]}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/schedule" -m '{"plant_year":2026,"plant_month":7,"plant_day":16,"daily_mix_hour":22,"daily_mix_minute":58}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/timer_irrigation" -m '{"slots":[{"hour":23,"minute":0}]}'
mosquitto_pub -h "$BROKER" -p "$PORT" -u "$USER" -P "$PASS" --insecure -t "greenhouse/config/mix_schedule_ext" -m '{"stir_evening_hour":18,"stir_evening_minute":0,"stir_duration_seconds":300}'
```

## Expected FSM Path

Dengan config lengkap, FSM keluar dari `IDLE` ke:

```text
WAIT_DAILY_MIX
PREPARE_DAILY_MIX
FILL_WATER
PRE_MIX_A
ADD_NUTRIENT_A
MIX_A
PRE_MIX_B
ADD_NUTRIENT_B
MIX_B
VALIDATE
READY
```

Saat mode timer irrigation aktif dan RTC masuk slot `23:00`, `READY` akan menjalankan timer irrigation.

## Sensor Conditions Needed

Config di atas hanya input konfigurasi. Agar FSM benar-benar maju sampai selesai, sensor juga harus memenuhi kondisi berikut:

- `FILL_WATER`: `tankVolume` harus naik sampai minimal `400 L`.
- `ADD_NUTRIENT_A`: `flowA` harus naik sampai `1500 ml`.
- `ADD_NUTRIENT_B`: `flowB` harus naik sampai `1500 ml`.
- `VALIDATE`: `ppm` harus masuk `1150 - 1250`.
- `VALIDATE`: `ph` harus masuk `5.5 - 6.5`.
- `READY` timer irrigation: mode soil/timer harus sudah `TIMER`, atau soil health fallback harus masuk timer sesuai logic `SoilHealthMonitor`.

