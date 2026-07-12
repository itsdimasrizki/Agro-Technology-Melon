#ifndef ERROR_CODE_H
#define ERROR_CODE_H

enum class ErrorCode {
    NONE,

    WATER_TIMEOUT,
    NUTRIENT_A_TIMEOUT,
    NUTRIENT_B_TIMEOUT,

    MIXER_DRY_RUN,

    RTC_FAILURE,
    PH_SENSOR_FAILURE,
    TDS_SENSOR_FAILURE,
    ULTRASONIC_FAILURE,

    OVER_PPM,

    PH_OUT_OF_RANGE,

    CORRECTION_FAILED,

    WATER_OVERFLOW,          // overflow warning (non-fatal: FSM tetap jalan, auto-clear saat volume turun)

    WAITING_FOR_FILL         // non-fatal warning: tangki di bawah target isi / safety floor — irigasi diblokir
};

#endif