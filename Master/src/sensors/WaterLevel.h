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

    float tankHeightCM = 50.0f;
    float tankCapacityLiter = 500.0f; 
};

#endif