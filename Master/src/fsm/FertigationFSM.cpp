#include "FertigationFSM.h"
#include "../config/Constants.h"
#include "../config/SystemConfig.h"

FertigationFSM::FertigationFSM(
    SensorManager& sensors,
    RelayManager& relays,
    RTCManager& rtc,
    RecipeManager& recipe,
    IrrigationRecipe& irrigation,
    FlowMeter& water,
    FlowMeter& a,
    FlowMeter& b,
    RecoveryManager& recoveryManager
)
:
sensorManager(sensors),
relayManager(relays),
rtcManager(rtc),
recipeManager(recipe),
irrigationRecipe(irrigation),
recovery(recoveryManager),
waterFlow(water),
nutrientAFlow(a),
nutrientBFlow(b)
{
    lastStateBeforeError = FertigationState::IDLE;

    waitingRecovery = false;

    state = FertigationState::IDLE;
    stateStartTime = 0;

    stateInitialized = false;

    targetWaterVolume = 0;
    targetNutrientA = 0;
    targetNutrientB = 0;

    targetPPM = 0;
    targetMinPH = 0;
    targetMaxPH = 0;
    lastMixDay = 0;
    lastMixMonth = 0;
    lastMixYear = 0;

    correctionCount = 0;
    correctionOrigin = FertigationState::VALIDATE;
}

// Begin
void FertigationFSM::begin() {
    // FIX: cek RTC sebelum apapun — tidak lagi pakai while(true) di RTCManager
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

    // Pilih startup state berdasarkan SystemConfig
#if SKIP_DAILY_SCHEDULE
    changeState(FertigationState::PREPARE_DAILY_MIX);
#else
    changeState(FertigationState::WAIT_DAILY_MIX);
#endif
}

// Update
void FertigationFSM::update() {
    // Refresh RTC snapshot sekali per loop (1 I2C read, semua getter konsisten)
    rtcManager.refresh();

    sensorManager.update();

    sensor = sensorManager.getData();

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

        case FertigationState::ADD_NUTRIENT_A:
            handleAddNutrientA();
            break;

        case FertigationState::MIX_A:
            handleMixA();
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
            // State tidak dikenal — kembali ke IDLE sebagai safe fallback
            logError("[FSM] Unknown state — fallback to IDLE");
            changeState(FertigationState::IDLE);
            break;
    }
}

// ChangeState
void FertigationFSM::changeState(FertigationState newState) {
    state = newState;

    stateStartTime = millis();

    stateInitialized = false;

    saveRecovery();

    logStateTransition(newState);
}

// GetState
FertigationState
FertigationFSM::getState() const {
    return state;
}

// CONDITION HELPERS

bool FertigationFSM::isTankSafeForMixing() {
    return sensor.tankVolume >= MIN_REMAINING_VOLUME;
}

bool FertigationFSM::isPPMInRange() {
    return abs(sensor.ppm - targetPPM) <= PPM_TOLERANCE;
}

bool FertigationFSM::isPPMOverdose() {
    return sensor.ppm > targetPPM + MAX_PPM_OVERDOSE;
}

bool FertigationFSM::isTodayAlreadyMixed() {
    uint16_t day = rtcManager.getPlantAgeDays();

    uint8_t month = rtcManager.getMonth();

    uint16_t year = rtcManager.getYear();

    return
        day == lastMixDay &&
        month == lastMixMonth &&
        year == lastMixYear;
}

bool FertigationFSM::isStateTimeout(unsigned long timeout) {
    return millis() - stateStartTime > timeout;
}

// ACTUATOR HELPERS

void FertigationFSM::startMixer() {
    relayManager.on(RELAY_MIXER);
}

void FertigationFSM::stopMixer() {
    relayManager.off(RELAY_MIXER);
}

void FertigationFSM::startWaterFilling() {
    relayManager.on(RELAY_WATER);
}

void FertigationFSM::stopWaterFilling() {
    relayManager.off(RELAY_WATER);
}

void FertigationFSM::startNutrientA() {
    relayManager.on(RELAY_NUTRIENT_A);
}

void FertigationFSM::stopNutrientA() {
    relayManager.off(RELAY_NUTRIENT_A);
}

