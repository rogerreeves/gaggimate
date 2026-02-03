#ifndef PROFILE_H
#define PROFILE_H

#include <Arduino.h>
#include <vector>

enum class TargetType { TARGET_TYPE_VOLUMETRIC, TARGET_TYPE_PRESSURE, TARGET_TYPE_FLOW, TARGET_TYPE_PUMPED };
enum class TargetOperator { LTE, GTE };
enum class PumpTarget { PUMP_TARGET_FLOW, PUMP_TARGET_PRESSURE };
enum class PhaseType { PHASE_TYPE_PREINFUSION, PHASE_TYPE_BREW };
enum class TransitionType { INSTANT, LINEAR, EASE_IN, EASE_OUT, EASE_IN_OUT };

struct Target {
    TargetType type{};
    TargetOperator operator_{};
    float value = 0.0f;

    bool isReached(float input) const {
        if (operator_ == TargetOperator::GTE) {
            return input >= value;
        }
        return input <= value;
    }
};

struct PumpAdvanced {
    PumpTarget target{};
    float pressure = 0.0f;
    float flow = 0.0f;
};

struct Transition {
    TransitionType type{};
    float duration = 0.0f;
    bool adaptive = false;
};

struct Phase {
    String name;
    PhaseType phase{};
    int valve = 0;
    float duration = 0.0f;
    bool pumpIsSimple = true;
    int pumpSimple = 0;
    float temperature = 0.0f;
    Transition transition{};
    PumpAdvanced pumpAdvanced{};
    std::vector<Target> targets;

    bool hasVolumetricTarget() const {
        for (const auto &target : targets) {
            if (target.type == TargetType::TARGET_TYPE_VOLUMETRIC && target.value > 0.0f) {
                return true;
            }
        }
        return false;
    }

    Target getVolumetricTarget() const {
        for (const auto &target : targets) {
            if (target.type == TargetType::TARGET_TYPE_VOLUMETRIC) {
                return target;
            }
        }
        return Target{};
    }
};

struct Profile {
    String id;
    String label;
    String type;
    String description;
    bool utility = false;
    float temperature = 0.0f;
    bool favorite = false;
    bool selected = false;
    std::vector<Phase> phases;

    unsigned int getPhaseCount() const {
        int brew = 0;
        int preinfusion = 0;
        for (const auto &phase : phases) {
            if (phase.phase == PhaseType::PHASE_TYPE_BREW) {
                brew = 1;
            } else {
                preinfusion = 1;
            }
        }
        return brew + preinfusion;
    }

    float getTotalDuration() const {
        float duration = 0.0f;
        for (const auto &phase : phases) {
            duration += phase.duration;
        }
        return duration;
    }

    float getTotalVolume() const {
        float volume = 0.0f;
        for (const auto &phase : phases) {
            if (phase.hasVolumetricTarget()) {
                volume = phase.getVolumetricTarget().value;
            }
        }
        return volume;
    }
};

#endif // PROFILE_H
