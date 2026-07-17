# Master Test Mode

Set flags in `test/TestFlags.h`.

Only one test mode may be enabled at a time:

```cpp
#define ENABLE_FSM_SIMULATION_TEST        0
#define ENABLE_RELAY_HARDWARE_TEST        0
#define ENABLE_FULL_SYSTEM_TEST           0
#define ENABLE_MQTT_CONFIGURATION_TEST    0
```

## FSM Simulation Test

Uses hardcoded configuration and dummy `SensorData`.
MQTT is skipped.
Relay output still follows FSM commands.

## Relay Hardware Test

Skips MQTT and FSM.
Runs each relay in `RELAY_HARDWARE_TEST_SEQUENCE` ON for `RELAY_TEST_ON_MS`, then OFF for `RELAY_TEST_OFF_MS`.

## Full System Integration Test

Uses hardcoded configuration with the same payload structure as production MQTT.
MQTT is skipped.
FSM, relay, and real sensors run normally.

## MQTT Configuration Test

Loads hardcoded payloads with the same field names and JSON structure as production MQTT.
MQTT is skipped after configuration is loaded.


## Cara kerja mode

- ENABLE_FSM_SIMULATION_TEST: MQTT/Web/sensor fisik dilewati, config
hardcode dimuat, SensorManager diberi dummy SensorData, RTC memakai
waktu dummy. FSM tetap berjalan normal memakai flow/state produksi.

- ENABLE_RELAY_HARDWARE_TEST: setup hanya init relay, lalu loop menyalakan
relay 1 per 1 selama 5 detik dan mati 3 detik. FSM/MQTT/sensor dilewati.

- ENABLE_FULL_SYSTEM_TEST: config hardcode dimuat, MQTT dimatikan, FSM +
relay + sensor/hardware asli tetap berjalan.

- ENABLE_MQTT_CONFIGURATION_TEST: payload MQTT diganti data hardcode
dengan struktur field produksi, lalu sistem jalan tanpa koneksi MQTT.

Aktifkan mode dengan mengubah satu flag di Master/test TestFlags.h:3 dari
0 ke 1. Hanya boleh satu aktif; jika lebih dari satu, compile akan
berhenti dengan #error.
