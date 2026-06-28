#include "MQTTManager.h"

// Static member definitions
String MQTTManager::incomingCommand = "";
bool   MQTTManager::commandFlag     = false;

// [RUNTIME] Static buffer untuk config masuk
String MQTTManager::incomingConfig = "";
bool   MQTTManager::configFlag     = false;

// =========================================
// Constructor
// =========================================
// [RUNTIME] Constructor baru: tambah RTCManager & WaterLevel untuk update config live
MQTTManager::MQTTManager(RelayManager& relay, RTCManager& rtc, WaterLevel& level)
    : mqttClient(wifiClient),
      relayManager(relay),
      rtcManager(rtc),
      waterLevelSensor(level)
{}

// =========================================
// begin() — koneksi WiFi + MQTT
// =========================================
void MQTTManager::begin() {
    connectWiFi();

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(onMessage);
    mqttClient.setBufferSize(512);

    connectMQTT();
}

// =========================================
// update() — dipanggil tiap loop()
// =========================================
void MQTTManager::update(
    const SensorData& sensorData,
    FertigationState fsmState
) {
    // Reconnect jika putus
    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.loop();

    // [RUNTIME] Handle config yang masuk via TOPIC_CONFIG_SET
    if (configFlag) {
        applyConfig(incomingConfig);
        configFlag = false;
    }

    // Publish FSM state jika berubah
    if (fsmState != lastFSMState) {
        publishFSMState(fsmState);
        publishRelayStatus();
        lastFSMState = fsmState;
    }

    // Publish sensor tiap interval
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

    doc["device_id"]   = MQTT_CLIENT_ID;
    doc["timestamp"]   = millis();

    JsonObject sensors  = doc["sensors"].to<JsonObject>();
    sensors["ph"]          = round(data.ph * 100.0f) / 100.0f;
    sensors["ppm"]         = (int)data.ppm;
    sensors["temperature"] = round(data.temperature * 10.0f) / 10.0f;
    sensors["water_level"] = round(data.waterLevel * 10.0f) / 10.0f;
    sensors["tank_volume"] = round(data.tankVolume * 10.0f) / 10.0f;
    sensors["soil_adc"]    = data.soilADC;

    JsonObject flow     = doc["flow"].to<JsonObject>();
    flow["water"] = round(data.flowWater * 100.0f) / 100.0f;
    flow["a"]     = round(data.flowA * 100.0f) / 100.0f;
    flow["b"]     = round(data.flowB * 100.0f) / 100.0f;

    char buf[384];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_SENSORS, buf, true);

    Serial.print("[MQTT] Published sensors: ");
    Serial.println(buf);
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
    // Relay aktif LOW, jadi digitalRead HIGH = OFF, LOW = ON
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    JsonObject relays = doc["relays"].to<JsonObject>();
    relays["water"]       = (digitalRead(21) == LOW);
    relays["nutrient_a"]  = (digitalRead(38) == LOW);
    relays["nutrient_b"]  = (digitalRead(39) == LOW);
    relays["irrigation"]  = (digitalRead(40) == LOW);

    char buf[192];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_RELAY_STATUS, buf, true);
}

// =========================================
// Command handling
// =========================================
bool MQTTManager::hasCommand() const {
    return commandFlag;
}

String MQTTManager::getCommand() {
    commandFlag = false;
    return incomingCommand;
}

bool MQTTManager::isConnected() {
    return mqttClient.connected();
}

// =========================================
// onMessage() — callback dari broker
// Format CMD  : {"relay":"irrigation","action":"on"}
// Format CONFIG: {"planting_date":"2026-06-01","mix_hour":5,...}
// =========================================
void MQTTManager::onMessage(
    const char* topic,
    byte* payload,
    unsigned int length
) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.print("[MQTT] Received on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(msg);

    if (String(topic) == TOPIC_CMD) {
        incomingCommand = msg;
        commandFlag     = true;
    }
    // [RUNTIME] Terima config dari dashboard/tool
    else if (String(topic) == TOPIC_CONFIG_SET) {
        incomingConfig = msg;
        configFlag     = true;
    }
}

