#include "MQTTManager.h"

// Static member definitions
String       MQTTManager::incomingCommand = "";
bool         MQTTManager::commandFlag     = false;
MQTTManager* MQTTManager::_instance      = nullptr;

// =========================================
// Constructor
// =========================================
MQTTManager::MQTTManager(RelayManager& relay, ConfigManager& config,
                         RTCManager& rtc, WaterLevel& wl)
    : mqttClient(wifiClient),
      relayManager(relay),
      configManager(config),
      rtcManager(rtc),
      waterLevel(wl)
{
    _instance = this;
}

// =========================================
// begin() — koneksi WiFi + MQTT
// =========================================
void MQTTManager::begin() {
    connectWiFi();

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(onMessage);
    mqttClient.setBufferSize(1024);

    connectMQTT();
}

// =========================================
// update() — dipanggil tiap loop()
// =========================================
void MQTTManager::update(
    const SensorData& sensorData,
    FertigationState fsmState
) {
    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.loop();

    if (fsmState != lastFSMState) {
        publishFSMState(fsmState);
        publishRelayStatus();
        lastFSMState = fsmState;
    }

    unsigned long now = millis();
    if (now - lastPublish >= MQTT_PUBLISH_INTERVAL) {
        lastPublish = now;
        publishSensors(sensorData);
    }
}

// =========================================
// publishSensors()
// =========================================
void MQTTManager::publishSensors(const SensorData& data) {
    JsonDocument doc;

    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    JsonObject sensors   = doc["sensors"].to<JsonObject>();
    sensors["ph"]          = round(data.ph * 100.0f) / 100.0f;
    sensors["ppm"]         = (int)data.ppm;
    sensors["temperature"] = round(data.temperature * 10.0f) / 10.0f;
    sensors["water_level"] = round(data.waterLevel * 10.0f) / 10.0f;
    sensors["tank_volume"] = round(data.tankVolume * 10.0f) / 10.0f;
    sensors["soil_adc"]    = data.soilADC;

    JsonObject flow = doc["flow"].to<JsonObject>();
    flow["water"] = round(data.flowWater * 100.0f) / 100.0f;
    flow["a"]     = round(data.flowA * 100.0f) / 100.0f;
    flow["b"]     = round(data.flowB * 100.0f) / 100.0f;

    char buf[384];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_SENSORS, buf, true);
}

// =========================================
// publishFSMState()
// =========================================
void MQTTManager::publishFSMState(FertigationState state) {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["state"]     = stateToString(state);
    doc["timestamp"] = millis();

    char buf[128];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_FSM_STATE, buf, true);

    Serial.print("[MQTT] FSM state: ");
    Serial.println(stateToString(state));
}

// =========================================
// publishRelayStatus()
// =========================================
void MQTTManager::publishRelayStatus() {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    JsonObject relays = doc["relays"].to<JsonObject>();
    relays["water"]      = (digitalRead(21) == LOW);
    relays["nutrient_a"] = (digitalRead(38) == LOW);
    relays["nutrient_b"] = (digitalRead(39) == LOW);
    relays["irrigation"] = (digitalRead(40) == LOW);

    char buf[192];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_RELAY_STATUS, buf, true);
}

// =========================================
// publishConfigAck()
// =========================================
void MQTTManager::publishConfigAck(const char* configName, bool success) {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["config"]    = configName;
    doc["status"]    = success ? "ok" : "error";
    doc["timestamp"] = millis();

    char buf[128];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_CONFIG_ACK, buf, false);
    Serial.printf("[MQTT] Config ACK: %s = %s\n", configName, success ? "ok" : "error");
}

// =========================================
// Command handling
// =========================================
bool MQTTManager::hasCommand() const { return commandFlag; }

String MQTTManager::getCommand() {
    commandFlag = false;
    return incomingCommand;
}

bool MQTTManager::isConnected() {
    return mqttClient.connected();
}

// =========================================
// onMessage() — static callback PubSubClient
// Delegate ke non-static handleMessage()
// =========================================
void MQTTManager::onMessage(
    const char* topic,
    byte* payload,
    unsigned int length
) {
    String msg;
    msg.reserve(length);
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.print("[MQTT] Received on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(msg);

    if (_instance) {
        _instance->handleMessage(topic, msg);
    }
}

// =========================================
// handleMessage() — non-static, bisa akses
// configManager, rtcManager, waterLevel
// =========================================
void MQTTManager::handleMessage(const char* topic, const String& payload) {
    // Command relay
    if (String(topic) == TOPIC_CMD) {
        incomingCommand = payload;
        commandFlag     = true;
        return;
    }

    // Config topics — parse JSON
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("[MQTT] JSON parse error on %s: %s\n", topic, err.c_str());
        publishConfigAck(topic, false);
        return;
    }

    const String t = String(topic);
    if      (t == TOPIC_CFG_PPM)      handleConfigPPM(doc);
    else if (t == TOPIC_CFG_PH)       handleConfigPH(doc);
    else if (t == TOPIC_CFG_RECIPE)   handleConfigRecipe(doc);
    else if (t == TOPIC_CFG_IRRIG)    handleConfigIrrigation(doc);
    else if (t == TOPIC_CFG_SYSTEM)   handleConfigSystem(doc);
    else if (t == TOPIC_CFG_SCHEDULE) handleConfigSchedule(doc);
}

// =========================================
// Config Handlers
// =========================================

void MQTTManager::handleConfigPPM(const JsonDocument& doc) {
    float tolerance = doc["ppm_tolerance"] | configManager.getPPMTolerance();
    float initA     = doc["initial_a"]     | configManager.getInitialNutrientA();
    float initB     = doc["initial_b"]     | configManager.getInitialNutrientB();

    configManager.setPPMConfig(tolerance, initA, initB);
    publishConfigAck("ppm", true);
}

