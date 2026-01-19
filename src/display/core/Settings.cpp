#include "Settings.h"
#include "SdBackup.h"

#include <algorithm>
#include <utility>

Settings::Settings() {
    preferences.begin(PREFERENCES_KEY, true);
    startupMode = preferences.getInt("sm", MODE_STANDBY);
    targetBrewTemp = preferences.getInt("tb", 90);
    targetSteamTemp = preferences.getInt("ts", 145);
    targetWaterTemp = preferences.getInt("tw", 80);
    targetDuration = preferences.getInt("td", 25000);
    targetVolume = preferences.getInt("tv", 36);
    targetGrindVolume = preferences.getDouble("tgv", 18.0);
    targetGrindDuration = preferences.getInt("tgd", 25000);
    doseMeasureEnabled = preferences.getBool("dm_en", true);
    doseAvgBeanWeight = preferences.getDouble("dm_bw", 0.1);
    doseTarget = preferences.getDouble("dm_tg", 18.5);
    doseCupEnabled = preferences.getBool("dm_ce", false);
    doseCupEmptyWeight = preferences.getDouble("dm_cw", 0.0);
    doseBeepEnabled = preferences.getBool("dm_bp", false);
    doseProceedBeanCount = preferences.getInt("dm_pc", 3);
    doseBeanCountLimit = preferences.getInt("dm_bcl", 20);
    doseDefaultDoseCount = preferences.getInt("dm_dc", 1);
    brewDelay = preferences.getDouble("del_br", 1000.0);
    grindDelay = preferences.getDouble("del_gd", 1000.0);
    delayAdjust = preferences.getBool("del_ad", true);
    temperatureOffset = preferences.getInt("to", DEFAULT_TEMPERATURE_OFFSET);
    pressureScaling = preferences.getFloat("ps", DEFAULT_PRESSURE_SCALING);
    pid = preferences.getString("pid", DEFAULT_PID);
    pumpModelCoeffs = preferences.getString("pmc", DEFAULT_PUMP_MODEL_COEFFS);
    wifiSsid = preferences.getString("ws", "");
    wifiPassword = preferences.getString("wp", "");
    mdnsName = preferences.getString("mn", DEFAULT_MDNS_NAME);
    homekit = preferences.getBool("hk", false);
    volumetricTarget = preferences.getBool("vt", true);
    otaChannel = preferences.getString("oc", DEFAULT_OTA_CHANNEL);
    infusePumpTime = preferences.getInt("ipt", 0);
    infuseBloomTime = preferences.getInt("ibt", 0);
    pressurizeTime = preferences.getInt("pt", 0);
    savedScale = preferences.getString("ssc", "");
    momentaryButtons = preferences.getBool("mb", false);
    boilerFillActive = preferences.getBool("bf_a", false);
    startupFillTime = preferences.getInt("bf_su", 5000);
    steamFillTime = preferences.getInt("bf_st", 5000);
    smartGrindActive = preferences.getBool("sg_a", false);
    smartGrindIp = preferences.getString("sg_i", "");
    smartGrindToggle = preferences.getBool("sg_t", false);
    smartGrindMode = preferences.getInt("sg_m", smartGrindToggle ? 1 : 0);
    homeAssistant = preferences.getBool("ha_a", false);
    homeAssistantIP = preferences.getString("ha_i", "");
    homeAssistantPort = preferences.getInt("ha_p", 1883);
    homeAssistantTopic = preferences.getString("ha_t", DEFAULT_HOME_ASSISTANT_TOPIC);
    homeAssistantUser = preferences.getString("ha_u", "");
    homeAssistantPassword = preferences.getString("ha_pw", "");
    standbyTimeout = preferences.getInt("sbt", DEFAULT_STANDBY_TIMEOUT_MS);
    screensaverEnabled = preferences.getBool("ss_en", true);
    screensaverTimeout = preferences.getInt("ss_to", 120000);
    restoreDoneOnce = preferences.getBool("rd_once", false);
    timezone = preferences.getString("tz", DEFAULT_TIMEZONE);
    clock24hFormat = preferences.getBool("clk_24h", true);
    selectedProfile = preferences.getString("sp", "");
    profilesMigrated = preferences.getBool("pm", false);
    favoritedProfiles = explode(preferences.getString("fp", ""), ',');
    profileOrder = explode(preferences.getString("po", ""), ',');
    steamPumpPercentage = preferences.getFloat("spp", DEFAULT_STEAM_PUMP_PERCENTAGE);
    steamPumpCutoff = preferences.getFloat("spc", DEFAULT_STEAM_PUMP_CUTOFF);
    historyIndex = preferences.getInt("hi", 0);
    autowakeupEnabled = preferences.getBool("ab_en", false);
    shellyEnabled = preferences.getBool("sh_en", false);
    shellyGrinderEnabled = preferences.getBool("sh_gr", false);
    shellyLedEnabled = preferences.getBool("sh_led", false);
    shellyMainPowerEnabled = preferences.getBool("sh_mp", false);
    shellyLedMode = preferences.getInt("sh_lm", 0);
    shellyDevicesJson = preferences.getString("sh_dev", "[]");
    shellyAssignmentsJson = preferences.getString("sh_asg", "[]");
    shellyScheduleJson = preferences.getString("sh_sch", "{}");

    // Load schedule format: "time1|days1;time2|days2" where days is 7-bit string (e.g., "1111100" for weekdays only)
    String schedulesStr = preferences.getString("ab_schedules", "");
    autowakeupSchedules.clear();

    if (schedulesStr.length() > 0) {
        int start = 0;
        int end = schedulesStr.indexOf(';');

        while (end != -1 || start < schedulesStr.length()) {
            String scheduleStr = (end != -1) ? schedulesStr.substring(start, end) : schedulesStr.substring(start);

            int pipePos = scheduleStr.indexOf('|');
            if (pipePos != -1) {
                String timeStr = scheduleStr.substring(0, pipePos);
                String daysStr = scheduleStr.substring(pipePos + 1);

                AutoWakeupSchedule schedule;
                schedule.time = timeStr;

                if (daysStr.length() == 7) {
                    for (int i = 0; i < 7; i++) {
                        schedule.days[i] = (daysStr.charAt(i) == '1');
                    }
                }

                autowakeupSchedules.push_back(schedule);
            }

            if (end == -1)
                break;
            start = end + 1;
            end = schedulesStr.indexOf(';', start);
        }
    }

    if (autowakeupSchedules.empty()) {
        autowakeupSchedules.push_back(AutoWakeupSchedule("07:00"));
    }

    // Display settings
    mainBrightness = preferences.getInt("main_b", 16);
    standbyBrightness = preferences.getInt("standby_b", 8);
    standbyBrightnessTimeout = preferences.getInt("standby_bt", 60000);
    standbyLandingScreen = preferences.getString("standby_ls", "menu");
    wifiApTimeout = preferences.getInt("wifi_apt", DEFAULT_WIFI_AP_TIMEOUT_MS);
    themeMode = preferences.getInt("theme", 0);

    // Sunrise settings
    sunriseR = preferences.getInt("sr_r", 0);
    sunriseG = preferences.getInt("sr_g", 0);
    sunriseB = preferences.getInt("sr_b", 255);
    sunriseW = preferences.getInt("sr_w", 50);
    sunriseExtBrightness = preferences.getInt("sr_exb", 255);
    emptyTankDistance = preferences.getInt("sr_ed", 200);
    fullTankDistance = preferences.getInt("sr_fd", 50);
    altRelayFunction = preferences.getInt("alt_relay", ALT_RELAY_GRIND);

    preferences.end();

    xTaskCreate(loopTask, "Settings::loop", configMINIMAL_STACK_SIZE * 6, this, 1, &taskHandle);
}

