#include "ShellyPlugin.h"

#include <algorithm>
#include <map>

#include <ESPmDNS.h>
#include <display/core/Controller.h>
#include <display/core/Event.h>

static const uint32_t kShellyRpcTimeoutMs = 2000;
static const uint32_t kShellySyncIntervalMs = 30000;

ShellyPlugin Shelly;

void ShellyPlugin::setup(Controller *controller, PluginManager *pluginManager) {
    this->controller = controller;
    this->pluginManager = pluginManager;

    loadConfig();

    pluginManager->on("ui:standby:enter", [this](Event const &) { handleStandbyEnter(); });
    pluginManager->on("ui:standby:exit", [this](Event const &) { handleStandbyExit(); });
    pluginManager->on("controller:grind:start", [this](Event const &) { handleGrindStart(); });
    pluginManager->on("controller:grind:end", [this](Event const &) { handleGrindEnd(); });

    if (enabled && ledEnabled) {
        String err;
        setRelayState(ShellyFunction::PowerLed, true, &err);
    }

    updateScheduleSync();
}

void ShellyPlugin::loop() {
    if (!enabled) {
        return;
    }

    const unsigned long now = millis();
    if (now - lastSyncAttempt >= kShellySyncIntervalMs) {
        lastSyncAttempt = now;
        updateScheduleSync();
    }
}

void ShellyPlugin::updateConfig(bool enabled, bool grinderEnabled, bool ledEnabled, bool mainPowerEnabled, ShellyLedMode ledMode) {
    const bool wasMainPowerEnabled = this->mainPowerEnabled;
    this->enabled = enabled;
    this->grinderEnabled = grinderEnabled;
    this->ledEnabled = ledEnabled;
    this->mainPowerEnabled = mainPowerEnabled;
    this->ledMode = ledMode;

    if (wasMainPowerEnabled && !mainPowerEnabled) {
        ShellyDevice device;
        uint8_t channel = 0;
        String err;
        if (getAssignedRelay(ShellyFunction::MainPower, &device, &channel)) {
            deleteScheduleJob(device, schedule.onJobId, &err);
            deleteScheduleJob(device, schedule.offJobId, &err);
        }
    }
    saveConfig();
}

bool ShellyPlugin::addDevice(const String &host, const String &username, const String &password, String *err) {
    ShellyDevice device;
    bool authRequired = false;
    if (!fetchDeviceInfo(host, username, password, &device, &authRequired, err)) {
        if (authRequired) {
            return false;
        }
        return false;
    }

    for (const auto &existing : devices) {
        if (existing.id == device.id || existing.host == device.host) {
            if (err) {
                *err = "Device already added";
            }
            return false;
        }
    }

    uint8_t channels = 1;
    if (!detectChannelCount(device, &channels, err)) {
        return false;
    }
    device.channels = channels;

    devices.push_back(device);
    saveConfig();
    return true;
}

bool ShellyPlugin::removeDevice(const String &deviceId) {
    bool removed = false;
    devices.erase(std::remove_if(devices.begin(), devices.end(),
                                 [&](const ShellyDevice &device) {
                                     if (device.id == deviceId) {
                                         removed = true;
                                         return true;
                                     }
                                     return false;
                                 }),
                 devices.end());

    if (!removed) {
        return false;
    }

    assignments.erase(std::remove_if(assignments.begin(), assignments.end(),
                                     [&](const ShellyAssignment &assignment) { return assignment.deviceId == deviceId; }),
                      assignments.end());
    saveConfig();
    return true;
}

bool ShellyPlugin::testRelay(const String &deviceId, uint8_t channel, String *err) {
    ShellyDevice device;
    uint8_t unusedChannel = 0;
    bool found = false;
    for (const auto &entry : devices) {
        if (entry.id == deviceId) {
            device = entry;
            found = true;
            break;
        }
    }
    if (!found) {
        if (err) {
            *err = "Device not found";
        }
        return false;
    }
    unusedChannel = channel;

    JsonDocument params;
    params["id"] = unusedChannel;
    params["on"] = true;

    JsonDocument result;
    if (!rpcCall(device, "Switch.Set", params, result, err)) {
        return false;
    }

    delay(300);
    params["on"] = false;
    JsonDocument resultOff;
    return rpcCall(device, "Switch.Set", params, resultOff, err);
}

