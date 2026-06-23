#include "IrrigationRecipe.h"
#include "SecretIrrigationRecipe.h"

IrrigationConfig
IrrigationRecipe::getRecipe(uint16_t ageDays) {
    IrrigationConfig config = {
        DEFAULT_DRY_THRESHOLD
    };

    for(size_t i = 0;i < NUM_IRRIGATION_STAGES;i++) {
        if(ageDays <= IRRIGATION_STAGES[i].maxAgeDays) {
            config.dryThreshold = IRRIGATION_STAGES[i].dryThreshold;
            break;
        }
    }

    return config;
}