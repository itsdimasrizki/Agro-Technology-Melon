#include "FertigationFSM.h"
#include "../config/Constants.h"
#include "../config/SystemConfig.h"

FertigationFSM::FertigationFSM(
    SensorManager&     sensors,
    RelayManager&      relays,
    RTCManager&        rtc,
    RecipeManager&     recipe,
    IrrigationRecipe&  irrigation,
    FlowMeter&         a,
    FlowMeter&         b,
    FlowMeter&         irrig,
    RecoveryManager&   recoveryManager,
    ConfigManager&     config,
    SoilHealthMonitor& soilHealth
)
:
sensorManager(sensors),
relayManager(relays),
rtcManager(rtc),
recipeManager(recipe),
irrigationRecipe(irrigation),
recovery(recoveryManager),
configManager(config),
nutrientAFlow(a),
nutrientBFlow(b),
irrigFlow(irrig),
soilHealthMonitor(soilHealth)
{
    lastStateBeforeError = FertigationState::IDLE;

    waitingRecovery = false;

    state = FertigationState::IDLE;
    stateStartTime = 0;

    stateInitialized = false;

    targetNutrientA = 0;
    targetNutrientB = 0;

    targetPPM = 0;
    targetMinPH = 0;
    targetMaxPH = 0;
    lastMixDay = 0;
    lastMixMonth = 0;
    lastMixYear = 0;

    lastPulseTime = 0;
    pulseOpenState = false;
    correctionOrigin = FertigationState::VALIDATE;

    _irrigJustCompleted = false;
    _activeSlotIdx = -1;
    _timerSlotRunning = false;
    _timerTargetML = 0.0f;
    _lastSlotMinute = 0xFFFF;
}

//! Begin
void FertigationFSM::begin() {
    if (!rtcManager.isOk()) {
        logError("[FSM] RTC gagal — masuk ERROR state");
        enterError(ErrorCode::RTC_FAILURE);
        return;
    }

    restoreRecovery();

    if (recovering) {
        logRecovery("[FSM] Resume From Power Failure");
        return;
    }

    if (!configManager.isConfigured()) {
        logStateAction("[FSM] Belum ada konfigurasi dari web — menunggu di IDLE");
        changeState(FertigationState::IDLE);
    } else {
#if SKIP_DAILY_SCHEDULE
        changeState(FertigationState::PREPARE_DAILY_MIX);
#else
        changeState(FertigationState::WAIT_DAILY_MIX);
#endif
    }
}

//! Update
void FertigationFSM::update() {
    rtcManager.refresh();
    sensorManager.update();
    sensor = sensorManager.getData();

    if (configManager.isConfigured()) {
        uint8_t numStages = configManager.getNumRecipeStages();
        if (numStages > 0) {
            uint16_t maxAge = configManager.getRecipeStage(numStages - 1).maxAgeDays;
            uint16_t age = rtcManager.getPlantAgeDays();
            if (age > maxAge) {
                logStateAction("[FSM] Umur tanaman melebihi batas resep maksimal — reset konfigurasi");
                configManager.clearConfig();
                changeState(FertigationState::IDLE);
            }
        }
    }

    switch (state) {
        case FertigationState::IDLE:
            handleIdle();
            break;

        case FertigationState::WAIT_DAILY_MIX:
            handleWaitDailyMix();
            break;

        case FertigationState::PREPARE_DAILY_MIX:
            handlePrepareDailyMix();
            break;

        case FertigationState::FILL_WATER:
            handleFillWater();
            break;

        case FertigationState::PRE_MIX_A:
            handlePreMixA();
            break;

        case FertigationState::ADD_NUTRIENT_A:
            handleAddNutrientA();
            break;

        case FertigationState::MIX_A:
            handleMixA();
            break;

        case FertigationState::PRE_MIX_B:
            handlePreMixB();
            break;

        case FertigationState::ADD_NUTRIENT_B:
            handleAddNutrientB();
            break;

        case FertigationState::MIX_B:
            handleMixB();
            break;

        case FertigationState::VALIDATE:
            handleValidate();
            break;

        case FertigationState::PRE_MIX_CORRECTION:
            handlePreMixCorrection();
            break;

        case FertigationState::CORRECT_PPM:
            handleCorrectPPM();
            break;

        case FertigationState::CORRECTION_MIX:
            handleCorrectionMix();
            break;

        case FertigationState::READY:
            handleReady();
            break;

        case FertigationState::PRE_IRRIGATION_MIX:
            handlePreIrrigationMix();
            break;

        case FertigationState::PRE_IRRIGATION_VALIDATE:
            handlePreIrrigationValidate();
            break;

        case FertigationState::IRRIGATION:
            handleIrrigation();
            break;

        case FertigationState::ERROR:
            handleError();
            break;

        default:
            logError("[FSM] Unknown state — fallback to IDLE");
            changeState(FertigationState::IDLE);
            break;
    }
}

//! ChangeState
void FertigationFSM::changeState(FertigationState newState) {
    state = newState;
    stateStartTime = millis();
    stateInitialized = false;
    saveRecovery();
    logStateTransition(newState);
}

