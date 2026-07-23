#ifndef FLOW_METER_H
#define FLOW_METER_H

#include <Arduino.h>

class FlowMeter {
public:

    FlowMeter(uint8_t pin);

    void begin(void (*isr)());

    float getVolumeLiter();

    float getFlowRate();

    void reset();

    void setPulseCount(uint32_t val);

    void setCountingEnabled(bool enabled);

    bool isCountingEnabled() const;

    void recordPulseFromISR();

    volatile uint32_t pulseCount;

private:

    uint8_t _pin;
    volatile bool _countingEnabled;

    // Debounce: abaikan pulse yang datang < DEBOUNCE_US setelah pulse sebelumnya.
    // Getaran solenoid/relay terjadi dalam mikro-detik; aliran asli min ~26ms
    // bahkan pada flow rate tertinggi submersible pompa nutrisi ini.
    volatile unsigned long _lastPulseUs = 0;
    static constexpr unsigned long DEBOUNCE_US = 5000UL; // 5ms

    unsigned long lastCalcTime;
    uint32_t lastPulseCount;

    static constexpr float PULSES_PER_LITER = 125.0f;
};

#endif
