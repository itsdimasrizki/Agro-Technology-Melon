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

#include "display/OLEDManager.h"

//////////////////////////////////////////////////
// OBJECTS
//////////////////////////////////////////////////

OLEDManager oled;
RelayManager relay;
RTCManager rtcManager;
ESPNowManager espNow;

FlowMeter flowWater(FLOW_WATER_PIN);
FlowMeter flowA(FLOW_A_PIN);
FlowMeter flowB(FLOW_B_PIN);

WaterLevel waterLevel(
    ULTRASONIC_TRIG,
    ULTRASONIC_ECHO
);

WaterTempSensor waterTemp(
    DS18B20_PIN
);

PHSensor phSensor(PH_PIN);

TDSSensor tdsSensor(TDS_PIN);

//////////////////////////////////////////////////
// SENSOR MANAGER
//////////////////////////////////////////////////

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

//////////////////////////////////////////////////
// RECIPE
//////////////////////////////////////////////////

RecipeManager recipeManager;
IrrigationRecipe irrigationRecipe;

//////////////////////////////////////////////////
// FSM
//////////////////////////////////////////////////

FertigationFSM fsm(
    sensorManager,
    relay,
    rtcManager,
    recipeManager,
    irrigationRecipe,
    flowWater,
    flowA,
    flowB
);

//////////////////////////////////////////////////
// ISR FLOW METER
//////////////////////////////////////////////////

void IRAM_ATTR flowWaterISR()
{
    flowWater.pulseCount++;
}

void IRAM_ATTR flowAISR()
{
    flowA.pulseCount++;
}

void IRAM_ATTR flowBISR()
{
    flowB.pulseCount++;
}

//////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("===== SMART AGRO MASTER =====");

    //////////////////////////////////////////////////
    // RELAY
    //////////////////////////////////////////////////

    relay.begin();

    //////////////////////////////////////////////////
    // FLOW METER
    //////////////////////////////////////////////////

    flowWater.begin(flowWaterISR);
    flowA.begin(flowAISR);
    flowB.begin(flowBISR);

    //////////////////////////////////////////////////
    // I2C
    //////////////////////////////////////////////////

    Wire.begin(
        I2C_SDA_PIN,
        I2C_SCL_PIN
    );

    //////////////////////////////////////////////////
    // SENSORS
    //////////////////////////////////////////////////

    waterLevel.begin();
    waterTemp.begin();

    phSensor.begin();
    tdsSensor.begin();

    //////////////////////////////////////////////////
    // RTC
    //////////////////////////////////////////////////

    rtcManager.begin();

    //////////////////////////////////////////////////
    // ESP NOW
    //////////////////////////////////////////////////

    espNow.begin();

    //////////////////////////////////////////////////
    // OLED
    //////////////////////////////////////////////////

    oled.begin();
    oled.showBoot();

    delay(2000);

    //////////////////////////////////////////////////
    // FSM
    //////////////////////////////////////////////////

    fsm.begin();

    Serial.println("System Ready");
}

//////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////

void loop()
{
    //////////////////////////////////////////////////
    // UPDATE SYSTEM
    //////////////////////////////////////////////////

    fsm.update();

    SensorData data =
        sensorManager.getData();

    //////////////////////////////////////////////////
    // OLED
    //////////////////////////////////////////////////

    static bool page = false;
    static unsigned long lastPage = 0;

    if (millis() - lastPage >= 5000)
    {
        lastPage = millis();

        page = !page;
    }

    if (page)
    {
        oled.showSensor(
            data.temperature,
            data.ph,
            data.ppm,
            data.soilADC
        );
    }
    else
    {
        oled.showFSM("MASTER");
    }

    //////////////////////////////////////////////////
    // SERIAL MONITOR
    //////////////////////////////////////////////////

    static unsigned long lastPrint = 0;

    if (millis() - lastPrint >= 1000)
    {
        lastPrint = millis();

        Serial.println();

        Serial.println("========== SENSOR ==========");

        Serial.print("Temp        : ");
        Serial.println(data.temperature);

        Serial.print("PH          : ");
        Serial.println(data.ph);

        Serial.print("PPM         : ");
        Serial.println(data.ppm);

        Serial.print("Water Level : ");
        Serial.println(data.waterLevel);

        Serial.print("Soil ADC    : ");
        Serial.println(data.soilADC);

        Serial.print("Flow Water  : ");
        Serial.println(data.flowWater);

        Serial.print("Flow A      : ");
        Serial.println(data.flowA);

        Serial.print("Flow B      : ");
        Serial.println(data.flowB);

        Serial.print("Volume      : ");
        Serial.println(data.currentVolume);

        Serial.println("============================");
    }
}