//! GetState
FertigationState
FertigationFSM::getState() const {
    return state;
}

//! GetError
ErrorCode
FertigationFSM::getError() const {
    return currentError;
}

//! CONDITION HELPERS
bool FertigationFSM::isTankSafeForMixing() {
    return sensor.tankVolume >= 0.0f;
}

bool FertigationFSM::isPPMInRange() {
    return abs(sensor.ppm - targetPPM) <= configManager.getPPMTolerance();
}

bool FertigationFSM::isStateTimeout(unsigned long timeout) {
    return millis() - stateStartTime > timeout;
}

//! ACTUATOR HELPERS
void FertigationFSM::startMixer() {
    relayManager.on(RELAY_PUMP_MIX);
}

void FertigationFSM::stopMixer() {
    relayManager.off(RELAY_PUMP_MIX);
}

// Solenoid valve pengisian air (RELAY_SOLENOID_WATER, ch5).
// Hardware: tipe NC (Normally Closed) — tanpa daya = valve tertutup = air berhenti.
// Fail-safe: jika ESP32 mati/hang, air TIDAK akan terus mengalir.
// Buzzer di-wire paralel ke output relay yang sama — ON = solenoid terbuka + buzzer bunyi.
void FertigationFSM::startWaterFillSolenoid() {
    _solenoidWaterOpen = true;
    relayManager.on(RELAY_SOLENOID_WATER);
}

void FertigationFSM::stopWaterFillSolenoid() {
    _solenoidWaterOpen = false;
    relayManager.off(RELAY_SOLENOID_WATER);
}

void FertigationFSM::startFillStirrer() {
    relayManager.on(RELAY_MIXER_STIR);
}

void FertigationFSM::stopFillStirrer() {
    relayManager.off(RELAY_MIXER_STIR);
}

void FertigationFSM::setWarning(ErrorCode error) {
    currentError = error;
}

void FertigationFSM::clearWarning() {
    currentError = ErrorCode::NONE;
}

void FertigationFSM::preMixNutrientA() {
    relayManager.on(RELAY_PUMP_A);
}

void FertigationFSM::stopPreMixNutrientA() {
    relayManager.off(RELAY_PUMP_A);
}

void FertigationFSM::startNutrientA() {
    relayManager.on(RELAY_PUMP_A);
    relayManager.on(RELAY_SOLENOID_A);
}

void FertigationFSM::stopNutrientA() {
    relayManager.off(RELAY_PUMP_A);
    relayManager.off(RELAY_SOLENOID_A);
}

void FertigationFSM::preMixNutrientB() {
    relayManager.on(RELAY_PUMP_B);
}

void FertigationFSM::stopPreMixNutrientB() {
    relayManager.off(RELAY_PUMP_B);
}

void FertigationFSM::startNutrientB() {
    relayManager.on(RELAY_PUMP_B);
    relayManager.on(RELAY_SOLENOID_B);
}

void FertigationFSM::stopNutrientB() {
    relayManager.off(RELAY_PUMP_B);
    relayManager.off(RELAY_SOLENOID_B);
}

//! RECIPE
void FertigationFSM::prepareDailyRecipe() {
    uint16_t age = rtcManager.getPlantAgeDays();

    // Ambil resep hari ini langsung — tidak ada lagi grouping N-hari.
    // Fungsi ini dipanggil SETELAH tangki penuh (di akhir FILL_WATER),
    // sehingga dosis selalu full dose tanpa scaling rasio.
    NutrientRecipe r = recipeManager.getRecipe(age);
    targetPPM    = (uint16_t)r.targetPPM;
    targetMinPH  = r.targetMinPH;
    targetMaxPH  = r.targetMaxPH;

    targetNutrientA = configManager.getInitialNutrientA();
    targetNutrientB = configManager.getInitialNutrientB();

    logRecipe();
}

// =========================================
// refreshDailyChecks()
// Dipanggil dari handleWaitDailyMix() setiap tick.
// Guard internal: hanya eksekusi sekali per hari (berdasarkan plant age).
// =========================================
void FertigationFSM::refreshDailyChecks() {
    uint16_t age = rtcManager.getPlantAgeDays();
    if (age == _lastDailyCheckDay) return;
    _lastDailyCheckDay = age;

    // 1. Refresh threshold irigasi harian (independen dari siklus mixing)
    currentIrrigation = irrigationRecipe.getRecipe(age);
    Serial.printf("[FSM] Daily refresh: dry=%u, wet=%u\n",
                  currentIrrigation.dryThreshold, currentIrrigation.wetThreshold);

    // 2. Reset flag stir harian
    _morningStirDone = false;
    _eveningStirDone = false;

    // 3. Cek minimum-liter dan trigger stir pagi jika lolos
    if (checkMinimumWater()) {
        if (!_morningStirDone && state == FertigationState::WAIT_DAILY_MIX) {
            startFillStirrer();
            _stirring        = true;
            _stirStartMs     = millis();
            _morningStirDone = true;
            logStateAction("[FSM] Stir pagi dimulai");
        }
    }
}

