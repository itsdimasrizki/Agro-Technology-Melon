#include "WaterTempSensor.h"

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
        return 25.0f;
    }

    sensors->requestTemperatures();

    float temp = sensors->getTempCByIndex(0);
    if (temp == DEVICE_DISCONNECTED_C || temp == 85.0f || temp < -10.0f || temp > 80.0f) {
        return 25.0f;
    }

    return temp;
}
