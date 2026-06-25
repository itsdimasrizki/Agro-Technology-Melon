#include "RecipeManager.h"
#include "SecretRecipe.h"

NutrientRecipe RecipeManager::getRecipe(
    uint16_t ageDays
)
{
    NutrientRecipe recipe = {
        DEFAULT_PPM,
        DEFAULT_MIN_PH,
        DEFAULT_MAX_PH
    };

    for (int i = 0; i < NUM_STAGES; i++) {
        if (ageDays <= RECIPE_STAGES[i].maxAgeDays) {
            recipe.targetPPM = RECIPE_STAGES[i].targetPPM;
            break;
        }
    }

    return recipe;
}