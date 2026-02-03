#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <vector>
#include "../models/profile.h"

class ProfileManager {
  public:
    ProfileManager();

    void setup() {}
    std::vector<String> listProfiles() const;
    bool loadProfile(const String &uuid, Profile &outProfile) const;
    bool saveProfile(Profile &profile);
    bool deleteProfile(const String &uuid);
    bool profileExists(const String &uuid) const;
    void selectProfile(const String &uuid);
    Profile &getSelectedProfile();
    bool loadSelectedProfile(Profile &outProfile) const;
    std::vector<String> getFavoritedProfiles(bool validate = false) const;

  private:
    std::vector<Profile> profiles_;
    Profile selectedProfile_{};
    String selectedId_ = "sim-default";

    void ensure_defaults();
};

#endif // PROFILEMANAGER_H
