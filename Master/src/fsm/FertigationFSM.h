#ifndef FERTIGATION_FSM_H
#define FERTIGATION_FSM_H

#include "FertigationState.h"

#include "../sensors/SensorManager.h"
#include "../actuators/RelayManager.h"
#include "../rtc/RTCManager.h"

#include "../recipe/RecipeManager.h"
#include "../recipe/IrrigationRecipe.h"

#include "../utils/RecoveryManager.h"
#include "../config/ConfigManager.h"

#include "../utils/ErrorCode.h"
#include "SoilHealthMonitor.h"

class FertigationFSM {
public:
    FertigationFSM(
        SensorManager&     sensors,
        RelayManager&      relays,
        RTCManager&        rtc,
        RecipeManager&     recipe,
        IrrigationRecipe&  irrigation,
        FlowMeter&         a,
        FlowMeter&         b,
        FlowMeter&         irrig,          // flow sensor irigasi (Timer mode)
        RecoveryManager&   recovery,
        ConfigManager&     config,
        SoilHealthMonitor& soilHealth      // monitor kesehatan sensor kelembapan
    );

    void begin();

    void update();

    FertigationState getState() const;
    ErrorCode        getError() const;

private:
    void changeState(
        FertigationState newState
    );

    // --- Condition helpers ---
    bool isTankSafeForMixing();
    bool isPPMInRange();
    bool isTodayAlreadyMixed();
    bool isStateTimeout(unsigned long timeout);

    // --- Actuator helpers ---
    void startMixer();
    void stopMixer();

    void startBuzzer();
    void stopBuzzer();
    void startFillStirrer();
    void stopFillStirrer();

    // Warning non-fatal (set currentError TANPA masuk state ERROR)
    void setWarning(ErrorCode error);
    void clearWarning();

    void preMixNutrientA();
    void stopPreMixNutrientA();
    void startNutrientA();
    void stopNutrientA();

    void preMixNutrientB();
    void stopPreMixNutrientB();
    void startNutrientB();
    void stopNutrientB();

    // --- Recipe ---
    void prepareDailyRecipe();

    // --- Mixing helpers ---
    // Cek tank aman, nyalakan mixer, set stateInitialized.
    // Return false jika gagal (masuk ERROR).
    bool beginMixing();

    // Return true saat mixTime tercapai, matikan mixer.
    bool updateMixing(uint32_t mixTime);

    // --- Transition helpers ---
    void gotoReady();
    void gotoValidate();
    void gotoCorrection();
    void gotoError(ErrorCode error);

    // Kembali ke state validate yang sesuai dengan correctionOrigin
    // (VALIDATE atau PRE_IRRIGATION_VALIDATE)
    void gotoPostCorrection();

    // --- Recovery helpers ---
    void saveRecovery();
    void restoreRecovery();
    void clearRecovery();

    // Encapsulate consuming recovery flag — set recovering=false dan log
    void consumeRecovery();

    // --- Error helpers ---
    void enterError(ErrorCode error);
    void setError(ErrorCode error);
    void clearError();
    void recoverFromError();

    // --- State handlers ---
    // KONVENSI: setiap handler:
    //   if (!stateInitialized) { ... stateInitialized = true; }
    //   beginMixing() akan set stateInitialized = true jika berhasil.
    //   Jika beginMixing() gagal, handler return tanpa set stateInitialized.
    void handleIdle();
    void handleWaitDailyMix();
    void handlePrepareDailyMix();
    void handleFillWater();
    void handlePreMixA();
    void handleAddNutrientA();
    void handleMixA();
    void handlePreMixB();
    void handleAddNutrientB();
    void handleMixB();
    void handleValidate();
    void handlePreMixCorrection();
    void handleCorrectPPM();
    void handleCorrectionMix();
    void handleReady();
    void handlePreIrrigationMix();
    void handlePreIrrigationValidate();
    void handleIrrigation();
    void handleError();

    // Timer Fallback Irrigation — dipanggil dari handleReady() saat mode TIMER
    void handleTimerIrrigation();

    // --- Log helpers ---
    void logStateTransition(FertigationState newState);
    void logStateAction(const char* message);
    void logRecipe(float remainingVolume, float ratio);
    void logError();
    void logError(const char* message);
    void logRecovery(const char* message);
    void logRecovery(const char* message, FertigationState recoveryState);

    // Konversi FertigationState enum ke nama string untuk debug
    const char* stateToString(FertigationState s);

private:
    ErrorCode currentError = ErrorCode::NONE;
    FertigationState state;

    unsigned long stateStartTime;

    bool stateInitialized;

    float targetWaterVolume;
    float targetNutrientA;
    float targetNutrientB;

    uint16_t targetPPM;
    float targetMinPH;
    float targetMaxPH;

    SensorManager& sensorManager;
    RelayManager& relayManager;
    RTCManager& rtcManager;
    RecipeManager& recipeManager;
    IrrigationRecipe& irrigationRecipe;
    RecoveryManager& recovery;
    ConfigManager& configManager;

    FlowMeter& nutrientAFlow;
    FlowMeter& nutrientBFlow;
    FlowMeter& irrigFlow;        // flow sensor irigasi untuk Timer mode

    NutrientRecipe   currentRecipe;
    IrrigationConfig currentIrrigation;

    SoilHealthMonitor& soilHealthMonitor;

    // FIX: uint16_t agar tidak overflow setelah hari ke-255
    uint16_t lastMixDay;
    uint8_t lastMixMonth;
    uint16_t lastMixYear;

    SensorData sensor;

    FertigationState lastStateBeforeError = FertigationState::IDLE;

    bool waitingRecovery = false;

    bool recovering = false;

    // Timer dan status untuk pulsing solenoid Nutrisi A & B (buka-tutup 1 detik)
    unsigned long lastPulseTime = 0;
    bool pulseOpenState = false;

    // Menyimpan state mana yang memulai koreksi PPM
    // (VALIDATE atau PRE_IRRIGATION_VALIDATE)
    // Digunakan oleh gotoPostCorrection() untuk kembali ke validate yang benar
    FertigationState correctionOrigin = FertigationState::VALIDATE;

    // State variables untuk deteksi level stabil pada pengisian air manual (FILL_WATER)
    float         lastTankVolume      = -1.0f;
    unsigned long lastLevelChangeTime = 0;

    // --- Timer Fallback Irrigation state ---
    // Flag: set true pada frame pertama setelah IRRIGATION selesai
    bool _irrigJustCompleted = false;

    // Tracking slot jadwal yang sedang aktif
    int8_t        _activeSlotIdx     = -1;   // -1 = tidak ada slot aktif
    bool          _timerSlotRunning  = false; // apakah sedang dalam sesi irigasi timer
    float         _timerTargetML     = 0.0f;  // target volume untuk slot ini (mL)
    unsigned long _lastSlotMinute    = 0xFFFF; // menit terakhir slot yang dipicu (cegah double-trigger)
};

#endif
