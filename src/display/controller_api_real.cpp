#include "controller_api.h"

#include <display/core/Controller.h>
#include <display/core/process/BrewProcess.h>
#include <display/main.h>
#include <display/core/ProfileManager.h>
#include <display/plugins/BLEScalePlugin.h>
#include <WiFi.h>
#include <cmath>
#include <ctime>

namespace {
ControllerSnapshot snapshot{};
}

static BrewProcessSnapshot build_brew_snapshot(Controller &controller) {
    BrewProcessSnapshot brew{};
    Process *process = controller.getProcess();
    if (!process) {
        process = controller.getLastProcess();
    }
    if (!process || process->getType() != MODE_BREW) {
        return brew;
    }

    auto *brewProcess = static_cast<BrewProcess *>(process);
    if (!brewProcess) {
        return brew;
    }

    brew.valid = true;
    brew.active = process->isActive();
    brew.target = brewProcess->target;
    brew.processStarted = brewProcess->processStarted;
    brew.currentPhaseStarted = brewProcess->currentPhaseStarted;
    brew.finished = brewProcess->finished;
    brew.currentVolume = static_cast<float>(brewProcess->currentVolume);
    brew.phaseIndex = static_cast<int>(brewProcess->phaseIndex);
    brew.phaseDuration = static_cast<float>(brewProcess->getPhaseDuration());
    brew.totalDuration = static_cast<float>(brewProcess->getTotalDuration());
    brew.brewVolume = static_cast<float>(brewProcess->getBrewVolume());
    brew.advancedPump = brewProcess->isAdvancedPump();
    brew.pumpPressure = static_cast<float>(brewProcess->getPumpPressure());
    brew.profile = brewProcess->profile;
    return brew;
}

void controller_api_init() {
    // no-op for firmware
}

void controller_api_tick_16ms() {
    // no-op for firmware
}

ControllerSnapshot controller_api_get() {
    Settings &settings = controller.getSettings();

    snapshot.ready = controller.isReady();
    snapshot.error = controller.isErrorState();
    snapshot.errorCode = controller.getError();
    snapshot.autotuning = controller.isAutotuning();
    snapshot.mode = controller.getMode();
    snapshot.active = controller.isActive();
    snapshot.grindActive = controller.isGrindActive();
    snapshot.updateActive = false;
    snapshot.updateAvailable = false;
    snapshot.updateStateValid = false;
    snapshot.apActive = false;
    snapshot.apActiveValid = false;

    snapshot.wifiConnected = (WiFi.status() == WL_CONNECTED);
    snapshot.bluetoothConnected = controller.getClientController()->isConnected();
    snapshot.bluetoothScalesHealthy = controller.isBluetoothScaleHealthy();
    snapshot.volumetricAvailable = controller.isVolumetricAvailable();
    snapshot.volumetricTarget = settings.isVolumetricTarget();
    snapshot.hasSavedScale = settings.getSavedScale().length() > 0;
    snapshot.brewScaleWarningShown = settings.getBrewScaleWarningShown();
    snapshot.smartGrindActive = settings.isSmartGrindActive();
    snapshot.altRelayFunction = settings.getAltRelayFunction();

    snapshot.doseMeasureEnabled = settings.isDoseMeasureEnabled();
    snapshot.doseMeasureDefaultDoseCount = settings.getDoseMeasureDefaultDoseCount();
    snapshot.doseMeasureTarget = settings.getDoseMeasureTarget();
    snapshot.doseMeasureAvgBeanWeight = settings.getDoseMeasureAvgBeanWeight();
    snapshot.doseMeasureCupEmptyWeight = settings.getDoseMeasureCupEmptyWeight();
    snapshot.doseMeasureCupEnabled = settings.isDoseMeasureCupEnabled();
    snapshot.doseMeasureBeepEnabled = settings.isDoseMeasureBeepEnabled();
    snapshot.doseMeasureProceedBeanCount = settings.getDoseMeasureProceedBeanCount();
    snapshot.doseMeasureBeanCountLimit = settings.getDoseMeasureBeanCountLimit();

    snapshot.grindDuration = settings.getTargetGrindDuration();
    snapshot.grindVolume = static_cast<float>(settings.getTargetGrindVolume());
    snapshot.targetDuration = controller.getProfileManager()->getSelectedProfile().getTotalDuration();
    snapshot.targetVolume = controller.getProfileManager()->getSelectedProfile().getTotalVolume();

    snapshot.currentTemp = static_cast<int>(controller.getCurrentTemp());
    snapshot.targetTemp = static_cast<int>(controller.getTargetTemp());
    snapshot.pressure = controller.getCurrentPressure();
    snapshot.pressureAvailable = controller.getSystemInfo().capabilities.pressure;
    snapshot.pressureScaling = static_cast<int>(std::ceil(settings.getPressureScaling()));
    snapshot.bluetoothWeight = 0.0;
    snapshot.bluetoothWeightValid = false;

    snapshot.screensaverEnabled = settings.isScreensaverEnabled();
    snapshot.screensaverTimeout = settings.getScreensaverTimeout();
    snapshot.mainBrightness = settings.getMainBrightness();
    snapshot.standbyBrightness = settings.getStandbyBrightness();
    snapshot.standbyBrightnessTimeout = settings.getStandbyBrightnessTimeout();
    snapshot.standbyLandingScreen = settings.getStandbyLandingScreen();
    snapshot.themeMode = settings.getThemeMode();

    snapshot.startupMode = settings.getStartupMode();
    snapshot.selectedProfileId = settings.getSelectedProfile();

    time_t now;
    struct tm timeinfo;
    snapshot.standbyTimeValid = false;
    if (snapshot.wifiConnected && getLocalTime(&timeinfo, 500)) {
        char timeBuf[9];
        const char *format = settings.isClock24hFormat() ? "%H:%M" : "%I:%M %p";
        strftime(timeBuf, sizeof(timeBuf), format, &timeinfo);
        snapshot.standbyTime = timeBuf;
        snapshot.standbyTimeValid = true;
        snapshot.christmasMode = (timeinfo.tm_mon == 11 && timeinfo.tm_mday < 27) || (timeinfo.tm_mon == 0 && timeinfo.tm_mday < 6);
    } else {
        snapshot.standbyTime = "";
        snapshot.christmasMode = false;
    }

    snapshot.brewProcess = build_brew_snapshot(controller);

    return snapshot;
}

void controller_api_set_dose_measure_target(double target) {
    controller.getSettings().setDoseMeasureTarget(target);
}

void controller_api_set_selected_profile(const String &profileId) {
    controller.getSettings().setSelectedProfile(profileId);
}

void controller_api_tare_scales() {
    BLEScales.tare();
}

void controller_api_beep_scales(int level) {
    BLEScales.beep(static_cast<uint8_t>(level));
}

void controller_api_set_brew_scale_warning_shown(bool shown) {
    controller.getSettings().setBrewScaleWarningShown(shown);
}
