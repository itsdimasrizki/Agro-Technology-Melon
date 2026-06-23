#ifndef FERTIGATION_FSM_H
#define FERTIGATION_FSM_H

#include "FertigationState.h"

#include "../sensors/SensorManager.h"
#include "../actuators/RelayManager.h"
#include "../rtc/RTCManager.h"

#include "../recipe/RecipeManager.h"
#include "../recipe/IrrigationRecipe.h"

#include "../utils/RecoveryManager.h"

class FertigationFSM {
public:
    FertigationFSM(
        SensorManager& sensors,
        RelayManager& relays,
        RTCManager& rtc,
        RecipeManager& recipe,
        IrrigationRecipe& irrigation,
        FlowMeter& water,
        FlowMeter& a,
        FlowMeter& b,
        RecoveryManager& recovery
    );

    void begin();

    void update();

    FertigationState getState() const;

private:
    void changeState(
        FertigationState newState
    );

    void handleIdle();

    void handleWaitDailyMix();

    void handlePrepareDailyMix();

    void handleFillWater();

    void handleAddNutrientA();

    void handleMixA();

    void handleAddNutrientB();

    void handleMixB();

    void handleValidate();

    void handleCorrectPPM();

    void handleCorrectionMix();

    void handleReady();

    void handlePreIrrigationMix();

    void handlePreIrrigationValidate();

    void handleIrrigation();

    void saveRecovery();

    void restoreRecovery();

    void handleError();

    void enterError();

    void recoverFromError();

private:
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

    FlowMeter& waterFlow;
    FlowMeter& nutrientAFlow;
    FlowMeter& nutrientBFlow;

    NutrientRecipe currentRecipe;
    IrrigationConfig currentIrrigation;

    uint8_t lastMixDay;
    uint8_t lastMixMonth;
    uint16_t lastMixYear;

    SensorData sensor;

    FertigationState lastStateBeforeError = FertigationState::IDLE;

    bool waitingRecovery = false;

    bool recovering = false;
};

#endif