#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "../sensors/SensorManager.h"
#include "../actuators/RelayManager.h"
#include "../fsm/FertigationState.h"

// =========================================
// WIFI & MQTT — isi sesuai environment kamu
// =========================================
#define WIFI_SSID       "SSID WIFI"       // ganti dengan WiFi di greenhouse
#define WIFI_PASSWORD   "PASSWORD WIFI"   // ganti dengan password WiFi

#define MQTT_BROKER     "aead004bf5144152b88233f1a1949418.s1.eu.hivemq.cloud"
#define MQTT_PORT       8883                   // TLS port HiveMQ Cloud
#define MQTT_CLIENT_ID  "greenhouse-master-01"
#define MQTT_USER       "greenhouse_esp32"
#define MQTT_PASS       "Kebonagungpanenmelon1"

// =========================================
// MQTT TOPICS
// =========================================
#define TOPIC_SENSORS       "greenhouse/sensors"
#define TOPIC_FSM_STATE     "greenhouse/fsm/state"
#define TOPIC_RELAY_STATUS  "greenhouse/actuators/status"
#define TOPIC_CMD           "greenhouse/actuators/cmd"

// Interval publish sensor (ms)
#define MQTT_PUBLISH_INTERVAL 5000UL

class MQTTManager {
public:
    MQTTManager(RelayManager& relay);

    // Panggil di setup()
    void begin();

    // Panggil di loop() — handle reconnect + publish interval
    void update(const SensorData& sensorData, FertigationState fsmState);

    // Publish FSM state saat berubah (panggil dari FSM atau Main)
    void publishFSMState(FertigationState state);

    // Publish status relay saat ada perubahan
    void publishRelayStatus();

    // Cek apakah ada command masuk dari dashboard
    bool hasCommand() const;

    // Ambil dan reset command terakhir
    String getCommand();

    bool isConnected();

private:
    void connectWiFi();
    void connectMQTT();

    void publishSensors(const SensorData& data);

    static void onMessage(
        const char* topic,
        byte* payload,
        unsigned int length
    );

    static String incomingCommand;
    static bool   commandFlag;

    WiFiClientSecure wifiClient;
    PubSubClient mqttClient;
    RelayManager& relayManager;

    unsigned long lastPublish = 0;
    FertigationState lastFSMState = FertigationState::IDLE;

    // Helper: konversi enum ke string
    const char* stateToString(FertigationState state);
};

#endif