// =========================================
// checkMinimumWater() — Opsi A: safety floor sederhana
// Blokir irigasi jika tangki mendekati kosong untuk mencegah dry-run pompa.
// Tidak terkait target fill harian — murni proteksi hardware.
// Return true jika aman, false jika diblokir.
// =========================================
bool FertigationFSM::checkMinimumWater() {
    if (sensor.tankVolume < TANK_SAFETY_FLOOR_LITER) {
        if (!_tankLowBlocked) {
            _tankLowBlocked = true;
            setWarning(ErrorCode::WAITING_FOR_FILL);
            Serial.printf("[FSM] Tank di bawah safety floor (%.1fL < %.1fL) — irigasi diblokir\n",
                          sensor.tankVolume, TANK_SAFETY_FLOOR_LITER);
        }
        return false;
    }

    if (_tankLowBlocked) {
        _tankLowBlocked = false;
        clearWarning();
        logStateAction("[FSM] Tank volume kembali aman — irigasi diizinkan");
    }
    return true;
}

// =========================================
// handleStirSchedule()
// Hentikan stir jika durasi tercapai.
// Trigger stir sore jika jam/menit sesuai konfigurasi.
// Hanya jalan saat state READY.
// =========================================
void FertigationFSM::handleStirSchedule() {
    // Hentikan stir yang sedang berjalan jika durasinya sudah habis
    if (_stirring) {
        if (millis() - _stirStartMs >= configManager.getStirDurationMs()) {
            stopFillStirrer();
            _stirring = false;
            logStateAction("[FSM] Stir selesai");
        }
        return;  // tidak trigger stir baru selagi masih stir
    }

    // Trigger stir sore
    uint8_t h = rtcManager.getHour();
    uint8_t m = rtcManager.getMinute();
    if (!_eveningStirDone &&
        h == configManager.getStirEveningHour() &&
        m == configManager.getStirEveningMinute()) {
        startFillStirrer();
        _stirring        = true;
        _stirStartMs     = millis();
        _eveningStirDone = true;
        logStateAction("[FSM] Stir sore dimulai");
    }
}

//! MIXING HELPERS
bool FertigationFSM::beginMixing() {
    if (!isTankSafeForMixing()) {
        logError("[FSM] Mixer Dry Run");
        gotoError(ErrorCode::MIXER_DRY_RUN);
        return false;
    }

    startMixer();
    stateInitialized = true;
    return true;
}

bool FertigationFSM::updateMixing(uint32_t mixTime) {
    if (millis() - stateStartTime >= mixTime) {
        stopMixer();
        return true;
    }
    return false;
}

//! TRANSITION HELPERS
void FertigationFSM::gotoReady() {
    changeState(FertigationState::READY);
}

void FertigationFSM::gotoValidate() {
    changeState(FertigationState::VALIDATE);
}

void FertigationFSM::gotoCorrection() {
    correctionOrigin = state;
    changeState(FertigationState::PRE_MIX_CORRECTION);
}

void FertigationFSM::gotoError(ErrorCode error) {
    enterError(error);
}

void FertigationFSM::gotoPostCorrection() {
    if (correctionOrigin == FertigationState::PRE_IRRIGATION_VALIDATE) {
        changeState(FertigationState::PRE_IRRIGATION_VALIDATE);
    } else {
        gotoValidate();
    }
}

//! RECOVERY HELPERS
void FertigationFSM::consumeRecovery() {
    if (recovering) {
        logRecovery("[FSM] Resuming from recovery");
        recovering = false;
    }
}

void FertigationFSM::saveRecovery() {
    RecoveryData data;

    data.state = static_cast<uint16_t>(state);
    data.waterPulse = 0; // Removed flowWater
    data.nutrientAPulse = nutrientAFlow.pulseCount;
    data.nutrientBPulse = nutrientBFlow.pulseCount;
    data.day = lastMixDay;
    data.month = lastMixMonth;
    data.year = lastMixYear;
    data.lastStateBeforeError = static_cast<uint16_t>(lastStateBeforeError);
    data.batchRunning = true;

    recovery.save(data);
}

void FertigationFSM::restoreRecovery() {
    RecoveryData data = recovery.load();

    if (!data.batchRunning)
        return;

    const uint16_t maxValidState = static_cast<uint16_t>(FertigationState::ERROR);

    if (data.state > maxValidState) {
        logError("[FSM] Recovery data corrupt — skip restore");
        return;
    }

    if (data.state == static_cast<uint16_t>(FertigationState::ERROR)) {
        logRecovery("[FSM] Stored state is ERROR — restore lastStateBeforeError");

        if (data.lastStateBeforeError <= maxValidState) {
            state = static_cast<FertigationState>(data.lastStateBeforeError);
        } else {
            state = FertigationState::WAIT_DAILY_MIX;
        }
    } else {
        state = static_cast<FertigationState>(data.state);
    }

    lastMixDay   = data.day;
    lastMixMonth = data.month;
    lastMixYear  = data.year;

    lastStateBeforeError = static_cast<FertigationState>(
        data.lastStateBeforeError <= maxValidState
            ? data.lastStateBeforeError
            : static_cast<uint16_t>(FertigationState::IDLE)
    );

    nutrientAFlow.setPulseCount(data.nutrientAPulse);
    nutrientBFlow.setPulseCount(data.nutrientBPulse);

    recovering = true;
    stateInitialized = false;

    logRecovery(
        "[FSM] Recovery State : ",
        state
    );
}

