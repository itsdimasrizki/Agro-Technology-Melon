#ifndef SECRET_IRRIGATION_RECIPE_H
#define SECRET_IRRIGATION_RECIPE_H

#include <stdint.h>

constexpr uint16_t DEFAULT_DRY_THRESHOLD = 1500;

struct IrrigationStage {
    uint16_t maxAgeDays;
    uint16_t dryThreshold;
};

constexpr IrrigationStage
IRRIGATION_STAGES[] =
{
    {7, 1000},
    {14, 1500},
    {28, 1700},
    {42, 1900},
    {56, 2100},
    {70, 2200}
};

constexpr size_t NUM_IRRIGATION_STAGES =
    sizeof(IRRIGATION_STAGES) /
    sizeof(IRRIGATION_STAGES[0]);

#endif