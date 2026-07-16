#include <Wire.h>

#include <WiFi.h>
#include <Preferences.h>

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

static constexpr unsigned long STATUS_LOG_INTERVAL_MS = 5000UL;
static const char* FIRMWARE_BUILD_ID = __DATE__ " " __TIME__;

void clearPreferencesNamespace(const char* ns) {
    Preferences prefs;
    prefs.begin(ns, false);
    prefs.clear();
    prefs.end();
}

void resetAppNVSOnFirmwareChange() {
    Preferences prefs;
    prefs.begin("fw_meta", false);
    String lastBuild = prefs.getString("build", "");

    if (lastBuild == FIRMWARE_BUILD_ID) {
        prefs.end();
        return;
    }

    prefs.end();

    clearPreferencesNamespace("cfg_meta");
    clearPreferencesNamespace("cfg_ppm");
    clearPreferencesNamespace("cfg_ph");
    clearPreferencesNamespace("cfg_recipe");
    clearPreferencesNamespace("cfg_irrig");
    clearPreferencesNamespace("cfg_sys");
    clearPreferencesNamespace("cfg_sched");
    clearPreferencesNamespace("cfg_timer");
    clearPreferencesNamespace("cfg_mix");
    clearPreferencesNamespace("cfg_soil");
    clearPreferencesNamespace("fertigation");

    prefs.begin("fw_meta", false);
    prefs.putString("build", FIRMWARE_BUILD_ID);
    prefs.end();

    logLine("INFO", "NVS", "app_config=reset reason=new_firmware");
}

const char* stateToString(FertigationState state) {
    switch (state) {
        case FertigationState::IDLE:                    return "IDLE";
        case FertigationState::WAIT_DAILY_MIX:          return "WAIT_DAILY_MIX";
        case FertigationState::PREPARE_DAILY_MIX:       return "PREPARE_DAILY_MIX";
        case FertigationState::FILL_WATER:              return "FILL_WATER";
        case FertigationState::PRE_MIX_A:               return "PRE_MIX_A";
        case FertigationState::ADD_NUTRIENT_A:          return "ADD_NUTRIENT_A";
        case FertigationState::MIX_A:                   return "MIX_A";
        case FertigationState::PRE_MIX_B:               return "PRE_MIX_B";
        case FertigationState::ADD_NUTRIENT_B:          return "ADD_NUTRIENT_B";
        case FertigationState::MIX_B:                   return "MIX_B";
        case FertigationState::VALIDATE:                return "VALIDATE";
        case FertigationState::PRE_MIX_CORRECTION:      return "PRE_MIX_CORRECTION";
        case FertigationState::CORRECT_PPM:             return "CORRECT_PPM";
        case FertigationState::CORRECTION_MIX:          return "CORRECTION_MIX";
        case FertigationState::READY:                   return "READY";
        case FertigationState::PRE_IRRIGATION_MIX:      return "PRE_IRRIGATION_MIX";
        case FertigationState::PRE_IRRIGATION_VALIDATE: return "PRE_IRRIGATION_VALIDATE";
        case FertigationState::IRRIGATION:              return "IRRIGATION";
        case FertigationState::ERROR:                   return "ERROR";
        default:                                        return "UNKNOWN";
    }
}

const char* errorToString(ErrorCode error) {
    switch (error) {
        case ErrorCode::NONE:                return "NONE";
        case ErrorCode::WATER_TIMEOUT:       return "WATER_TIMEOUT";
        case ErrorCode::NUTRIENT_A_TIMEOUT:  return "NUTRIENT_A_TIMEOUT";
        case ErrorCode::NUTRIENT_B_TIMEOUT:  return "NUTRIENT_B_TIMEOUT";
        case ErrorCode::MIXER_DRY_RUN:       return "MIXER_DRY_RUN";
        case ErrorCode::RTC_FAILURE:         return "RTC_FAILURE";
        case ErrorCode::PH_SENSOR_FAILURE:   return "PH_SENSOR_FAILURE";
        case ErrorCode::TDS_SENSOR_FAILURE:  return "TDS_SENSOR_FAILURE";
        case ErrorCode::ULTRASONIC_FAILURE:  return "ULTRASONIC_FAILURE";
        case ErrorCode::OVER_PPM:            return "OVER_PPM";
        case ErrorCode::PH_OUT_OF_RANGE:     return "PH_OUT_OF_RANGE";
        case ErrorCode::CORRECTION_FAILED:   return "CORRECTION_FAILED";
        case ErrorCode::WATER_OVERFLOW:      return "WATER_OVERFLOW";
        case ErrorCode::WAITING_FOR_FILL:    return "WAITING_FOR_FILL";
        default:                             return "UNKNOWN_ERROR";
    }
}

