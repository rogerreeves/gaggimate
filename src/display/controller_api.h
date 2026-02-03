#ifndef CONTROLLER_API_H
#define CONTROLLER_API_H

#include <display/core/constants.h>
#include <display/core/process/Process.h>
#if defined(GAGGIMATE_NATIVE)
#include "../simulator/include/display/models/profile.h"
#else
#include <display/models/profile.h>
#endif

#ifndef ERROR_CODE_RUNAWAY
#define ERROR_CODE_RUNAWAY 4
#endif

struct BrewProcessSnapshot {
    bool valid = false;
    bool active = false;
    ProcessTarget target = ProcessTarget::TIME;
    unsigned long processStarted = 0;
    unsigned long currentPhaseStarted = 0;
    unsigned long finished = 0;
    float currentVolume = 0.0f;
    int phaseIndex = 0;
    float phaseDuration = 0.0f;
    float totalDuration = 0.0f;
    float brewVolume = 0.0f;
    bool advancedPump = false;
    float pumpPressure = 0.0f;
    Profile profile{};
};

struct ControllerSnapshot {
    bool ready = false;
    bool error = false;
    int errorCode = 0;
    bool autotuning = false;
    int mode = MODE_STANDBY;
    bool active = false;
    bool grindActive = false;
    bool updateActive = false;
    bool updateAvailable = false;
    bool updateStateValid = false;
    bool apActive = false;
    bool apActiveValid = false;
    bool wifiConnected = false;
    bool bluetoothConnected = false;
    bool bluetoothScalesHealthy = false;
    bool volumetricAvailable = false;
    bool volumetricTarget = false;
    bool hasSavedScale = false;
    bool brewScaleWarningShown = false;
    bool smartGrindActive = false;
    int altRelayFunction = ALT_RELAY_NONE;

    bool doseMeasureEnabled = false;
    int doseMeasureDefaultDoseCount = 1;
    double doseMeasureTarget = 18.5;
    double doseMeasureAvgBeanWeight = 0.1;
    double doseMeasureCupEmptyWeight = 0.0;
    bool doseMeasureCupEnabled = true;
    bool doseMeasureBeepEnabled = true;
    int doseMeasureProceedBeanCount = 3;
    int doseMeasureBeanCountLimit = 20;

    int grindDuration = 0;
    float grindVolume = 0.0f;
    float targetDuration = 0.0f;
    float targetVolume = 0.0f;

    int currentTemp = 0;
    int targetTemp = 0;
    float pressure = 0.0f;
    bool pressureAvailable = false;
    int pressureScaling = DEFAULT_PRESSURE_SCALING;
    double bluetoothWeight = 0.0;
    bool bluetoothWeightValid = false;

    bool screensaverEnabled = true;
    float screensaverTimeout = 2.0f;
    int mainBrightness = 100;
    int standbyBrightness = 20;
    int standbyBrightnessTimeout = 0;
    String standbyLandingScreen = "menu";
    String standbyTime = "";
    bool standbyTimeValid = false;
    bool christmasMode = false;
    int themeMode = 0;

    int startupMode = MODE_BREW;
    String selectedProfileId = "";

    BrewProcessSnapshot brewProcess{};
};

void controller_api_init();
void controller_api_tick_16ms();
ControllerSnapshot controller_api_get();

// Settings mutators used by UI
void controller_api_set_dose_measure_target(double target);
void controller_api_set_selected_profile(const String &profileId);
void controller_api_tare_scales();
void controller_api_beep_scales(int level);
void controller_api_set_brew_scale_warning_shown(bool shown);

#endif // CONTROLLER_API_H
