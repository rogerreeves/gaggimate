#include "display/controller_api.h"

#include <cmath>
#include <ctime>

namespace {
ControllerSnapshot snapshot{};
unsigned long lastTick = 0;
unsigned long brewStart = 0;
unsigned long grindStart = 0;
bool brewing = false;
bool grinding = false;
constexpr float kPi = 3.1415926535f;

Profile make_profile(const String &id, const String &label) {
    Profile profile{};
    profile.id = id;
    profile.label = label;
    profile.type = "standard";
    profile.description = "Simulator profile";
    profile.temperature = 93.0f;
    profile.favorite = true;
    profile.selected = (id == "sim-default");

    Phase preinfusion{};
    preinfusion.name = "Preinfusion";
    preinfusion.phase = PhaseType::PHASE_TYPE_PREINFUSION;
    preinfusion.valve = 0;
    preinfusion.duration = 5.0f * 1000.0f;
    preinfusion.pumpIsSimple = true;
    preinfusion.pumpSimple = 60;
    preinfusion.temperature = 93.0f;

    Phase brew{};
    brew.name = "Brew";
    brew.phase = PhaseType::PHASE_TYPE_BREW;
    brew.valve = 1;
    brew.duration = 25.0f * 1000.0f;
    brew.pumpIsSimple = true;
    brew.pumpSimple = 80;
    brew.temperature = 93.0f;
    Target vol{};
    vol.type = TargetType::TARGET_TYPE_VOLUMETRIC;
    vol.operator_ = TargetOperator::GTE;
    vol.value = 36.0f;
    brew.targets.push_back(vol);

    profile.phases.push_back(preinfusion);
    profile.phases.push_back(brew);
    return profile;
}

void update_time() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[9];
    std::strftime(buf, sizeof(buf), "%H:%M", &tm);
    snapshot.standbyTime = buf;
    snapshot.standbyTimeValid = true;
    snapshot.christmasMode = (tm.tm_mon == 11 && tm.tm_mday < 27) || (tm.tm_mon == 0 && tm.tm_mday < 6);
}

void update_brew_state(unsigned long now) {
    if (!brewing) {
        return;
    }
    const unsigned long elapsed = now - brewStart;
    const unsigned long total = 30 * 1000;
    if (elapsed >= total) {
        brewing = false;
        snapshot.active = false;
        snapshot.brewProcess.active = false;
        snapshot.brewProcess.finished = now;
        return;
    }

    snapshot.active = true;
    snapshot.brewProcess.active = true;
    snapshot.brewProcess.valid = true;
    snapshot.brewProcess.processStarted = brewStart;
    snapshot.brewProcess.currentPhaseStarted = brewStart;
    snapshot.brewProcess.finished = 0;
    snapshot.brewProcess.target = ProcessTarget::VOLUMETRIC;
    snapshot.brewProcess.totalDuration = static_cast<float>(total);
    snapshot.brewProcess.phaseIndex = elapsed < 5000 ? 0 : 1;
    snapshot.brewProcess.phaseDuration = elapsed < 5000 ? 5000.0f : 25000.0f;
    snapshot.brewProcess.currentVolume = static_cast<float>(elapsed) / total * 36.0f;
    snapshot.brewProcess.brewVolume = 36.0f;
    snapshot.brewProcess.advancedPump = false;
    snapshot.brewProcess.pumpPressure = 9.0f;
}

void update_grind_state(unsigned long now) {
    if (!grinding) {
        return;
    }
    if (now - grindStart > 8000) {
        grinding = false;
        snapshot.grindActive = false;
        return;
    }
    snapshot.grindActive = true;
}
}