void Settings::batchUpdate(const SettingsCallback &callback) {
    callback(this);
    save();
}

void Settings::save(bool noDelay) {
    if (noDelay) {
        doSave();
        return;
    }
    dirty = true;
}

void Settings::setTargetBrewTemp(const int target_brew_temp) {
    targetBrewTemp = target_brew_temp;
    save();
}

void Settings::setTargetSteamTemp(const int target_steam_temp) {
    targetSteamTemp = target_steam_temp;
    save();
}

void Settings::setTargetWaterTemp(const int target_water_temp) {
    targetWaterTemp = target_water_temp;
    save();
}

void Settings::setTemperatureOffset(const int temperature_offset) {
    temperatureOffset = temperature_offset;
    save();
}

void Settings::setPressureScaling(const float pressure_scaling) {
    pressureScaling = pressure_scaling;
    save();
}

void Settings::setTargetDuration(const int target_duration) {
    targetDuration = target_duration;
    save();
}

void Settings::setTargetVolume(int target_volume) {
    targetVolume = target_volume;
    save();
}

void Settings::setTargetGrindVolume(double target_grind_volume) {
    targetGrindVolume = target_grind_volume;
    save();
}

void Settings::setTargetGrindDuration(const int target_duration) {
    targetGrindDuration = target_duration;
    save();
}

void Settings::setDoseMeasureEnabled(bool enabled) {
    doseMeasureEnabled = enabled;
    save();
}

void Settings::setDoseMeasureAvgBeanWeight(double avg_bean_weight) {
    doseAvgBeanWeight = std::max(0.0, avg_bean_weight);
    save();
}

void Settings::setDoseMeasureTarget(double target_weight) {
    doseTarget = std::max(0.0, target_weight);
    save();
}

void Settings::setDoseMeasureCupEnabled(bool enabled) {
    doseCupEnabled = enabled;
    save();
}

void Settings::setDoseMeasureCupEmptyWeight(double empty_weight) {
    doseCupEmptyWeight = std::max(0.0, empty_weight);
    save();
}

void Settings::setDoseMeasureBeepEnabled(bool enabled) {
    doseBeepEnabled = enabled;
    save();
}


void Settings::setDoseMeasureProceedBeanCount(int bean_count) {
    doseProceedBeanCount = std::clamp(bean_count, 0, 50);
    save();
}

