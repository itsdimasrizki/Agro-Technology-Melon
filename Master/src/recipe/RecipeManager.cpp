#include "RecipeManager.h"

RecipeManager::RecipeManager(ConfigManager& config)
    : _config(config)
{}

NutrientRecipe RecipeManager::getRecipe(uint16_t ageDays) {
    NutrientRecipe recipe = {
        0.0f,
        _config.getDefaultMinPH(),
        _config.getDefaultMaxPH()
    };

    uint8_t numStages = _config.getNumRecipeStages();
    for (uint8_t i = 0; i < numStages; i++) {
        RecipeStageConfig stage = _config.getRecipeStage(i);
        if (ageDays <= stage.maxAgeDays) {
            recipe.targetPPM    = stage.targetPPM;
            recipe.targetMinPH  = stage.minPH;
            recipe.targetMaxPH  = stage.maxPH;
            break;
        }
    }

    return recipe;
}