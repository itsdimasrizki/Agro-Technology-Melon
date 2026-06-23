#include "FertigationFSM.h"
#include "../config/Constants.h"

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
}

// Begin
// void FertigationFSM::begin() {
//     changeState(FertigationState::WAIT_DAILY_MIX);
// }
void FertigationFSM::begin() {
    restoreRecovery();


    if(recovering) {
        Serial.println("[FSM] Resume From Power Failure");

        return;
    }

    // changeState(FertigationState::WAIT_DAILY_MIX);
    changeState(FertigationState::PREPARE_DAILY_MIX);
}

// Update
void FertigationFSM::update() {
    sensorManager.update();

    sensor = sensorManager.getData();

    switch(state) {
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
    }
}

// ChangeState
void FertigationFSM::changeState(FertigationState newState) {
    state = newState;

    stateStartTime = millis();

    stateInitialized = false;

    saveRecovery();

    Serial.print("[FSM] -> ");
    Serial.println(static_cast<int>(newState));
}

// GetState
FertigationState
FertigationFSM::getState() const {
    return state;
}

void FertigationFSM::handleIdle() {
    relayManager.allOff();
    // if(systemEnabled) {
    //     changeState(FertigationState::WAIT_DAILY_MIX);
    // }
}

void FertigationFSM::handleWaitDailyMix() {
    uint8_t hour = rtcManager.getHour();

    uint8_t minute = rtcManager.getMinute();

    uint16_t day = rtcManager.getPlantAgeDays();

    uint8_t month = rtcManager.getMonth();

    uint16_t year = rtcManager.getYear();

    bool alreadyMixedToday =
        day == lastMixDay &&
        month == lastMixMonth &&
        year == lastMixYear;

    if (hour == DAILY_MIX_HOUR && minute == DAILY_MIX_MINUTE && !alreadyMixedToday) {
        lastMixDay = day;
        lastMixMonth = month;
        lastMixYear = year;
        changeState(FertigationState::PREPARE_DAILY_MIX);
    }
}

void FertigationFSM::handlePrepareDailyMix() {
    uint16_t age = rtcManager.getPlantAgeDays();

    currentRecipe = recipeManager.getRecipe(age);

    currentIrrigation = irrigationRecipe.getRecipe(age);

    targetPPM = currentRecipe.targetPPM;
    targetMinPH = currentRecipe.targetMinPH;
    targetMaxPH = currentRecipe.targetMaxPH;
    targetWaterVolume = DAILY_TARGET_VOLUME - sensor.tankVolume;
    if(targetWaterVolume < 0) {
        targetWaterVolume = 0;
    }
    
    targetNutrientA = INITIAL_NUTRIENT_A;
    targetNutrientB = INITIAL_NUTRIENT_B;

    waterFlow.reset();
    nutrientAFlow.reset();
    nutrientBFlow.reset();
    recovery.clear();
    changeState(FertigationState::FILL_WATER);

}

void FertigationFSM::handleFillWater() {
    if (millis() - stateStartTime > WATER_FILL_TIMEOUT) {
        enterError();
        return;
    }
    
    if(!stateInitialized) {
        if(recovering) {
            Serial.println("[FSM] Resume FILL_WATER");
        }

        recovering = false;
        
        stateInitialized = true;

        Serial.println("[FSM] Filling Water...");
    }

    if(sensor.flowWater < targetWaterVolume) {
        relayManager.on(RELAY_WATER);
    } else {
        relayManager.off(RELAY_WATER);

        changeState(FertigationState::ADD_NUTRIENT_A);
    }
}

void FertigationFSM::handleAddNutrientA() {
    if (millis() - stateStartTime > NUTRIENT_TIMEOUT) {
        enterError();
        return;
    }

    if(!stateInitialized) {
        if(recovering) {
            Serial.println("[FSM] Resume ADD_NUTRIENT_A");
        }

        recovering = false;

        relayManager.on(RELAY_NUTRIENT_A);

        stateInitialized = true;

        Serial.println("[FSM] Add Nutrient A");
    }

    if(sensor.flowA >= targetNutrientA) {
        relayManager.off(RELAY_NUTRIENT_A);

        changeState(FertigationState::MIX_A);
    }
}

void FertigationFSM::handleMixA() {
    if(millis() - stateStartTime >= MIX_A_TIME) {
        changeState(FertigationState::ADD_NUTRIENT_B);
    }
}

void FertigationFSM::handleAddNutrientB() {
    if (millis() - stateStartTime > NUTRIENT_TIMEOUT) {
        enterError();
        return;
    }

    if(!stateInitialized) {
        if(recovering) {
            Serial.println("[FSM] Resume ADD_NUTRIENT_B");
        }
        recovering = false;

        relayManager.on(RELAY_NUTRIENT_B);

        stateInitialized = true;

        Serial.println("[FSM] Add Nutrient B");
    }

    if(sensor.flowB >= targetNutrientB) {
        relayManager.off(RELAY_NUTRIENT_B);

        changeState(FertigationState::MIX_B);
    }
}

