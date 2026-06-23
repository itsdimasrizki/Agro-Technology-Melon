#ifndef FERTIGATION_STATE_H
#define FERTIGATION_STATE_H

enum class FertigationState {
    IDLE,

    WAIT_DAILY_MIX,

    PREPARE_DAILY_MIX,

    FILL_WATER,

    ADD_NUTRIENT_A,

    MIX_A,

    ADD_NUTRIENT_B,

    MIX_B,

    VALIDATE,

    CORRECT_PPM,

    READY,

    IRRIGATION,

    ERROR
};

#endif