void MQTTManager::handleConfigPH(const JsonDocument& doc) {
    float minPH = doc["min_ph"] | configManager.getDefaultMinPH();
    float maxPH = doc["max_ph"] | configManager.getDefaultMaxPH();

    configManager.setPHConfig(minPH, maxPH);
    publishConfigAck("ph", true);
}

void MQTTManager::handleConfigRecipe(const JsonDocument& doc) {
    JsonArrayConst arr = doc["stages"].as<JsonArrayConst>();
    if (arr.isNull()) {
        publishConfigAck("recipe", false);
        return;
    }

    RecipeStageConfig stages[MAX_RECIPE_STAGES];
    uint8_t count = 0;

    for (JsonObjectConst s : arr) {
        if (count >= MAX_RECIPE_STAGES) break;
        stages[count].maxAgeDays = s["max_age_days"] | (uint16_t)0;
        stages[count].targetPPM  = s["target_ppm"]   | 0.0f;
        stages[count].minPH      = s["min_ph"]        | configManager.getDefaultMinPH();
        stages[count].maxPH      = s["max_ph"]        | configManager.getDefaultMaxPH();
        count++;
    }

    configManager.setRecipeStages(stages, count);
    publishConfigAck("recipe", true);
}

void MQTTManager::handleConfigIrrigation(const JsonDocument& doc) {
    JsonArrayConst arr = doc["stages"].as<JsonArrayConst>();
    if (arr.isNull()) {
        publishConfigAck("irrigation", false);
        return;
    }

    IrrigationStageConfig stages[MAX_IRRIGATION_STAGES];
    uint8_t count = 0;

    for (JsonObjectConst s : arr) {
        if (count >= MAX_IRRIGATION_STAGES) break;
        stages[count].maxAgeDays    = s["max_age_days"]   | (uint16_t)0;
        stages[count].dryThreshold  = s["dry_threshold"]  | (uint16_t)3900;
        stages[count].wetThreshold  = s["wet_threshold"]  | (uint16_t)3650;
        count++;
    }

    configManager.setIrrigationStages(stages, count);
    publishConfigAck("irrigation", true);
}

void MQTTManager::handleConfigSystem(const JsonDocument& doc) {
    uint16_t plants  = doc["total_plants"]              | configManager.getTotalPlants();
    float maxCons    = doc["max_consumption_per_plant"] | configManager.getMaxConsumptionPerPlant();
    float dailyVol   = doc["daily_target_volume"]       | configManager.getDailyTargetVolume();
    float tankCap    = doc["tank_capacity_liter"]       | configManager.getTankCapacityLiter();

    configManager.setSystemConfig(plants, maxCons, dailyVol, tankCap);

    // Propagate ke WaterLevel sensor langsung
    waterLevel.setTankCapacity(tankCap);

    publishConfigAck("system", true);
}

void MQTTManager::handleConfigSchedule(const JsonDocument& doc) {
    uint16_t year   = doc["plant_year"]      | configManager.getPlantYear();
    uint8_t  month  = doc["plant_month"]     | configManager.getPlantMonth();
    uint8_t  day    = doc["plant_day"]       | configManager.getPlantDay();
    uint8_t  hour   = doc["daily_mix_hour"]  | configManager.getDailyMixHour();
    uint8_t  minute = doc["daily_mix_minute"]| configManager.getDailyMixMinute();

    configManager.setScheduleConfig(year, month, day, hour, minute);

    // Propagate ke RTCManager langsung
    rtcManager.setPlantingDate(year, month, day);
    rtcManager.setDailyMixSchedule(hour, minute);

    publishConfigAck("schedule", true);
}

// =========================================
// connectWiFi()
// =========================================
void MQTTManager::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.print("[WiFi] Connecting to ");
    Serial.print(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\n[WiFi] Connected, IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WiFi] Failed — will retry next update()");
    }
}

// =========================================
// connectMQTT()
// =========================================
void MQTTManager::connectMQTT() {
    if (mqttClient.connected()) return;

    wifiClient.setInsecure();

    Serial.print("[MQTT] Connecting to HiveMQ Cloud...");

    bool ok = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);

    if (ok) {
        Serial.println(" connected!");

        // Subscribe semua topics
        mqttClient.subscribe(TOPIC_CMD);
        mqttClient.subscribe(TOPIC_CFG_PPM);
        mqttClient.subscribe(TOPIC_CFG_PH);
        mqttClient.subscribe(TOPIC_CFG_RECIPE);
        mqttClient.subscribe(TOPIC_CFG_IRRIG);
        mqttClient.subscribe(TOPIC_CFG_SYSTEM);
        mqttClient.subscribe(TOPIC_CFG_SCHEDULE);

        Serial.println("[MQTT] Subscribed to all topics");
    } else {
        Serial.print(" failed, rc=");
        Serial.println(mqttClient.state());
    }
}

// =========================================
// stateToString()
// =========================================
const char* MQTTManager::stateToString(FertigationState state) {
    switch (state) {
        case FertigationState::IDLE:                    return "IDLE";
        case FertigationState::WAIT_DAILY_MIX:          return "WAIT_DAILY_MIX";
        case FertigationState::PREPARE_DAILY_MIX:       return "PREPARE_DAILY_MIX";
        case FertigationState::FILL_WATER:              return "FILL_WATER";
        case FertigationState::ADD_NUTRIENT_A:          return "ADD_NUTRIENT_A";
        case FertigationState::MIX_A:                   return "MIX_A";
        case FertigationState::ADD_NUTRIENT_B:          return "ADD_NUTRIENT_B";
        case FertigationState::MIX_B:                   return "MIX_B";
        case FertigationState::VALIDATE:                return "VALIDATE";
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