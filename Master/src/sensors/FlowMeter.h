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

    volatile uint32_t pulseCount;

private:

    uint8_t _pin;

    unsigned long lastCalcTime;
    uint32_t lastPulseCount;

    static constexpr float PULSES_PER_LITER = 450.0f;
};

#endif