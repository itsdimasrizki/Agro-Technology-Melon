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

private:

    uint8_t _trigPin;
    uint8_t _echoPin;

    float tankHeightCM      = 45.0f;
    float tankDiameterCM    = 20.6f;
    float tankCapacityLiter = 15.0f;

public:
    // Dipanggil ConfigManager setelah load config dari NVS/MQTT
    void setTankCapacity(float capacityLiter) {
        if (capacityLiter > 0.0f) tankCapacityLiter = capacityLiter;
    }
    void setTankHeight(float heightCM) {
        if (heightCM > 0.0f) tankHeightCM = heightCM;
    }
    void setTankDiameter(float diameterCM) {
        if (diameterCM > 0.0f) tankDiameterCM = diameterCM;
    }
};

#endif
