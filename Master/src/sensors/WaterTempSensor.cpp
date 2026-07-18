#include "WaterTempSensor.h"

namespace {
constexpr float FALLBACK_TEMP_C = 27.6f;
constexpr float MIN_VALID_TEMP_C = -10.0f;
constexpr float MAX_VALID_TEMP_C = 80.0f;
}

WaterTempSensor::WaterTempSensor(uint8_t pin) {
    _pin = pin;
}

void WaterTempSensor::begin() {
    oneWire = new OneWire(_pin);

    sensors = new DallasTemperature(oneWire);

    sensors->begin();
}

float WaterTempSensor::getTemperature() {
    if (sensors == nullptr) {
        return FALLBACK_TEMP_C;
    }

    sensors->requestTemperatures();

    float temp = sensors->getTempCByIndex(0);

    // DS18B20 commonly returns 85C before a valid conversion.
    if (temp == DEVICE_DISCONNECTED_C || temp == 85.0f || temp < MIN_VALID_TEMP_C || temp > MAX_VALID_TEMP_C) {
        return FALLBACK_TEMP_C;
    }

    return temp;
}
