#ifndef TIMER_IRRIGATION_SCHEDULER_H
#define TIMER_IRRIGATION_SCHEDULER_H

#include <Arduino.h>
#include "../actuators/RelayManager.h"
#include "../rtc/RTCManager.h"

struct IrrigationSlot {
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t stopHour;
    uint8_t stopMinute;
};

enum class TimingState : uint8_t {
    IDLE,
    MIXING,
    IRRIGATING,
    RTC_ERROR
};

class TimerIrrigationScheduler {
public:
    TimerIrrigationScheduler(RTCManager& rtcManager, RelayManager& relayManager);

    void begin();
    void update();

    TimingState getState() const;
    const char* stateName() const;

private:
    static constexpr uint16_t MIXING_DURATION_MINUTES = 1;

    RTCManager& rtc;
    RelayManager& relay;

    TimingState state = TimingState::IDLE;
    int8_t activeSlot = -1;

    void applyState(TimingState nextState, int8_t slotIndex);
    bool isMixingWindow(uint16_t minuteNow) const;
    int8_t findActiveIrrigationSlot(uint16_t minuteNow) const;
};

#endif