const char* modeToString(IrrigationMode mode) {
    return mode == IrrigationMode::TIMER ? "TIMER" : "HUMIDITY";
}

const char* phStatus(float ph) {
    if (ph < 5.9f) return "PH_UP";
    if (ph > 6.5f) return "PH_DOWN";
    return "OK";
}

void logLine(const char* level, const char* component, const char* message) {
    Serial.printf(
        "t=%010lu | %-5s | %-8s | %s\n",
        millis(),
        level,
        component,
        message
    );
}

void logBootStep(const char* component, const char* status) {
    char message[96];
    snprintf(message, sizeof(message), "init=%s", status);
    logLine("INFO", component, message);
}

void appendRelay(char* buffer, size_t size, bool& first, uint8_t relayIndex) {
    size_t used = strlen(buffer);
    snprintf(
        buffer + used,
        size - used,
        "%s%u",
        first ? "" : ",",
        relayIndex
    );
    first = false;
}

void formatActiveRelays(char* buffer, size_t size) {
    snprintf(buffer, size, "[");
    bool first = true;
    if (relay.isOn(RELAY_MIXER_STIR)) appendRelay(buffer, size, first, 1);
    if (relay.isOn(RELAY_SOLENOID_A)) appendRelay(buffer, size, first, 2);
    if (relay.isOn(RELAY_SOLENOID_B)) appendRelay(buffer, size, first, 3);
    if (relay.isOn(RELAY_SOLENOID_IRRIG)) appendRelay(buffer, size, first, 4);
    if (relay.isOn(RELAY_WATER_INLET)) appendRelay(buffer, size, first, 5);
    if (relay.isOn(RELAY_PUMP_A)) appendRelay(buffer, size, first, 6);
    if (relay.isOn(RELAY_PUMP_B)) appendRelay(buffer, size, first, 7);
    if (relay.isOn(RELAY_PUMP_MIX)) appendRelay(buffer, size, first, 8);
    strncat(buffer, "]", size - strlen(buffer) - 1);
}

void formatSoilRules(char* buffer, size_t size, const SoilRuleFlags& rules) {
    snprintf(
        buffer,
        size,
        "%c%c%c%c",
        rules.heartbeatTimeout ? 'H' : '-',
        rules.outOfRange ? 'O' : '-',
        rules.flatline ? 'F' : '-',
        rules.noResponse ? 'N' : '-'
    );
}

void logSystemStatus(const SensorData& data) {
    char activeRelays[32];
    formatActiveRelays(activeRelays, sizeof(activeRelays));

    SoilRuleFlags rules = soilHealth.getActiveRules();
    char soilRules[8];
    formatSoilRules(soilRules, sizeof(soilRules), rules);

    unsigned long now = millis();

    Serial.printf(
        "t=%010lu | INFO  | STATUS   | state=%s error=%s\n",
        now,
        stateToString(fsm.getState()),
        errorToString(fsm.getError())
    );
    Serial.printf(
        "t=%010lu | INFO  | NETWORK  | wifi=%s mqtt=%s ip=%s\n",
        now,
        WiFi.status() == WL_CONNECTED ? "UP" : "DOWN",
        mqtt.isConnected() ? "UP" : "DOWN",
        WiFi.localIP().toString().c_str()
    );
    Serial.printf(
        "t=%010lu | INFO  | SENSOR   | temp=%.1fC ph=%.2f/%s ppm=%.0f tank=%.1fL soil=%u\n",
        now,
        data.temperature,
        data.ph,
        phStatus(data.ph),
        data.ppm,
        data.tankVolume,
        data.soilADC
    );
    Serial.printf(
        "t=%010lu | INFO  | FLOW     | A=%.3fL B=%.3fL relays=%s\n",
        now,
        data.flowA,
        data.flowB,
        activeRelays
    );
    Serial.printf(
        "t=%010lu | INFO  | SOIL     | mode=%s health=%u rules=%s\n",
        now,
        modeToString(soilHealth.getMode()),
        soilHealth.getHealthScore(),
        soilRules
    );
}

