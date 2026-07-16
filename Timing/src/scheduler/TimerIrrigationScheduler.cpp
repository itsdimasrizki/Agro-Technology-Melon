#include "TimerIrrigationScheduler.h"

static constexpr IrrigationSlot IRRIGATION_SLOTS[] = {
    {23,  18, 23,  19},
    {7,  0, 7,  5},
    {8,  0, 8,  5},
    {9,  0, 9,  5},
    {10,  0, 10,  5},
    {11,  0, 11,  5},
    {12,  0, 12,  5},
    {13,  0, 13,  5},
    {14,  0, 14,  5},
    {15, 0, 15, 5},
    {16, 0, 16, 5},
};

static constexpr uint8_t NUM_IRRIGATION_SLOTS =
    sizeof(IRRIGATION_SLOTS) / sizeof(IRRIGATION_SLOTS[0]);

static uint16_t toMinuteOfDay(uint8_t hour, uint8_t minute) {
    return static_cast<uint16_t>(hour * 60U + minute);
}

TimerIrrigationScheduler::TimerIrrigationScheduler(
    RTCManager& rtcManager,
    RelayManager& relayManager
)
:
rtc(rtcManager),
relay(relayManager)
{}

void TimerIrrigationScheduler::begin() {
    relay.allOff();

    state = TimingState::IDLE;
    activeSlot = -1;
}

void TimerIrrigationScheduler::update() {
    rtc.refresh();

    if (!rtc.isOk()) {
        applyState(TimingState::RTC_ERROR, -1);
        return;
    }

    uint16_t minuteNow = rtc.minuteOfDay();
    int8_t slotIndex = findActiveIrrigationSlot(minuteNow);

    if (slotIndex >= 0) {
        applyState(TimingState::IRRIGATING, slotIndex);
        return;
    }

    if (isMixingWindow(minuteNow)) {
        applyState(TimingState::MIXING, -1);
        return;
    }

    applyState(TimingState::IDLE, -1);
}

TimingState TimerIrrigationScheduler::getState() const {
    return state;
}

const char* TimerIrrigationScheduler::stateName() const {
    switch (state) {
        case TimingState::IDLE:       return "IDLE";
        case TimingState::MIXING:     return "MIXING";
        case TimingState::IRRIGATING: return "IRRIGATING";
        case TimingState::RTC_ERROR:  return "RTC_ERROR";
    }

    return "UNKNOWN";
}

void TimerIrrigationScheduler::applyState(TimingState nextState, int8_t slotIndex) {
    if (state == nextState && activeSlot == slotIndex) return;

    state = nextState;
    activeSlot = slotIndex;

    relay.off(RELAY_MIXER_STIR);
    relay.off(RELAY_SOLENOID_IRRIG);
    relay.off(RELAY_PUMP_MIX);

    switch (state) {
        case TimingState::MIXING:
            relay.on(RELAY_MIXER_STIR);
            relay.on(RELAY_PUMP_MIX);
            break;

        case TimingState::IRRIGATING:
            relay.on(RELAY_SOLENOID_IRRIG);
            relay.on(RELAY_PUMP_MIX);
            break;

        case TimingState::IDLE:
        case TimingState::RTC_ERROR:
            break;
    }
}

bool TimerIrrigationScheduler::isMixingWindow(uint16_t minuteNow) const {
    uint16_t firstStart = toMinuteOfDay(
        IRRIGATION_SLOTS[0].startHour,
        IRRIGATION_SLOTS[0].startMinute
    );
    uint16_t mixStart = firstStart >= MIXING_DURATION_MINUTES
        ? firstStart - MIXING_DURATION_MINUTES
        : 0;

    return minuteNow >= mixStart && minuteNow < firstStart;
}

int8_t TimerIrrigationScheduler::findActiveIrrigationSlot(uint16_t minuteNow) const {
    for (uint8_t i = 0; i < NUM_IRRIGATION_SLOTS; i++) {
        uint16_t start = toMinuteOfDay(IRRIGATION_SLOTS[i].startHour, IRRIGATION_SLOTS[i].startMinute);
        uint16_t stop = toMinuteOfDay(IRRIGATION_SLOTS[i].stopHour, IRRIGATION_SLOTS[i].stopMinute);

        if (minuteNow >= start && minuteNow < stop) {
            return static_cast<int8_t>(i);
        }
    }

    return -1;
}
