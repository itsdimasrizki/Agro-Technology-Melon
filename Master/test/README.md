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
