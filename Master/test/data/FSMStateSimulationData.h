#ifndef FSM_STATE_SIMULATION_DATA_H
#define FSM_STATE_SIMULATION_DATA_H

#include "../../src/config/ConfigManager.h"
#include "../../src/fsm/FertigationState.h"
#include "../../src/sensors/SensorManager.h"

namespace FSMStateSimulationData {

static SensorData makeBaseSensorData(const ConfigManager& config) {
    SensorData data{};
    data.temperature = 25.0f;
    data.ph = 6.0f;
    data.ppm = 700.0f;
    data.tankVolume = config.getTargetFillVolume();
    data.soilADC = 3200;
    data.flowWater = 0.0f;
    data.flowA = 0.0f;
    data.flowB = 0.0f;
    data.waterLevel = data.tankVolume;
    return data;
}

static SensorData dataForState(FertigationState state, const ConfigManager& config) {
    SensorData data = makeBaseSensorData(config);
    RecipeStageConfig recipe = config.getRecipeStage(0);
    IrrigationStageConfig irrigation = config.getIrrigationStage(0);

    switch (state) {
        case FertigationState::IDLE:
        case FertigationState::WAIT_DAILY_MIX:
        case FertigationState::PREPARE_DAILY_MIX:
            data.tankVolume = config.getTargetFillVolume();
            data.waterLevel = data.tankVolume;
            data.ppm = recipe.targetPPM;
            data.soilADC = irrigation.wetThreshold;
            break;

        case FertigationState::FILL_WATER:
            data.tankVolume = config.getTargetFillVolume();
            data.waterLevel = data.tankVolume;
            break;

        case FertigationState::PRE_MIX_A:
        case FertigationState::ADD_NUTRIENT_A:
            data.flowA = config.getInitialNutrientA();
            break;

        case FertigationState::MIX_A:
        case FertigationState::PRE_MIX_B:
        case FertigationState::ADD_NUTRIENT_B:
            data.flowA = config.getInitialNutrientA();
            data.flowB = config.getInitialNutrientB();
            break;

        case FertigationState::MIX_B:
        case FertigationState::VALIDATE:
            data.ppm = recipe.targetPPM;
            data.flowA = config.getInitialNutrientA();
            data.flowB = config.getInitialNutrientB();
            break;

        case FertigationState::PRE_MIX_CORRECTION:
        case FertigationState::CORRECT_PPM:
        case FertigationState::CORRECTION_MIX:
            data.ppm = recipe.targetPPM;
            data.flowA = 0.05f;
            data.flowB = 0.05f;
            break;

        case FertigationState::READY:
            data.ppm = recipe.targetPPM;
            data.soilADC = irrigation.dryThreshold;
            break;

        case FertigationState::PRE_IRRIGATION_MIX:
        case FertigationState::PRE_IRRIGATION_VALIDATE:
            data.ppm = recipe.targetPPM;
            data.soilADC = irrigation.dryThreshold;
            break;

        case FertigationState::IRRIGATION:
            data.ppm = recipe.targetPPM;
            data.soilADC = irrigation.wetThreshold;
            break;

        case FertigationState::ERROR:
            data.soilADC = irrigation.wetThreshold;
            break;
    }

    return data;
}

}

#endif
