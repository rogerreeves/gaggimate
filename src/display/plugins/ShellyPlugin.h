#pragma once

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <display/core/Plugin.h>
#include <display/core/Settings.h>

enum class ShellyFunction : uint8_t {
    None = 0,
    MainPower = 1,
    PowerLed = 2,
    Grinder = 3,
};

enum class ShellyLedMode : uint8_t {
    WithMainPower = 0,
    ActiveOnly = 1,
};

struct ShellyDevice {
    String id;
    String name;
    String model;
    String host;
    String username;
    String password;
    bool authEnabled = false;
    uint8_t channels = 1;
};

struct ShellyAssignment {
    String deviceId;
    uint8_t channel = 0;
    ShellyFunction function = ShellyFunction::None;
};

struct ShellySchedule {
    bool enabled = false;
    String onTime = "06:30";
    String offTime = "10:30";
    bool days[7] = {true, true, true, true, true, false, false};
    String onJobId;
    String offJobId;
};

struct ShellyScanResult {
    String host;
    String name;
    String model;
    String id;
    bool authRequired = false;
    uint8_t channels = 1;
};

class ShellyPlugin : public Plugin {
  public:
    void setup(Controller *controller, PluginManager *pluginManager) override;
    void loop() override;

    bool isEnabled() const { return enabled; }
    bool isGrinderEnabled() const { return grinderEnabled; }
    bool isLedEnabled() const { return ledEnabled; }
    bool isMainPowerEnabled() const { return mainPowerEnabled; }
    ShellyLedMode getLedMode() const { return ledMode; }

    std::vector<ShellyDevice> getDevices() const { return devices; }
    std::vector<ShellyAssignment> getAssignments() const { return assignments; }
    ShellySchedule getSchedule() const { return schedule; }
    std::vector<ShellyScanResult> getScanResults() const { return scanResults; }
    bool isScanInProgress() const { return scanInProgress; }
    String getScheduleSyncStatus() const { return scheduleSyncStatus; }
    String getScheduleSyncMessage() const { return scheduleSyncMessage; }

    bool addDevice(const String &host, const String &username, const String &password, String *err);
    bool removeDevice(const String &deviceId);
    bool testRelay(const String &deviceId, uint8_t channel, String *err);

    bool startScan(String *err);

    bool setAssignments(const std::vector<ShellyAssignment> &newAssignments, String *err);
    bool updateSchedule(const ShellySchedule &newSchedule, String *err);
    void updateConfig(bool enabled, bool grinderEnabled, bool ledEnabled, bool mainPowerEnabled, ShellyLedMode ledMode);

    bool setRelayState(ShellyFunction function, bool on, String *err);

  private:
    Controller *controller = nullptr;
    PluginManager *pluginManager = nullptr;

    bool enabled = false;
    bool grinderEnabled = false;
    bool ledEnabled = false;
    bool mainPowerEnabled = false;
    ShellyLedMode ledMode = ShellyLedMode::WithMainPower;

    std::vector<ShellyDevice> devices;
    std::vector<ShellyAssignment> assignments;
    ShellySchedule schedule{};
    std::vector<ShellyScanResult> scanResults;
    bool scanInProgress = false;

    String scheduleSyncStatus = "unknown";
    String scheduleSyncMessage = "";

    unsigned long lastSyncAttempt = 0;

    void loadConfig();
    void saveConfig();

    String buildTimespec(const String &time, const bool days[7]) const;
    bool rpcCall(const ShellyDevice &device, const char *method, JsonDocument &params, JsonDocument &result, String *err);
    bool fetchDeviceInfo(const String &host, const String &username, const String &password, ShellyDevice *outDevice,
                         bool *authRequired, String *err);
    bool detectChannelCount(const ShellyDevice &device, uint8_t *count, String *err);

    bool ensureScheduleOnDevice(const ShellyDevice &device, uint8_t channel, bool enabled, const String &timespec, bool on,
                                String *jobId, String *err);
    bool deleteScheduleJob(const ShellyDevice &device, const String &jobId, String *err);
    void updateScheduleSync();
    bool getAssignedRelay(ShellyFunction function, ShellyDevice *device, uint8_t *channel);
    bool validateAssignments(const std::vector<ShellyAssignment> &newAssignments, String *err) const;

    void handleStandbyEnter();
    void handleStandbyExit();
    void handleGrindStart();
    void handleGrindEnd();
};

extern ShellyPlugin Shelly;
