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

    // --- Need-refill alert polling (untuk MQTTManager) ---
    // Dipicu dari handleFillWater() selama tankVolume < targetFillVolume.
    // MQTTManager poll tiap update(), publish, lalu clearNeedRefillAlert().
    bool  isNeedRefillAlertPending() const { return _needRefillAlertPending; }
    float getRefillDeficit()         const { return _refillDeficit; }
    void  clearNeedRefillAlert()           { _needRefillAlertPending = false; }
    float getFillStartVolume()       const { return _fillStartVolume; }

private:
    void changeState(
        FertigationState newState
    );

    // --- Condition helpers ---
    bool isTankSafeForMixing();
    bool isPPMInRange();
    bool isPPMAcceptableForUse();
    bool isStateTimeout(unsigned long timeout);

    // --- Actuator helpers ---
    void startMixer();
    void stopMixer();
    void startIrrigationOutput();
    void stopIrrigationOutput();

    void openWaterInlet();
    void closeWaterInlet();
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
    void stopNutrientFlowCounting();

    // --- Recipe ---
    void prepareDailyRecipe();

    // Refresh currentIrrigation threshold + minimum-liter check.
    // Stir pagi TIDAK dipicu di sini — RELAY_MIXER_STIR tidak boleh nyala
    // selama WAIT_DAILY_MIX. Dipanggil sekali per hari dari handleWaitDailyMix().
    void refreshDailyChecks();

    // Hitung minimum-liter dinamis dan blokir irigasi jika tidak cukup.
    // Return true jika volume tangki aman, false jika diblokir.
    bool checkMinimumWater();

    // Trigger stir sore berdasarkan jam dari MQTT config, dan hentikan stir
    // jika durasinya sudah tercapai. Hanya jalan di state READY.
    void handleStirSchedule();

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
    void logRecipe();
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

    // Ratchet solenoid: pompa mati dulu, solenoid tutup 75ms kemudian
    // agar cairan di selang tertahan di ketinggian dan tidak balik ke bawah
    unsigned long _solenoidACloseTime      = 0;
    bool          _waitingToCloseSolenoidA = false;
    unsigned long _solenoidBCloseTime      = 0;
    bool          _waitingToCloseSolenoidB = false;

    // Menyimpan state mana yang memulai koreksi PPM
    // (VALIDATE atau PRE_IRRIGATION_VALIDATE)
    // Digunakan oleh gotoPostCorrection() untuk kembali ke validate yang benar
    FertigationState correctionOrigin = FertigationState::VALIDATE;

    // State variables untuk deteksi level stabil pada pengisian air otomatis (FILL_WATER)
    float         lastTankVolume      = -1.0f;
    unsigned long lastLevelChangeTime = 0;
    float         _fillStartVolume    = 0.0f;   // volume awal sebelum solenoid inlet dibuka

    // --- Stirring & Daily-check state ---
    // Hari terakhir refreshDailyChecks() dijalankan (cegah re-run pada tick yang sama)
    uint16_t      _lastDailyCheckDay  = 0xFFFF;
    bool          _eveningStirDone    = false;   // stir sore sudah dijalankan hari ini
    bool          _stirring           = false;   // apakah stirrer sedang aktif
    unsigned long _stirStartMs        = 0;       // kapan stirrer dinyalakan

    // --- Safety guard state (checkMinimumWater Opsi A) ---
    bool          _tankLowBlocked     = false;   // flag irigasi diblokir karena tank < safety floor

    // --- Need-refill alert state (FILL_WATER waiting for target) ---
    float         _refillDeficit         = 0.0f;    // kekurangan liter (untuk payload MQTT alert)
    bool          _needRefillAlertPending = false;  // flag agar MQTTManager bisa poll dan publish
    unsigned long _lastRefillAlertMs      = 0;      // timestamp pengiriman alert terakhir (rate-limiter)

    // --- Timer Fallback Irrigation state ---
    // Flag: set true pada frame pertama setelah IRRIGATION selesai
    bool _irrigJustCompleted = false;

    // Tracking slot jadwal yang sedang aktif
    int8_t _activeSlotIdx = -1;       // -1 = tidak ada slot aktif
    bool   _timerSlotRunning = false; // apakah sedang dalam window irigasi timer
};

#endif
