#pragma once

#include "fsm/FertigationState.h"
#include "sensors/SensorManager.h"
#include "data/MQTTConfigurationTestData.h"

static const SensorData FSM_SIMULATION_DEFAULT_SENSOR = {
    26.0f,
    6.0f,
    900.0f,
    10.0f,
    3900,
    0.0f,
    0.3f,
    0.3f,
    10.0f
};

inline void loadFSMStateSimulationConfiguration(ConfigManager& configManager) {
    loadMQTTConfigurationTestData(configManager);
}

inline SensorData getFSMStateSimulationData(FertigationState state) {
    SensorData data = FSM_SIMULATION_DEFAULT_SENSOR;

    switch (state) {
        case FertigationState::FILL_WATER:
            data.tankVolume = 10.0f;
            data.waterLevel = 10.0f;
            break;

        case FertigationState::ADD_NUTRIENT_A:
        case FertigationState::CORRECT_PPM:
            data.flowA = 0.3f;
            data.flowB = 0.3f;
            break;

        case FertigationState::ADD_NUTRIENT_B:
            data.flowB = 0.3f;
            break;

        case FertigationState::READY:
            data.soilADC = 3900;
            break;

        case FertigationState::IRRIGATION:
            data.soilADC = 3500;
            break;

        default:
            break;
    }

    return data;
}
