#pragma once

#include "actuators/RelayManager.h"

static const unsigned long RELAY_TEST_ON_MS  = 5000UL;
static const unsigned long RELAY_TEST_OFF_MS = 3000UL;

static const RelayChannel RELAY_HARDWARE_TEST_SEQUENCE[] = {
    RELAY_MIXER_STIR,
    RELAY_SOLENOID_A,
    RELAY_SOLENOID_B,
    RELAY_SOLENOID_IRRIG,
    RELAY_WATER_INLET,
    RELAY_PUMP_A,
    RELAY_PUMP_B,
    RELAY_PUMP_MIX
};

static const uint8_t RELAY_HARDWARE_TEST_COUNT =
    sizeof(RELAY_HARDWARE_TEST_SEQUENCE) / sizeof(RELAY_HARDWARE_TEST_SEQUENCE[0]);
