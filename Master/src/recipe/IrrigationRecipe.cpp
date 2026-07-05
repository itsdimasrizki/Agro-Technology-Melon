#include "IrrigationRecipe.h"

IrrigationRecipe::IrrigationRecipe(ConfigManager& config)
    : _config(config)
{}

IrrigationConfig IrrigationRecipe::getRecipe(uint16_t ageDays) {
    // Default fallback (stage pertama jika tidak ada yang cocok)
    IrrigationConfig config = {3900, 3650};

    uint8_t numStages = _config.getNumIrrigationStages();
    for (uint8_t i = 0; i < numStages; i++) {
        IrrigationStageConfig stage = _config.getIrrigationStage(i);
        if (ageDays <= stage.maxAgeDays) {
            config.dryThreshold = stage.dryThreshold;
            config.wetThreshold = stage.wetThreshold;
            break;
        }
    }

    return config;
}