void FertigationFSM::clearRecovery() {
    recovery.clear();
}

//! ERROR HELPERS
void FertigationFSM::enterError(ErrorCode error) {
    relayManager.allOff();
    setError(error);
    lastStateBeforeError = state;
    changeState(FertigationState::ERROR);
}

void FertigationFSM::setError(ErrorCode error) {
    currentError = error;
    waitingRecovery = true;
}

void FertigationFSM::clearError() {
    waitingRecovery = false;
    recovering = true;
}

void FertigationFSM::recoverFromError() {
    relayManager.allOff();
    stateInitialized = false;
    clearError();
    logRecovery(
        "[FSM] Recover to state : ",
        lastStateBeforeError
    );
    changeState(lastStateBeforeError);
}

//! STATE HANDLERS
void FertigationFSM::handleIdle() {
    relayManager.allOff();

    if (configManager.isConfigured()) {
        logStateAction("[FSM] Konfigurasi terdeteksi — memulai penjadwalan fertigasi");
#if SKIP_DAILY_SCHEDULE
        changeState(FertigationState::PREPARE_DAILY_MIX);
#else
        changeState(FertigationState::WAIT_DAILY_MIX);
#endif
    }
}

void FertigationFSM::handleWaitDailyMix() {
    uint8_t hour   = rtcManager.getHour();
    uint8_t minute = rtcManager.getMinute();
    uint16_t day   = rtcManager.getPlantAgeDays();
    uint8_t month  = rtcManager.getMonth();
    uint16_t year  = rtcManager.getYear();

    // Refresh harian: update threshold irigasi, cek minimum-liter, trigger stir pagi.
    // refreshDailyChecks() di-guard internal agar hanya jalan sekali per hari.
    refreshDailyChecks();

    // Trigger mixing tiap pagi di jam yang dikonfigurasi — guard per hari agar tidak retrigger
    // dalam menit yang sama (cek lastMixDay vs plant age hari ini)
    if (hour == rtcManager.getDailyMixHour() && minute == rtcManager.getDailyMixMinute()
        && day != lastMixDay) {
        lastMixDay   = day;
        lastMixMonth = month;
        lastMixYear  = year;
        changeState(FertigationState::PREPARE_DAILY_MIX);
    }
}

void FertigationFSM::handlePrepareDailyMix() {
    // Reset flow meter dan tunggu pengisian air manual di FILL_WATER.
    // prepareDailyRecipe() dipanggil setelah tangki penuh (di akhir handleFillWater()).
    nutrientAFlow.reset();
    nutrientBFlow.reset();
    clearRecovery();
    changeState(FertigationState::FILL_WATER);
}

