#include <Wire.h>

#include "config/PinConfig.h"

#include "actuators/RelayManager.h"

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

#include "communication/ESPNowManager.h"

#include "utils/RecoveryManager.h"

RelayManager relay;

// FLOW METERS
FlowMeter flowWater(FLOW_WATER_PIN);
FlowMeter flowA(FLOW_A_PIN);
FlowMeter flowB(FLOW_B_PIN);

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
    flowWater,
    flowA,
    flowB
);

// Recipe
RecipeManager recipeManager;
IrrigationRecipe irrigationRecipe;

// FSM
FertigationFSM fsm(
    sensorManager,
    relay,
    rtcManager,
    recipeManager,
    irrigationRecipe,
    flowWater,
    flowA,
    flowB,
    recovery
);

// ISR
void IRAM_ATTR flowWaterISR() {
    flowWater.pulseCount++;
}

void IRAM_ATTR flowAISR() {
    flowA.pulseCount++;
}

void IRAM_ATTR flowBISR() {
    flowB.pulseCount++;
}

void setup() {
    Serial.begin(115200);

    relay.begin();

    flowWater.begin(flowWaterISR);
    flowA.begin(flowAISR);
    flowB.begin(flowBISR);

    Wire.begin(
        I2C_SDA_PIN,
        I2C_SCL_PIN
    );

    waterLevel.begin();
    waterTemp.begin();
    rtcManager.begin();

    recovery.begin();

    phSensor.begin();
    tdsSensor.begin();

    if(!espNow.begin()) {
        Serial.println("ESP-NOW Error");
    }

    fsm.begin();

    Serial.println("System Ready");
    delay(5000);
}

void loop() {
    fsm.update();
    static unsigned long lastPrint = 0;

    if(millis() - lastPrint >= 1000){
        lastPrint = millis();

        SensorData data = sensorManager.getData();

        Serial.print("Temp : ");
        Serial.println(data.temperature);

        Serial.print("PH : ");
        Serial.println(data.ph);

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

        Serial.println("----------------");
    }
}