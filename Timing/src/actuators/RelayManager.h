#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>

enum RelayChannel {
    RELAY_MIXER_STIR = 1,
    RELAY_SOLENOID_A,
    RELAY_SOLENOID_B,
    RELAY_SOLENOID_IRRIG,
    RELAY_WATER_INLET,
    RELAY_PUMP_A,
    RELAY_PUMP_B,
    RELAY_PUMP_MIX
};

class RelayManager {
public:
    void begin();

    void on(RelayChannel relay);
    void off(RelayChannel relay);
    bool isOn(RelayChannel relay) const;

    void allOff();

private:
    uint8_t getPin(RelayChannel relay) const;
};

#endif
