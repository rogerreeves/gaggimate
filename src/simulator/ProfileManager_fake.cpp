#include "include/display/core/ProfileManager.h"

#include <algorithm>

namespace {
Profile make_profile(const String &id, const String &label, float temp, bool favorite) {
    Profile profile{};
    profile.id = id;
    profile.label = label;
    profile.type = "standard";
    profile.description = "Simulator profile";
    profile.temperature = temp;
    profile.favorite = favorite;
    profile.selected = false;

    Phase preinfusion{};
    preinfusion.name = "Preinfusion";
    preinfusion.phase = PhaseType::PHASE_TYPE_PREINFUSION;
    preinfusion.valve = 0;
    preinfusion.duration = 5.0f * 1000.0f;
    preinfusion.pumpIsSimple = true;
    preinfusion.pumpSimple = 60;
    preinfusion.temperature = temp;

    Phase brew{};
    brew.name = "Brew";
    brew.phase = PhaseType::PHASE_TYPE_BREW;
    brew.valve = 1;
    brew.duration = 25.0f * 1000.0f;
    brew.pumpIsSimple = true;
    brew.pumpSimple = 80;
    brew.temperature = temp;
    Target vol{};
    vol.type = TargetType::TARGET_TYPE_VOLUMETRIC;
    vol.operator_ = TargetOperator::GTE;
    vol.value = 36.0f;
    brew.targets.push_back(vol);

    profile.phases.push_back(preinfusion);
    profile.phases.push_back(brew);
    return profile;
}
}

ProfileManager::ProfileManager() { ensure_defaults(); }

void ProfileManager::ensure_defaults() {
    if (!profiles_.empty()) {
        return;
    }
    profiles_.push_back(make_profile("sim-default", "Simulator", 93.0f, true));
    profiles_.push_back(make_profile("sim-dark", "Dark Roast", 95.0f, true));
    selectedProfile_ = profiles_.front();
    selectedProfile_.selected = true;
    selectedId_ = selectedProfile_.id;
}

std::vector<String> ProfileManager::listProfiles() const {
    std::vector<String> ids;
    ids.reserve(profiles_.size());
    for (const auto &profile : profiles_) {
        ids.push_back(profile.id);
    }
    return ids;
}

bool ProfileManager::loadProfile(const String &uuid, Profile &outProfile) const {
    for (const auto &profile : profiles_) {
        if (profile.id == uuid) {
            outProfile = profile;
            return true;
        }
    }
    return false;
}

bool ProfileManager::saveProfile(Profile &profile) {
    for (auto &existing : profiles_) {
        if (existing.id == profile.id) {
            existing = profile;
            return true;
        }
    }
    profiles_.push_back(profile);
    return true;
}

bool ProfileManager::deleteProfile(const String &uuid) {
    auto it = std::remove_if(profiles_.begin(), profiles_.end(), [&](const Profile &profile) { return profile.id == uuid; });
    if (it == profiles_.end()) {
        return false;
    }
    profiles_.erase(it, profiles_.end());
    if (selectedId_ == uuid && !profiles_.empty()) {
        selectProfile(profiles_.front().id);
    }
    return true;
}

bool ProfileManager::profileExists(const String &uuid) const {
    for (const auto &profile : profiles_) {
        if (profile.id == uuid) {
            return true;
        }
    }
    return false;
}

void ProfileManager::selectProfile(const String &uuid) {
    selectedId_ = uuid;
    for (auto &profile : profiles_) {
        profile.selected = (profile.id == uuid);
        if (profile.selected) {
            selectedProfile_ = profile;
        }
    }
}

Profile &ProfileManager::getSelectedProfile() { return selectedProfile_; }

bool ProfileManager::loadSelectedProfile(Profile &outProfile) const { return loadProfile(selectedId_, outProfile); }

std::vector<String> ProfileManager::getFavoritedProfiles(bool) const {
    std::vector<String> favs;
    for (const auto &profile : profiles_) {
        if (profile.favorite) {
            favs.push_back(profile.id);
        }
    }
    if (favs.empty() && !profiles_.empty()) {
        favs.push_back(profiles_.front().id);
    }
    return favs;
}
