#include "RelayManager.h"
#include "../config/PinConfig.h"

void RelayManager::begin() {
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
        case RELAY_MIXER_STIR:          // ch1
            return RELAY_1_PIN;
        case RELAY_SOLENOID_A:  
            return RELAY_2_PIN;
        case RELAY_SOLENOID_B:  
            return RELAY_3_PIN;
        case RELAY_SOLENOID_IRRIG:  
            return RELAY_4_PIN;
        case RELAY_SOLENOID_WATER:        // ch5 — solenoid air + buzzer (parallel)
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
    digitalWrite(getPin(relay), LOW);
}

void RelayManager::off(RelayChannel relay) {
    digitalWrite(getPin(relay), HIGH);
}

void RelayManager::allOff() {
    off(RELAY_MIXER_STIR);
    off(RELAY_SOLENOID_A);
    off(RELAY_SOLENOID_B);
    off(RELAY_SOLENOID_IRRIG);
    off(RELAY_SOLENOID_WATER);
    off(RELAY_PUMP_A);
    off(RELAY_PUMP_B);
    off(RELAY_PUMP_MIX);
}