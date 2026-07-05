#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include <Arduino.h>
#include "../config/ConfigManager.h"

struct NutrientRecipe {
    float targetPPM;
    float targetMinPH;
    float targetMaxPH;
};

class RecipeManager {
public:
    RecipeManager(ConfigManager& config);

    NutrientRecipe getRecipe(uint16_t ageDays);

private:
    ConfigManager& _config;
};

#endif