#pragma once

#include <ArduinoJson.h>
#include "config/ConfigManager.h"

// Full system integration test — konfigurasi identik dengan produksi:
//   - Toren 640L / 700L, dimensi nyata (142cm × 83cm)
//   - Resep 7 tahap (0→500→750→900→1100→1200→1100 ppm)
//   - Timer irrigation 10 slot, 07:00–16:00, masing-masing 3 menit
//   - Stir sore 18:00, 5 menit
//
// Satu-satunya perbedaan dengan produksi: timer mixing dikompres via
// #if ENABLE_FULL_SYSTEM_TEST di Constants.h (semua jadi 1 menit).
// Timeout & batas air tetap nilai asli agar pengisian air nyata tervalidasi.
//
// SKIP_DAILY_SCHEDULE otomatis aktif saat ENABLE_FULL_SYSTEM_TEST=1
// (lihat FertigationFSM.cpp::begin()), sehingga sistem langsung masuk
// PREPARE_DAILY_MIX tanpa menunggu jam 04:00.

static const char FULL_TEST_PAYLOAD_SYSTEM[] = R"json(
{
  "total_plants": 80,
  "max_consumption_per_plant": 2.0,
  "target_fill_volume": 640.0,
  "tank_capacity_liter": 700.0,
  "tank_height_cm": 142.0,
  "tank_diameter_cm": 83.0
}
)json";

static const char FULL_TEST_PAYLOAD_SCHEDULE[] = R"json(
{
  "plant_year": 2026,
  "plant_month": 7,
  "plant_day": 5,
  "daily_mix_hour": 4,
  "daily_mix_minute": 0
}
)json";

static const char FULL_TEST_PAYLOAD_PPM[] = R"json(
{
  "ppm_tolerance": 50.0,
  "initial_a": 1.0,
  "initial_b": 1.0
}
)json";

static const char FULL_TEST_PAYLOAD_PH[] = R"json(
{
  "min_ph": 5.9,
  "max_ph": 8.0
}
)json";

static const char FULL_TEST_PAYLOAD_RECIPE[] = R"json(
{
  "stages": [
    { "max_age_days": 1,  "target_ppm": 0.0,    "min_ph": 5.9, "max_ph": 6.5 },
    { "max_age_days": 3,  "target_ppm": 500.0,  "min_ph": 5.9, "max_ph": 6.5 },
    { "max_age_days": 5,  "target_ppm": 750.0,  "min_ph": 5.9, "max_ph": 6.5 },
    { "max_age_days": 8,  "target_ppm": 900.0,  "min_ph": 5.9, "max_ph": 6.5 },
    { "max_age_days": 14, "target_ppm": 1100.0, "min_ph": 5.9, "max_ph": 6.5 },
    { "max_age_days": 58, "target_ppm": 1200.0, "min_ph": 5.9, "max_ph": 6.5 },
    { "max_age_days": 70, "target_ppm": 1100.0, "min_ph": 5.9, "max_ph": 6.5 }
  ]
}
)json";

static const char FULL_TEST_PAYLOAD_IRRIGATION[] = R"json(
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

// Slot sama persis dengan produksi (07:00–16:00, setiap jam, 3 menit).
// Timer irrigation hanya aktif saat READY — selama siklus mixing berlangsung
// relay irigasi tidak menyala.
static const char FULL_TEST_PAYLOAD_TIMER_IRRIGATION[] = R"json(
{
  "slots": [
    { "start_hour": 7,  "start_minute": 0, "end_hour": 7,  "end_minute": 3 },
    { "start_hour": 8,  "start_minute": 0, "end_hour": 8,  "end_minute": 3 },
    { "start_hour": 9,  "start_minute": 0, "end_hour": 9,  "end_minute": 3 },
    { "start_hour": 10, "start_minute": 0, "end_hour": 10, "end_minute": 3 },
    { "start_hour": 11, "start_minute": 0, "end_hour": 11, "end_minute": 3 },
    { "start_hour": 12, "start_minute": 0, "end_hour": 12, "end_minute": 3 },
    { "start_hour": 13, "start_minute": 0, "end_hour": 13, "end_minute": 3 },
    { "start_hour": 14, "start_minute": 0, "end_hour": 14, "end_minute": 3 },
    { "start_hour": 15, "start_minute": 0, "end_hour": 15, "end_minute": 3 },
    { "start_hour": 16, "start_minute": 0, "end_hour": 16, "end_minute": 3 }
  ]
}
)json";

