#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>

enum RelayChannel {
    RELAY_MIXER_STIR = 1,      // ch1 — stirrer pengaduk mixing chamber (sebelumnya: RELAY_SOLENOID_WATER)
    RELAY_SOLENOID_A,
    RELAY_SOLENOID_B,
    RELAY_SOLENOID_IRRIG,
    RELAY_WATER_INLET,         // ch5 — solenoid NC air masuk (menggantikan buzzer)
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

    bool isValidRelayIndex(uint8_t index) const;

    void allOff();

private:

    uint8_t getPin(RelayChannel relay);
};

#endif
