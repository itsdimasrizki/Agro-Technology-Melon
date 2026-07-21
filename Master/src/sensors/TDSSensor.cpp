#include "TDSSensor.h"

namespace {
constexpr float FALLBACK_TEMP_C = 27.6f;
constexpr float MIN_VALID_TEMP_C = -10.0f;
constexpr float MAX_VALID_TEMP_C = 80.0f;

// Kalibrasi TDS DFRobot secara software.
// Rumus: (Nilai Asli dari TDS Meter Manual) / (Nilai Bacaan ESP32)
// Contoh: 900.0 / 2145.0 = 0.4195
constexpr float TDS_K_VALUE = 0.4195f;
}

TDSSensor::TDSSensor(uint8_t pin) {
    _pin = pin;
}

void TDSSensor::begin() {}

float TDSSensor::readPPM(float waterTemp) {
    if (waterTemp < MIN_VALID_TEMP_C || waterTemp > MAX_VALID_TEMP_C) {
        waterTemp = FALLBACK_TEMP_C;
    }

    const int samples = 30;

    uint32_t adcSum = 0;

    for (int i = 0; i < samples; i++) {
        adcSum += analogRead(_pin);
    }

    float adc = adcSum / (float)samples;

    float voltage = adc * 3.3f / 4095.0f;

    float compensationCoefficient = 1.0f + 0.02f * (waterTemp - 25.0f);
    if (compensationCoefficient <= 0.0f) {
        compensationCoefficient = 1.0f;
    }

    float compensationVoltage = voltage / compensationCoefficient;

    float tdsValue = 
        (133.42f * compensationVoltage * compensationVoltage * compensationVoltage)
        -
        (255.86f * compensationVoltage * compensationVoltage)
        +
        (857.39f * compensationVoltage);

    return tdsValue * 0.5f * TDS_K_VALUE;
}
