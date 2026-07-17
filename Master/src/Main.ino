#include <Wire.h>
#include <ArduinoJson.h>

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

#include "../test/TestFlags.h"

#if ENABLE_FSM_SIMULATION_TEST
#include "../test/data/FSMStateSimulationData.h"
#endif

#if ENABLE_RELAY_HARDWARE_TEST
#include "../test/data/RelayHardwareTestData.h"
#endif

#if ENABLE_FULL_SYSTEM_TEST
#include "../test/data/FullSystemIntegrationTestData.h"
#endif

#if ENABLE_MQTT_CONFIGURATION_TEST || ENABLE_FULL_SYSTEM_TEST || ENABLE_FSM_SIMULATION_TEST
#include "../test/data/MQTTConfigurationTestData.h"
#endif

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

#if ENABLE_MQTT_CONFIGURATION_TEST || ENABLE_FULL_SYSTEM_TEST || ENABLE_FSM_SIMULATION_TEST
static bool loadJsonPayload(const char* payload, JsonDocument& doc) {
    char buffer[1024];
    strncpy_P(buffer, payload, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return deserializeJson(doc, buffer) == DeserializationError::Ok;
}

static void applySystemPayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    float targetFill = doc["target_fill_volume"] | configManager.getTargetFillVolume();
    float tankCap = doc["tank_capacity_liter"] | configManager.getTankCapacityLiter();
    if (tankCap > 0.0f && targetFill > tankCap) targetFill = tankCap;

    configManager.setSystemConfig(
        doc["total_plants"] | configManager.getTotalPlants(),
        doc["max_consumption_per_plant"] | configManager.getMaxConsumptionPerPlant(),
        targetFill,
        tankCap,
        doc["tank_height_cm"] | configManager.getTankHeightCM(),
        doc["tank_diameter_cm"] | configManager.getTankDiameterCM()
    );
}

static void applySchedulePayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    configManager.setScheduleConfig(
        doc["plant_year"] | configManager.getPlantYear(),
        doc["plant_month"] | configManager.getPlantMonth(),
        doc["plant_day"] | configManager.getPlantDay(),
        doc["daily_mix_hour"] | configManager.getDailyMixHour(),
        doc["daily_mix_minute"] | configManager.getDailyMixMinute()
    );
}

static void applyPPMPayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    configManager.setPPMConfig(
        doc["ppm_tolerance"] | configManager.getPPMTolerance(),
        doc["initial_a"] | configManager.getInitialNutrientA(),
        doc["initial_b"] | configManager.getInitialNutrientB()
    );
}

static void applyPHPayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    configManager.setPHConfig(
        doc["min_ph"] | configManager.getDefaultMinPH(),
        doc["max_ph"] | configManager.getDefaultMaxPH()
    );
}

static void applyRecipePayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    JsonArrayConst arr = doc["stages"].as<JsonArrayConst>();
    if (arr.isNull()) return;

    RecipeStageConfig stages[MAX_RECIPE_STAGES];
    uint8_t count = 0;
    for (JsonObjectConst s : arr) {
        if (count >= MAX_RECIPE_STAGES) break;
        stages[count].maxAgeDays = s["max_age_days"] | (uint16_t)0;
        stages[count].targetPPM = s["target_ppm"] | 0.0f;
        stages[count].minPH = s["min_ph"] | configManager.getDefaultMinPH();
        stages[count].maxPH = s["max_ph"] | configManager.getDefaultMaxPH();
        count++;
    }

    if (count > 0) {
        configManager.setRecipeStages(stages, count);
    }
}

static void applyIrrigationPayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    JsonArrayConst arr = doc["stages"].as<JsonArrayConst>();
    if (arr.isNull()) return;

    IrrigationStageConfig stages[MAX_IRRIGATION_STAGES];
    uint8_t count = 0;
    for (JsonObjectConst s : arr) {
        if (count >= MAX_IRRIGATION_STAGES) break;
        stages[count].maxAgeDays = s["max_age_days"] | (uint16_t)0;
        stages[count].dryThreshold = s["dry_threshold"] | (uint16_t)3900;
        stages[count].wetThreshold = s["wet_threshold"] | (uint16_t)3650;
        count++;
    }

    if (count > 0) {
        configManager.setIrrigationStages(stages, count);
    }
}