void FertigationFSM::handleMixB() {
    if(millis() - stateStartTime >= MIX_B_TIME) {
        changeState(FertigationState::VALIDATE);
    }
}

void FertigationFSM::handleValidate() {
    // bool ppmOK = sensor.ppm >= targetPPM;

    // bool phOK = sensor.ph >= targetMinPH && sensor.ph <= targetMaxPH;

    // if(ppmOK && phOK) {
    if (sensor.ppm >= targetPPM) {
        changeState(FertigationState::READY);
    } else {
        changeState(FertigationState::CORRECT_PPM);
    }
}

void FertigationFSM::handleCorrectPPM() {
    if(sensor.ppm >= targetPPM) {
        changeState(
            FertigationState::READY
        );

        return;
    }

    if(!stateInitialized) {
        if(!recovering) {
            nutrientAFlow.reset();
            nutrientBFlow.reset();
        }
        recovering = false;

        relayManager.on(RELAY_NUTRIENT_A);

        relayManager.on(RELAY_NUTRIENT_B);

        stateInitialized = true;

        Serial.println("[FSM] Correct PPM");
    }

    if(sensor.flowA >= CORRECTION_DOSE && sensor.flowB >= CORRECTION_DOSE) {
        relayManager.off(RELAY_NUTRIENT_A);

        relayManager.off(RELAY_NUTRIENT_B);

        changeState(FertigationState::CORRECTION_MIX);
    }
}

void FertigationFSM::handleCorrectionMix() {
    if(millis() - stateStartTime >= CORRECTION_MIX_TIME) {
        changeState(FertigationState::VALIDATE);
    }
}

void FertigationFSM::handleReady() {
    if(sensor.soilADC >= currentIrrigation.dryThreshold) {
        changeState(FertigationState::PRE_IRRIGATION_MIX);
    }
}

void FertigationFSM::handlePreIrrigationMix() {
    if(!stateInitialized) {
        Serial.println("[FSM] Pre Irrigation Mix");

        // relay mixer nyala disini
        // relayManager.on(RELAY_MIXER);

        stateInitialized = true;
    }

    if(millis() - stateStartTime >= PRE_IRRIGATION_MIX_TIME) {
        // relayManager.off(RELAY_MIXER);

        changeState(FertigationState::PRE_IRRIGATION_VALIDATE);
    }
}

void FertigationFSM::handleIrrigation() {
    if(!stateInitialized) {
        relayManager.on(RELAY_IRRIGATION);

        stateInitialized = true;
    }

    if(sensor.soilADC <= currentIrrigation.wetThreshold) {
        relayManager.off(RELAY_IRRIGATION);

        changeState(FertigationState::READY);
    }
}

void FertigationFSM::handlePreIrrigationValidate() {
    bool ppmOK = sensor.ppm >= targetPPM;

    bool phOK = sensor.ph >= targetMinPH && sensor.ph <= targetMaxPH;

    if(ppmOK && phOK) {
        changeState(FertigationState::IRRIGATION);
    } else {
        changeState(FertigationState::CORRECT_PPM);
    }
}

void FertigationFSM::handleError() {
    if(!stateInitialized) {
        relayManager.allOff();

        Serial.println("[FSM] ERROR");

        Serial.println("[FSM] Waiting Recovery...");

        stateInitialized = true;
    }

    if(waitingRecovery == false) {
        recoverFromError();
    }
}

void FertigationFSM::enterError() {
    lastStateBeforeError =
        state;

    waitingRecovery = true;

    changeState(
        FertigationState::ERROR
    );
}

void FertigationFSM::recoverFromError() {
    relayManager.allOff();

    stateInitialized = false;

    waitingRecovery = false;

    recovering = true;
    
    Serial.print("[FSM] Recover to state : ");

    Serial.println(static_cast<int>(lastStateBeforeError));

    changeState(lastStateBeforeError);
}

void FertigationFSM::saveRecovery() {
    RecoveryData data;

    data.state = static_cast<uint8_t>(state);

    data.waterPulse = waterFlow.pulseCount;

    data.nutrientAPulse = nutrientAFlow.pulseCount;

    data.nutrientBPulse = nutrientBFlow.pulseCount;

    data.day = lastMixDay;

    data.month = lastMixMonth;

    data.year = lastMixYear;

    data.batchRunning = true;

    recovery.save(data);
}

void FertigationFSM::restoreRecovery() {
    RecoveryData data = recovery.load();

    if(!data.batchRunning)
        return;

    lastMixDay = data.day;

    lastMixMonth = data.month;

    lastMixYear = data.year;

    waterFlow.pulseCount = data.waterPulse;

    nutrientAFlow.pulseCount = data.nutrientAPulse;

    nutrientBFlow.pulseCount = data.nutrientBPulse;

    state = static_cast<FertigationState>(data.state);

    recovering = true;

    stateInitialized = false;

    Serial.print("[FSM] Recovery State : ");

    Serial.println(static_cast<int>(state));
}