void FertigationFSM::handleFillWater() {
    // --- Timeout check (harus pertama, sebelum inisialisasi) ---
    if (isStateTimeout(WATER_FILL_TIMEOUT)) {
        stopWaterFillSolenoid();
        stopFillStirrer();
        logError("[FSM] FILL_WATER timeout — solenoid ditutup, masuk ERROR");
        gotoError(ErrorCode::WATER_TIMEOUT);
        return;
    }

    if (!stateInitialized) {
        consumeRecovery();

        float tankVolume = sensor.tankVolume;
        float targetFillVolume = configManager.getTargetFillVolume();

        lastTankVolume      = tankVolume;
        lastLevelChangeTime = millis();
        _lastRefillAlertMs  = 0;  // paksa kirim alert segera
        _solenoidWaterOpen  = false;

        startFillStirrer();

        // Jika tangki sudah memenuhi target, skip fill langsung ke PRE_MIX_A
        if (tankVolume >= targetFillVolume) {
            logStateAction("[FSM] Tangki sudah cukup — skip auto-fill, langsung ke PRE_MIX_A");
            stopFillStirrer();
            prepareDailyRecipe();
            changeState(FertigationState::PRE_MIX_A);
            return;
        }

        // Buka solenoid air untuk mulai pengisian otomatis
        startWaterFillSolenoid();
        stateInitialized = true;
        Serial.printf("[FSM] Auto-fill dimulai: vol=%.1fL, target=%.1fL, butuh=%.1fL\n",
                      tankVolume, targetFillVolume, targetFillVolume - tankVolume);
    }

    float tankVolume       = sensor.tankVolume;
    float targetFillVolume = configManager.getTargetFillVolume();

    // Log progress pengisian setiap tick (pakai rate-limiter alert untuk hindari spam Serial)
    static unsigned long _lastFillLogMs = 0;
    if (millis() - _lastFillLogMs >= 5000) {
        _lastFillLogMs = millis();
        Serial.printf("[FSM] Auto-fill progress: vol=%.2fL / %.1fL (margin=%.2fL)\n",
                      tankVolume, targetFillVolume, FILL_STOP_MARGIN_LITER);
    }

    // Deteksi perubahan level yang nyata (di atas threshold noise ultrasonik)
    if (fabs(tankVolume - lastTankVolume) > WATER_LEVEL_NOISE_THRESHOLD) {
        lastTankVolume      = tankVolume;
        lastLevelChangeTime = millis();
    }

    // Overflow check: warning non-fatal — FSM tetap jalan, TIDAK masuk ERROR
    // Solenoid ditutup segera saat overflow terdeteksi agar tidak memperburuk kondisi
    if (tankVolume > configManager.getTankCapacityLiter()) {
        if (_solenoidWaterOpen) {
            stopWaterFillSolenoid();
            logStateAction("[FSM] OVERFLOW terdeteksi — solenoid ditutup darurat");
        }
        setWarning(ErrorCode::WATER_OVERFLOW);
    } else if (currentError == ErrorCode::WATER_OVERFLOW) {
        clearWarning();
    }

    // Tutup solenoid lebih awal saat volume mendekati target (kurang FILL_STOP_MARGIN_LITER)
    // Kompensasi trailing flow setelah valve menutup (solenoid NC, air berhenti saat OFF)
    if (_solenoidWaterOpen && tankVolume >= targetFillVolume - FILL_STOP_MARGIN_LITER) {
        stopWaterFillSolenoid();
        lastLevelChangeTime = millis();  // reset timer stabil dari sini
        Serial.printf("[FSM] Solenoid ditutup (vol=%.2fL >= target-margin=%.2fL) — menunggu level stabil\n",
                      tankVolume, targetFillVolume - FILL_STOP_MARGIN_LITER);
    }

    // Setelah solenoid tertutup, tunggu level benar-benar stabil sebelum lanjut
    if (!_solenoidWaterOpen) {
        if (millis() - lastLevelChangeTime >= WATER_LEVEL_STABLE_TIMEOUT) {
            stopFillStirrer();
            // Hitung dosis nutrisi SETELAH tangki penuh — full dose, tanpa scaling rasio
            prepareDailyRecipe();
            logStateAction("[FSM] Level stabil — pengisian selesai, lanjut ke PRE_MIX_A");
            changeState(FertigationState::PRE_MIX_A);
            return;
        }
    }

    // Kirim alert progress ke MQTT secara periodik selama solenoid masih terbuka
    if (_solenoidWaterOpen) {
        if (millis() - _lastRefillAlertMs >= NEED_REFILL_ALERT_INTERVAL_MS) {
            _lastRefillAlertMs      = millis();
            _refillDeficit          = targetFillVolume - tankVolume;
            _needRefillAlertPending = true;
            Serial.printf("[FSM] Auto-fill berlangsung: vol=%.1fL, target=%.1fL, kurang=%.1fL\n",
                          tankVolume, targetFillVolume, _refillDeficit);
        }
    }
}

void FertigationFSM::handlePreMixA() {
    if (!stateInitialized) {
        consumeRecovery();
        preMixNutrientA();
        stateInitialized = true;
        logStateAction("[FSM] Pre-Mix Nutrient A Tank");
    }

    if (isStateTimeout(PRE_MIX_TANK_TIME)) {
        changeState(FertigationState::ADD_NUTRIENT_A);
    }
}

void FertigationFSM::handleAddNutrientA() {
    if (isStateTimeout(NUTRIENT_TIMEOUT)) {
        gotoError(ErrorCode::NUTRIENT_A_TIMEOUT);
        return;
    }

    if (!stateInitialized) {
        consumeRecovery();
        lastPulseTime = millis();
        pulseOpenState = true;
        relayManager.on(RELAY_PUMP_A);
        relayManager.on(RELAY_SOLENOID_A);
        stateInitialized = true;
        logStateAction("[FSM] Dosing Nutrient A (Pulsed)");
    }

    // Solenoid A / Pompa A pulsing (buka 1s, tutup 1s)
    if (millis() - lastPulseTime >= 1000) {
        pulseOpenState = !pulseOpenState;
        lastPulseTime = millis();
        if (pulseOpenState) {
            relayManager.on(RELAY_PUMP_A);
            relayManager.on(RELAY_SOLENOID_A);
            logStateAction("[FSM] Nutrient A Solenoid OPEN");
        } else {
            relayManager.off(RELAY_PUMP_A);
            relayManager.off(RELAY_SOLENOID_A);
            logStateAction("[FSM] Nutrient A Solenoid CLOSED");
        }
    }

    if (sensor.flowA >= targetNutrientA) {
        relayManager.off(RELAY_PUMP_A);
        relayManager.off(RELAY_SOLENOID_A);
        changeState(FertigationState::MIX_A);
    }
}

void FertigationFSM::handleMixA() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }
        logStateAction("[FSM] Mixing Larutan A (Main Chamber)");
    }

    if (updateMixing(MIX_A_TIME)) {
        changeState(FertigationState::PRE_MIX_B);
    }
}

void FertigationFSM::handlePreMixB() {
    if (!stateInitialized) {
        consumeRecovery();
        preMixNutrientB();
        stateInitialized = true;
        logStateAction("[FSM] Pre-Mix Nutrient B Tank");
    }

    if (isStateTimeout(PRE_MIX_TANK_TIME)) {
        changeState(FertigationState::ADD_NUTRIENT_B);
    }
}

