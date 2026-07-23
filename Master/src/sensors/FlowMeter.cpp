#include "FlowMeter.h"

FlowMeter::FlowMeter(uint8_t pin) {
    _pin = pin;

    pulseCount = 0;
    _countingEnabled = true;

    lastCalcTime = millis();

    lastPulseCount = 0;
}

void FlowMeter::begin(void (*isr)()) {
    pinMode(_pin, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(_pin),
        isr,
        RISING
    );
}

float FlowMeter::getVolumeLiter() {
    uint32_t pulses;
    noInterrupts();
    pulses = pulseCount;
    interrupts();

    return pulses / PULSES_PER_LITER;
}

float FlowMeter::getFlowRate() {
    unsigned long currentTime =
        millis();

    float elapsed = (currentTime - lastCalcTime) / 1000.0f;

    if(elapsed <= 0)
        return 0;

    uint32_t currentPulse;

    noInterrupts();
    currentPulse = pulseCount;
    interrupts();

    uint32_t pulses = currentPulse - lastPulseCount;

    lastPulseCount = currentPulse;

    lastCalcTime =  currentTime;

    return (pulses / PULSES_PER_LITER) * (60.0f / elapsed);
}

void FlowMeter::reset() {
    noInterrupts();

    pulseCount = 0;
    lastPulseCount = 0;

    interrupts();

    lastCalcTime = millis();
}

void FlowMeter::setPulseCount(uint32_t val) {
    noInterrupts();

    pulseCount = val;
    lastPulseCount = val;

    interrupts();

    lastCalcTime = millis();
}

void FlowMeter::setCountingEnabled(bool enabled) {
    noInterrupts();
    _countingEnabled = enabled;
    interrupts();
}

bool FlowMeter::isCountingEnabled() const {
    return _countingEnabled;
}

void IRAM_ATTR FlowMeter::recordPulseFromISR() {
    if (!_countingEnabled) return;
    unsigned long now = micros();
    if (now - _lastPulseUs < DEBOUNCE_US) return;
    _lastPulseUs = now;
    pulseCount++;
}
