#pragma once

#include <ArduinoJson.h>
#include "config/ConfigManager.h"

static const char TEST_MQTT_PAYLOAD_PPM[] = R"json(
{
  "ppm_tolerance": 50.0,
  "initial_a": 0.2,
  "initial_b": 0.2
}
)json";

static const char TEST_MQTT_PAYLOAD_PH[] = R"json(
{
  "min_ph": 5.5,
  "max_ph": 6.5
}
)json";

static const char TEST_MQTT_PAYLOAD_RECIPE[] = R"json(
{
  "stages": [
    {
      "max_age_days": 120,
      "target_ppm": 900.0,
      "min_ph": 5.5,
      "max_ph": 6.5
    }
  ]
}
)json";

static const char TEST_MQTT_PAYLOAD_IRRIGATION[] = R"json(
{
  "stages": [
    {
      "max_age_days": 120,
      "dry_threshold": 3900,
      "wet_threshold": 3650
    }
  ]
}
)json";

static const char TEST_MQTT_PAYLOAD_SYSTEM[] = R"json(
{
  "total_plants": 80,
  "max_consumption_per_plant": 0.05,
  "target_fill_volume": 1.0,
  "tank_capacity_liter": 15.0,
  "tank_height_cm": 45.0,
  "tank_diameter_cm": 20.6
}
)json";

static const char TEST_MQTT_PAYLOAD_SCHEDULE[] = R"json(
{
  "plant_year": 2026,
  "plant_month": 7,
  "plant_day": 1,
  "daily_mix_hour": 5,
  "daily_mix_minute": 0
}
)json";

static const char TEST_MQTT_PAYLOAD_TIMER_IRRIGATION[] = R"json(
{
  "slots": [
    {
      "hour": 6,
      "minute": 0
    },
    {
      "hour": 17,
      "minute": 0
    }
  ]
}
)json";

static const char TEST_MQTT_PAYLOAD_MIX_SCHEDULE_EXT[] = R"json(
{
  "stir_evening_hour": 18,
  "stir_evening_minute": 0,
  "stir_duration_seconds": 300
}
)json";

inline void loadMQTTConfigurationTestData(ConfigManager& configManager) {
    JsonDocument ppmDoc;
    deserializeJson(ppmDoc, TEST_MQTT_PAYLOAD_PPM);
    configManager.setPPMConfig(
        ppmDoc["ppm_tolerance"].as<float>(),
        ppmDoc["initial_a"].as<float>(),
        ppmDoc["initial_b"].as<float>()
    );

    JsonDocument phDoc;
    deserializeJson(phDoc, TEST_MQTT_PAYLOAD_PH);
    configManager.setPHConfig(
        phDoc["min_ph"].as<float>(),
        phDoc["max_ph"].as<float>()
    );

    JsonDocument recipeDoc;
    deserializeJson(recipeDoc, TEST_MQTT_PAYLOAD_RECIPE);
    RecipeStageConfig recipeStages[MAX_RECIPE_STAGES];
    uint8_t recipeCount = 0;
    for (JsonObjectConst s : recipeDoc["stages"].as<JsonArrayConst>()) {
        if (recipeCount >= MAX_RECIPE_STAGES) break;
        recipeStages[recipeCount].maxAgeDays = s["max_age_days"].as<uint16_t>();
        recipeStages[recipeCount].targetPPM  = s["target_ppm"].as<float>();
        recipeStages[recipeCount].minPH      = s["min_ph"].as<float>();
        recipeStages[recipeCount].maxPH      = s["max_ph"].as<float>();
        recipeCount++;
    }
    configManager.setRecipeStages(recipeStages, recipeCount);

    JsonDocument irrigationDoc;
    deserializeJson(irrigationDoc, TEST_MQTT_PAYLOAD_IRRIGATION);
    IrrigationStageConfig irrigationStages[MAX_IRRIGATION_STAGES];
    uint8_t irrigationCount = 0;
    for (JsonObjectConst s : irrigationDoc["stages"].as<JsonArrayConst>()) {
        if (irrigationCount >= MAX_IRRIGATION_STAGES) break;
        irrigationStages[irrigationCount].maxAgeDays   = s["max_age_days"].as<uint16_t>();
        irrigationStages[irrigationCount].dryThreshold = s["dry_threshold"].as<uint16_t>();
        irrigationStages[irrigationCount].wetThreshold = s["wet_threshold"].as<uint16_t>();
        irrigationCount++;
    }
    configManager.setIrrigationStages(irrigationStages, irrigationCount);

    JsonDocument systemDoc;
    deserializeJson(systemDoc, TEST_MQTT_PAYLOAD_SYSTEM);
    configManager.setSystemConfig(
        systemDoc["total_plants"].as<uint16_t>(),
        systemDoc["max_consumption_per_plant"].as<float>(),
        systemDoc["target_fill_volume"].as<float>(),
        systemDoc["tank_capacity_liter"].as<float>(),
        systemDoc["tank_height_cm"].as<float>(),
        systemDoc["tank_diameter_cm"].as<float>()
    );

    JsonDocument scheduleDoc;
    deserializeJson(scheduleDoc, TEST_MQTT_PAYLOAD_SCHEDULE);
    configManager.setScheduleConfig(
        scheduleDoc["plant_year"].as<uint16_t>(),
        scheduleDoc["plant_month"].as<uint8_t>(),
        scheduleDoc["plant_day"].as<uint8_t>(),
        scheduleDoc["daily_mix_hour"].as<uint8_t>(),
        scheduleDoc["daily_mix_minute"].as<uint8_t>()
    );

    JsonDocument timerDoc;
    deserializeJson(timerDoc, TEST_MQTT_PAYLOAD_TIMER_IRRIGATION);
    IrrigationSlot slots[MAX_IRRIG_SLOTS];
    uint8_t slotCount = 0;
    for (JsonObjectConst s : timerDoc["slots"].as<JsonArrayConst>()) {
        if (slotCount >= MAX_IRRIG_SLOTS) break;
        slots[slotCount].hour   = s["hour"].as<uint8_t>();
        slots[slotCount].minute = s["minute"].as<uint8_t>();
        slotCount++;
    }
    configManager.setTimerIrrigationConfig(slots, slotCount);

    JsonDocument mixDoc;
    deserializeJson(mixDoc, TEST_MQTT_PAYLOAD_MIX_SCHEDULE_EXT);
    configManager.setStirSchedule(
        mixDoc["stir_evening_hour"].as<uint8_t>(),
        mixDoc["stir_evening_minute"].as<uint8_t>(),
        mixDoc["stir_duration_seconds"].as<uint32_t>() * 1000UL
    );
}