void FertigationFSM::handleAddNutrientB() {
    if (isStateTimeout(NUTRIENT_TIMEOUT)) {
        gotoError(ErrorCode::NUTRIENT_B_TIMEOUT);
        return;
    }

    if (!stateInitialized) {
        consumeRecovery();
        lastPulseTime = millis();
        pulseOpenState = true;
        relayManager.on(RELAY_PUMP_B);
        relayManager.on(RELAY_SOLENOID_B);
        stateInitialized = true;
        logStateAction("[FSM] Dosing Nutrient B (Pulsed)");
    }

    // Solenoid B / Pompa B pulsing (buka 1s, tutup 1s)
    if (millis() - lastPulseTime >= 1000) {
        pulseOpenState = !pulseOpenState;
        lastPulseTime = millis();
        if (pulseOpenState) {
            relayManager.on(RELAY_PUMP_B);
            relayManager.on(RELAY_SOLENOID_B);
            logStateAction("[FSM] Nutrient B Solenoid OPEN");
        } else {
            relayManager.off(RELAY_PUMP_B);
            relayManager.off(RELAY_SOLENOID_B);
            logStateAction("[FSM] Nutrient B Solenoid CLOSED");
        }
    }

    if (sensor.flowB >= targetNutrientB) {
        relayManager.off(RELAY_PUMP_B);
        relayManager.off(RELAY_SOLENOID_B);
        changeState(FertigationState::MIX_B);
    }
}

void FertigationFSM::handleMixB() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }
        logStateAction("[FSM] Mixing Larutan B (Main Chamber)");
    }

    if (updateMixing(MIX_B_TIME)) {
        gotoValidate();
    }
}

void FertigationFSM::handleValidate() {
    if (isPPMInRange()) {
        gotoReady();
    } else {
        gotoCorrection();
    }
}

void FertigationFSM::handlePreMixCorrection() {
    if (!stateInitialized) {
        consumeRecovery();
        
        preMixNutrientA();
        preMixNutrientB();

        stateInitialized = true;
        logStateAction("[FSM] Pre-Mix Nutrisi A & B sebelum Koreksi PPM");
    }

    if (isStateTimeout(PRE_MIX_CORRECTION_TIME)) {
        changeState(FertigationState::CORRECT_PPM);
    }
}

void FertigationFSM::handleCorrectPPM() {
    if (isPPMInRange()) {
        gotoReady();
        return;
    }

    if (!stateInitialized) {
        consumeRecovery();

        if (!recovering) {
            nutrientAFlow.reset();
            nutrientBFlow.reset();
        }

        lastPulseTime = millis();
        pulseOpenState = true;

        if (sensor.flowA < CORRECTION_DOSE) {
            relayManager.on(RELAY_PUMP_A);
            relayManager.on(RELAY_SOLENOID_A);
        }
        if (sensor.flowB < CORRECTION_DOSE) {
            relayManager.on(RELAY_PUMP_B);
            relayManager.on(RELAY_SOLENOID_B);
        }

        stateInitialized = true;
        logStateAction("[FSM] Injeksi Dosis Koreksi PPM (Pulsed)");
    }

    // Pulsing solenoid Nutrisi A & B bersamaan (buka 1s, tutup 1s)
    if (millis() - lastPulseTime >= 1000) {
        pulseOpenState = !pulseOpenState;
        lastPulseTime = millis();

        if (pulseOpenState) {
            if (sensor.flowA < CORRECTION_DOSE) {
                relayManager.on(RELAY_PUMP_A);
                relayManager.on(RELAY_SOLENOID_A);
                logStateAction("[FSM] Nutrient A Solenoid OPEN (Correction)");
            }
            if (sensor.flowB < CORRECTION_DOSE) {
                relayManager.on(RELAY_PUMP_B);
                relayManager.on(RELAY_SOLENOID_B);
                logStateAction("[FSM] Nutrient B Solenoid OPEN (Correction)");
            }
        } else {
            relayManager.off(RELAY_PUMP_A);
            relayManager.off(RELAY_SOLENOID_A);
            relayManager.off(RELAY_PUMP_B);
            relayManager.off(RELAY_SOLENOID_B);
            logStateAction("[FSM] Nutrient A & B Solenoids CLOSED (Correction)");
        }
    }

    // Matikan segera jika sudah melampaui target
    if (sensor.flowA >= CORRECTION_DOSE) {
        relayManager.off(RELAY_PUMP_A);
        relayManager.off(RELAY_SOLENOID_A);
    }
    if (sensor.flowB >= CORRECTION_DOSE) {
        relayManager.off(RELAY_PUMP_B);
        relayManager.off(RELAY_SOLENOID_B);
    }

    if (sensor.flowA >= CORRECTION_DOSE && sensor.flowB >= CORRECTION_DOSE) {
        relayManager.off(RELAY_PUMP_A);
        relayManager.off(RELAY_SOLENOID_A);
        relayManager.off(RELAY_PUMP_B);
        relayManager.off(RELAY_SOLENOID_B);
        changeState(FertigationState::CORRECTION_MIX);
    }
}

