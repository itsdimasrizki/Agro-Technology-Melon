#include "SensorManager.h"

SensorManager::SensorManager(
    WaterTempSensor& temp,
    PHSensor& ph,
    TDSSensor& tds,
    WaterLevel& level,
    ESPNowManager& esp,
    FlowMeter& water,
    FlowMeter& a,
    FlowMeter& b
)
:
tempSensor(temp),
phSensor(ph),
tdsSensor(tds),
levelSensor(level),
espNow(esp),
flowWater(water),
flowA(a),
flowB(b),
data{}
{
    _lastTempUpdate    = 0;
    _lastPhTdsUpdate   = 0;
    _lastLevelUpdate   = 0;
}

void SensorManager::update() {
    unsigned long now = millis();

    // DS18B20 butuh minimal 750ms konversi
    if (now - _lastTempUpdate >= 750UL) {
        data.temperature = tempSensor.getTemperature();
        _lastTempUpdate = now;
    }

    // pH dan TDS — baca setiap 500ms
    if (now - _lastPhTdsUpdate >= 500UL) {
        data.ph  = phSensor.readPH();
        data.ppm = tdsSensor.readPPM(data.temperature);
        _lastPhTdsUpdate = now;
    }

    // Ultrasonic — baca setiap 100ms
    if (now - _lastLevelUpdate >= 100UL) {
        data.waterLevel  = levelSensor.getLevelPercent();
        data.tankVolume  = levelSensor.getVolumeLiter();
        _lastLevelUpdate = now;
    }

    // Flow meter — realtime via ISR, baca langsung setiap update
    data.flowWater = flowWater.getVolumeLiter();
    data.flowA     = flowA.getVolumeLiter();
    data.flowB     = flowB.getVolumeLiter();

    // Soil data dari ESP-NOW
    if (espNow.hasNewData()) {
        SoilData soil = espNow.getData();

        data.soilADC = soil.averageRawADC;

        espNow.clearFlag();
    }
}

SensorData
SensorManager::getData()
const {
    return data;
}