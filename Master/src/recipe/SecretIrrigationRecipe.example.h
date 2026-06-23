#ifndef SECRET_IRRIGATION_RECIPE_H
#define SECRET_IRRIGATION_RECIPE_H

#include <stddef.h>
#include <cstddef>
#include <stdint.h>

constexpr uint16_t DEFAULT_DRY_THRESHOLD = 1500;

struct IrrigationStage {
    uint16_t maxAgeDays;
    uint16_t dryThreshold;
    uint16_t wetThreshold;
};

const IrrigationStage
IRRIGATION_STAGES[] = {
    {7, 0, 0},
    {13, 0, 0},
};

constexpr size_t NUM_IRRIGATION_STAGES =
    sizeof(IRRIGATION_STAGES) /
    sizeof(IRRIGATION_STAGES[0]);

#endif