void Settings::setDoseMeasureBeanCountLimit(int bean_count) {
    doseBeanCountLimit = std::clamp(bean_count, 0, 50);
    save();
}

void Settings::setDoseMeasureDefaultDoseCount(int dose_count) {
    doseDefaultDoseCount = std::clamp(dose_count, 1, 5);
    save();
}

void Settings::setBrewDelay(double brew_Delay) {
    brewDelay = std::clamp(brew_Delay, 0.0, 4000.0);
    save();
}

void Settings::setGrindDelay(double grind_Delay) {
    grindDelay = std::clamp(grind_Delay, 0.0, 4000.0);
    save();
}

void Settings::setDelayAdjust(bool delay_adjust) {
    delayAdjust = delay_adjust;
    save();
}

void Settings::setStartupMode(const int startup_mode) {
    startupMode = startup_mode;
    save();
}

void Settings::setStandbyTimeout(int standby_timeout) {
    standbyTimeout = standby_timeout;
    save();
}

void Settings::setScreensaverEnabled(bool enabled) {
    screensaverEnabled = enabled;
    save();
}

void Settings::setScreensaverTimeout(int screensaver_timeout) {
    screensaverTimeout = screensaver_timeout;
    save();
}

void Settings::setRestoreDoneOnce(bool done) {
    restoreDoneOnce = done;
    save();
}

void Settings::setInfuseBloomTime(int infuse_bloom_time) {
    infuseBloomTime = infuse_bloom_time;
    save();
}

void Settings::setInfusePumpTime(int infuse_pump_time) {
    infusePumpTime = infuse_pump_time;
    save();
}

void Settings::setPressurizeTime(int pressurize_time) {
    pressurizeTime = pressurize_time;
    save();
}

void Settings::setPid(const String &pid) {
    this->pid = pid;
    save();
}

void Settings::setPumpModelCoeffs(const String &pumpModelCoeffs) {
    this->pumpModelCoeffs = pumpModelCoeffs;
    save();
}

void Settings::setWifiSsid(const String &wifiSsid) {
    String current = this->wifiSsid;
    String next = wifiSsid;
    current.trim();
    next.trim();
    const bool wasBlank = current.length() == 0;
    const bool isBlank = next.length() == 0;
    this->wifiSsid = wifiSsid;
    if (!isBlank || (!wasBlank && isBlank)) {
        restoreDoneOnce = false;
    }
    save();
}

void Settings::setWifiPassword(const String &wifiPassword) {
    this->wifiPassword = wifiPassword;
    save();
}

void Settings::setMdnsName(const String &mdnsName) {
    this->mdnsName = mdnsName;
    save();
}

void Settings::setHomekit(const bool homekit) {
    this->homekit = homekit;
    save();
}

void Settings::setVolumetricTarget(bool volumetric_target) {
    this->volumetricTarget = volumetric_target;
    save();
}

void Settings::setOTAChannel(const String &otaChannel) {
    this->otaChannel = otaChannel;
    save();
}

void Settings::setSavedScale(const String &savedScale) {
    this->savedScale = savedScale;
    save();
}

void Settings::setBoilerFillActive(bool boiler_fill_active) {
    boilerFillActive = boiler_fill_active;
    save();
}

void Settings::setStartupFillTime(int startup_fill_time) {
    startupFillTime = startup_fill_time;
    save();
}

void Settings::setSteamFillTime(int steam_fill_time) {
    steamFillTime = steam_fill_time;
    save();
}

void Settings::setSmartGrindActive(bool smart_grind_active) {
    smartGrindActive = smart_grind_active;
    save();
}

void Settings::setSmartGrindIp(String smart_grind_ip) {
    this->smartGrindIp = std::move(smart_grind_ip);
    save();
}

void Settings::setSmartGrindMode(int smart_grind_mode) {
    this->smartGrindMode = smart_grind_mode;
    save();
}

void Settings::setHomeAssistant(const bool homeAssistant) {
    this->homeAssistant = homeAssistant;
    save();
}

void Settings::setHomeAssistantIP(const String &homeAssistantIP) {
    this->homeAssistantIP = homeAssistantIP;
    save();
}

void Settings::setHomeAssistantPort(const int homeAssistantPort) {
    this->homeAssistantPort = homeAssistantPort;
    save();
}
void Settings::setHomeAssistantTopic(const String &homeAssistantTopic) {
    this->homeAssistantTopic = homeAssistantTopic;
    save();
}
void Settings::setHomeAssistantUser(const String &homeAssistantUser) {
    this->homeAssistantUser = homeAssistantUser;
    save();
}
void Settings::setHomeAssistantPassword(const String &homeAssistantPassword) {
    this->homeAssistantPassword = homeAssistantPassword;
    save();
}

void Settings::setMomentaryButtons(bool momentary_buttons) {
    momentaryButtons = momentary_buttons;
    save();
}

void Settings::setTimezone(String timezone) {
    this->timezone = std::move(timezone);
    save();
}

