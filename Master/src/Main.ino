#include <Wire.h>

#include <WiFi.h>

#include "config/PinConfig.h"
#include "config/ConfigManager.h"

#include "actuators/RelayManager.h"

#include "communication/MQTTManager.h"

#include "recipe/RecipeManager.h"
#include "recipe/IrrigationRecipe.h"

#include "rtc/RTCManager.h"

#include "sensors/SensorManager.h"

#include "sensors/PHSensor.h"
#include "sensors/TDSSensor.h"
#include "sensors/FlowMeter.h"
#include "sensors/WaterLevel.h"
#include "sensors/WaterTempSensor.h"

#include "fsm/FertigationFSM.h"
#include "fsm/SoilHealthMonitor.h"

#include "communication/ESPNowManager.h"

#include "utils/RecoveryManager.h"

RelayManager relay;

// FLOW METERS
// Gunakan PinConfig.h pins
FlowMeter flowA(FLOW_A_PIN);
FlowMeter flowB(FLOW_B_PIN);
FlowMeter flowIrrig(FLOW_IRRIG_PIN); // Flow sensor irigasi (Timer mode)

// Water Level
WaterLevel waterLevel(
    ULTRASONIC_TRIG,
    ULTRASONIC_ECHO
);

// Water Temperature
WaterTempSensor waterTemp(
    DS18B20_PIN
);

RTCManager rtcManager;

ESPNowManager espNow;

RecoveryManager recovery;

ConfigManager configManager;

SoilHealthMonitor soilHealth(espNow, configManager);

// PH TDS
PHSensor phSensor(PH_PIN);

TDSSensor tdsSensor(TDS_PIN);

// Sensor Manager
SensorManager sensorManager(
    waterTemp,
    phSensor,
    tdsSensor,
    waterLevel,
    espNow,
    flowA,
    flowB
);

// Recipe
RecipeManager recipeManager(configManager);
IrrigationRecipe irrigationRecipe(configManager);

// FSM
FertigationFSM fsm(
    sensorManager,
    relay,
    rtcManager,
    recipeManager,
    irrigationRecipe,
    flowA,
    flowB,
    flowIrrig,
    recovery,
    configManager,
    soilHealth
);

MQTTManager mqtt(relay, configManager, rtcManager, waterLevel, soilHealth, fsm);

// ISR

void IRAM_ATTR flowAISR() {
    flowA.pulseCount++;
}

void IRAM_ATTR flowBISR() {
    flowB.pulseCount++;
}

void IRAM_ATTR flowIrrigISR() {
    flowIrrig.pulseCount++;
}

void setup() {
    Serial.begin(115200);
    // Tunggu USB CDC siap (wajib untuk ESP32-S3 USB CDC)
    unsigned long t = millis();
    while (!Serial && (millis() - t) < 3000) delay(10);
    Serial.println("[BOOT] Serial ready");

    // 1. Inisialisasi ConfigManager
    Serial.println("[1] ConfigManager...");
    configManager.begin();

    Serial.println("[2] Relay...");
    relay.begin();

    Serial.println("[3] FlowMeters...");
    flowA.begin(flowAISR);
    flowB.begin(flowBISR);
    flowIrrig.begin(flowIrrigISR);

    Serial.println("[4] Wire I2C...");
    Wire.begin(
        I2C_SDA_PIN,
        I2C_SCL_PIN
    );

    Serial.println("[5] WaterLevel...");
    waterLevel.begin();
    waterLevel.setTankCapacity(configManager.getTankCapacityLiter());

    Serial.println("[6] WaterTemp...");
    waterTemp.begin();
    
    Serial.println("[7] RTC...");
    rtcManager.begin();
    rtcManager.setPlantingDate(configManager.getPlantYear(), configManager.getPlantMonth(), configManager.getPlantDay());
    rtcManager.setDailyMixSchedule(configManager.getDailyMixHour(), configManager.getDailyMixMinute());

    Serial.println("[8] Recovery...");
    recovery.begin();

    // recovery.clear();

    Serial.println("[9] PH & TDS Sensor...");
    phSensor.begin();
    tdsSensor.begin();

    Serial.println("[10] ESP-NOW...");
    if(!espNow.begin()) {
        Serial.println("  -> ESP-NOW Error!");
    } else {
        Serial.println("  -> ESP-NOW OK");
    }

    Serial.println("[10.5] SoilHealthMonitor...");
    soilHealth.begin();

    Serial.println("[11] FSM...");
    fsm.begin();

    Serial.println("[12] MQTT...");
    mqtt.begin();

    Serial.println("=== System Ready ===");
    delay(5000);
}

void loop() {
    fsm.update();

    SensorData sensorData = sensorManager.getData();
    mqtt.update(sensorData, fsm.getState(), fsm.getError());

    static unsigned long lastPrint = 0;

    if(millis() - lastPrint >= 1000){
        lastPrint = millis();

        SensorData data = sensorManager.getData();

        Serial.print("Alamat MAC ESP32-S3: ");
        Serial.println(WiFi.macAddress());

        Serial.print("Temp : ");
        Serial.println(data.temperature);

        Serial.print("PH : ");
        Serial.print(data.ph);
        if (data.ph < 5.9) {
            Serial.println(" (Indikator: Butuh PH UP)");
        } else if (data.ph > 6.5) {
            Serial.println(" (Indikator: Butuh PH DOWN)");
        } else {
            Serial.println(" (Indikator: NORMAL)");
        }

        Serial.print("PPM : ");
        Serial.println(data.ppm);

        Serial.print("Water : ");
        Serial.println(data.waterLevel);

        Serial.print("Soil : ");
        Serial.println(data.soilADC);

        Serial.print("Flow Water : ");
        Serial.println(data.flowWater);

        Serial.print("Flow A : ");
        Serial.println(data.flowA);

        Serial.print("Flow B : ");
        Serial.println(data.flowB);

        Serial.print("Tank Volume : ");
        Serial.println(data.tankVolume);

        Serial.print("MQTT : ");
        Serial.println(mqtt.isConnected() ? "Connected" : "Disconnected");

        Serial.println("----------------");
    }
}