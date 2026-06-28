#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include <Arduino.h>

class WaterLevel {
public:

    WaterLevel(
        uint8_t trigPin,
        uint8_t echoPin
    );

    void begin();

    float getDistanceCM();

    float getLevelPercent();
    
    float getVolumeLiter();

    // [RUNTIME] Set ukuran toren via MQTT (greenhouse/config/set)
    // Field JSON: "tank_height_cm" dan/atau "tank_capacity_liter"
    void setTankConfig(float heightCM, float capacityLiter);

private:

    uint8_t _trigPin;
    uint8_t _echoPin;

    // [HARDCODED LAMA] Uncomment untuk hardcoded tanpa MQTT (testing):
    // float tankHeightCM      = 45.0f;  // tinggi toren (cm)
    // float tankCapacityLiter = 15.0f;  // kapasitas toren (liter)

    // [RUNTIME] Diisi di begin() dari RuntimeConfig, atau diupdate via setTankConfig()
    float tankHeightCM;
    float tankCapacityLiter;
};

#endif