#ifndef RELAY_HARDWARE_TEST_DATA_H
#define RELAY_HARDWARE_TEST_DATA_H

#include "../../src/actuators/RelayManager.h"

namespace RelayHardwareTestData {

static constexpr unsigned long ON_DURATION_MS = 5000UL;
static constexpr unsigned long OFF_DURATION_MS = 3000UL;

static const RelayChannel RELAYS[] = {
    RELAY_MIXER_STIR,
    RELAY_SOLENOID_A,
    RELAY_SOLENOID_B,
    RELAY_SOLENOID_IRRIG,
    RELAY_WATER_INLET,
    RELAY_PUMP_A,
    RELAY_PUMP_B,
    RELAY_PUMP_MIX
};

static const char* const RELAY_NAMES[] = {
    "relay_1_mixer_stir",
    "relay_2_solenoid_a",
    "relay_3_solenoid_b",
    "relay_4_solenoid_irrigation",
    "relay_5_water_inlet",
    "relay_6_pump_a",
    "relay_7_pump_b",
    "relay_8_pump_mix"
};

static constexpr uint8_t RELAY_COUNT = sizeof(RELAYS) / sizeof(RELAYS[0]);

}

#endif
