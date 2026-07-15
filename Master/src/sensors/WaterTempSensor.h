#ifndef WATER_TEMP_SENSOR_H
#define WATER_TEMP_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class WaterTempSensor {
public:

    WaterTempSensor(uint8_t pin);

    void begin();

    float getTemperature();

private:

    OneWire* oneWire = nullptr;
    DallasTemperature* sensors = nullptr;

    uint8_t _pin;
};

#endif
