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
}

void SensorManager::update() {
    data.temperature = tempSensor.getTemperature();

    data.ph = phSensor.readPH();

    data.ppm = tdsSensor.readPPM(data.temperature);

    data.waterLevel = levelSensor.getLevelPercent();

    data.flowWater = flowWater.getVolumeLiter();

    data.flowA = flowA.getVolumeLiter();

    data.flowB = flowB.getVolumeLiter();

    // data.currentVolume = flowWater.getVolumeLiter();
    // data.currentVolume = data.flowWater;
    data.currentVolume = 0.0f;
    // data.currentVolume = levelSensor.getVolumeLiter();

    if(espNow.hasNewData()) {
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