#ifndef IRRIGATION_RECIPE_H
#define IRRIGATION_RECIPE_H

#include <stdint.h>
#include "../config/ConfigManager.h"

struct IrrigationConfig {
    uint16_t dryThreshold;
    uint16_t wetThreshold;
};

class IrrigationRecipe {
public:
    IrrigationRecipe(ConfigManager& config);

    IrrigationConfig getRecipe(uint16_t ageDays);

private:
    ConfigManager& _config;
};

#endif