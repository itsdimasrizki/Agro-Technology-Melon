#ifndef FLOW_METER_H
#define FLOW_METER_H

#include <Arduino.h>
#include "../config/Constants.h"  // FLOW_METER_PULSES_PER_LITER

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

    unsigned long lastCalcTime;
    uint32_t lastPulseCount;

    static constexpr float PULSES_PER_LITER = FLOW_METER_PULSES_PER_LITER;
};

#endif
