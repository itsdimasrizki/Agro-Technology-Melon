#include "config/PinConfig.h"
#include "sensor/SoilSensor.h"
#include "communication/ESPNowSender.h"

constexpr unsigned long SEND_INTERVAL_MS = 1000UL;
constexpr unsigned long STATUS_LOG_INTERVAL_MS = 5000UL;

SoilSensor    soilSensor(
    SOIL_SENSOR_1_PIN,
    SOIL_SENSOR_2_PIN,
    SOIL_SENSOR_3_PIN,
    SOIL_SENSOR_4_PIN
);

ESPNowSender  espNow;

unsigned long lastSendTime = 0;
unsigned long lastStatusLog = 0;
bool lastSendOk = false;

void logLine(const char* level, const char* component, const char* message) {
    Serial.printf(
        "t=%010lu | %-5s | %-8s | %s\n",
        millis(),
        level,
        component,
        message
    );
}

void logSoilStatus(const SoilData& data, bool queueOk, bool deliveryOk) {
    Serial.printf(
        "t=%010lu | INFO  | SOIL     | s1=%u s2=%u s3=%u s4=%u avg=%u espnow_queue=%s espnow_delivery=%s\n",
        millis(),
        data.sensor1,
        data.sensor2,
        data.sensor3,
        data.sensor4,
        data.averageRawADC,
        queueOk ? "OK" : "FAIL",
        deliveryOk ? "OK" : "FAIL"
    );
}

void setup() {
    Serial.begin(115200);

    soilSensor.begin();
    logLine("INFO", "BOOT", "soil_sensor=ready");

    if (!espNow.begin()) {
        logLine("ERROR", "ESPNOW", "init=failed");
        while (true) { delay(1000); }
    }

    logLine("INFO", "ESPNOW", "init=ready");
    char targetMac[18];
    espNow.getTargetMac(targetMac, sizeof(targetMac));
    Serial.printf(
        "t=%010lu | INFO  | ESPNOW   | sleeve_mac=%s target_master=%s\n",
        millis(),
        WiFi.macAddress().c_str(),
        targetMac
    );
    logLine("INFO", "BOOT", "system=ready");
}

void loop() {
    soilSensor.update();

    unsigned long now = millis();

    if (now - lastSendTime >= SEND_INTERVAL_MS) {
        lastSendTime = now;

        SoilData data = soilSensor.getData();

        lastSendOk = espNow.send(data);
    }

    if (now - lastStatusLog >= STATUS_LOG_INTERVAL_MS) {
        lastStatusLog = now;
        logSoilStatus(soilSensor.getData(), lastSendOk, espNow.wasLastSendDelivered());
    }
}