// =========================================
// [RUNTIME] applyConfig()
// Parse JSON dari TOPIC_CONFIG_SET dan update gConfig,
// RTCManager, serta WaterLevel secara live.
//
// Contoh JSON:
// {
//   "planting_date"       : "2026-06-01",
//   "mix_hour"            : 5,
//   "mix_minute"          : 0,
//   "tank_height_cm"      : 45.0,
//   "tank_capacity_liter" : 15.0
// }
// Setiap field opsional — hanya field yang ada yang diupdate.
// =========================================
void MQTTManager::applyConfig(const String& json) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);

    if (err) {
        Serial.print("[CONFIG] JSON parse error: ");
        Serial.println(err.c_str());
        return;
    }

    bool changed = false;

    // --- mix_hour ---
    if (!doc["mix_hour"].isNull()) {
        gConfig.mixHour = doc["mix_hour"].as<uint8_t>();
        Serial.print("[CONFIG] mix_hour = ");
        Serial.println(gConfig.mixHour);
        changed = true;
    }

    // --- mix_minute ---
    if (!doc["mix_minute"].isNull()) {
        gConfig.mixMinute = doc["mix_minute"].as<uint8_t>();
        Serial.print("[CONFIG] mix_minute = ");
        Serial.println(gConfig.mixMinute);
        changed = true;
    }

    // --- planting_date: format "YYYY-MM-DD" ---
    if (!doc["planting_date"].isNull()) {
        const char* dateStr = doc["planting_date"].as<const char*>();
        int y, m, d;
        if (sscanf(dateStr, "%d-%d-%d", &y, &m, &d) == 3) {
            gConfig.plantingYear  = (uint16_t)y;
            gConfig.plantingMonth = (uint8_t)m;
            gConfig.plantingDay   = (uint8_t)d;
            rtcManager.setPlantingDate(
                gConfig.plantingYear,
                gConfig.plantingMonth,
                gConfig.plantingDay
            );
            changed = true;
        } else {
            Serial.println("[CONFIG] Format planting_date salah. Gunakan: YYYY-MM-DD");
        }
    }

    // --- tank_height_cm ---
    if (!doc["tank_height_cm"].isNull()) {
        gConfig.tankHeightCM = doc["tank_height_cm"].as<float>();
        changed = true;
    }

    // --- tank_capacity_liter ---
    if (!doc["tank_capacity_liter"].isNull()) {
        gConfig.tankCapacityLiter = doc["tank_capacity_liter"].as<float>();
        changed = true;
    }

    // Update WaterLevel jika ada perubahan dimensi toren
    if (!doc["tank_height_cm"].isNull() || !doc["tank_capacity_liter"].isNull()) {
        waterLevelSensor.setTankConfig(
            gConfig.tankHeightCM,
            gConfig.tankCapacityLiter
        );
    }

    if (changed) {
        Serial.println("[CONFIG] Config berhasil diupdate.");
        publishConfigStatus();
    } else {
        Serial.println("[CONFIG] Tidak ada field yang dikenali dalam JSON.");
    }
}

// =========================================
// [RUNTIME] publishConfigStatus()
// Publish konfirmasi config aktif ke TOPIC_CONFIG_STATUS
// =========================================
void MQTTManager::publishConfigStatus() {
    JsonDocument doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    // Jam mixing
    JsonObject mixing = doc["mix_schedule"].to<JsonObject>();
    mixing["hour"]   = gConfig.mixHour;
    mixing["minute"] = gConfig.mixMinute;

    // Tanggal tanam
    char dateStr[12];
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d",
        gConfig.plantingYear,
        gConfig.plantingMonth,
        gConfig.plantingDay
    );
    doc["planting_date"] = dateStr;

    // Ukuran toren
    JsonObject tank = doc["tank"].to<JsonObject>();
    tank["height_cm"]      = gConfig.tankHeightCM;
    tank["capacity_liter"] = gConfig.tankCapacityLiter;

    char buf[256];
    serializeJson(doc, buf);

    mqttClient.publish(TOPIC_CONFIG_STATUS, buf, true);

    Serial.print("[CONFIG] Status dikirim: ");
    Serial.println(buf);
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

    // HiveMQ Cloud pakai TLS — skip verifikasi sertifikat
    wifiClient.setInsecure();

    Serial.print("[MQTT] Connecting to HiveMQ Cloud...");

    bool ok = mqttClient.connect(
        MQTT_CLIENT_ID,
        MQTT_USER,
        MQTT_PASS
    );

    if (ok) {
        Serial.println(" connected!");
        mqttClient.subscribe(TOPIC_CMD);
        Serial.print("[MQTT] Subscribed to: ");
        Serial.println(TOPIC_CMD);

        // [RUNTIME] Subscribe ke config topic
        mqttClient.subscribe(TOPIC_CONFIG_SET);
        Serial.print("[MQTT] Subscribed to: ");
        Serial.println(TOPIC_CONFIG_SET);
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