bool ShellyPlugin::startScan(String *err) {
    if (scanInProgress) {
        if (err) {
            *err = "Scan already in progress";
        }
        return false;
    }

    scanInProgress = true;
    scanResults.clear();

    int count = MDNS.queryService("shelly", "tcp");
    if (count <= 0) {
        scanInProgress = false;
        if (err) {
            *err = "No Shelly devices found via mDNS";
        }
        return false;
    }

    for (int i = 0; i < count; i++) {
        ShellyScanResult result;
        result.host = MDNS.IP(i).toString();
        ShellyDevice device;
        bool authRequired = false;
        String fetchErr;
        if (fetchDeviceInfo(result.host, "", "", &device, &authRequired, &fetchErr)) {
            result.name = device.name;
            result.model = device.model;
            result.id = device.id;
            result.channels = device.channels;
            result.authRequired = device.authEnabled;
        } else {
            result.name = "Unknown";
            result.model = "Unknown";
            result.id = "";
            result.authRequired = authRequired;
        }
        scanResults.push_back(result);
    }

    scanInProgress = false;
    return true;
}

bool ShellyPlugin::setAssignments(const std::vector<ShellyAssignment> &newAssignments, String *err) {
    if (!validateAssignments(newAssignments, err)) {
        return false;
    }
    assignments = newAssignments;
    saveConfig();
    return true;
}

bool ShellyPlugin::updateSchedule(const ShellySchedule &newSchedule, String *err) {
    schedule = newSchedule;
    if (!mainPowerEnabled || !enabled) {
        saveConfig();
        return true;
    }

    ShellyDevice device;
    uint8_t channel = 0;
    if (!getAssignedRelay(ShellyFunction::MainPower, &device, &channel)) {
        if (err) {
            *err = "Main Power relay not assigned";
        }
        return false;
    }

    const String timespecOn = buildTimespec(schedule.onTime, schedule.days);
    const String timespecOff = buildTimespec(schedule.offTime, schedule.days);
    bool ok = ensureScheduleOnDevice(device, channel, schedule.enabled, timespecOn, true, &schedule.onJobId, err);
    if (!ok) {
        return false;
    }
    ok = ensureScheduleOnDevice(device, channel, schedule.enabled, timespecOff, false, &schedule.offJobId, err);
    if (!ok) {
        return false;
    }

    saveConfig();
    updateScheduleSync();
    if (schedule.enabled) {
        Settings &settings = controller->getSettings();
        settings.setAutoWakeupEnabled(true);

        AutoWakeupSchedule autoSchedule;
        autoSchedule.time = schedule.onTime;
        for (int i = 0; i < 7; i++) {
            autoSchedule.days[i] = schedule.days[i];
        }
        settings.setAutoWakeupSchedules({autoSchedule});
        settings.save(true);
    }
    return true;
}

bool ShellyPlugin::setRelayState(ShellyFunction function, bool on, String *err) {
    ShellyDevice device;
    uint8_t channel = 0;
    if (!getAssignedRelay(function, &device, &channel)) {
        if (err) {
            *err = "Assigned relay not found";
        }
        return false;
    }

    JsonDocument params;
    params["id"] = channel;
    params["on"] = on;
    JsonDocument result;
    return rpcCall(device, "Switch.Set", params, result, err);
}

