#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

// =========================================
// Fixed stage limits (hardcode by design)
// =========================================
constexpr uint8_t MAX_RECIPE_STAGES     = 7;
constexpr uint8_t MAX_IRRIGATION_STAGES = 7;
constexpr uint8_t MAX_IRRIG_SLOTS       = 10;  // Maks slot jadwal irigasi per hari (Timer mode)

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

// Satu slot jadwal irigasi untuk Timer Fallback mode
struct IrrigationSlot {
    uint8_t hour;    // jam (0-23)
    uint8_t minute;  // menit (0-59)
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
    float    getTargetFillVolume()       const { return _targetFillVolume; }
    float    getTankCapacityLiter()      const { return _tankCapacityLiter; }
    float    getTankHeightCM()           const { return _tankHeightCM; }
    float    getTankDiameterCM()         const { return _tankDiameterCM; }
    void     setSystemConfig(uint16_t plants, float maxConsumption,
                             float targetFillVolume, float tankCapacity,
                             float tankHeightCM, float tankDiameterCM);

    // ---- Schedule ----
    uint16_t getPlantYear()      const { return _plantYear; }
    uint8_t  getPlantMonth()     const { return _plantMonth; }
    uint8_t  getPlantDay()       const { return _plantDay; }
    uint8_t  getDailyMixHour()   const { return _dailyMixHour; }
    uint8_t  getDailyMixMinute() const { return _dailyMixMinute; }
    void     setScheduleConfig(uint16_t year, uint8_t month, uint8_t day,
                               uint8_t mixHour, uint8_t mixMinute);

    // ---- Stirring Schedule ----
    // Dikonfigurasi via MQTT topic greenhouse/config/mix_schedule_ext
    uint8_t  getStirEveningHour()    const { return _stirEveningHour; }
    uint8_t  getStirEveningMinute()  const { return _stirEveningMinute; }
    uint32_t getStirDurationMs()     const { return _stirDurationMs; }
    void     setStirSchedule(uint8_t stirEvHour, uint8_t stirEvMin, uint32_t stirDurMs);

    // True jika minimal satu konfigurasi pernah diterima dari MQTT
    bool isConfigured() const { return _configured; }

    // Hapus konfigurasi dan set ke default kosong
    void clearConfig();

    // ---- Timer Irrigation (Timer Fallback mode) ----
    float          getDailyWaterVolumeMLPerPlant() const { return _dailyWaterVolumeMLPerPlant; }
    uint8_t        getNumIrrigationSlots()         const { return _numIrrigationSlots; }
    IrrigationSlot getIrrigationSlot(uint8_t i)    const;
    void           setTimerIrrigationConfig(float mlPerPlant,
                                            const IrrigationSlot* slots, uint8_t count);

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
    float    _targetFillVolume       = 1.0f;
    float    _tankCapacityLiter      = 15.0f;
    float    _tankHeightCM           = 45.0f;
    float    _tankDiameterCM         = 20.6f;

    // ---- Schedule ----
    uint16_t _plantYear      = 2026;
    uint8_t  _plantMonth     = 6;
    uint8_t  _plantDay       = 1;
    uint8_t  _dailyMixHour   = 5;
    uint8_t  _dailyMixMinute = 0;

    // ---- Stirring Schedule ----
    // Tersimpan di NVS namespace "cfg_mix"
    uint8_t  _stirEveningHour    = 18;        // jam stir sore (default 18:00)
    uint8_t  _stirEveningMinute  = 0;         // menit stir sore
    uint32_t _stirDurationMs     = 300000UL;  // durasi stir per sesi (default 5 menit)

    // ---- Timer Irrigation ----
    float          _dailyWaterVolumeMLPerPlant = 200.0f;  // default 200 ml/tanaman/hari
    IrrigationSlot _irrigationSlots[MAX_IRRIG_SLOTS];
    uint8_t        _numIrrigationSlots = 0;

    // Flag: apakah sudah pernah dikonfigurasi via MQTT
    bool _configured = false;
};

#endif
