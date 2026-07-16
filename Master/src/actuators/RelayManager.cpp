#include "RelayManager.h"
#include "../config/PinConfig.h"

static uint8_t relayOnLevel(RelayChannel relay) {
    switch (relay) {
        case RELAY_MIXER_STIR:
        case RELAY_PUMP_A:
        case RELAY_PUMP_B:
        case RELAY_PUMP_MIX:
            return LOW;

        case RELAY_SOLENOID_A:
        case RELAY_SOLENOID_B:
        case RELAY_SOLENOID_IRRIG:
        case RELAY_WATER_INLET:
            return HIGH;
    }

    return HIGH;
}

static uint8_t relayOffLevel(RelayChannel relay) {
    return relayOnLevel(relay) == HIGH ? LOW : HIGH;
}

void RelayManager::begin() {
    allOff();

    pinMode(RELAY_1_PIN, OUTPUT);
    pinMode(RELAY_2_PIN, OUTPUT);
    pinMode(RELAY_3_PIN, OUTPUT);
    pinMode(RELAY_4_PIN, OUTPUT);
    pinMode(RELAY_5_PIN, OUTPUT);
    pinMode(RELAY_6_PIN, OUTPUT);
    pinMode(RELAY_7_PIN, OUTPUT);
    pinMode(RELAY_8_PIN, OUTPUT);

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
    digitalWrite(getPin(relay), relayOnLevel(relay));
}

void RelayManager::off(RelayChannel relay) {
    digitalWrite(getPin(relay), relayOffLevel(relay));
}

bool RelayManager::isOn(RelayChannel relay) const {
    switch (relay) {
        case RELAY_MIXER_STIR:    return digitalRead(RELAY_1_PIN) == relayOnLevel(relay);
        case RELAY_SOLENOID_A:    return digitalRead(RELAY_2_PIN) == relayOnLevel(relay);
        case RELAY_SOLENOID_B:    return digitalRead(RELAY_3_PIN) == relayOnLevel(relay);
        case RELAY_SOLENOID_IRRIG: return digitalRead(RELAY_4_PIN) == relayOnLevel(relay);
        case RELAY_WATER_INLET:   return digitalRead(RELAY_5_PIN) == relayOnLevel(relay);
        case RELAY_PUMP_A:        return digitalRead(RELAY_6_PIN) == relayOnLevel(relay);
        case RELAY_PUMP_B:        return digitalRead(RELAY_7_PIN) == relayOnLevel(relay);
        case RELAY_PUMP_MIX:      return digitalRead(RELAY_8_PIN) == relayOnLevel(relay);
    }

    return false;
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