void logStateEvent(FertigationState state, ErrorCode error) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "state=%s error=%s",
        stateToString(state),
        errorToString(error)
    );
    logLine(error == ErrorCode::NONE ? "INFO" : "WARN", "FSM", message);
}

void logConnectionEvent(bool mqttConnected) {
    char message[64];
    snprintf(message, sizeof(message), "mqtt=%s", mqttConnected ? "UP" : "DOWN");
    logLine(mqttConnected ? "INFO" : "WARN", "NETWORK", message);
}

void IRAM_ATTR flowAISR() {
    flowA.recordPulseFromISR();
}

void IRAM_ATTR flowBISR() {
    flowB.recordPulseFromISR();
}

void IRAM_ATTR flowIrrigISR() {
    flowIrrig.recordPulseFromISR();
}

void setup() {
    Serial.begin(115200);
    // Tunggu USB CDC siap (wajib untuk ESP32-S3 USB CDC)
    unsigned long t = millis();
    while (!Serial && (millis() - t) < 3000) delay(10);
    logLine("INFO", "BOOT", "serial=ready");

    resetAppNVSOnFirmwareChange();

    configManager.begin();
    logBootStep("CONFIG", configManager.isConfigured() ? "configured" : "waiting_config");

    relay.begin();
    logBootStep("RELAY", "ready");

    flowA.begin(flowAISR);
    flowB.begin(flowBISR);
    flowIrrig.begin(flowIrrigISR);
    logBootStep("FLOW", "ready");

    Wire.begin(
        I2C_SDA_PIN,
        I2C_SCL_PIN
    );
    logBootStep("I2C", "ready");

    waterLevel.begin();
    waterLevel.setTankCapacity(configManager.getTankCapacityLiter());
    waterLevel.setTankHeight(configManager.getTankHeightCM());
    waterLevel.setTankDiameter(configManager.getTankDiameterCM());
    logBootStep("WATERLVL", "ready");

    waterTemp.begin();
    logBootStep("TEMP", "ready");
    
    rtcManager.begin();
    rtcManager.setPlantingDate(configManager.getPlantYear(), configManager.getPlantMonth(), configManager.getPlantDay());
    rtcManager.setDailyMixSchedule(configManager.getDailyMixHour(), configManager.getDailyMixMinute());
    logBootStep("RTC", rtcManager.isOk() ? "ready" : "error");

    recovery.begin();
    logBootStep("RECOVERY", "ready");

    // recovery.clear();

    phSensor.begin();
    tdsSensor.begin();
    logBootStep("PH_TDS", "ready");

    bool espNowOk = espNow.begin();
    logBootStep("ESPNOW", espNowOk ? "ready" : "error");

    soilHealth.begin();
    logBootStep("SOIL", modeToString(soilHealth.getMode()));

    fsm.begin();
    logStateEvent(fsm.getState(), fsm.getError());

    mqtt.begin();
    logConnectionEvent(mqtt.isConnected());

    logLine("INFO", "BOOT", "system=ready");
    delay(5000);
}

void loop() {
    fsm.update();

    SensorData sensorData = sensorManager.getData();
    mqtt.update(sensorData, fsm.getState(), fsm.getError());

    static FertigationState lastLoggedState = fsm.getState();
    static ErrorCode lastLoggedError = fsm.getError();
    static bool lastLoggedMqtt = mqtt.isConnected();
    static unsigned long lastStatusLog = 0;

    FertigationState currentState = fsm.getState();
    ErrorCode currentError = fsm.getError();
    bool mqttConnected = mqtt.isConnected();

    if (currentState != lastLoggedState || currentError != lastLoggedError) {
        lastLoggedState = currentState;
        lastLoggedError = currentError;
        logStateEvent(currentState, currentError);
    }

    if (mqttConnected != lastLoggedMqtt) {
        lastLoggedMqtt = mqttConnected;
        logConnectionEvent(mqttConnected);
    }

    if (millis() - lastStatusLog >= STATUS_LOG_INTERVAL_MS) {
        lastStatusLog = millis();
        logSystemStatus(sensorData);
    }
}