void ShellyPlugin::loadConfig() {
    Settings &settings = controller->getSettings();
    enabled = settings.isShellyEnabled();
    grinderEnabled = settings.isShellyGrinderEnabled();
    ledEnabled = settings.isShellyLedEnabled();
    mainPowerEnabled = settings.isShellyMainPowerEnabled();
    ledMode = static_cast<ShellyLedMode>(settings.getShellyLedMode());

    String devicesJson = settings.getShellyDevicesJson();
    if (devicesJson.length()) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, devicesJson);
        if (!err && doc.is<JsonArray>()) {
            devices.clear();
            for (JsonVariant v : doc.as<JsonArray>()) {
                ShellyDevice device;
                device.id = v["id"] | "";
                device.name = v["name"] | "";
                device.model = v["model"] | "";
                device.host = v["host"] | "";
                device.username = v["username"] | "";
                device.password = v["password"] | "";
                device.authEnabled = v["auth"] | false;
                device.channels = v["channels"] | 1;
                if (!device.id.isEmpty()) {
                    devices.push_back(device);
                }
            }
        }
    }

    String assignmentsJson = settings.getShellyAssignmentsJson();
    if (assignmentsJson.length()) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, assignmentsJson);
        if (!err && doc.is<JsonArray>()) {
            assignments.clear();
            for (JsonVariant v : doc.as<JsonArray>()) {
                ShellyAssignment assignment;
                assignment.deviceId = v["deviceId"] | "";
                assignment.channel = v["channel"] | 0;
                assignment.function = static_cast<ShellyFunction>(v["function"] | 0);
                if (!assignment.deviceId.isEmpty()) {
                    assignments.push_back(assignment);
                }
            }
        }
    }

    String scheduleJson = settings.getShellyScheduleJson();
    if (scheduleJson.length()) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, scheduleJson);
        if (!err && doc.is<JsonObject>()) {
            schedule.enabled = doc["enabled"] | false;
            schedule.onTime = doc["onTime"] | "06:30";
            schedule.offTime = doc["offTime"] | "10:30";
            for (int i = 0; i < 7; i++) {
                schedule.days[i] = doc["days"][i] | schedule.days[i];
            }
            schedule.onJobId = doc["onJobId"] | "";
            schedule.offJobId = doc["offJobId"] | "";
        }
    }
}

void ShellyPlugin::saveConfig() {
    Settings &settings = controller->getSettings();
    settings.setShellyEnabled(enabled);
    settings.setShellyGrinderEnabled(grinderEnabled);
    settings.setShellyLedEnabled(ledEnabled);
    settings.setShellyMainPowerEnabled(mainPowerEnabled);
    settings.setShellyLedMode(static_cast<int>(ledMode));

    {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (const auto &device : devices) {
            JsonObject obj = arr.createNestedObject();
            obj["id"] = device.id;
            obj["name"] = device.name;
            obj["model"] = device.model;
            obj["host"] = device.host;
            obj["username"] = device.username;
            obj["password"] = device.password;
            obj["auth"] = device.authEnabled;
            obj["channels"] = device.channels;
        }
        String out;
        serializeJson(doc, out);
        settings.setShellyDevicesJson(out);
    }

    {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (const auto &assignment : assignments) {
            JsonObject obj = arr.createNestedObject();
            obj["deviceId"] = assignment.deviceId;
            obj["channel"] = assignment.channel;
            obj["function"] = static_cast<uint8_t>(assignment.function);
        }
        String out;
        serializeJson(doc, out);
        settings.setShellyAssignmentsJson(out);
    }

    {
        JsonDocument doc;
        doc["enabled"] = schedule.enabled;
        doc["onTime"] = schedule.onTime;
        doc["offTime"] = schedule.offTime;
        JsonArray days = doc["days"].to<JsonArray>();
        for (int i = 0; i < 7; i++) {
            days.add(schedule.days[i]);
        }
        doc["onJobId"] = schedule.onJobId;
        doc["offJobId"] = schedule.offJobId;
        String out;
        serializeJson(doc, out);
        settings.setShellyScheduleJson(out);
    }

    settings.save(true);
}

String ShellyPlugin::buildTimespec(const String &time, const bool days[7]) const {
    int hour = 0;
    int minute = 0;
    const int colon = time.indexOf(':');
    if (colon >= 0) {
        hour = time.substring(0, colon).toInt();
        minute = time.substring(colon + 1).toInt();
    }

    String daySpec;
    for (int i = 0; i < 7; i++) {
        if (!days[i]) {
            continue;
        }
        int shellyDay = i == 6 ? 0 : i + 1;
        if (!daySpec.isEmpty()) {
            daySpec += ",";
        }
        daySpec += String(shellyDay);
    }
    if (daySpec.isEmpty()) {
        daySpec = "*";
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "0 %d %d * * %s", minute, hour, daySpec.c_str());
    return String(buffer);
}