void Settings::setClockFormat(bool clock_24h_format) {
    this->clock24hFormat = clock_24h_format;
    save();
}

void Settings::setSelectedProfile(String selected_profile) {
    this->selectedProfile = std::move(selected_profile);
    save();
}

void Settings::setProfilesMigrated(bool profiles_migrated) {
    profilesMigrated = profiles_migrated;
    save();
}

void Settings::setFavoritedProfiles(std::vector<String> favorited_profiles) {
    favoritedProfiles = std::move(favorited_profiles);
    save();
}

void Settings::addFavoritedProfile(String profile) {
    favoritedProfiles.emplace_back(profile);
    save();
}

void Settings::removeFavoritedProfile(String profile) {
    favoritedProfiles.erase(std::remove(favoritedProfiles.begin(), favoritedProfiles.end(), profile), favoritedProfiles.end());
    favoritedProfiles.shrink_to_fit();
    save();
}

void Settings::setProfileOrder(std::vector<String> profile_order) {
    std::vector<String> cleaned;
    cleaned.reserve(profile_order.size());
    for (auto &id : profile_order) {
        if (id.isEmpty())
            continue;
        if (std::find(cleaned.begin(), cleaned.end(), id) == cleaned.end()) {
            cleaned.emplace_back(std::move(id));
        }
    }

    profileOrder = std::move(cleaned);
    save();
}

void Settings::setMainBrightness(int main_brightness) {
    mainBrightness = main_brightness;
    save();
}

void Settings::setStandbyBrightness(int standby_brightness) {
    standbyBrightness = standby_brightness;
    save();
}

void Settings::setStandbyBrightnessTimeout(int standby_brightness_timeout) {
    standbyBrightnessTimeout = standby_brightness_timeout;
    save();
}

void Settings::setStandbyLandingScreen(const String &screen) {
    standbyLandingScreen = screen;
    save();
}

void Settings::setWifiApTimeout(int timeout) {
    wifiApTimeout = timeout;
    save();
}

void Settings::setSteamPumpPercentage(float steam_pump_percentage) {
    steamPumpPercentage = steam_pump_percentage;
    save();
}

void Settings::setSteamPumpCutoff(float steam_pump_cutoff) {
    steamPumpCutoff = steam_pump_cutoff;
    save();
}

void Settings::setThemeMode(int theme_mode) {
    themeMode = theme_mode;
    save();
}

void Settings::setHistoryIndex(int history_index) {
    historyIndex = history_index;
    save();
}

void Settings::setSunriseR(int sunrise_r) {
    sunriseR = sunrise_r;
    save();
}

void Settings::setSunriseG(int sunrise_g) {
    sunriseG = sunrise_g;
    save();
}

void Settings::setSunriseB(int sunrise_b) {
    sunriseB = sunrise_b;
    save();
}

void Settings::setSunriseW(int sunrise_w) {
    sunriseW = sunrise_w;
    save();
}

void Settings::setSunriseExtBrightness(int sunrise_ext_brightness) {
    sunriseExtBrightness = sunrise_ext_brightness;
    save();
}

void Settings::setEmptyTankDistance(int empty_tank_distance) {
    emptyTankDistance = empty_tank_distance;
    save();
}

void Settings::setFullTankDistance(int full_tank_distance) {
    fullTankDistance = full_tank_distance;
    save();
}

void Settings::setAltRelayFunction(int alt_relay_function) { altRelayFunction = alt_relay_function; }

void Settings::setAutoWakeupEnabled(bool enabled) {
    autowakeupEnabled = enabled;
    save();
}

void Settings::setAutoWakeupSchedules(const std::vector<AutoWakeupSchedule> &schedules) {
    autowakeupSchedules = schedules;
    save();
}

void Settings::setShellyEnabled(bool enabled) {
    shellyEnabled = enabled;
}

void Settings::setShellyGrinderEnabled(bool enabled) {
    shellyGrinderEnabled = enabled;
}

void Settings::setShellyLedEnabled(bool enabled) {
    shellyLedEnabled = enabled;
}

void Settings::setShellyMainPowerEnabled(bool enabled) {
    shellyMainPowerEnabled = enabled;
}

void Settings::setShellyLedMode(int mode) {
    shellyLedMode = mode;
}

void Settings::setShellyDevicesJson(String json) {
    shellyDevicesJson = std::move(json);
}

void Settings::setShellyAssignmentsJson(String json) {
    shellyAssignmentsJson = std::move(json);
}

void Settings::setShellyScheduleJson(String json) {
    shellyScheduleJson = std::move(json);
}