void FertigationFSM::startNutrientB() {
    relayManager.on(RELAY_NUTRIENT_B);
}

void FertigationFSM::stopNutrientB() {
    relayManager.off(RELAY_NUTRIENT_B);
}

// RECIPE

void FertigationFSM::prepareDailyRecipe() {
    uint16_t age = rtcManager.getPlantAgeDays();

    currentRecipe = recipeManager.getRecipe(age);

    currentIrrigation = irrigationRecipe.getRecipe(age);

    targetPPM = currentRecipe.targetPPM;
    targetMinPH = currentRecipe.targetMinPH;
    targetMaxPH = currentRecipe.targetMaxPH;
    float remainingVolume = sensor.tankVolume;

    targetWaterVolume = DAILY_TARGET_VOLUME - remainingVolume;

    if (targetWaterVolume < 0.0f) {
        targetWaterVolume = 0.0f;
    }

    float ratio = targetWaterVolume / DAILY_TARGET_VOLUME;

    if (ratio > 1.0f) {
        ratio = 1.0f;
    }

    if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    targetNutrientA = INITIAL_NUTRIENT_A * ratio;
    targetNutrientB = INITIAL_NUTRIENT_B * ratio;

    logRecipe(remainingVolume, ratio);
}

// MIXING HELPERS

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

// TRANSITION HELPERS

void FertigationFSM::gotoReady() {
    changeState(FertigationState::READY);
}

void FertigationFSM::gotoValidate() {
    changeState(FertigationState::VALIDATE);
}

void FertigationFSM::gotoCorrection() {
    // Simpan state mana yang memulai koreksi agar gotoPostCorrection()
    // bisa kembali ke validate yang benar
    correctionOrigin = state;

    changeState(FertigationState::CORRECT_PPM);
}

void FertigationFSM::gotoError(ErrorCode error) {
    enterError(error);
}

void FertigationFSM::gotoPostCorrection() {
    // FIX: kembali ke validate yang sesuai konteks asal koreksi
    if (correctionOrigin == FertigationState::PRE_IRRIGATION_VALIDATE) {
        changeState(FertigationState::PRE_IRRIGATION_VALIDATE);
    } else {
        // Default: dari VALIDATE (siklus mixing harian)
        gotoValidate();
    }
}

// ============================================================
// RECOVERY HELPERS
// ============================================================

void FertigationFSM::consumeRecovery() {
    // FIX: encapsulate 'recovering = false' agar tidak tersebar di setiap handler
    if (recovering) {
        logRecovery("[FSM] Resuming from recovery");
        recovering = false;
    }
}

void FertigationFSM::saveRecovery() {
    RecoveryData data;

    data.state = static_cast<uint16_t>(state);

    data.waterPulse = waterFlow.pulseCount;

    data.nutrientAPulse = nutrientAFlow.pulseCount;

    data.nutrientBPulse = nutrientBFlow.pulseCount;

    data.day = lastMixDay;

    data.month = lastMixMonth;

    data.year = lastMixYear;

    // FIX: simpan lastStateBeforeError agar recovery dari ERROR bisa kembali ke state yang benar
    data.lastStateBeforeError = static_cast<uint16_t>(lastStateBeforeError);

    data.batchRunning = true;

    recovery.save(data);
}