bool ShellyPlugin::rpcCall(const ShellyDevice &device, const char *method, JsonDocument &params, JsonDocument &result, String *err) {
    if (WiFi.status() != WL_CONNECTED) {
        if (err) {
            *err = "WiFi not connected";
        }
        return false;
    }

    HTTPClient http;
    http.setTimeout(kShellyRpcTimeoutMs);
    String url = "http://" + device.host + "/rpc";
    if (!http.begin(url)) {
        if (err) {
            *err = "HTTP begin failed";
        }
        return false;
    }

    if (device.authEnabled && device.password.length()) {
        http.setAuthorization(device.username.c_str(), device.password.c_str());
    }

    JsonDocument payload;
    payload["id"] = 1;
    payload["method"] = method;
    if (!params.isNull()) {
        payload["params"] = params;
    }

    String body;
    serializeJson(payload, body);
    int code = http.POST(body);
    if (code == 401 || code == 403) {
        if (err) {
            *err = "Authentication required";
        }
        http.end();
        return false;
    }
    if (code <= 0) {
        if (err) {
            *err = "RPC request failed";
        }
        http.end();
        return false;
    }

    String response = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError jsonErr = deserializeJson(doc, response);
    if (jsonErr) {
        if (err) {
            *err = "Invalid RPC response";
        }
        return false;
    }
    if (doc["error"].is<JsonObject>()) {
        if (err) {
            *err = doc["error"]["message"] | "RPC error";
        }
        return false;
    }

    result.set(doc["result"]);
    return true;
}

bool ShellyPlugin::fetchDeviceInfo(const String &host, const String &username, const String &password, ShellyDevice *outDevice,
                                   bool *authRequired, String *err) {
    ShellyDevice device;
    device.host = host;
    device.username = username;
    device.password = password;
    device.authEnabled = !password.isEmpty();

    JsonDocument params;
    JsonDocument result;
    String tmpErr;
    if (!rpcCall(device, "Shelly.GetDeviceInfo", params, result, &tmpErr)) {
        if (authRequired) {
            *authRequired = tmpErr.indexOf("Authentication") >= 0 || tmpErr.indexOf("auth") >= 0;
        }
        if (err) {
            *err = tmpErr;
        }
        return false;
    }

    if (authRequired) {
        *authRequired = false;
    }

    device.id = result["id"] | "";
    device.name = result["name"] | "";
    device.model = result["model"] | "";
    device.authEnabled = !password.isEmpty();

    *outDevice = device;
    return true;
}

bool ShellyPlugin::detectChannelCount(const ShellyDevice &device, uint8_t *count, String *err) {
    uint8_t detected = 0;
    for (uint8_t i = 0; i < 4; i++) {
        JsonDocument params;
        params["id"] = i;
        JsonDocument result;
        String tmpErr;
        if (rpcCall(device, "Switch.GetStatus", params, result, &tmpErr)) {
            detected = i + 1;
        } else {
            break;
        }
    }

    if (detected == 0) {
        if (err) {
            *err = "No switch channels detected";
        }
        return false;
    }

    if (count) {
        *count = detected;
    }
    return true;
}

bool ShellyPlugin::ensureScheduleOnDevice(const ShellyDevice &device, uint8_t channel, bool enabled, const String &timespec,
                                          bool on, String *jobId, String *err) {
    if (!enabled) {
        if (jobId && jobId->length()) {
            return deleteScheduleJob(device, *jobId, err);
        }
        return true;
    }

    JsonDocument call;
    call["method"] = "Switch.Set";
    JsonObject params = call["params"].to<JsonObject>();
    params["id"] = channel;
    params["on"] = on;

    JsonDocument paramsDoc;
    paramsDoc["timespec"] = timespec;
    paramsDoc["calls"] = JsonArray();
    JsonArray calls = paramsDoc["calls"].to<JsonArray>();
    calls.add(call);

    JsonDocument result;
    if (jobId && jobId->length()) {
        paramsDoc["id"] = *jobId;
        if (rpcCall(device, "Schedule.Update", paramsDoc, result, err)) {
            return true;
        }
    }

    if (!rpcCall(device, "Schedule.Create", paramsDoc, result, err)) {
        return false;
    }
    if (jobId) {
        *jobId = result["id"] | "";
    }
    return true;
}

bool ShellyPlugin::deleteScheduleJob(const ShellyDevice &device, const String &jobId, String *err) {
    if (jobId.isEmpty()) {
        return true;
    }
    JsonDocument params;
    params["id"] = jobId;
    JsonDocument result;
    return rpcCall(device, "Schedule.Delete", params, result, err);
}

