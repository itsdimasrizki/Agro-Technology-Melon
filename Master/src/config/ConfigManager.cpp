#include "ConfigManager.h"

// =========================================
// NVS Keys
// =========================================
// Namespace max 15 karakter di ESP32 Preferences
static const char* NS_PPM   = "cfg_ppm";
static const char* NS_PH    = "cfg_ph";
static const char* NS_REC   = "cfg_recipe";
static const char* NS_IRR   = "cfg_irrig";
static const char* NS_SYS   = "cfg_sys";
static const char* NS_SCHED = "cfg_sched";
static const char* NS_META  = "cfg_meta";
static const char* NS_TIMER = "cfg_timer";  // Timer Fallback Irrigation

// =========================================
// begin() — load dari NVS atau pakai default
// =========================================
void ConfigManager::begin() {
    applyDefaults();
    loadFromNVS();

    Serial.println("[CFG] ConfigManager ready");
    Serial.print("[CFG] Configured: ");
    Serial.println(_configured ? "YES (from NVS)" : "NO (using defaults)");
}

// =========================================
// applyDefaults() — nilai fallback
// =========================================
void ConfigManager::applyDefaults() {
    _ppmTolerance     = 0.0f;
    _initialNutrientA = 0.0f;
    _initialNutrientB = 0.0f;

    _defaultMinPH = 0.0f;
    _defaultMaxPH = 0.0f;

    _numRecipeStages = 0;
    memset(_recipeStages, 0, sizeof(_recipeStages));

    _numIrrigationStages = 0;
    memset(_irrigationStages, 0, sizeof(_irrigationStages));

    _totalPlants            = 0;
    _maxConsumptionPerPlant = 0.0f;
    _dailyTargetVolume      = 0.0f;
    _tankCapacityLiter      = 0.0f;

    _plantYear      = 0;
    _plantMonth     = 0;
    _plantDay       = 0;
    _dailyMixHour   = 0;
    _dailyMixMinute = 0;

    _dailyWaterVolumeMLPerPlant = 0.0f;
    _numIrrigationSlots        = 0;
    memset(_irrigationSlots, 0, sizeof(_irrigationSlots));

    _configured = false;
}

// =========================================
// loadFromNVS()
// =========================================
void ConfigManager::loadFromNVS() {
    // Meta — cek apakah sudah pernah dikonfigurasi
    _prefs.begin(NS_META, true);
    _configured = _prefs.getBool("configured", false);
    _prefs.end();

    if (!_configured) return;

    // PPM
    _prefs.begin(NS_PPM, true);
    _ppmTolerance     = _prefs.getFloat("tolerance", _ppmTolerance);
    _initialNutrientA = _prefs.getFloat("initA",     _initialNutrientA);
    _initialNutrientB = _prefs.getFloat("initB",     _initialNutrientB);
    _prefs.end();

    // PH
    _prefs.begin(NS_PH, true);
    _defaultMinPH = _prefs.getFloat("minPH", _defaultMinPH);
    _defaultMaxPH = _prefs.getFloat("maxPH", _defaultMaxPH);
    _prefs.end();

    // Recipe
    _prefs.begin(NS_REC, true);
    _numRecipeStages = _prefs.getUChar("count", _numRecipeStages);
    if (_numRecipeStages > MAX_RECIPE_STAGES) _numRecipeStages = MAX_RECIPE_STAGES;
    _prefs.getBytes("stages", _recipeStages,
                    _numRecipeStages * sizeof(RecipeStageConfig));
    _prefs.end();

    // Irrigation
    _prefs.begin(NS_IRR, true);
    _numIrrigationStages = _prefs.getUChar("count", _numIrrigationStages);
    if (_numIrrigationStages > MAX_IRRIGATION_STAGES) _numIrrigationStages = MAX_IRRIGATION_STAGES;
    _prefs.getBytes("stages", _irrigationStages,
                    _numIrrigationStages * sizeof(IrrigationStageConfig));
    _prefs.end();

    // System
    _prefs.begin(NS_SYS, true);
    _totalPlants            = _prefs.getUShort("plants",  _totalPlants);
    _maxConsumptionPerPlant = _prefs.getFloat("maxCons",  _maxConsumptionPerPlant);
    _dailyTargetVolume      = _prefs.getFloat("dayVol",   _dailyTargetVolume);
    _tankCapacityLiter      = _prefs.getFloat("tankCap",  _tankCapacityLiter);
    _prefs.end();

    // Schedule
    _prefs.begin(NS_SCHED, true);
    _plantYear      = _prefs.getUShort("pYear",  _plantYear);
    _plantMonth     = _prefs.getUChar("pMonth",  _plantMonth);
    _plantDay       = _prefs.getUChar("pDay",    _plantDay);
    _dailyMixHour   = _prefs.getUChar("mixHour", _dailyMixHour);
    _dailyMixMinute = _prefs.getUChar("mixMin",  _dailyMixMinute);
    _prefs.end();

    // Timer Irrigation
    _prefs.begin(NS_TIMER, true);
    _dailyWaterVolumeMLPerPlant = _prefs.getFloat("mlPerPlant", _dailyWaterVolumeMLPerPlant);
    _numIrrigationSlots = _prefs.getUChar("slotCount", _numIrrigationSlots);
    if (_numIrrigationSlots > MAX_IRRIG_SLOTS) _numIrrigationSlots = MAX_IRRIG_SLOTS;
    _prefs.getBytes("slots", _irrigationSlots,
                    _numIrrigationSlots * sizeof(IrrigationSlot));
    _prefs.end();

    Serial.println("[CFG] Loaded from NVS");
}