static const char FULL_TEST_PAYLOAD_MIX_SCHEDULE_EXT[] = R"json(
{
  "stir_evening_hour": 18,
  "stir_evening_minute": 0,
  "stir_duration_seconds": 300
}
)json";

inline void loadFullSystemIntegrationTestConfiguration(ConfigManager& configManager) {
    JsonDocument systemDoc;
    deserializeJson(systemDoc, FULL_TEST_PAYLOAD_SYSTEM);
    float tankCap   = systemDoc["tank_capacity_liter"].as<float>();
    float fillTarget = systemDoc["target_fill_volume"].as<float>();
    if (tankCap > 0.0f && fillTarget > tankCap) fillTarget = tankCap;
    configManager.setSystemConfig(
        systemDoc["total_plants"].as<uint16_t>(),
        systemDoc["max_consumption_per_plant"].as<float>(),
        fillTarget,
        tankCap,
        systemDoc["tank_height_cm"].as<float>(),
        systemDoc["tank_diameter_cm"].as<float>()
    );

    JsonDocument scheduleDoc;
    deserializeJson(scheduleDoc, FULL_TEST_PAYLOAD_SCHEDULE);
    configManager.setScheduleConfig(
        scheduleDoc["plant_year"].as<uint16_t>(),
        scheduleDoc["plant_month"].as<uint8_t>(),
        scheduleDoc["plant_day"].as<uint8_t>(),
        scheduleDoc["daily_mix_hour"].as<uint8_t>(),
        scheduleDoc["daily_mix_minute"].as<uint8_t>()
    );

    JsonDocument ppmDoc;
    deserializeJson(ppmDoc, FULL_TEST_PAYLOAD_PPM);
    configManager.setPPMConfig(
        ppmDoc["ppm_tolerance"].as<float>(),
        ppmDoc["initial_a"].as<float>(),
        ppmDoc["initial_b"].as<float>()
    );

    JsonDocument phDoc;
    deserializeJson(phDoc, FULL_TEST_PAYLOAD_PH);
    configManager.setPHConfig(
        phDoc["min_ph"].as<float>(),
        phDoc["max_ph"].as<float>()
    );

    JsonDocument recipeDoc;
    deserializeJson(recipeDoc, FULL_TEST_PAYLOAD_RECIPE);
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
    deserializeJson(irrigationDoc, FULL_TEST_PAYLOAD_IRRIGATION);
    IrrigationStageConfig irrigStages[MAX_IRRIGATION_STAGES];
    uint8_t irrigCount = 0;
    for (JsonObjectConst s : irrigationDoc["stages"].as<JsonArrayConst>()) {
        if (irrigCount >= MAX_IRRIGATION_STAGES) break;
        irrigStages[irrigCount].maxAgeDays   = s["max_age_days"].as<uint16_t>();
        irrigStages[irrigCount].dryThreshold = s["dry_threshold"].as<uint16_t>();
        irrigStages[irrigCount].wetThreshold = s["wet_threshold"].as<uint16_t>();
        irrigCount++;
    }
    configManager.setIrrigationStages(irrigStages, irrigCount);

    JsonDocument timerDoc;
    deserializeJson(timerDoc, FULL_TEST_PAYLOAD_TIMER_IRRIGATION);
    IrrigationSlot slots[MAX_IRRIG_SLOTS];
    uint8_t slotCount = 0;
    for (JsonObjectConst s : timerDoc["slots"].as<JsonArrayConst>()) {
        if (slotCount >= MAX_IRRIG_SLOTS) break;
        slots[slotCount].startHour   = s["start_hour"].as<uint8_t>();
        slots[slotCount].startMinute = s["start_minute"].as<uint8_t>();
        slots[slotCount].endHour     = s["end_hour"].as<uint8_t>();
        slots[slotCount].endMinute   = s["end_minute"].as<uint8_t>();
        slotCount++;
    }
    configManager.setTimerIrrigationConfig(slots, slotCount);

    JsonDocument mixDoc;
    deserializeJson(mixDoc, FULL_TEST_PAYLOAD_MIX_SCHEDULE_EXT);
    configManager.setStirSchedule(
        mixDoc["stir_evening_hour"].as<uint8_t>(),
        mixDoc["stir_evening_minute"].as<uint8_t>(),
        mixDoc["stir_duration_seconds"].as<uint32_t>() * 1000UL
    );
}
