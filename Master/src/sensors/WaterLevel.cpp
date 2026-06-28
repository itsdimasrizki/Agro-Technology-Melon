#include "WaterLevel.h"
#include "../config/RuntimeConfig.h"

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

    // [HARDCODED LAMA] Uncomment untuk hardcoded tanpa MQTT (testing):
    // tankHeightCM      = 45.0f;
    // tankCapacityLiter = 15.0f;

    // [RUNTIME] Init dari RuntimeConfig
    // Nilai default gConfig = 45cm / 15L (sama dengan hardcoded lama)
    tankHeightCM      = gConfig.tankHeightCM;
    tankCapacityLiter = gConfig.tankCapacityLiter;
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

    return duration * 0.0343f / 2.0f;
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

// [RUNTIME] Dipanggil oleh MQTTManager saat config tank diterima
void WaterLevel::setTankConfig(float heightCM, float capacityLiter) {
    tankHeightCM      = heightCM;
    tankCapacityLiter = capacityLiter;

    Serial.print("[WaterLevel] Tank config diupdate: tinggi=");
    Serial.print(tankHeightCM);
    Serial.print("cm, kapasitas=");
    Serial.print(tankCapacityLiter);
    Serial.println("L");
}