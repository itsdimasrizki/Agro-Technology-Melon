#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

// =========================================
// Fixed stage limits (hardcode by design)
// =========================================
constexpr uint8_t MAX_RECIPE_STAGES     = 7;
constexpr uint8_t MAX_IRRIGATION_STAGES = 7;

// =========================================
// Stage structs (runtime, dari ConfigManager)
// =========================================
struct RecipeStageConfig {
    uint16_t maxAgeDays;
    float    targetPPM;
    float    minPH;
    float    maxPH;
};

struct IrrigationStageConfig {
    uint16_t maxAgeDays;
    uint16_t dryThreshold;
    uint16_t wetThreshold;
};

// =========================================
// ConfigManager
// Menyimpan & menyediakan semua konfigurasi
// runtime yang diterima dari MQTT.
// Disimpan ke NVS (Preferences) supaya
// bertahan saat restart.
// =========================================
class ConfigManager {
public:
    // Panggil di setup() sebelum komponen lain
    void begin();

    // ---- PPM ----
    float    getPPMTolerance()     const { return _ppmTolerance; }
    float    getInitialNutrientA() const { return _initialNutrientA; }
    float    getInitialNutrientB() const { return _initialNutrientB; }
    void     setPPMConfig(float tolerance, float initA, float initB);

    // ---- pH ----
    float    getDefaultMinPH() const { return _defaultMinPH; }
    float    getDefaultMaxPH() const { return _defaultMaxPH; }
    void     setPHConfig(float minPH, float maxPH);

    // ---- Recipe Stages ----
    uint8_t           getNumRecipeStages()          const { return _numRecipeStages; }
    RecipeStageConfig getRecipeStage(uint8_t index) const;
    void              setRecipeStages(const RecipeStageConfig* stages, uint8_t count);

    // ---- Irrigation Stages ----
    uint8_t               getNumIrrigationStages()          const { return _numIrrigationStages; }
    IrrigationStageConfig getIrrigationStage(uint8_t index) const;
    void                  setIrrigationStages(const IrrigationStageConfig* stages, uint8_t count);

    // ---- System ----
    uint16_t getTotalPlants()            const { return _totalPlants; }
    float    getMaxConsumptionPerPlant() const { return _maxConsumptionPerPlant; }
    float    getDailyTargetVolume()      const { return _dailyTargetVolume; }
    float    getTankCapacityLiter()      const { return _tankCapacityLiter; }
    void     setSystemConfig(uint16_t plants, float maxConsumption,
                             float dailyVolume, float tankCapacity);

    // ---- Schedule ----
    uint16_t getPlantYear()      const { return _plantYear; }
    uint8_t  getPlantMonth()     const { return _plantMonth; }
    uint8_t  getPlantDay()       const { return _plantDay; }
    uint8_t  getDailyMixHour()   const { return _dailyMixHour; }
    uint8_t  getDailyMixMinute() const { return _dailyMixMinute; }
    void     setScheduleConfig(uint16_t year, uint8_t month, uint8_t day,
                               uint8_t mixHour, uint8_t mixMinute);

    // True jika minimal satu konfigurasi pernah diterima dari MQTT
    bool isConfigured() const { return _configured; }

private:
    void applyDefaults();
    void loadFromNVS();
    void saveAll();

    Preferences _prefs;

    // ---- PPM ----
    float _ppmTolerance     = 50.0f;
    float _initialNutrientA = 0.8f;
    float _initialNutrientB = 0.8f;

    // ---- pH ----
    float _defaultMinPH = 5.5f;
    float _defaultMaxPH = 6.5f;

    // ---- Recipe ----
    RecipeStageConfig _recipeStages[MAX_RECIPE_STAGES];
    uint8_t           _numRecipeStages = 0;

    // ---- Irrigation ----
    IrrigationStageConfig _irrigationStages[MAX_IRRIGATION_STAGES];
    uint8_t               _numIrrigationStages = 0;

    // ---- System ----
    uint16_t _totalPlants            = 80;
    float    _maxConsumptionPerPlant = 15.0f;
    float    _dailyTargetVolume      = 1.0f;
    float    _tankCapacityLiter      = 15.0f;

    // ---- Schedule ----
    uint16_t _plantYear      = 2026;
    uint8_t  _plantMonth     = 6;
    uint8_t  _plantDay       = 1;
    uint8_t  _dailyMixHour   = 5;
    uint8_t  _dailyMixMinute = 0;

    // Flag: apakah sudah pernah dikonfigurasi via MQTT
    bool _configured = false;
};

#endif
