#include "MQTTManager.h"

// Static member definitions
String MQTTManager::incomingCommand = "";
bool   MQTTManager::commandFlag     = false;

// =========================================
// Constructor
// =========================================
MQTTManager::MQTTManager(RelayManager& relay)
    : mqttClient(wifiClient),
      relayManager(relay)
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
    StaticJsonDocument<384> doc;

    doc["device_id"]   = MQTT_CLIENT_ID;
    doc["timestamp"]   = millis();

    JsonObject sensors  = doc.createNestedObject("sensors");
    sensors["ph"]          = round(data.ph * 100.0f) / 100.0f;
    sensors["ppm"]         = (int)data.ppm;
    sensors["temperature"] = round(data.temperature * 10.0f) / 10.0f;
    sensors["water_level"] = round(data.waterLevel * 10.0f) / 10.0f;
    sensors["tank_volume"] = round(data.tankVolume * 10.0f) / 10.0f;
    sensors["soil_adc"]    = data.soilADC;

    JsonObject flow     = doc.createNestedObject("flow");
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
    StaticJsonDocument<128> doc;
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
    StaticJsonDocument<192> doc;
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();

    JsonObject relays = doc.createNestedObject("relays");
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

bool MQTTManager::isConnected() const {
    return mqttClient.connected();
}

// =========================================
// onMessage() — callback dari broker
// Format JSON: {"relay":"irrigation","action":"on"}
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