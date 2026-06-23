#ifndef IRRIGATION_RECIPE_H
#define IRRIGATION_RECIPE_H

#include <stdint.h>

struct IrrigationConfig {
    uint16_t dryThreshold;
    uint16_t wetThreshold;
};

class IrrigationRecipe {
public:
    IrrigationConfig getRecipe(
        uint16_t ageDays
    );
};

#endif