#pragma once

#include <ArduinoJson.h>
#include "config/ConfigManager.h"

// Data khusus timer irrigation. Dipakai hanya saat IRRIGATION_MODE_SOURCE = 1
// di Master/src/config/SystemConfig.h.
//
// Setiap slot adalah window RTC:
// - start_hour/start_minute = relay irigasi ON
// - end_hour/end_minute     = relay irigasi OFF
static const char TIMER_IRRIGATION_PAYLOAD[] = R"json(
{
  "slots": [
    {
      "start_hour": 7,
      "start_minute": 0,
      "end_hour": 7,
      "end_minute": 3
    },
    {
      "start_hour": 8,
      "start_minute": 0,
      "end_hour": 8,
      "end_minute": 3
    },
    {
      "start_hour": 9,
      "start_minute": 0,
      "end_hour": 9,
      "end_minute": 3
    },
    {
      "start_hour": 10,
      "start_minute": 0,
      "end_hour": 10,
      "end_minute": 3
    },
    {
      "start_hour": 11,
      "start_minute": 0,
      "end_hour": 11,
      "end_minute": 3
    },
    {
      "start_hour": 12,
      "start_minute": 0,
      "end_hour": 12,
      "end_minute": 3
    },
    {
      "start_hour": 13,
      "start_minute": 0,
      "end_hour": 13,
      "end_minute": 3
    },
    {
      "start_hour": 14,
      "start_minute": 0,
      "end_hour": 14,
      "end_minute": 3
    },
    {
      "start_hour": 15,
      "start_minute": 0,
      "end_hour": 15,
      "end_minute": 3
    },
    {
      "start_hour": 16,
      "start_minute": 0,
      "end_hour": 16,
      "end_minute": 3
    }
  ]
}
)json";

inline void loadTimerIrrigationData(ConfigManager& configManager) {
    JsonDocument timerDoc;
    deserializeJson(timerDoc, TIMER_IRRIGATION_PAYLOAD);

    IrrigationSlot slots[MAX_IRRIG_SLOTS];
    uint8_t slotCount = 0;
    for (JsonObjectConst s : timerDoc["slots"].as<JsonArrayConst>()) {
        if (slotCount >= MAX_IRRIG_SLOTS) break;
        slots[slotCount].startHour = s["start_hour"].as<uint8_t>();
        slots[slotCount].startMinute = s["start_minute"].as<uint8_t>();
        slots[slotCount].endHour = s["end_hour"].as<uint8_t>();
        slots[slotCount].endMinute = s["end_minute"].as<uint8_t>();
        slotCount++;
    }

    configManager.setTimerIrrigationConfig(slots, slotCount);
}
