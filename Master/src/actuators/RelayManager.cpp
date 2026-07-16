#include "RelayManager.h"
#include "../config/PinConfig.h"

static uint8_t relayOnLevel(RelayChannel relay) {
    switch (relay) {
        case RELAY_PUMP_A:
        case RELAY_PUMP_B:
        case RELAY_WATER_INLET:
        case RELAY_PUMP_MIX:
            return LOW;
        
        case RELAY_MIXER_STIR:
        case RELAY_SOLENOID_A:
        case RELAY_SOLENOID_B:
        case RELAY_SOLENOID_IRRIG:
            return HIGH;
    }

    return HIGH;
}

static bool relayOffUsesInput(RelayChannel relay) {
    return relayOnLevel(relay) == LOW;
}

static bool relayIsUnmanaged(RelayChannel relay) {
    return relay == RELAY_PUMP_A || relay == RELAY_PUMP_B;
}

static uint8_t relayOffLevel(RelayChannel relay) {
    return relayOnLevel(relay) == HIGH ? LOW : HIGH;
}

static bool relayState[9] = {};

static void configureRelayPin(uint8_t pin, RelayChannel relay) {
    if (relayIsUnmanaged(relay)) {
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT);
        return;
    }

    if (relayOffUsesInput(relay)) {
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT);
        return;
    }

    digitalWrite(pin, relayOffLevel(relay));
    pinMode(pin, OUTPUT);
    digitalWrite(pin, relayOffLevel(relay));
}

void RelayManager::begin() {
    configureRelayPin(RELAY_1_PIN, RELAY_MIXER_STIR);
    configureRelayPin(RELAY_2_PIN, RELAY_SOLENOID_A);
    configureRelayPin(RELAY_3_PIN, RELAY_SOLENOID_B);
    configureRelayPin(RELAY_4_PIN, RELAY_SOLENOID_IRRIG);
    configureRelayPin(RELAY_5_PIN, RELAY_WATER_INLET);
    configureRelayPin(RELAY_6_PIN, RELAY_PUMP_A);
    configureRelayPin(RELAY_7_PIN, RELAY_PUMP_B);
    configureRelayPin(RELAY_8_PIN, RELAY_PUMP_MIX);

    allOff();
}

uint8_t RelayManager::getPin(RelayChannel relay) {
    switch(relay) {
        case RELAY_MIXER_STIR:          
            return RELAY_1_PIN;
        case RELAY_SOLENOID_A:  
            return RELAY_2_PIN;
        case RELAY_SOLENOID_B:  
            return RELAY_3_PIN;
        case RELAY_SOLENOID_IRRIG:  
            return RELAY_4_PIN;
        case RELAY_WATER_INLET:        
            return RELAY_5_PIN;
        case RELAY_PUMP_A:           
            return RELAY_6_PIN;
        case RELAY_PUMP_B:           
            return RELAY_7_PIN;
        case RELAY_PUMP_MIX:           
            return RELAY_8_PIN;
    }

    return RELAY_1_PIN;
}

void RelayManager::on(RelayChannel relay) {
    uint8_t pin = getPin(relay);

    if (relayIsUnmanaged(relay)) {
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT);
        relayState[relay] = false;
        return;
    }

    digitalWrite(pin, relayOnLevel(relay));
    pinMode(pin, OUTPUT);
    digitalWrite(pin, relayOnLevel(relay));

    relayState[relay] = true;
}

void RelayManager::off(RelayChannel relay) {
    uint8_t pin = getPin(relay);

    if (relayIsUnmanaged(relay)) {
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT);
        relayState[relay] = false;
        return;
    }

    if (relayOffUsesInput(relay)) {
        digitalWrite(pin, LOW);
        pinMode(pin, INPUT);
    } else {
        digitalWrite(pin, relayOffLevel(relay));
        pinMode(pin, OUTPUT);
        digitalWrite(pin, relayOffLevel(relay));
    }

    relayState[relay] = false;
}

bool RelayManager::isOn(RelayChannel relay) const {
    return relay >= RELAY_MIXER_STIR && relay <= RELAY_PUMP_MIX
        ? relayState[relay]
        : false;
}

bool RelayManager::isValidRelayIndex(uint8_t index) const {
    return index >= 1 && index <= 8;
}

void RelayManager::allOff() {
    off(RELAY_MIXER_STIR);
    off(RELAY_SOLENOID_A);
    off(RELAY_SOLENOID_B);
    off(RELAY_SOLENOID_IRRIG);
    off(RELAY_WATER_INLET);
    off(RELAY_PUMP_A);
    off(RELAY_PUMP_B);
    off(RELAY_PUMP_MIX);
}
