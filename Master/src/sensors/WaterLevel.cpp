#include "WaterLevel.h"

WaterLevel::WaterLevel(
    uint8_t trigPin,
    uint8_t echoPin
) {
    _trigPin = trigPin;
    _echoPin = echoPin;
}

void WaterLevel::begin() {
    pinMode(_trigPin, OUTPUT);

    pinMode(_echoPin, INPUT);
}

float WaterLevel::getDistanceCM() {
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(_trigPin, LOW);

    long duration = pulseIn(_echoPin, HIGH, 30000);

    if(duration <= 0)
        return -1;

    float raw = duration * 0.0343f / 2.0f;

    // Proses melalui median filter N=7 untuk meredam noise saat permukaan bergolak.
    // Filter ini aktif terus-menerus dan aman digunakan di semua kondisi.
    return _distanceFilter.process(raw);
}

float WaterLevel::getLevelPercent() {
    float distance =
        getDistanceCM();

    if(distance < 0)
        return -1;

    float level = ((tankHeightCM - distance) / tankHeightCM) * 100.0f;

    level = constrain(level, 0, 100);

    return level;
}

float WaterLevel::getVolumeLiter() {
    float level = getLevelPercent();

    if(level < 0)
        return -1;

    return (level / 100.0f) * tankCapacityLiter;
}