void Settings::fillJson(JsonObject obj) const {
    obj["startupMode"] = startupMode == MODE_BREW ? "brew" : "standby";
    obj["targetSteamTemp"] = targetSteamTemp;
    obj["targetWaterTemp"] = targetWaterTemp;
    obj["homekit"] = homekit;
    obj["volumetricTarget"] = volumetricTarget;
    obj["otaChannel"] = otaChannel;
    obj["savedScale"] = savedScale;
    obj["homeAssistant"] = homeAssistant;
    obj["haUser"] = homeAssistantUser;
    obj["haPassword"] = homeAssistantPassword;
    obj["haIP"] = homeAssistantIP;
    obj["haPort"] = homeAssistantPort;
    obj["haTopic"] = homeAssistantTopic;
    obj["pid"] = pid;
    obj["pumpModelCoeffs"] = pumpModelCoeffs;
    obj["wifiSsid"] = wifiSsid;
    obj["wifiPassword"] = wifiPassword;
    obj["mdnsName"] = mdnsName;
    obj["temperatureOffset"] = String(temperatureOffset);
    obj["pressureScaling"] = String(pressureScaling);
    obj["boilerFillActive"] = boilerFillActive;
    obj["startupFillTime"] = startupFillTime / 1000;
    obj["steamFillTime"] = steamFillTime / 1000;
    obj["smartGrindActive"] = smartGrindActive;
    obj["smartGrindToggle"] = smartGrindToggle;
    obj["smartGrindIp"] = smartGrindIp;
    obj["smartGrindMode"] = smartGrindMode;
    obj["doseMeasureEnabled"] = doseMeasureEnabled;
    obj["doseMeasureAvgBeanWeight"] = doseAvgBeanWeight;
    obj["doseMeasureTarget"] = doseTarget;
    obj["doseMeasureCupEnabled"] = doseCupEnabled;
    obj["doseMeasureCupEmptyWeight"] = doseCupEmptyWeight;
    obj["doseMeasureBeepEnabled"] = doseBeepEnabled;
    obj["doseMeasureProceedBeanCount"] = doseProceedBeanCount;
    obj["doseMeasureBeanCountLimit"] = doseBeanCountLimit;
    obj["doseMeasureDefaultDoseCount"] = doseDefaultDoseCount;
    obj["momentaryButtons"] = momentaryButtons;
    obj["brewDelay"] = brewDelay;
    obj["grindDelay"] = grindDelay;
    obj["delayAdjust"] = delayAdjust;
    obj["timezone"] = timezone;
    obj["clock24hFormat"] = clock24hFormat;
    obj["selectedProfile"] = selectedProfile;
    obj["standbyTimeout"] = standbyTimeout / 1000;
    obj["screensaverEnabled"] = screensaverEnabled;
    obj["screensaverTimeout"] = screensaverTimeout / 60000.0f;
    obj["mainBrightness"] = mainBrightness;
    obj["standbyBrightness"] = standbyBrightness;
    obj["standbyBrightnessTimeout"] = standbyBrightnessTimeout / 1000;
    obj["standbyLandingScreen"] = standbyLandingScreen;
    obj["steamPumpPercentage"] = steamPumpPercentage;
    obj["steamPumpCutoff"] = steamPumpCutoff;
    obj["themeMode"] = themeMode;
    obj["sunriseR"] = sunriseR;
    obj["sunriseG"] = sunriseG;
    obj["sunriseB"] = sunriseB;
    obj["sunriseW"] = sunriseW;
    obj["sunriseExtBrightness"] = sunriseExtBrightness;
    obj["emptyTankDistance"] = emptyTankDistance;
    obj["fullTankDistance"] = fullTankDistance;
    obj["altRelayFunction"] = altRelayFunction;
    obj["autowakeupEnabled"] = autowakeupEnabled;
    obj["shellyEnabled"] = shellyEnabled;
    obj["shellyGrinderEnabled"] = shellyGrinderEnabled;
    obj["shellyLedEnabled"] = shellyLedEnabled;
    obj["shellyMainPowerEnabled"] = shellyMainPowerEnabled;
    obj["shellyLedMode"] = shellyLedMode;
    obj["shellyDevicesJson"] = shellyDevicesJson;
    obj["shellyAssignmentsJson"] = shellyAssignmentsJson;
    obj["shellyScheduleJson"] = shellyScheduleJson;

    String schedulesStr = "";
    for (size_t i = 0; i < autowakeupSchedules.size(); i++) {
        if (i > 0)
            schedulesStr += ";";
        schedulesStr += autowakeupSchedules[i].time + "|";
        for (int j = 0; j < 7; j++) {
            schedulesStr += autowakeupSchedules[i].days[j] ? "1" : "0";
        }
    }
    obj["autowakeupSchedules"] = schedulesStr;
}

