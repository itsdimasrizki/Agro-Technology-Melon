#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "PHSensor.h"
#include "TDSSensor.h"
#include "WaterTempSensor.h"
#include "WaterLevel.h"
#include "../communication/ESPNowManager.h"
#include "FlowMeter.h"

struct SensorData {
    float temperature;
    float ph;
    float ppm;

    float tankVolume;

    uint16_t soilADC;

    float flowWater;
    float flowA;
    float flowB;

    float waterLevel;
};

class SensorManager {
public:
    SensorManager(
        WaterTempSensor& temp,
        PHSensor& ph,
        TDSSensor& tds,
        WaterLevel& level,
        ESPNowManager& espNow,
        FlowMeter& water,
        FlowMeter& a,
        FlowMeter& b
    );

    void update();

    SensorData getData() const;

private:
    SensorData data;

    WaterTempSensor& tempSensor;
    PHSensor& phSensor;
    TDSSensor& tdsSensor;
    WaterLevel& levelSensor;

    ESPNowManager& espNow;

    FlowMeter& flowWater;
    FlowMeter& flowA;
    FlowMeter& flowB;

    // Throttle timestamp per sensor (millis)
    unsigned long _lastTempUpdate;
    unsigned long _lastPhTdsUpdate;
    unsigned long _lastLevelUpdate;
};

#endif