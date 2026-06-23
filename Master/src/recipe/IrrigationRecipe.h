#ifndef IRRIGATION_RECIPE_H
#define IRRIGATION_RECIPE_H

#include <stdint.h>

struct IrrigationConfig {
    uint16_t dryThreshold;
};

class IrrigationRecipe {
public:
    IrrigationConfig getRecipe(
        uint16_t ageDays
    );
};

#endif