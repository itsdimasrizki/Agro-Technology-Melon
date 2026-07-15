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
        case RELAY_WATER_INLET:        // ch5
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

bool RelayManager::isOn(RelayChannel relay) const {
    switch (relay) {
        case RELAY_MIXER_STIR:    return digitalRead(RELAY_1_PIN) == LOW;
        case RELAY_SOLENOID_A:    return digitalRead(RELAY_2_PIN) == LOW;
        case RELAY_SOLENOID_B:    return digitalRead(RELAY_3_PIN) == LOW;
        case RELAY_SOLENOID_IRRIG: return digitalRead(RELAY_4_PIN) == LOW;
        case RELAY_WATER_INLET:   return digitalRead(RELAY_5_PIN) == LOW;
        case RELAY_PUMP_A:        return digitalRead(RELAY_6_PIN) == LOW;
        case RELAY_PUMP_B:        return digitalRead(RELAY_7_PIN) == LOW;
        case RELAY_PUMP_MIX:      return digitalRead(RELAY_8_PIN) == LOW;
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