void FertigationFSM::restoreRecovery() {
    RecoveryData data = recovery.load();

    if (!data.batchRunning)
        return;

    // FIX: validasi range state sebelum cast ke enum
    // Jika nilai tidak valid, jangan restore — mulai dari WAIT_DAILY_MIX
    const uint16_t maxValidState = static_cast<uint16_t>(FertigationState::ERROR);

    if (data.state > maxValidState) {
        logError("[FSM] Recovery data corrupt — skip restore");
        return;
    }

    // FIX: jika state tersimpan adalah ERROR, gunakan lastStateBeforeError
    // agar sistem tidak stuck di ERROR saat boot
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

    // FIX: gunakan setPulseCount() dengan interrupt guard
    // bukan direct assignment ke volatile tanpa guard
    waterFlow.setPulseCount(data.waterPulse);
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

// ============================================================
// ERROR HELPERS
// ============================================================

void FertigationFSM::enterError(ErrorCode error) {
    // FIX: matikan semua relay SEGERA di sini, tidak menunggu handleError()
    // di loop berikutnya — relay mati dalam siklus yang sama saat error terdeteksi
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

// ============================================================
// STATE DIAGRAM
//
// Startup (normal)
//      |
//      v
// WAIT_DAILY_MIX
//      | jam DAILY_MIX_HOUR:DAILY_MIX_MINUTE && !isTodayAlreadyMixed
//      v
// PREPARE_DAILY_MIX
//      |
//      v
// FILL_WATER
//      | flowWater >= targetWaterVolume
//      v
// ADD_NUTRIENT_A
//      | flowA >= targetNutrientA
//      v
// MIX_A (MIX_A_TIME)
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

void FertigationFSM::handleIdle() {
    relayManager.allOff();
}

void FertigationFSM::handleWaitDailyMix() {
    uint8_t hour   = rtcManager.getHour();
    uint8_t minute = rtcManager.getMinute();
    uint16_t day   = rtcManager.getPlantAgeDays();
    uint8_t month  = rtcManager.getMonth();
    uint16_t year  = rtcManager.getYear();

    if (hour == DAILY_MIX_HOUR && minute == DAILY_MIX_MINUTE && !isTodayAlreadyMixed()) {
        lastMixDay   = day;
        lastMixMonth = month;
        lastMixYear  = year;
        changeState(FertigationState::PREPARE_DAILY_MIX);
    }
}

void FertigationFSM::handlePrepareDailyMix() {
    prepareDailyRecipe();

    // Reset correction counter untuk siklus mixing baru
    correctionCount = 0;

    waterFlow.reset();
    nutrientAFlow.reset();
    nutrientBFlow.reset();
    clearRecovery();
    changeState(FertigationState::FILL_WATER);
}

void FertigationFSM::handleFillWater() {
    if (isStateTimeout(WATER_FILL_TIMEOUT)) {
        gotoError(ErrorCode::WATER_TIMEOUT);
        return;
    }

    if (!stateInitialized) {
        // Selesaikan recovery flag sebelum melanjutkan operasi normal
        consumeRecovery();

        stateInitialized = true;

        logStateAction("[FSM] Filling Water...");
    }

    if (sensor.flowWater < targetWaterVolume) {
        startWaterFilling();
    } else {
        stopWaterFilling();

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

        startNutrientA();

        stateInitialized = true;

        logStateAction("[FSM] Add Nutrient A");
    }

    if (sensor.flowA >= targetNutrientA) {
        stopNutrientA();

        changeState(FertigationState::MIX_A);
    }
}

void FertigationFSM::handleMixA() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }

        logStateAction("[FSM] Mixing A");
    }

    if (updateMixing(MIX_A_TIME)) {
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

        startNutrientB();

        stateInitialized = true;

        logStateAction("[FSM] Add Nutrient B");
    }

    if (sensor.flowB >= targetNutrientB) {
        stopNutrientB();

        changeState(FertigationState::MIX_B);
    }
}

void FertigationFSM::handleMixB() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }

        logStateAction("[FSM] Mixing B");
    }

    if (updateMixing(MIX_B_TIME)) {
        // Reset correction counter — memasuki VALIDATE dari siklus mixing awal
        correctionCount = 0;
        gotoValidate();
    }
}

void FertigationFSM::handleValidate() {
    if (isPPMOverdose()) {
        gotoError(ErrorCode::OVER_PPM);
        return;
    }

    if (isPPMInRange()) {
        gotoReady();
    } else {
        gotoCorrection();
    }
}

