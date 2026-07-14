#include "MQTTManager.h"
#include "../config/PinConfig.h"

// Static member definitions
String       MQTTManager::incomingCommand = "";
bool         MQTTManager::commandFlag     = false;
MQTTManager* MQTTManager::_instance      = nullptr;

// =========================================
// Constructor
// =========================================
MQTTManager::MQTTManager(RelayManager& relay, ConfigManager& config,
                         RTCManager& rtc, WaterLevel& wl,
                         SoilHealthMonitor& sh, FertigationFSM& f)
    : mqttClient(wifiClient),
      relayManager(relay),
      configManager(config),
      rtcManager(rtc),
      waterLevel(wl),
      soilHealth(sh),
      fsm(f)
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
    FertigationState fsmState,
    ErrorCode errorCode
) {
    connectWiFi();

    // MQTT tidak boleh mencoba konek jika Wi-Fi belum tersambung.
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.loop();

    if (fsmState != lastFSMState) {
        publishFSMState(fsmState, errorCode);
        publishRelayStatus();
        lastFSMState = fsmState;
    }

    IrrigationMode currentIrrigMode = soilHealth.getMode();
    if (currentIrrigMode != lastIrrigMode) {
        publishSoilHealth();
        lastIrrigMode = currentIrrigMode;
    }

    // Polling tank-low alert dari FSM (flag + getter pattern, tanpa coupling balik)
    if (fsm.isTankLowAlertPending()) {
        publishTankLowAlert(fsm.getTankLowDeficit());
        fsm.clearTankLowAlert();
    }

    unsigned long now = millis();
    if (now - lastPublish >= MQTT_PUBLISH_INTERVAL) {
        lastPublish = now;
        publishSensors(sensorData);
    }

    if (now - lastSoilPublish >= MQTT_PUBLISH_INTERVAL) {
        lastSoilPublish = now;
        publishSoilHealth();
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
void MQTTManager::publishFSMState(FertigationState state, ErrorCode errorCode) {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["state"]     = stateToString(state);
    doc["timestamp"] = millis();

    if (errorCode != ErrorCode::NONE) {
        doc["error_code"] = errorCodeToString(errorCode);
    }

    char buf[160];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_FSM_STATE, buf, true);

    Serial.print("[MQTT] FSM state: ");
    Serial.print(stateToString(state));
    if (errorCode != ErrorCode::NONE) {
        Serial.print(" | error: ");
        Serial.print(errorCodeToString(errorCode));
    }
    Serial.println();
}

// =========================================
// publishRelayStatus()
// =========================================
void MQTTManager::publishRelayStatus() {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    JsonObject relays = doc["relays"].to<JsonObject>();
    relays["water"]      = (digitalRead(RELAY_5_PIN) == LOW);
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

    if (String(topic) == TOPIC_CMD_SOIL_RESET) {
        handleSoilResetMode(payload);
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
    else if (t == TOPIC_CFG_TIMER_IRRIG) handleConfigTimerIrrigation(doc);
    else if (t == TOPIC_CFG_MIX_EXT)  handleConfigMixScheduleExt(doc);
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

    // Pada boot pertama (atau saat Wi-Fi sebelumnya tidak tersedia),
    // WiFiManager menampilkan captive portal di hotspot ini. Kredensial
    // yang dipilih dari HP disimpan aman di NVS oleh library.
    static bool provisioned = false;
    if (!provisioned) {
        WiFiManager wifiManager;
        wifiManager.setTitle("Melon Master Wi-Fi");
        wifiManager.setCustomHeadElement(R"rawliteral(
<style>
  :root { color-scheme: light; }
  body { background:#f6fff8 !important; color:#174d2b !important; font-family:Arial,sans-serif !important; }
  h1, h2 { color:#16803b !important; }
  button, input[type=submit] { background:#16803b !important; border-radius:8px !important; color:#fff !important; }
  input, select { border:1px solid #79bd8d !important; border-radius:7px !important; }
  .msg { border-color:#79bd8d !important; }
</style>)rawliteral");

        Serial.println("[WiFi] Membuka portal konfigurasi: MelonMaster-Setup");
        Serial.println("[WiFi] Hubungkan HP ke hotspot tersebut (password: Melon123), lalu buka 192.168.4.1.");

        // Fungsi ini otomatis memakai jaringan yang tersimpan. Jika gagal,
        // hotspot captive portal dibuat sehingga konfigurasi tidak perlu di-hardcode.
        if (!wifiManager.autoConnect("MelonMaster-Setup", "Melon123")) {
            Serial.println("[WiFi] Portal ditutup tanpa koneksi; restart untuk mencoba lagi.");
            return;
        }

        provisioned = true;
        Serial.print("[WiFi] Connected, IP: ");
        Serial.println(WiFi.localIP());
        return;
    }

    // Setelah konfigurasi awal, reconnect berjalan non-blocking agar loop
    // pengendalian fertigation tidak tertahan bila router sedang mati.
    const unsigned long now = millis();
    if (now - lastWiFiRetry >= 10000UL) {
        lastWiFiRetry = now;
        Serial.println("[WiFi] Koneksi terputus, mencoba reconnect...");
        WiFi.reconnect();
    }
}

// =========================================
// connectMQTT()
// =========================================
void MQTTManager::connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) return;
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
        mqttClient.subscribe(TOPIC_CFG_TIMER_IRRIG);
        mqttClient.subscribe(TOPIC_CFG_MIX_EXT);
        mqttClient.subscribe(TOPIC_CMD_SOIL_RESET);

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

// =========================================
// errorCodeToString()
// =========================================
const char* MQTTManager::errorCodeToString(ErrorCode error) {
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
        case ErrorCode::RECIPE_PH_CONFLICT:  return "RECIPE_PH_CONFLICT";
        case ErrorCode::TANK_LOW:            return "TANK_LOW";
        default:                             return "UNKNOWN_ERROR";
    }
}

// =========================================
// handleConfigTimerIrrigation()
// =========================================
void MQTTManager::handleConfigTimerIrrigation(const JsonDocument& doc) {
    float mlPerPlant = doc["daily_water_volume_ml_per_plant"] | configManager.getDailyWaterVolumeMLPerPlant();
    JsonArrayConst arr = doc["slots"].as<JsonArrayConst>();
    
    if (arr.isNull()) {
        publishConfigAck("timer_irrigation", false);
        return;
    }

    IrrigationSlot slots[MAX_IRRIG_SLOTS];
    uint8_t count = 0;

    for (JsonObjectConst s : arr) {
        if (count >= MAX_IRRIG_SLOTS) break;
        slots[count].hour   = s["hour"]   | (uint8_t)0;
        slots[count].minute = s["minute"] | (uint8_t)0;
        count++;
    }

    configManager.setTimerIrrigationConfig(mlPerPlant, slots, count);
    publishConfigAck("timer_irrigation", true);
}

// =========================================
// handleConfigMixScheduleExt()
// Topic: greenhouse/config/mix_schedule_ext
// Payload contoh:
// {
//   "mix_interval_days":    3,
//   "per_plant_need_liter": 0.5,
//   "stir_evening_hour":    18,
//   "stir_evening_minute":  0,
//   "stir_duration_seconds": 300
// }
// =========================================
void MQTTManager::handleConfigMixScheduleExt(const JsonDocument& doc) {
    uint16_t intervalDays = doc["mix_interval_days"]    | configManager.getMixIntervalDays();
    float    perPlant     = doc["per_plant_need_liter"] | configManager.getPerPlantNeedLiter();
    uint8_t  stirEvHour   = doc["stir_evening_hour"]    | configManager.getStirEveningHour();
    uint8_t  stirEvMin    = doc["stir_evening_minute"]  | configManager.getStirEveningMinute();
    uint32_t stirDurSec   = doc["stir_duration_seconds"]| (uint32_t)(configManager.getStirDurationMs() / 1000UL);

    configManager.setMixScheduleExt(intervalDays, perPlant, stirEvHour, stirEvMin,
                                    stirDurSec * 1000UL);
    publishConfigAck("mix_schedule_ext", true);
}

// =========================================
// handleSoilResetMode()
// =========================================
void MQTTManager::handleSoilResetMode(const String& payload) {
    soilHealth.resetToHumidityMode();
    publishSoilHealth();
}

// =========================================
// publishTankLowAlert()
// =========================================
void MQTTManager::publishTankLowAlert(float deficitLiter) {
    JsonDocument doc;
    doc["device_id"]     = MQTT_CLIENT_ID;
    doc["deficit_liter"] = round(deficitLiter * 100.0f) / 100.0f;
    doc["timestamp"]     = millis();

    char buf[128];
    serializeJson(doc, buf);
    mqttClient.publish(TOPIC_ALERT_TANK_LOW, buf, false);

    Serial.printf("[MQTT] Tank LOW alert published: deficit=%.2fL\n", deficitLiter);
}

// =========================================
// publishSoilHealth()
// =========================================
void MQTTManager::publishSoilHealth() {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    doc["mode"] = (soilHealth.getMode() == IrrigationMode::TIMER) ? "TIMER" : "HUMIDITY";
    doc["health_score"] = soilHealth.getHealthScore();

    SoilRuleFlags rules = soilHealth.getActiveRules();
    JsonObject r = doc["active_rules"].to<JsonObject>();
    r["heartbeat_timeout"] = rules.heartbeatTimeout;
    r["out_of_range"]      = rules.outOfRange;
    r["flatline"]          = rules.flatline;
    r["no_response"]       = rules.noResponse;

    char buf[256];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_SOIL_HEALTH, buf, true);
    
    Serial.printf("[MQTT] Published Soil Health. Mode: %s, Health: %d%%\n", 
                  (soilHealth.getMode() == IrrigationMode::TIMER) ? "TIMER" : "HUMIDITY", 
                  soilHealth.getHealthScore());
}
