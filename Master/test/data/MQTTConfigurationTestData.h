#ifndef MQTT_CONFIGURATION_TEST_DATA_H
#define MQTT_CONFIGURATION_TEST_DATA_H

#include <Arduino.h>

namespace MQTTConfigurationTestData {

static const char SYSTEM_PAYLOAD[] PROGMEM = R"json(
{
  "total_plants": 80,
  "max_consumption_per_plant": 15.0,
  "target_fill_volume": 10.0,
  "tank_capacity_liter": 15.0,
  "tank_height_cm": 45.0,
  "tank_diameter_cm": 20.6
}
)json";

static const char SCHEDULE_PAYLOAD[] PROGMEM = R"json(
{
  "plant_year": 2026,
  "plant_month": 7,
  "plant_day": 1,
  "daily_mix_hour": 5,
  "daily_mix_minute": 0
}
)json";

static const char PPM_PAYLOAD[] PROGMEM = R"json(
{
  "ppm_tolerance": 50,
  "initial_a": 0.8,
  "initial_b": 0.8
}
)json";

static const char PH_PAYLOAD[] PROGMEM = R"json(
{
  "min_ph": 5.5,
  "max_ph": 6.5
}
)json";

static const char RECIPE_PAYLOAD[] PROGMEM = R"json(
{
  "stages": [
    {
      "max_age_days": 10,
      "target_ppm": 700,
      "min_ph": 5.5,
      "max_ph": 6.5
    },
    {
      "max_age_days": 30,
      "target_ppm": 900,
      "min_ph": 5.6,
      "max_ph": 6.4
    },
    {
      "max_age_days": 60,
      "target_ppm": 1100,
      "min_ph": 5.6,
      "max_ph": 6.4
    }
  ]
}
)json";

static const char IRRIGATION_PAYLOAD[] PROGMEM = R"json(
{
  "stages": [
    {
      "max_age_days": 10,
      "dry_threshold": 3900,
      "wet_threshold": 3650
    },
    {
      "max_age_days": 30,
      "dry_threshold": 3800,
      "wet_threshold": 3500
    },
    {
      "max_age_days": 60,
      "dry_threshold": 3700,
      "wet_threshold": 3400
    }
  ]
}
)json";

static const char TIMER_IRRIGATION_PAYLOAD[] PROGMEM = R"json(
{
  "slots": [
    {
      "hour": 7,
      "minute": 0
    },
    {
      "hour": 16,
      "minute": 0
    }
  ]
}
)json";

static const char MIX_SCHEDULE_EXT_PAYLOAD[] PROGMEM = R"json(
{
  "stir_evening_hour": 18,
  "stir_evening_minute": 0,
  "stir_duration_seconds": 300
}
)json";

}

#endif