void Settings::applyJson(const JsonObject &obj) {
    if (obj.containsKey("startupMode")) {
        String mode = obj["startupMode"].as<String>();
        startupMode = mode == "brew" ? MODE_BREW : MODE_STANDBY;
    }
    if (obj.containsKey("targetSteamTemp"))
        targetSteamTemp = obj["targetSteamTemp"].as<int>();
    if (obj.containsKey("targetWaterTemp"))
        targetWaterTemp = obj["targetWaterTemp"].as<int>();
    if (obj.containsKey("homekit"))
        homekit = obj["homekit"].as<bool>();
    if (obj.containsKey("volumetricTarget"))
        volumetricTarget = obj["volumetricTarget"].as<bool>();
    if (obj.containsKey("otaChannel"))
        otaChannel = obj["otaChannel"].as<String>();
    if (obj.containsKey("savedScale"))
        savedScale = obj["savedScale"].as<String>();
    if (obj.containsKey("temperatureOffset"))
        temperatureOffset = obj["temperatureOffset"].as<int>();
    if (obj.containsKey("pressureScaling"))
        pressureScaling = obj["pressureScaling"].as<float>();
    if (obj.containsKey("pid"))
        pid = obj["pid"].as<String>();
    if (obj.containsKey("pumpModelCoeffs"))
        pumpModelCoeffs = obj["pumpModelCoeffs"].as<String>();
    if (obj.containsKey("wifiSsid"))
        wifiSsid = obj["wifiSsid"].as<String>();
    if (obj.containsKey("wifiPassword"))
        wifiPassword = obj["wifiPassword"].as<String>();
    if (obj.containsKey("mdnsName"))
        mdnsName = obj["mdnsName"].as<String>();
    if (obj.containsKey("boilerFillActive"))
        boilerFillActive = obj["boilerFillActive"].as<bool>();
    if (obj.containsKey("startupFillTime"))
        startupFillTime = obj["startupFillTime"].as<int>() * 1000;
    if (obj.containsKey("steamFillTime"))
        steamFillTime = obj["steamFillTime"].as<int>() * 1000;
    if (obj.containsKey("smartGrindActive"))
        smartGrindActive = obj["smartGrindActive"].as<bool>();
    if (obj.containsKey("smartGrindIp"))
        smartGrindIp = obj["smartGrindIp"].as<String>();
    if (obj.containsKey("smartGrindMode"))
        smartGrindMode = obj["smartGrindMode"].as<int>();
    if (obj.containsKey("smartGrindToggle"))
        smartGrindToggle = obj["smartGrindToggle"].as<bool>();
    if (obj.containsKey("doseMeasureEnabled"))
        doseMeasureEnabled = obj["doseMeasureEnabled"].as<bool>();
    if (obj.containsKey("doseMeasureAvgBeanWeight"))
        doseAvgBeanWeight = obj["doseMeasureAvgBeanWeight"].as<double>();
    if (obj.containsKey("doseMeasureTarget"))
        doseTarget = obj["doseMeasureTarget"].as<double>();
    if (obj.containsKey("doseMeasureCupEnabled"))
        doseCupEnabled = obj["doseMeasureCupEnabled"].as<bool>();
    if (obj.containsKey("doseMeasureCupEmptyWeight"))
        doseCupEmptyWeight = obj["doseMeasureCupEmptyWeight"].as<double>();
    if (obj.containsKey("doseMeasureBeepEnabled"))
        doseBeepEnabled = obj["doseMeasureBeepEnabled"].as<bool>();
    if (obj.containsKey("doseMeasureProceedBeanCount"))
        doseProceedBeanCount = obj["doseMeasureProceedBeanCount"].as<int>();
    if (obj.containsKey("doseMeasureBeanCountLimit"))
        doseBeanCountLimit = obj["doseMeasureBeanCountLimit"].as<int>();
    if (obj.containsKey("doseMeasureDefaultDoseCount"))
        doseDefaultDoseCount = obj["doseMeasureDefaultDoseCount"].as<int>();
    if (obj.containsKey("homeAssistant"))
        homeAssistant = obj["homeAssistant"].as<bool>();
    if (obj.containsKey("haUser"))
        homeAssistantUser = obj["haUser"].as<String>();
    if (obj.containsKey("haPassword"))
        homeAssistantPassword = obj["haPassword"].as<String>();
    if (obj.containsKey("haIP"))
        homeAssistantIP = obj["haIP"].as<String>();
    if (obj.containsKey("haPort"))
        homeAssistantPort = obj["haPort"].as<int>();
    if (obj.containsKey("haTopic"))
        homeAssistantTopic = obj["haTopic"].as<String>();
    if (obj.containsKey("momentaryButtons"))
        momentaryButtons = obj["momentaryButtons"].as<bool>();
    if (obj.containsKey("delayAdjust"))
        delayAdjust = obj["delayAdjust"].as<bool>();
    if (obj.containsKey("brewDelay"))
        brewDelay = obj["brewDelay"].as<double>();
    if (obj.containsKey("grindDelay"))
        grindDelay = obj["grindDelay"].as<double>();
    if (obj.containsKey("timezone"))
        timezone = obj["timezone"].as<String>();
    if (obj.containsKey("clock24hFormat"))
        clock24hFormat = obj["clock24hFormat"].as<bool>();
    if (obj.containsKey("selectedProfile"))
        selectedProfile = obj["selectedProfile"].as<String>();
    if (obj.containsKey("standbyTimeout"))
        standbyTimeout = obj["standbyTimeout"].as<int>() * 1000;
    if (obj.containsKey("screensaverEnabled"))
        screensaverEnabled = obj["screensaverEnabled"].as<bool>();
    if (obj.containsKey("screensaverTimeout"))
        screensaverTimeout = static_cast<int>(obj["screensaverTimeout"].as<float>() * 60000.0f);
    if (obj.containsKey("mainBrightness"))
        mainBrightness = obj["mainBrightness"].as<int>();
    if (obj.containsKey("standbyBrightness"))
        standbyBrightness = obj["standbyBrightness"].as<int>();
    if (obj.containsKey("standbyBrightnessTimeout"))
        standbyBrightnessTimeout = obj["standbyBrightnessTimeout"].as<int>() * 1000;
    if (obj.containsKey("standbyLandingScreen"))
        standbyLandingScreen = obj["standbyLandingScreen"].as<String>();
    if (obj.containsKey("steamPumpPercentage"))
        steamPumpPercentage = obj["steamPumpPercentage"].as<float>();
    if (obj.containsKey("steamPumpCutoff"))
        steamPumpCutoff = obj["steamPumpCutoff"].as<float>();
    if (obj.containsKey("themeMode"))
        themeMode = obj["themeMode"].as<int>();
    if (obj.containsKey("sunriseR"))
        sunriseR = obj["sunriseR"].as<int>();
    if (obj.containsKey("sunriseG"))
        sunriseG = obj["sunriseG"].as<int>();
    if (obj.containsKey("sunriseB"))
        sunriseB = obj["sunriseB"].as<int>();
    if (obj.containsKey("sunriseW"))
        sunriseW = obj["sunriseW"].as<int>();
    if (obj.containsKey("sunriseExtBrightness"))
        sunriseExtBrightness = obj["sunriseExtBrightness"].as<int>();
    if (obj.containsKey("emptyTankDistance"))
        emptyTankDistance = obj["emptyTankDistance"].as<int>();
    if (obj.containsKey("fullTankDistance"))
        fullTankDistance = obj["fullTankDistance"].as<int>();
    if (obj.containsKey("altRelayFunction"))
        altRelayFunction = obj["altRelayFunction"].as<int>();
    if (obj.containsKey("autowakeupEnabled"))
        autowakeupEnabled = obj["autowakeupEnabled"].as<bool>();
    if (obj.containsKey("shellyEnabled"))
        shellyEnabled = obj["shellyEnabled"].as<bool>();
    if (obj.containsKey("shellyGrinderEnabled"))
        shellyGrinderEnabled = obj["shellyGrinderEnabled"].as<bool>();
    if (obj.containsKey("shellyLedEnabled"))
        shellyLedEnabled = obj["shellyLedEnabled"].as<bool>();
    if (obj.containsKey("shellyMainPowerEnabled"))
        shellyMainPowerEnabled = obj["shellyMainPowerEnabled"].as<bool>();
    if (obj.containsKey("shellyLedMode"))
        shellyLedMode = obj["shellyLedMode"].as<int>();
    if (obj.containsKey("shellyDevicesJson"))
        shellyDevicesJson = obj["shellyDevicesJson"].as<String>();
    if (obj.containsKey("shellyAssignmentsJson"))
        shellyAssignmentsJson = obj["shellyAssignmentsJson"].as<String>();
    if (obj.containsKey("shellyScheduleJson"))
        shellyScheduleJson = obj["shellyScheduleJson"].as<String>();
    if (obj.containsKey("autowakeupSchedules")) {
        String schedulesStr = obj["autowakeupSchedules"].as<String>();
        autowakeupSchedules.clear();
        if (schedulesStr.length() > 0) {
            int start = 0;
            int end = schedulesStr.indexOf(';');
            while (end != -1 || start < schedulesStr.length()) {
                String scheduleStr = (end != -1) ? schedulesStr.substring(start, end) : schedulesStr.substring(start);
                int pipePos = scheduleStr.indexOf('|');
                if (pipePos != -1) {
                    String timeStr = scheduleStr.substring(0, pipePos);
                    String daysStr = scheduleStr.substring(pipePos + 1);
                    AutoWakeupSchedule schedule;
                    schedule.time = timeStr;
                    if (daysStr.length() == 7) {
                        for (int i = 0; i < 7; i++) {
                            schedule.days[i] = (daysStr.charAt(i) == '1');
                        }
                    }
                    autowakeupSchedules.push_back(schedule);
                }
                if (end == -1)
                    break;
                start = end + 1;
                end = schedulesStr.indexOf(';', start);
            }
        }
        if (autowakeupSchedules.empty()) {
            autowakeupSchedules.push_back(AutoWakeupSchedule("07:00"));
        }
    }
}

