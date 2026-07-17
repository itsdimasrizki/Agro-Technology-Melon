#pragma once

#include "data/MQTTConfigurationTestData.h"

inline void loadFullSystemIntegrationTestConfiguration(ConfigManager& configManager) {
    loadMQTTConfigurationTestData(configManager);
}
