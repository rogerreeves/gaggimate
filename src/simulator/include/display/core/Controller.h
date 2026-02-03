#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <display/core/constants.h>
#include "../../../../display/ui/default/DefaultUI.h"

class ProfileManager;

class Controller {
  public:
    Controller() = default;

    void setMode(int newMode) { mode_ = newMode; }
    int getMode() const { return mode_; }

    void activate() {}
    void deactivate() {}
    void clear() {}

    void lowerTemp() {}
    void raiseTemp() {}
    void lowerBrewTarget() {}
    void raiseBrewTarget() {}
    void lowerGrindTarget() {}
    void raiseGrindTarget() {}

    void activateStandby() { mode_ = MODE_STANDBY; }
    void activateGrind() {}
    void deactivateGrind() {}

    void onTargetToggle() {}
    void onProfileSave() const {}
    void onProfileSaveAsNew() {}
    void onFlush() {}
    void onScreenReady() {}
    void updateLastAction() {}

    DefaultUI *getUI() const { return ui_; }
    void setUI(DefaultUI *ui) { ui_ = ui; }

    ProfileManager *getProfileManager() const { return profileManager_; }
    void setProfileManager(ProfileManager *manager) { profileManager_ = manager; }

  private:
    int mode_ = MODE_BREW;
    DefaultUI *ui_ = nullptr;
    ProfileManager *profileManager_ = nullptr;
};

#endif // CONTROLLER_H
