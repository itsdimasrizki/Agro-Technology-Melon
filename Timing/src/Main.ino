#include <Arduino.h>
#include <Wire.h>

#include "config/PinConfig.h"
#include "actuators/RelayManager.h"
#include "rtc/RTCManager.h"
#include "scheduler/TimerIrrigationScheduler.h"

RelayManager relay;
RTCManager rtcManager;
TimerIrrigationScheduler scheduler(rtcManager, relay);

static constexpr unsigned long STATUS_LOG_INTERVAL_MS = 5000UL;
static constexpr uint16_t RTC_BOOTSTRAP_YEAR = 2026;
static constexpr uint8_t RTC_BOOTSTRAP_MONTH = 7;
static constexpr uint8_t RTC_BOOTSTRAP_DAY = 16;
static constexpr uint8_t RTC_BOOTSTRAP_HOUR = 23;
static constexpr uint8_t RTC_BOOTSTRAP_MINUTE = 2;
static constexpr uint8_t RTC_BOOTSTRAP_SECOND = 0;

void logLine(const char* level, const char* component, const char* message) {
    Serial.printf(
        "t=%010lu | %-5s | %-8s | %s\n",
        millis(),
        level,
        component,
        message
    );
}

void logStatus() {
    Serial.printf(
        "t=%010lu | INFO  | STATUS   | rtc=%s date=%04u-%02u-%02u time=%02u:%02u state=%s mix=%u irrig=%u pump_mix=%u\n",
        millis(),
        rtcManager.isOk() ? "OK" : "ERROR",
        rtcManager.getYear(),
        rtcManager.getMonth(),
        rtcManager.getDay(),
        rtcManager.getHour(),
        rtcManager.getMinute(),
        scheduler.stateName(),
        relay.isOn(RELAY_MIXER_STIR),
        relay.isOn(RELAY_SOLENOID_IRRIG),
        relay.isOn(RELAY_PUMP_MIX)
    );
}

bool isRtcBeforeBootstrapTime() {
    if (rtcManager.getYear() != RTC_BOOTSTRAP_YEAR) {
        return rtcManager.getYear() < RTC_BOOTSTRAP_YEAR;
    }
    if (rtcManager.getMonth() != RTC_BOOTSTRAP_MONTH) {
        return rtcManager.getMonth() < RTC_BOOTSTRAP_MONTH;
    }
    return rtcManager.getDay() < RTC_BOOTSTRAP_DAY;
}

void setup() {
    Serial.begin(115200);
    unsigned long serialStart = millis();
    while (!Serial && (millis() - serialStart) < 3000) delay(10);

    logLine("INFO", "BOOT", "serial=ready");

    relay.begin();
    logLine("INFO", "RELAY", "ready");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    logLine("INFO", "I2C", "ready");

    rtcManager.begin();
    logLine("INFO", "RTC", rtcManager.isOk() ? "ready" : "error");
    if (rtcManager.isOk() && isRtcBeforeBootstrapTime()) {
        rtcManager.setDateTime(
            RTC_BOOTSTRAP_YEAR,
            RTC_BOOTSTRAP_MONTH,
            RTC_BOOTSTRAP_DAY,
            RTC_BOOTSTRAP_HOUR,
            RTC_BOOTSTRAP_MINUTE,
            RTC_BOOTSTRAP_SECOND
        );
        logLine("INFO", "RTC", "time=bootstrapped");
    }

    scheduler.begin();
    logLine("INFO", "SCHED", "ready");

    logLine("INFO", "BOOT", "system=ready");
}

void loop() {
    scheduler.update();

    static unsigned long lastStatusLog = 0;
    if (millis() - lastStatusLog >= STATUS_LOG_INTERVAL_MS) {
        lastStatusLog = millis();
        logStatus();
    }

    delay(250);
}