void Settings::doSave() {
    if (!dirty) {
        return;
    }
    dirty = false;
    ESP_LOGI("Settings", "Saving settings");
    preferences.begin(PREFERENCES_KEY, false);
    preferences.putInt("sm", startupMode);
    preferences.putInt("tb", targetBrewTemp);
    preferences.putInt("ts", targetSteamTemp);
    preferences.putInt("tw", targetWaterTemp);
    preferences.putInt("td", targetDuration);
    preferences.putInt("tv", targetVolume);
    preferences.putDouble("tgv", targetGrindVolume);
    preferences.putInt("tgd", targetGrindDuration);
    preferences.putBool("dm_en", doseMeasureEnabled);
    preferences.putDouble("dm_bw", doseAvgBeanWeight);
    preferences.putDouble("dm_tg", doseTarget);
    preferences.putBool("dm_ce", doseCupEnabled);
    preferences.putDouble("dm_cw", doseCupEmptyWeight);
    preferences.putBool("dm_bp", doseBeepEnabled);
    preferences.putInt("dm_pc", doseProceedBeanCount);
    preferences.putInt("dm_bcl", doseBeanCountLimit);
    preferences.putInt("dm_dc", doseDefaultDoseCount);
    preferences.putDouble("del_br", brewDelay);
    preferences.putDouble("del_gd", grindDelay);
    preferences.putBool("del_ad", delayAdjust);
    preferences.putInt("to", temperatureOffset);
    preferences.putFloat("ps", pressureScaling);
    preferences.putString("pid", pid);
    preferences.putString("pmc", pumpModelCoeffs);
    preferences.putString("ws", wifiSsid);
    preferences.putString("wp", wifiPassword);
    preferences.putString("mn", mdnsName);
    preferences.putBool("hk", homekit);
    preferences.putBool("vt", volumetricTarget);
    preferences.putString("oc", otaChannel);
    preferences.putInt("ipt", infusePumpTime);
    preferences.putInt("ibt", infuseBloomTime);
    preferences.putInt("pt", pressurizeTime);
    preferences.putString("ssc", savedScale);
    preferences.putBool("bf_a", boilerFillActive);
    preferences.putInt("bf_su", startupFillTime);
    preferences.putInt("bf_st", steamFillTime);
    preferences.putBool("sg_a", smartGrindActive);
    preferences.putString("sg_i", smartGrindIp);
    preferences.putBool("sg_t", smartGrindToggle);
    preferences.putInt("sg_m", smartGrindMode);
    preferences.putBool("ha_a", homeAssistant);
    preferences.putString("ha_i", homeAssistantIP);
    preferences.putInt("ha_p", homeAssistantPort);
    preferences.putString("ha_t", homeAssistantTopic);
    preferences.putString("ha_u", homeAssistantUser);
    preferences.putString("ha_pw", homeAssistantPassword);
    preferences.putString("tz", timezone);
    preferences.putBool("clk_24h", clock24hFormat);
    preferences.putString("sp", selectedProfile);
    preferences.putInt("sbt", standbyTimeout);
    preferences.putBool("ss_en", screensaverEnabled);
    preferences.putInt("ss_to", screensaverTimeout);
    preferences.putBool("rd_once", restoreDoneOnce);
    preferences.putBool("pm", profilesMigrated);
    preferences.putBool("mb", momentaryButtons);
    preferences.putString("fp", implode(favoritedProfiles, ","));
    preferences.putString("po", implode(profileOrder, ","));
    preferences.putFloat("spp", steamPumpPercentage);
    preferences.putFloat("spc", steamPumpCutoff);
    preferences.putInt("hi", historyIndex);
    preferences.putBool("ab_en", autowakeupEnabled);
    preferences.putBool("sh_en", shellyEnabled);
    preferences.putBool("sh_gr", shellyGrinderEnabled);
    preferences.putBool("sh_led", shellyLedEnabled);
    preferences.putBool("sh_mp", shellyMainPowerEnabled);
    preferences.putInt("sh_lm", shellyLedMode);
    preferences.putString("sh_dev", shellyDevicesJson);
    preferences.putString("sh_asg", shellyAssignmentsJson);
    preferences.putString("sh_sch", shellyScheduleJson);

    // Save schedule format
    String schedulesForSave = "";
    for (size_t i = 0; i < autowakeupSchedules.size(); i++) {
        if (i > 0)
            schedulesForSave += ";";
        schedulesForSave += autowakeupSchedules[i].time + "|";

        // Convert days array to 7-bit string
        for (int j = 0; j < 7; j++) {
            schedulesForSave += autowakeupSchedules[i].days[j] ? "1" : "0";
        }
    }
    preferences.putString("ab_schedules", schedulesForSave);

    // Display settings
    preferences.putInt("main_b", mainBrightness);
    preferences.putInt("standby_b", standbyBrightness);
    preferences.putInt("standby_bt", standbyBrightnessTimeout);
    preferences.putString("standby_ls", standbyLandingScreen);
    preferences.putInt("wifi_apt", wifiApTimeout);
    preferences.putInt("theme", themeMode);

    // Sunrise Settings
    preferences.putInt("sr_r", sunriseR);
    preferences.putInt("sr_g", sunriseG);
    preferences.putInt("sr_b", sunriseB);
    preferences.putInt("sr_w", sunriseW);
    preferences.putInt("sr_exb", sunriseExtBrightness);
    preferences.putInt("sr_ed", emptyTankDistance);
    preferences.putInt("sr_fd", fullTankDistance);
    preferences.putInt("alt_relay", altRelayFunction);

    preferences.end();

}

void Settings::loopTask(void *arg) {
    auto *settings = static_cast<Settings *>(arg);
    while (true) {
        settings->doSave();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