// =========================================
// saveAll()
// =========================================
void ConfigManager::saveAll() {
    // PPM
    _prefs.begin(NS_PPM, false);
    _prefs.putFloat("tolerance", _ppmTolerance);
    _prefs.putFloat("initA",     _initialNutrientA);
    _prefs.putFloat("initB",     _initialNutrientB);
    _prefs.end();

    // PH
    _prefs.begin(NS_PH, false);
    _prefs.putFloat("minPH", _defaultMinPH);
    _prefs.putFloat("maxPH", _defaultMaxPH);
    _prefs.end();

    // Recipe
    _prefs.begin(NS_REC, false);
    _prefs.putUChar("count", _numRecipeStages);
    _prefs.putBytes("stages", _recipeStages,
                    _numRecipeStages * sizeof(RecipeStageConfig));
    _prefs.end();

    // Irrigation
    _prefs.begin(NS_IRR, false);
    _prefs.putUChar("count", _numIrrigationStages);
    _prefs.putBytes("stages", _irrigationStages,
                    _numIrrigationStages * sizeof(IrrigationStageConfig));
    _prefs.end();

    // System
    _prefs.begin(NS_SYS, false);
    _prefs.putUShort("plants",  _totalPlants);
    _prefs.putFloat("maxCons",  _maxConsumptionPerPlant);
    _prefs.putFloat("dayVol",   _dailyTargetVolume);
    _prefs.putFloat("tankCap",  _tankCapacityLiter);
    _prefs.end();

    // Schedule
    _prefs.begin(NS_SCHED, false);
    _prefs.putUShort("pYear",  _plantYear);
    _prefs.putUChar("pMonth",  _plantMonth);
    _prefs.putUChar("pDay",    _plantDay);
    _prefs.putUChar("mixHour", _dailyMixHour);
    _prefs.putUChar("mixMin",  _dailyMixMinute);
    _prefs.end();

    // Timer Irrigation
    _prefs.begin(NS_TIMER, false);
    _prefs.putFloat("mlPerPlant", _dailyWaterVolumeMLPerPlant);
    _prefs.putUChar("slotCount",  _numIrrigationSlots);
    _prefs.putBytes("slots",      _irrigationSlots,
                    _numIrrigationSlots * sizeof(IrrigationSlot));
    _prefs.end();

    // Meta — tandai sudah dikonfigurasi
    _prefs.begin(NS_META, false);
    _prefs.putBool("configured", true);
    _prefs.end();

    _configured = true;
}

// =========================================
// clearConfig() — menghapus semua konfigurasi
// =========================================
void ConfigManager::clearConfig() {
    applyDefaults();

    // Hapus data di NVS
    _prefs.begin(NS_META, false);
    _prefs.putBool("configured", false);
    _prefs.end();

    _prefs.begin(NS_PPM, false);   _prefs.clear(); _prefs.end();
    _prefs.begin(NS_PH, false);    _prefs.clear(); _prefs.end();
    _prefs.begin(NS_REC, false);   _prefs.clear(); _prefs.end();
    _prefs.begin(NS_IRR, false);   _prefs.clear(); _prefs.end();
    _prefs.begin(NS_SYS, false);   _prefs.clear(); _prefs.end();
    _prefs.begin(NS_SCHED, false); _prefs.clear(); _prefs.end();
    _prefs.begin(NS_TIMER, false); _prefs.clear(); _prefs.end();

    _configured = false;
    Serial.println("[CFG] Configuration cleared/wiped.");
}