static void applyTimerIrrigationPayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    JsonArrayConst arr = doc["slots"].as<JsonArrayConst>();
    if (arr.isNull()) return;

    IrrigationSlot slots[MAX_IRRIG_SLOTS];
    uint8_t count = 0;
    for (JsonObjectConst s : arr) {
        if (count >= MAX_IRRIG_SLOTS) break;
        slots[count].hour = s["hour"] | (uint8_t)0;
        slots[count].minute = s["minute"] | (uint8_t)0;
        count++;
    }

    configManager.setTimerIrrigationConfig(slots, count);
}

static void applyMixScheduleExtPayload(const char* payload) {
    JsonDocument doc;
    if (!loadJsonPayload(payload, doc)) return;

    configManager.setStirSchedule(
        doc["stir_evening_hour"] | configManager.getStirEveningHour(),
        doc["stir_evening_minute"] | configManager.getStirEveningMinute(),
        (doc["stir_duration_seconds"] | (uint32_t)(configManager.getStirDurationMs() / 1000UL)) * 1000UL
    );
}

static void applyHardcodedMQTTConfiguration() {
#if ENABLE_FULL_SYSTEM_TEST
    applySystemPayload(FullSystemIntegrationTestData::SYSTEM_PAYLOAD);
    applySchedulePayload(FullSystemIntegrationTestData::SCHEDULE_PAYLOAD);
    applyPPMPayload(FullSystemIntegrationTestData::PPM_PAYLOAD);
    applyPHPayload(FullSystemIntegrationTestData::PH_PAYLOAD);
    applyRecipePayload(FullSystemIntegrationTestData::RECIPE_PAYLOAD);
    applyIrrigationPayload(FullSystemIntegrationTestData::IRRIGATION_PAYLOAD);
    applyTimerIrrigationPayload(FullSystemIntegrationTestData::TIMER_IRRIGATION_PAYLOAD);
    applyMixScheduleExtPayload(FullSystemIntegrationTestData::MIX_SCHEDULE_EXT_PAYLOAD);
#else
    applySystemPayload(MQTTConfigurationTestData::SYSTEM_PAYLOAD);
    applySchedulePayload(MQTTConfigurationTestData::SCHEDULE_PAYLOAD);
    applyPPMPayload(MQTTConfigurationTestData::PPM_PAYLOAD);
    applyPHPayload(MQTTConfigurationTestData::PH_PAYLOAD);
    applyRecipePayload(MQTTConfigurationTestData::RECIPE_PAYLOAD);
    applyIrrigationPayload(MQTTConfigurationTestData::IRRIGATION_PAYLOAD);
    applyTimerIrrigationPayload(MQTTConfigurationTestData::TIMER_IRRIGATION_PAYLOAD);
    applyMixScheduleExtPayload(MQTTConfigurationTestData::MIX_SCHEDULE_EXT_PAYLOAD);
#endif

    rtcManager.setPlantingDate(configManager.getPlantYear(), configManager.getPlantMonth(), configManager.getPlantDay());
    rtcManager.setDailyMixSchedule(configManager.getDailyMixHour(), configManager.getDailyMixMinute());
    waterLevel.setTankCapacity(configManager.getTankCapacityLiter());
    waterLevel.setTankHeight(configManager.getTankHeightCM());
    waterLevel.setTankDiameter(configManager.getTankDiameterCM());
}
#endif

#if TEST_MODE_ANY
static void setupTestClockIfNeeded() {
#if ENABLE_FSM_SIMULATION_TEST
    rtcManager.setTestClock(DateTime(2026, 7, 2, configManager.getDailyMixHour(), configManager.getDailyMixMinute(), 0));
#elif ENABLE_FULL_SYSTEM_TEST || ENABLE_MQTT_CONFIGURATION_TEST
    if (!rtcManager.isOk()) {
        rtcManager.setTestClock(DateTime(2026, 7, 2, configManager.getDailyMixHour(), configManager.getDailyMixMinute(), 0));
    }
#endif
}
#endif