void ShellyPlugin::updateScheduleSync() {
    scheduleSyncStatus = "unknown";
    scheduleSyncMessage = "";

    if (!enabled || !mainPowerEnabled) {
        scheduleSyncStatus = "disabled";
        scheduleSyncMessage = "Main power scheduling disabled";
        return;
    }

    ShellyDevice device;
    uint8_t channel = 0;
    if (!getAssignedRelay(ShellyFunction::MainPower, &device, &channel)) {
        scheduleSyncStatus = "error";
        scheduleSyncMessage = "Main Power relay not assigned";
        return;
    }

    JsonDocument params;
    JsonDocument result;
    String err;
    if (!rpcCall(device, "Schedule.List", params, result, &err)) {
        scheduleSyncStatus = "error";
        scheduleSyncMessage = err;
        return;
    }

    const String timespecOn = buildTimespec(schedule.onTime, schedule.days);
    const String timespecOff = buildTimespec(schedule.offTime, schedule.days);

    bool onMatch = false;
    bool offMatch = false;
    JsonArray jobs = result["jobs"].as<JsonArray>();
    for (JsonVariant v : jobs) {
        String id = v["id"] | "";
        String timespec = v["timespec"] | "";
        if (id == schedule.onJobId && timespec == timespecOn) {
            onMatch = true;
        }
        if (id == schedule.offJobId && timespec == timespecOff) {
            offMatch = true;
        }
    }

    if (schedule.enabled && onMatch && offMatch) {
        scheduleSyncStatus = "in_sync";
        scheduleSyncMessage = "Schedule matches Shelly";
    } else if (!schedule.enabled) {
        scheduleSyncStatus = "disabled";
        scheduleSyncMessage = "Schedule disabled";
    } else {
        scheduleSyncStatus = "out_of_sync";
        scheduleSyncMessage = "Shelly schedule differs";
    }
}

bool ShellyPlugin::getAssignedRelay(ShellyFunction function, ShellyDevice *device, uint8_t *channel) {
    for (const auto &assignment : assignments) {
        if (assignment.function != function) {
            continue;
        }
        for (const auto &entry : devices) {
            if (entry.id == assignment.deviceId) {
                if (device) {
                    *device = entry;
                }
                if (channel) {
                    *channel = assignment.channel;
                }
                return true;
            }
        }
    }
    return false;
}

bool ShellyPlugin::validateAssignments(const std::vector<ShellyAssignment> &newAssignments, String *err) const {
    std::map<ShellyFunction, String> functionMap;
    std::map<String, std::map<uint8_t, ShellyFunction>> relayMap;

    for (const auto &assignment : newAssignments) {
        if (assignment.function == ShellyFunction::None) {
            continue;
        }
        auto funcIt = functionMap.find(assignment.function);
        if (funcIt != functionMap.end()) {
            if (err) {
                *err = "Function assigned to multiple relays";
            }
            return false;
        }
        functionMap[assignment.function] = assignment.deviceId;

        ShellyFunction &existing = relayMap[assignment.deviceId][assignment.channel];
        if (existing != ShellyFunction::None) {
            if (err) {
                *err = "Relay assigned to multiple functions";
            }
            return false;
        }
        existing = assignment.function;
    }
    return true;
}

void ShellyPlugin::handleStandbyEnter() {
    if (!enabled || !ledEnabled) {
        return;
    }
    if (ledMode == ShellyLedMode::ActiveOnly) {
        String err;
        setRelayState(ShellyFunction::PowerLed, false, &err);
    }
}

void ShellyPlugin::handleStandbyExit() {
    if (!enabled || !ledEnabled) {
        return;
    }
    if (ledMode == ShellyLedMode::ActiveOnly) {
        String err;
        setRelayState(ShellyFunction::PowerLed, true, &err);
    } else if (ledMode == ShellyLedMode::WithMainPower) {
        String err;
        setRelayState(ShellyFunction::PowerLed, true, &err);
    }
}

void ShellyPlugin::handleGrindStart() {
    if (!enabled || !grinderEnabled) {
        return;
    }
    String err;
    setRelayState(ShellyFunction::Grinder, true, &err);
}

void ShellyPlugin::handleGrindEnd() {
    if (!enabled || !grinderEnabled) {
        return;
    }
    String err;
    setRelayState(ShellyFunction::Grinder, false, &err);
}
