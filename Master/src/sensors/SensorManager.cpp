#include "SensorManager.h"

SensorManager::SensorManager(
    WaterTempSensor& temp,
    PHSensor& ph,
    TDSSensor& tds,
    WaterLevel& level,
    ESPNowManager& esp,
    FlowMeter& a,
    FlowMeter& b
)
:
tempSensor(temp),
phSensor(ph),
tdsSensor(tds),
levelSensor(level),
espNow(esp),
flowA(a),
flowB(b),
data{}
{
    _readState      = STATE_READ_PH;
    _lastStateChange = 0;
    _lastLevelUpdate  = 0;
}

void SensorManager::update() {
#if ENABLE_FSM_SIMULATION_TEST
    return;
#endif

    unsigned long now = millis();

    switch (_readState) {
        case STATE_READ_PH:
            if (now - _lastStateChange >= PH_SAMPLE_INTERVAL) {
                data.ph          = phSensor.readPH();
                _lastStateChange = now;

                _readState = STATE_WAIT_PH;
            }
            break;

        case STATE_WAIT_PH:
            if (now - _lastStateChange >= SENSOR_GAP_MS) {
                _lastStateChange = now;
                _readState = STATE_READ_TEMP;
            }
            break;

        case STATE_READ_TEMP:
            if (now - _lastStateChange >= TEMP_CONVERT_MS) {
                data.temperature = tempSensor.getTemperature();
                _lastStateChange = now;
                _readState = STATE_WAIT_TEMP;
            }
            break;

        case STATE_WAIT_TEMP:
            if (now - _lastStateChange >= SENSOR_GAP_MS) {
                _lastStateChange = now;
                _readState = STATE_READ_TDS;
            }
            break;

        case STATE_READ_TDS:
            if (now - _lastStateChange >= PH_SAMPLE_INTERVAL) {
                data.ppm         = tdsSensor.readPPM(data.temperature);
                _lastStateChange = now;
                _readState = STATE_WAIT_TDS;
            }
            break;

        case STATE_WAIT_TDS:
            if (now - _lastStateChange >= SENSOR_GAP_MS) {
                _lastStateChange = now;
                _readState = STATE_READ_PH;
            }
            break;
    }

    if (now - _lastLevelUpdate >= 100UL) {
        float tankVolume = levelSensor.getVolumeLiter();

        // Keep the last valid volume when ultrasonic read times out.
        if (tankVolume >= 0.0f) {
            data.tankVolume = tankVolume;
            data.waterLevel = tankVolume;
        }
        _lastLevelUpdate = now;
    }

    data.flowWater = 0.0f; // flowWater pin removed on current PCB
    data.flowA     = flowA.getVolumeLiter();
    data.flowB     = flowB.getVolumeLiter();

    if (espNow.hasNewData()) {
        SoilData soil = espNow.getData();
        data.soilADC  = soil.averageRawADC;
        espNow.clearFlag();
    }
}

SensorData
SensorManager::getData()
const {
    return data;
}

#if ENABLE_FSM_SIMULATION_TEST
void SensorManager::setTestData(const SensorData& testData) {
    data = testData;
}
#endif