void FertigationFSM::handleCorrectionMix() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }
        logStateAction("[FSM] Mixing Dosis Koreksi (Main Chamber)");
    }

    if (updateMixing(CORRECTION_MIX_TIME)) {
        gotoPostCorrection();
    }
}

void FertigationFSM::handleReady() {
    // Update monitor status
    soilHealthMonitor.update(sensor.soilADC, _irrigJustCompleted, currentIrrigation.wetThreshold);
    _irrigJustCompleted = false;

    // Jalankan stir schedule (pagi sudah dipicu oleh refreshDailyChecks, ini untuk sore + stop timer)
    handleStirSchedule();

    // Cek minimum-liter dinamis — blokir irigasi jika tangki tidak cukup.
    // Stir schedule tetap jalan meski irigasi diblokir.
    if (!checkMinimumWater()) {
        return;
    }

    // Check mode
    if (soilHealthMonitor.getMode() == IrrigationMode::TIMER) {
        handleTimerIrrigation();
        return;
    }

    // Normal HUMIDITY mode
    if (sensor.soilADC >= currentIrrigation.dryThreshold) {
        changeState(FertigationState::PRE_IRRIGATION_MIX);
    }
}

void FertigationFSM::handlePreIrrigationMix() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }
    }

    if (updateMixing(PRE_IRRIGATION_MIX_TIME)) {
        changeState(FertigationState::PRE_IRRIGATION_VALIDATE);
    }
}

void FertigationFSM::handlePreIrrigationValidate() {
    bool ppmOK = isPPMInRange();
    bool phOK  = sensor.ph >= targetMinPH && sensor.ph <= targetMaxPH;

    if (ppmOK && phOK) {
        changeState(FertigationState::IRRIGATION);
    } else if (sensor.ppm < targetPPM) {
        gotoCorrection();
    } else {
        gotoError(ErrorCode::PH_OUT_OF_RANGE);
    }
}

void FertigationFSM::handleIrrigation() {
    if (!stateInitialized) {
        // Nyalakan Pompa Mixing & Solenoid Irigasi
        relayManager.on(RELAY_PUMP_MIX);
        relayManager.on(RELAY_SOLENOID_IRRIG);
        
        stateInitialized = true;
        logStateAction("[FSM] Start Irrigation");
    }

    if (sensor.soilADC <= currentIrrigation.wetThreshold) {
        relayManager.off(RELAY_PUMP_MIX);
        relayManager.off(RELAY_SOLENOID_IRRIG);
        _irrigJustCompleted = true; // Set flag for SoilHealthMonitor
        gotoReady();
    }
}

void FertigationFSM::handleTimerIrrigation() {
    uint8_t rtcHour = rtcManager.getHour();
    uint8_t rtcMinute = rtcManager.getMinute();

    if (!_timerSlotRunning) {
        uint8_t numSlots = configManager.getNumIrrigationSlots();
        if (numSlots == 0) return;

        // Cegah pemicuan berulang pada menit yang sama
        if (rtcMinute == _lastSlotMinute) {
            return;
        }

        int8_t triggeredSlot = -1;
        for (uint8_t i = 0; i < numSlots; i++) {
            IrrigationSlot slot = configManager.getIrrigationSlot(i);
            if (slot.hour == rtcHour && slot.minute == rtcMinute) {
                triggeredSlot = i;
                break;
            }
        }

        if (triggeredSlot != -1) {
            // Hitung target volume untuk slot ini dalam mL
            float dailyVolML = configManager.getDailyWaterVolumeMLPerPlant() * configManager.getTotalPlants();
            _timerTargetML = dailyVolML / numSlots;

            // Reset flow meter irigasi
            irrigFlow.reset();

            // Jalankan solenoid + pompa irigasi
            relayManager.on(RELAY_PUMP_MIX);
            relayManager.on(RELAY_SOLENOID_IRRIG);

            _timerSlotRunning = true;
            _activeSlotIdx = triggeredSlot;
            _lastSlotMinute = rtcMinute;

            Serial.printf("[FSM] Timer Fallback Irrigation Triggered: Slot %d at %02d:%02d. Target: %.1f mL (%.3f L)\n",
                          _activeSlotIdx, rtcHour, rtcMinute, _timerTargetML, _timerTargetML / 1000.0f);
        }
    } else {
        // Sedang irigasi timer, monitor flow sensor
        float currentVolLiters = irrigFlow.getVolumeLiter();
        float targetLiters = _timerTargetML / 1000.0f;

        // Log progress setiap 5 detik
        static unsigned long lastLog = 0;
        if (millis() - lastLog >= 5000) {
            lastLog = millis();
            Serial.printf("[FSM] Timer Irrig Slot %d Progress: %.3f L / %.3f L\n", _activeSlotIdx, currentVolLiters, targetLiters);
        }

        if (currentVolLiters >= targetLiters) {
            // Tutup solenoid + matikan pompa
            relayManager.off(RELAY_PUMP_MIX);
            relayManager.off(RELAY_SOLENOID_IRRIG);

            Serial.printf("[FSM] Timer Fallback Irrigation Completed: Slot %d. Total: %.3f L\n", _activeSlotIdx, currentVolLiters);

            _timerSlotRunning = false;
            _activeSlotIdx = -1;
            _irrigJustCompleted = true; // Pemicu evaluation pasca-watering pada SoilHealthMonitor
        }
    }
}