void controller_api_init() {
    snapshot.ready = true;
    snapshot.mode = MODE_BREW;
    snapshot.startupMode = MODE_BREW;
    snapshot.active = false;
    snapshot.grindActive = false;
    snapshot.updateActive = false;
    snapshot.updateAvailable = false;
    snapshot.updateStateValid = true;
    snapshot.apActive = false;
    snapshot.apActiveValid = true;
    snapshot.wifiConnected = true;
    snapshot.bluetoothConnected = true;
    snapshot.bluetoothScalesHealthy = true;
    snapshot.volumetricAvailable = true;
    snapshot.volumetricTarget = true;
    snapshot.hasSavedScale = true;
    snapshot.brewScaleWarningShown = false;
    snapshot.smartGrindActive = true;
    snapshot.altRelayFunction = ALT_RELAY_GRIND;

    snapshot.doseMeasureEnabled = true;
    snapshot.doseMeasureDefaultDoseCount = 1;
    snapshot.doseMeasureTarget = 18.5;
    snapshot.doseMeasureAvgBeanWeight = 0.1;
    snapshot.doseMeasureCupEmptyWeight = 0.0;
    snapshot.doseMeasureCupEnabled = true;
    snapshot.doseMeasureBeepEnabled = true;
    snapshot.doseMeasureProceedBeanCount = 3;
    snapshot.doseMeasureBeanCountLimit = 20;

    snapshot.grindDuration = 8;
    snapshot.grindVolume = 0.0f;
    snapshot.targetDuration = 30.0f;
    snapshot.targetVolume = 36.0f;

    snapshot.currentTemp = 92;
    snapshot.targetTemp = 93;
    snapshot.pressure = 0.0f;
    snapshot.pressureAvailable = true;
    snapshot.pressureScaling = DEFAULT_PRESSURE_SCALING;
    snapshot.bluetoothWeight = 0.0;
    snapshot.bluetoothWeightValid = true;

    snapshot.screensaverEnabled = true;
    snapshot.screensaverTimeout = 2.0f;
    snapshot.mainBrightness = 100;
    snapshot.standbyBrightness = 20;
    snapshot.standbyBrightnessTimeout = 5000;
    snapshot.standbyLandingScreen = "menu";
    snapshot.themeMode = 0;

    snapshot.selectedProfileId = "sim-default";
    snapshot.brewProcess.profile = make_profile("sim-default", "Simulator");
    snapshot.brewProcess.valid = false;

    update_time();
    lastTick = millis();
}

void controller_api_tick_16ms() {
    const unsigned long now = millis();
    const unsigned long delta = now - lastTick;
    if (delta < 16) {
        return;
    }
    lastTick = now;

    update_time();

    const float phase = static_cast<float>(now % 60000) / 60000.0f;
    snapshot.currentTemp = static_cast<int>(snapshot.targetTemp - 2 + std::sin(phase * 2.0f * kPi) * 2.0f);

    if (!brewing && (now / 15000) % 4 == 1) {
        brewing = true;
        brewStart = now;
        snapshot.mode = MODE_BREW;
        snapshot.brewProcess.profile = make_profile("sim-default", "Simulator");
    }

    if (!grinding && (now / 20000) % 5 == 2) {
        grinding = true;
        grindStart = now;
        snapshot.mode = MODE_GRIND;
    }

    if (!brewing && !grinding) {
        const int modeCycle = static_cast<int>((now / 10000) % 5);
        switch (modeCycle) {
        case 0:
            snapshot.mode = MODE_BREW;
            break;
        case 1:
            snapshot.mode = MODE_STEAM;
            break;
        case 2:
            snapshot.mode = MODE_WATER;
            break;
        case 3:
            snapshot.mode = MODE_GRIND;
            break;
        default:
            snapshot.mode = MODE_STANDBY;
            break;
        }
    }

    update_brew_state(now);
    update_grind_state(now);

    if (snapshot.active) {
        snapshot.pressure = 9.0f + std::sin(phase * 2.0f * kPi) * 0.8f;
    } else {
        snapshot.pressure = 0.0f;
    }

    snapshot.bluetoothWeight = snapshot.active ? snapshot.brewProcess.currentVolume
                                               : (std::sin(phase * 2.0f * kPi) * 2.0f + 18.0f);
    snapshot.bluetoothWeightValid = true;
}

ControllerSnapshot controller_api_get() { return snapshot; }

void controller_api_set_dose_measure_target(double target) { snapshot.doseMeasureTarget = target; }

void controller_api_set_selected_profile(const String &profileId) { snapshot.selectedProfileId = profileId; }

void controller_api_tare_scales() {
    // no-op for simulator
}

void controller_api_beep_scales(int level) {
    (void)level;
}

void controller_api_set_brew_scale_warning_shown(bool shown) { snapshot.brewScaleWarningShown = shown; }
