#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "../sensors/SensorManager.h"
#include "../actuators/RelayManager.h"
#include "../fsm/FertigationState.h"
#include "../config/ConfigManager.h"
#include "../rtc/RTCManager.h"
#include "../sensors/WaterLevel.h"
#include "../utils/ErrorCode.h"
#include "../fsm/SoilHealthMonitor.h"

// =========================================
// WIFI & MQTT — isi sesuai environment kamu
// =========================================
#include "WiFiCredentials.h"

#define MQTT_BROKER    "aead004bf5144152b88233f1a1949418.s1.eu.hivemq.cloud"
#define MQTT_PORT      8883
#define MQTT_CLIENT_ID "greenhouse-master-01"
#define MQTT_USER      "greenhouse_esp32"
#define MQTT_PASS      "Kebonagungpanenmelon1"

// =========================================
// MQTT TOPICS — Telemetry (publish)
// =========================================
#define TOPIC_SENSORS       "greenhouse/sensors"
#define TOPIC_FSM_STATE     "greenhouse/fsm/state"
#define TOPIC_RELAY_STATUS  "greenhouse/actuators/status"
#define TOPIC_CONFIG_ACK    "greenhouse/config/ack"

// =========================================
// MQTT TOPICS — Commands (subscribe)
// =========================================
#define TOPIC_CMD           "greenhouse/actuators/cmd"
#define TOPIC_CFG_PPM       "greenhouse/config/ppm"
#define TOPIC_CFG_PH        "greenhouse/config/ph"
#define TOPIC_CFG_RECIPE    "greenhouse/config/recipe"
#define TOPIC_CFG_IRRIG     "greenhouse/config/irrigation"
#define TOPIC_CFG_SYSTEM    "greenhouse/config/system"
#define TOPIC_CFG_SCHEDULE  "greenhouse/config/schedule"
#define TOPIC_CFG_TIMER_IRRIG "greenhouse/config/timer_irrigation"
#define TOPIC_CMD_SOIL_RESET  "greenhouse/soil/reset_mode"

// =========================================
// MQTT TOPICS — Telemetry (publish)
// =========================================
#define TOPIC_SOIL_HEALTH   "greenhouse/soil/health"

// Interval publish sensor (ms)
#define MQTT_PUBLISH_INTERVAL 5000UL

class MQTTManager {
public:
    MQTTManager(RelayManager& relay, ConfigManager& config,
                RTCManager& rtc, WaterLevel& waterLevel, SoilHealthMonitor& soilHealth);

    // Panggil di setup()
    void begin();

    // Panggil di loop() — handle reconnect + publish interval
    void update(const SensorData& sensorData, FertigationState fsmState, ErrorCode errorCode = ErrorCode::NONE);

    // Publish FSM state saat berubah (panggil dari FSM atau Main)
    void publishFSMState(FertigationState state, ErrorCode errorCode = ErrorCode::NONE);

    // Publish status relay saat ada perubahan
    void publishRelayStatus();

    // Publish status kesehatan sensor tanah + mode irigasi
    void publishSoilHealth();

    // Cek apakah ada command masuk dari dashboard
    bool hasCommand() const;

    // Ambil dan reset command terakhir
    String getCommand();

    bool isConnected();

private:
    void connectWiFi();
    void connectMQTT();

    void publishSensors(const SensorData& data);
    void publishConfigAck(const char* configName, bool success);

    // Non-static handler — bisa akses member
    void handleMessage(const char* topic, const String& payload);

    // Config handlers
    void handleConfigPPM(const JsonDocument& doc);
    void handleConfigPH(const JsonDocument& doc);
    void handleConfigRecipe(const JsonDocument& doc);
    void handleConfigIrrigation(const JsonDocument& doc);
    void handleConfigSystem(const JsonDocument& doc);
    void handleConfigSchedule(const JsonDocument& doc);
    void handleConfigTimerIrrigation(const JsonDocument& doc);
    void handleSoilResetMode(const String& payload);

    // Static callback PubSubClient — gunakan pointer ke instance
    static void onMessage(const char* topic, byte* payload, unsigned int length);
    static MQTTManager* _instance;

    static String incomingCommand;
    static bool   commandFlag;

    WiFiClientSecure wifiClient;
    PubSubClient     mqttClient;
    RelayManager&    relayManager;
    ConfigManager&   configManager;
    RTCManager&      rtcManager;
    WaterLevel&      waterLevel;
    SoilHealthMonitor& soilHealth;

    unsigned long lastPublish = 0;
    unsigned long lastSoilPublish = 0;
    FertigationState lastFSMState = FertigationState::IDLE;
    IrrigationMode lastIrrigMode = IrrigationMode::HUMIDITY;

    // Helper: konversi enum ke string
    const char* stateToString(FertigationState state);
    const char* errorCodeToString(ErrorCode error);
};

#endif