// =========================================
// Setters — dipanggil MQTTManager
// =========================================
void ConfigManager::setPPMConfig(float tolerance, float initA, float initB) {
    _ppmTolerance     = tolerance;
    _initialNutrientA = initA;
    _initialNutrientB = initB;
    saveAll();
    Serial.printf("[CFG] PPM updated: tol=%.0f, A=%.2fL, B=%.2fL\n",
                  tolerance, initA, initB);
}

void ConfigManager::setPHConfig(float minPH, float maxPH) {
    _defaultMinPH = minPH;
    _defaultMaxPH = maxPH;
    saveAll();
    Serial.printf("[CFG] pH updated: min=%.2f, max=%.2f\n", minPH, maxPH);
}

void ConfigManager::setRecipeStages(const RecipeStageConfig* stages, uint8_t count) {
    if (count > MAX_RECIPE_STAGES) count = MAX_RECIPE_STAGES;
    _numRecipeStages = count;
    memcpy(_recipeStages, stages, count * sizeof(RecipeStageConfig));
    saveAll();
    Serial.printf("[CFG] Recipe updated: %d stages\n", count);
}

void ConfigManager::setIrrigationStages(const IrrigationStageConfig* stages, uint8_t count) {
    if (count > MAX_IRRIGATION_STAGES) count = MAX_IRRIGATION_STAGES;
    _numIrrigationStages = count;
    memcpy(_irrigationStages, stages, count * sizeof(IrrigationStageConfig));
    saveAll();
    Serial.printf("[CFG] Irrigation updated: %d stages\n", count);
}

void ConfigManager::setSystemConfig(uint16_t plants, float maxConsumption,
                                    float dailyVolume, float tankCapacity) {
    _totalPlants            = plants;
    _maxConsumptionPerPlant = maxConsumption;
    _dailyTargetVolume      = dailyVolume;
    _tankCapacityLiter      = tankCapacity;
    saveAll();
    Serial.printf("[CFG] System updated: plants=%d, dailyVol=%.2fL, tankCap=%.2fL\n",
                  plants, dailyVolume, tankCapacity);
}

void ConfigManager::setScheduleConfig(uint16_t year, uint8_t month, uint8_t day,
                                      uint8_t mixHour, uint8_t mixMinute) {
    _plantYear      = year;
    _plantMonth     = month;
    _plantDay       = day;
    _dailyMixHour   = mixHour;
    _dailyMixMinute = mixMinute;
    saveAll();
    Serial.printf("[CFG] Schedule updated: plant=%04d-%02d-%02d, mix=%02d:%02d\n",
                  year, month, day, mixHour, mixMinute);
}

// =========================================
// Getters dengan bounds check
// =========================================
RecipeStageConfig ConfigManager::getRecipeStage(uint8_t index) const {
    if (index >= _numRecipeStages) {
        return {0, 0.0f, 0.0f, 0.0f};
    }
    return _recipeStages[index];
}

IrrigationStageConfig ConfigManager::getIrrigationStage(uint8_t index) const {
    if (index >= _numIrrigationStages) {
        return {0, 0, 0};
    }
    return _irrigationStages[index];
}

IrrigationSlot ConfigManager::getIrrigationSlot(uint8_t index) const {
    if (index >= _numIrrigationSlots) {
        return {0, 0};
    }
    return _irrigationSlots[index];
}

void ConfigManager::setTimerIrrigationConfig(float mlPerPlant,
                                             const IrrigationSlot* slots,
                                             uint8_t count) {
    if (count > MAX_IRRIG_SLOTS) count = MAX_IRRIG_SLOTS;
    _dailyWaterVolumeMLPerPlant = mlPerPlant;
    _numIrrigationSlots         = count;
    memcpy(_irrigationSlots, slots, count * sizeof(IrrigationSlot));
    saveAll();
    Serial.printf("[CFG] Timer irrigation updated: %.0fmL/plant, %d slots\n",
                  mlPerPlant, count);
}