void FertigationFSM::handleCorrectPPM() {
    // Cek apakah sudah dalam range — jika ya, langsung ke READY
    if (isPPMInRange()) {
        gotoReady();
        return;
    }

    // Cek overdose — jika ya, masuk ERROR
    if (isPPMOverdose()) {
        gotoError(ErrorCode::OVER_PPM);
        return;
    }

    // FIX: batasi jumlah iterasi koreksi untuk mencegah infinite loop
    // Jika sensor TDS rusak (stuck rendah), correctionCount mencegah pompa jalan terus
    if (correctionCount >= MAX_CORRECTION_COUNT) {
        logError("[FSM] Max correction attempts reached");
        gotoError(ErrorCode::CORRECTION_FAILED);
        return;
    }

    if (!stateInitialized) {
        consumeRecovery();

        // Reset flow meter nutrisi untuk mengukur dosis koreksi baru
        if (!recovering) {
            nutrientAFlow.reset();
            nutrientBFlow.reset();
        }

        startNutrientA();
        startNutrientB();

        stateInitialized = true;

        logStateAction("[FSM] Correct PPM");
    }

    if (sensor.flowA >= CORRECTION_DOSE && sensor.flowB >= CORRECTION_DOSE) {
        stopNutrientA();
        stopNutrientB();

        correctionCount++;

        changeState(FertigationState::CORRECTION_MIX);
    }
}

void FertigationFSM::handleCorrectionMix() {
    if (!stateInitialized) {
        if (!beginMixing()) {
            return;
        }

        logStateAction("[FSM] Correction Mix");
    }

    if (updateMixing(CORRECTION_MIX_TIME)) {
        // FIX: kembali ke validate yang sesuai konteks (VALIDATE atau PRE_IRRIGATION_VALIDATE)
        gotoPostCorrection();
    }
}

void FertigationFSM::handleReady() {
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
    if (isPPMOverdose()) {
        gotoError(ErrorCode::OVER_PPM);
        return;
    }

    bool ppmOK = isPPMInRange();
    bool phOK  = sensor.ph >= targetMinPH && sensor.ph <= targetMaxPH;

    if (ppmOK && phOK) {
        // FIX: reset correctionCount saat mulai irigasi baru
        correctionCount = 0;
        changeState(FertigationState::IRRIGATION);
    } else if (sensor.ppm < targetPPM) {
        // PPM rendah → lakukan koreksi
        // correctionOrigin akan di-set ke PRE_IRRIGATION_VALIDATE oleh gotoCorrection()
        gotoCorrection();
    } else {
        // PPM normal tapi pH out of range — FIX: error code yang benar
        // Sebelumnya: gotoError(OVER_PPM) — menyesatkan jika masalahnya pH
        gotoError(ErrorCode::PH_OUT_OF_RANGE);
    }
}

void FertigationFSM::handleIrrigation() {
    if (!stateInitialized) {
        relayManager.on(RELAY_IRRIGATION);

        stateInitialized = true;
    }

    if (sensor.soilADC <= currentIrrigation.wetThreshold) {
        relayManager.off(RELAY_IRRIGATION);

        gotoReady();
    }
}

void FertigationFSM::handleError() {
    if (!stateInitialized) {
        // allOff sudah dipanggil di enterError() — panggil lagi sebagai double-safety
        relayManager.allOff();

        logError();

        stateInitialized = true;
    }

    // waitingRecovery = true saat enterError dipanggil.
    // Desain: auto-recover — tidak perlu intervensi operator secara manual.
    // Dokumentasi: ini disengaja. Sistem akan recover ke state sebelum error.
    if (waitingRecovery == false) {
        recoverFromError();
    }
}

// ============================================================
// LOG HELPERS
// ============================================================

const char* FertigationFSM::stateToString(FertigationState s) {
    switch (s) {
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

void FertigationFSM::logStateTransition(FertigationState newState) {
    // FIX: cetak nama state bukan angka untuk readability di Serial Monitor
    Serial.print("[FSM] -> ");
    Serial.println(stateToString(newState));
}

void FertigationFSM::logStateAction(const char* message) {
    Serial.println(message);
}

void FertigationFSM::logRecipe(float remainingVolume, float ratio) {
    Serial.println();
    Serial.println("===== PARTIAL RECIPE =====");

    Serial.print("Remaining Volume : ");
    Serial.println(remainingVolume);

    Serial.print("Water To Add : ");
    Serial.println(targetWaterVolume);

    Serial.print("Ratio : ");
    Serial.println(ratio);

    Serial.print("Nutrient A : ");
    Serial.println(targetNutrientA);

    Serial.print("Nutrient B : ");
    Serial.println(targetNutrientB);

    Serial.println("==========================");
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
