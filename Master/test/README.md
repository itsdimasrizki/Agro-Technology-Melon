# Master Test Mode

Test mode dikontrol dari `test/TestFlags.h`.

Hanya satu flag boleh bernilai `1` dalam satu build:

```cpp
#define ENABLE_FSM_SIMULATION_TEST        0
#define ENABLE_RELAY_HARDWARE_TEST        0
#define ENABLE_FULL_SYSTEM_TEST           0
#define ENABLE_MQTT_CONFIGURATION_TEST    0
```

Jika lebih dari satu flag aktif, build berhenti dengan compile error.

## Mode

- `ENABLE_FSM_SIMULATION_TEST`: menjalankan FSM tanpa MQTT, Web, ESP-NOW, dan sensor asli. Data dummy diberikan melalui `test/data/FSMStateSimulationData.h`.
- `ENABLE_RELAY_HARDWARE_TEST`: menyalakan setiap relay satu per satu selama 5 detik, lalu OFF 3 detik. FSM dan MQTT tidak dijalankan.
- `ENABLE_FULL_SYSTEM_TEST`: menjalankan FSM + relay + hardware tanpa MQTT. Sensor asli tetap digunakan jika tersedia, konfigurasi hardcode memakai payload yang sama dengan MQTT produksi.
- `ENABLE_MQTT_CONFIGURATION_TEST`: mengganti payload MQTT dengan JSON hardcode dari `test/data/MQTTConfigurationTestData.h`, lalu menerapkan konfigurasi melalui setter produksi.

## Hardware yang tetap dibutuhkan

Relay hardware test membutuhkan board dan modul relay fisik. Full system test membutuhkan hardware sensor dan aktuator agar hasilnya mewakili sistem nyata. FSM simulation test tidak membutuhkan sensor asli, tetapi tetap berjalan di board ESP32 karena firmware memakai API Arduino.

• Selesai. Saya menambahkan TEST MODE framework untuk Master/ tanpa mengubah
  FSM flow dan tanpa mengubah RelayManager.

  File Diubah

  - Master/src/Main.ino:34: wiring test mode, hardcoded MQTT payload loader,
    relay test loop, MQTT bypass saat test.

  - Master/src/sensors/SensorManager.h:30: optional test sensor override.
  - Master/src/sensors/SensorManager.cpp:23: dummy sensor data path saat
    test.

  - Master/src/rtc/RTCManager.h:32: optional test clock.
  - Master/src/rtc/RTCManager.cpp:17: RTC bypass hanya saat test.
  - Master/src/config/Constants.h:4: durasi FSM dipercepat hanya untuk FSM
    simulation.

  File Test Baru

  - Master/test/TestFlags.h:4
  - Master/test/data/MQTTConfigurationTestData.h:8
  - Master/test/data/FSMStateSimulationData.h:10
  - Master/test/data/RelayHardwareTestData.h:8
  - Master/test/data/FullSystemIntegrationTestData.h:8
  - Master/test/README.md:1

  Field MQTT Produksi

  - greenhouse/config/system: total_plants, max_consumption_per_plant,
    target_fill_volume, tank_capacity_liter, tank_height_cm,
    tank_diameter_cm

  - greenhouse/config/schedule: plant_year, plant_month, plant_day,
    daily_mix_hour, daily_mix_minute

  - greenhouse/config/ppm: ppm_tolerance, initial_a, initial_b
  - greenhouse/config/ph: min_ph, max_ph
  - greenhouse/config/recipe: stages[].max_age_days, stages[].target_ppm,
    stages[].min_ph, stages[].max_ph

  - greenhouse/config/irrigation: stages[].max_age_days,
    stages[].dry_threshold, stages[].wet_threshold

  - Tambahan runtime: greenhouse/config/timer_irrigation dengan
    slots[].hour, slots[].minute; greenhouse/config/mix_schedule_ext dengan
    stir_evening_hour, stir_evening_minute, stir_duration_seconds

  Struktur Payload
  Payload hardcode dibuat sebagai JSON produksi di Master/test/data/
  MQTTConfigurationTestData.h:8. FullSystemIntegrationTestData.h memakai
  struktur yang sama persis dengan payload MQTT produksi melalui alias data
  tersebut.

  Cara Kerja Mode

  - ENABLE_FSM_SIMULATION_TEST: MQTT/Web/ESP-NOW/sensor asli dilewati.
    SensorManager diberi SensorData dummy berdasarkan state FSM, dan RTC
    memakai test clock.

  - ENABLE_RELAY_HARDWARE_TEST: hanya relay.begin() lalu setiap relay ON 5
    detik dan OFF 3 detik. FSM dan MQTT tidak dijalankan.

  - ENABLE_FULL_SYSTEM_TEST: config hardcode diterapkan dari payload
    produksi, MQTT dilewati, FSM + relay + sensor/hardware asli berjalan.

  - ENABLE_MQTT_CONFIGURATION_TEST: payload MQTT diganti JSON hardcode,
    diparse, lalu diterapkan lewat setter produksi ConfigManager.

  Aktivasi
  Ubah satu flag saja di Master/test/TestFlags.h:4 dari 0 ke 1. Jika lebih
  dari satu aktif, compile berhenti lewat #error.

  Tidak Bisa Dites Tanpa Hardware

  - Relay hardware test butuh board ESP32 dan modul relay fisik.
  - Full system test butuh sensor pH, TDS, water level, flow meter, soil
    data/ESP-NOW, relay, pompa, dan solenoid untuk hasil nyata.

  - FSM simulation tidak butuh sensor asli, tetapi tetap butuh target
    firmware ESP32/Arduino untuk menjalankan binary.

  Verifikasi: pio run sukses untuk default produksi dan untuk keempat mode
  test satu per satu.