#include "RecoveryManager.h"

void RecoveryManager::begin() {
    prefs.begin("fertigation", false);
}

void RecoveryManager::save(const RecoveryData& data) {
    prefs.putBytes("state", &data, sizeof(data));
}

RecoveryData
RecoveryManager::load() {
    RecoveryData data{};

    data.batchRunning = false;

    prefs.getBytes("state", &data, sizeof(data));

    return data;
}

void RecoveryManager::clear() {
    prefs.remove("state");
}