#if ENABLE_RELAY_HARDWARE_TEST
static void runRelayHardwareTest() {
    static uint8_t relayIndex = 0;
    static bool relayOn = false;
    static unsigned long lastChangeMs = 0;

    unsigned long now = millis();
    unsigned long interval = relayOn
        ? RelayHardwareTestData::ON_DURATION_MS
        : RelayHardwareTestData::OFF_DURATION_MS;

    if (now - lastChangeMs < interval) return;
    lastChangeMs = now;

    RelayChannel channel = RelayHardwareTestData::RELAYS[relayIndex];
    if (relayOn) {
        relay.off(channel);
        Serial.printf("t=%010lu | INFO  | TEST     | %s=off\n", now, RelayHardwareTestData::RELAY_NAMES[relayIndex]);
        relayOn = false;
        relayIndex = (relayIndex + 1) % RelayHardwareTestData::RELAY_COUNT;
    } else {
        channel = RelayHardwareTestData::RELAYS[relayIndex];
        relay.on(channel);
        Serial.printf("t=%010lu | INFO  | TEST     | %s=on\n", now, RelayHardwareTestData::RELAY_NAMES[relayIndex]);
        relayOn = true;
    }
}
#endif

#if ENABLE_FSM_SIMULATION_TEST
static void updateFSMTestData() {
    SensorData data = FSMStateSimulationData::dataForState(fsm.getState(), configManager);
    sensorManager.setTestData(data);
}
#endif

void setup() {
    Serial.begin(115200);
    // Tunggu USB CDC siap (wajib untuk ESP32-S3 USB CDC)
    unsigned long t = millis();
    while (!Serial && (millis() - t) < 3000) delay(10);
    logLine("INFO", "BOOT", "serial=ready");

    resetAppNVSOnFirmwareChange();

    configManager.begin();
#if ENABLE_MQTT_CONFIGURATION_TEST || ENABLE_FULL_SYSTEM_TEST || ENABLE_FSM_SIMULATION_TEST
    applyHardcodedMQTTConfiguration();
#endif
    logBootStep("CONFIG", configManager.isConfigured() ? "configured" : "waiting_config");

    relay.begin();
    logBootStep("RELAY", "ready");

#if ENABLE_RELAY_HARDWARE_TEST
    logLine("INFO", "TEST", "mode=relay_hardware");
    return;
#endif

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
#if TEST_MODE_ANY
    setupTestClockIfNeeded();
#endif
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

#if ENABLE_FSM_SIMULATION_TEST
    updateFSMTestData();
    logLine("INFO", "TEST", "mode=fsm_simulation");
#elif ENABLE_FULL_SYSTEM_TEST
    logLine("INFO", "TEST", "mode=full_system");
#elif ENABLE_MQTT_CONFIGURATION_TEST
    logLine("INFO", "TEST", "mode=mqtt_configuration");
#endif

    fsm.begin();
    logStateEvent(fsm.getState(), fsm.getError());

#if !TEST_MODE_ANY
    mqtt.begin();
    logConnectionEvent(mqtt.isConnected());
#endif

    logLine("INFO", "BOOT", "system=ready");
    delay(5000);
}

void loop() {
#if ENABLE_RELAY_HARDWARE_TEST
    runRelayHardwareTest();
    return;
#endif

#if ENABLE_FSM_SIMULATION_TEST
    updateFSMTestData();
#endif

    fsm.update();

    SensorData sensorData = sensorManager.getData();
#if !TEST_MODE_ANY
    mqtt.update(sensorData, fsm.getState(), fsm.getError());
#endif

    static FertigationState lastLoggedState = fsm.getState();
    static ErrorCode lastLoggedError = fsm.getError();
#if !TEST_MODE_ANY
    static bool lastLoggedMqtt = mqtt.isConnected();
#endif
    static unsigned long lastStatusLog = 0;

    FertigationState currentState = fsm.getState();
    ErrorCode currentError = fsm.getError();
#if !TEST_MODE_ANY
    bool mqttConnected = mqtt.isConnected();
#endif

    if (currentState != lastLoggedState || currentError != lastLoggedError) {
        lastLoggedState = currentState;
        lastLoggedError = currentError;
        logStateEvent(currentState, currentError);
    }

#if !TEST_MODE_ANY
    if (mqttConnected != lastLoggedMqtt) {
        lastLoggedMqtt = mqttConnected;
        logConnectionEvent(mqttConnected);
    }
#endif

    if (millis() - lastStatusLog >= STATUS_LOG_INTERVAL_MS) {
        lastStatusLog = millis();
        logSystemStatus(sensorData);
    }
}
