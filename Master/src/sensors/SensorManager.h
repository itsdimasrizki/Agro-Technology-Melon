#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "sensors/PHSensor.h"
#include "sensors/TDSSensor.h"
#include "sensors/WaterTempSensor.h"
#include "sensors/WaterLevel.h"
#include "../communication/ESPNowManager.h"
#include "sensors/FlowMeter.h"

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

    FlowMeter& flowA;
    FlowMeter& flowB;

    // State machine untuk pembacaan sequential (mencegah GND nabrak)
    enum ReadState {
        STATE_READ_PH   = 0,
        STATE_WAIT_PH   = 1,
        STATE_READ_TEMP = 2,
        STATE_WAIT_TEMP = 3,
        STATE_READ_TDS  = 4,
        STATE_WAIT_TDS  = 5
    };

    ReadState     _readState;
    unsigned long _lastStateChange;   // kapan state terakhir berganti
    unsigned long _lastLevelUpdate;   // throttle water level (ultrasonic)

    // Interval delay antar sensor (ms)
    static const unsigned long PH_SAMPLE_INTERVAL   = 500UL;   // baca PH setiap 500ms (averaging)
    static const unsigned long SENSOR_GAP_MS         = 30000UL; // 30 detik jeda antar sensor
    static const unsigned long TEMP_CONVERT_MS       = 750UL;   // DS18B20 konversi
};

#endif