void FertigationFSM::handleError() {
    if (!stateInitialized) {
        relayManager.allOff();
        logError();
        stateInitialized = true;
    }

    if (waitingRecovery == false) {
        recoverFromError();
    }
}

//! LOG HELPERS
const char* FertigationFSM::stateToString(FertigationState s) {
    switch (s) {
        case FertigationState::IDLE:                    return "IDLE";
        case FertigationState::WAIT_DAILY_MIX:          return "WAIT_DAILY_MIX";
        case FertigationState::PREPARE_DAILY_MIX:       return "PREPARE_DAILY_MIX";
        case FertigationState::FILL_WATER:              return "FILL_WATER";
        case FertigationState::PRE_MIX_A:               return "PRE_MIX_A";
        case FertigationState::ADD_NUTRIENT_A:          return "ADD_NUTRIENT_A";
        case FertigationState::MIX_A:                   return "MIX_A";
        case FertigationState::PRE_MIX_B:               return "PRE_MIX_B";
        case FertigationState::ADD_NUTRIENT_B:          return "ADD_NUTRIENT_B";
        case FertigationState::MIX_B:                   return "MIX_B";
        case FertigationState::VALIDATE:                return "VALIDATE";
        case FertigationState::PRE_MIX_CORRECTION:      return "PRE_MIX_CORRECTION";
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

void FertigationFSM::logStateTransition(FertigationState newState) {
    Serial.print("[FSM] -> ");
    Serial.println(stateToString(newState));
}

void FertigationFSM::logStateAction(const char* message) {
    Serial.println(message);
}

void FertigationFSM::logRecipe() {
    Serial.println();
    Serial.println("===== DAILY RECIPE =====");
    Serial.print("Target PPM : ");
    Serial.println(targetPPM);
    Serial.print("pH Range   : ");
    Serial.print(targetMinPH);
    Serial.print(" - ");
    Serial.println(targetMaxPH);
    Serial.print("Nutrient A : ");
    Serial.println(targetNutrientA);
    Serial.print("Nutrient B : ");
    Serial.println(targetNutrientB);
    Serial.println("========================");
}

void FertigationFSM::logError() {
    Serial.print("[FSM] ERROR : ");
    Serial.println(static_cast<int>(currentError));
}

void FertigationFSM::logError(const char* message) {
    Serial.println(message);
}

void FertigationFSM::logRecovery(const char* message) {
    Serial.println(message);
}

void FertigationFSM::logRecovery(
    const char* message,
    FertigationState recoveryState
) {
    Serial.print(message);
    Serial.println(stateToString(recoveryState));
}

// ============================================================
// STATE DIAGRAM
//
// Startup (normal)
//      |
//      v
// WAIT_DAILY_MIX
//      | jam DAILY_MIX_HOUR:DAILY_MIX_MINUTE && plantAgeDays != lastMixDay
//      v
// PREPARE_DAILY_MIX
//      |
//      v
// FILL_WATER
//      | tankVolume < targetFillVolume: buka RELAY_SOLENOID_WATER (solenoid NC + buzzer parallel)
//      |   → solenoid tutup saat vol >= targetFillVolume - FILL_STOP_MARGIN_LITER
//      |   → tunggu level stabil (tidak berubah >= WATER_LEVEL_STABLE_TIMEOUT)
//      |   → stirrer OFF, lanjut
//      | tankVolume >= targetFillVolume: langsung skip ke PRE_MIX_A (tanpa buka solenoid)
//      v
// PRE_MIX_A
//      |
//      v
// ADD_NUTRIENT_A
//      | flowA >= targetNutrientA
//      v
// MIX_A (MIX_A_TIME)
//      |
//      v
// PRE_MIX_B
//      |
//      v
// ADD_NUTRIENT_B
//      | flowB >= targetNutrientB
//      v
// MIX_B (MIX_B_TIME)
//      |
//      v
// VALIDATE
//      | ppm OK
//      v
// READY
//
// VALIDATE
//      | ppm low
//      v
// CORRECT_PPM
//      | dosis tercapai
//      v
// PRE_MIX_CORRECTION
//      |
//      v
// CORRECTION_MIX (CORRECTION_MIX_TIME)
//      |
//      v-- gotoPostCorrection() --
//      | correctionOrigin = VALIDATE → VALIDATE
//      | correctionOrigin = PRE_IRRIGATION_VALIDATE → PRE_IRRIGATION_VALIDATE
//
// READY
//      | soilADC >= dryThreshold
//      v
// PRE_IRRIGATION_MIX (PRE_IRRIGATION_MIX_TIME)
//      |
//      v
// PRE_IRRIGATION_VALIDATE
//      | ppm+pH OK
//      v
// IRRIGATION
//      | soilADC <= wetThreshold
//      v
// READY
//
// Any timeout / overdose / mixer dry-run / pH out of range / correction failed
//      |
//      v
// ERROR
//      | auto-recover (recoverFromError)
//      v
// lastStateBeforeError
// ============================================================