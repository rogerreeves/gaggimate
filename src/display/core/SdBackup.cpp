#include "SdBackup.h"

#include <ArduinoJson.h>
#include <SD_MMC.h>
#include <SPIFFS.h>
#include <display/core/Settings.h>

namespace {
constexpr const char *kBackupRoot = "/gaggimate/backup";
constexpr const char *kSettingsBackup = "/gaggimate/backup/settings.json";
constexpr const char *kProfilesBackupDir = "/gaggimate/backup/profiles";
constexpr const char *kProfilesBackupTmpDir = "/gaggimate/backup/profiles_new";
constexpr size_t kCopyBufferSize = 4096;

bool ensureDir(fs::FS &fs, const char *path, String *err) {
    if (fs.exists(path)) {
        return true;
    }
    if (!fs.mkdir(path)) {
        if (err) {
            *err = "mkdir failed: ";
            *err += path;
        }
        return false;
    }
    return true;
}

bool ensureBackupDir(String *err) {
    if (!ensureDir(SD_MMC, "/gaggimate", err)) {
        return false;
    }
    return ensureDir(SD_MMC, kBackupRoot, err);
}

bool removeDirRecursive(fs::FS &fs, const char *dir, String *err) {
    File root = fs.open(dir);
    if (!root) {
        if (err) {
            *err = "open dir failed: ";
            *err += dir;
        }
        return false;
    }
    if (!root.isDirectory()) {
        root.close();
        if (err) {
            *err = "not a dir: ";
            *err += dir;
        }
        return false;
    }
    File file = root.openNextFile();
    while (file) {
        const char *name = file.name();
        if (file.isDirectory()) {
            if (!removeDirRecursive(fs, name, err)) {
                file.close();
                root.close();
                return false;
            }
        } else if (!fs.remove(name)) {
            if (err) {
                *err = "remove failed: ";
                *err += name;
            }
            file.close();
            root.close();
            return false;
        }
        file = root.openNextFile();
    }
    root.close();
    if (!fs.rmdir(dir)) {
        if (err) {
            *err = "rmdir failed: ";
            *err += dir;
        }
        return false;
    }
    return true;
}

bool copyDirRecursive(fs::FS &fromFS, const char *fromDir, fs::FS &toFS, const char *toDir, String *err) {
    File root = fromFS.open(fromDir);
    if (!root || !root.isDirectory()) {
        if (err) {
            *err = "open dir failed: ";
            *err += fromDir;
        }
        return false;
    }
    if (!ensureDir(toFS, toDir, err)) {
        root.close();
        return false;
    }
    File file = root.openNextFile();
    while (file) {
        String name = file.name();
        String source = name;
        if (!name.startsWith("/")) {
            source = String(fromDir);
            if (!source.endsWith("/")) {
                source += "/";
            }
            source += name;
        }
        String leaf = name.substring(name.lastIndexOf('/') + 1);
        String target = String(toDir) + "/" + leaf;
        if (file.isDirectory()) {
            if (!copyDirRecursive(fromFS, source.c_str(), toFS, target.c_str(), err)) {
                file.close();
                root.close();
                return false;
            }
        } else {
            if (!SdBackup::atomicCopyFile(fromFS, source.c_str(), toFS, target.c_str(), err)) {
                file.close();
                root.close();
                return false;
            }
        }
        file = root.openNextFile();
    }
    root.close();
    return true;
}

bool atomicCopyDir(fs::FS &fromFS, const char *fromDir, fs::FS &toFS, const char *toDir, const char *tmpDir, String *err) {
    if (toFS.exists(tmpDir)) {
        removeDirRecursive(toFS, tmpDir, nullptr);
    }
    if (!copyDirRecursive(fromFS, fromDir, toFS, tmpDir, err)) {
        return false;
    }
    if (toFS.exists(toDir)) {
        if (!removeDirRecursive(toFS, toDir, err)) {
            return false;
        }
    }
    if (!toFS.rename(tmpDir, toDir)) {
        if (err) {
            *err = "rename dir failed";
        }
        return false;
    }
    return true;
}
} // namespace

bool SdBackup::begin() { return SD_MMC.cardType() != CARD_NONE; }

bool SdBackup::available() { return SD_MMC.cardType() != CARD_NONE; }

bool SdBackup::fileExists(const char *path) { return SD_MMC.exists(path); }

bool SdBackup::atomicCopyFile(fs::FS &fromFS, const char *from, fs::FS &toFS, const char *to, String *err) {
    File src = fromFS.open(from, "r");
    if (!src) {
        if (err) {
            *err = "open src failed: ";
            *err += from;
        }
        return false;
    }
    String tmp = String(to) + ".tmp";
    File dst = toFS.open(tmp.c_str(), "w");
    if (!dst) {
        if (err) {
            *err = "open dst failed: ";
            *err += tmp;
        }
        src.close();
        return false;
    }
    uint8_t buffer[kCopyBufferSize];
    size_t readLen = 0;
    while ((readLen = src.read(buffer, sizeof(buffer))) > 0) {
        if (dst.write(buffer, readLen) != readLen) {
            if (err) {
                *err = "write failed: ";
                *err += tmp;
            }
            src.close();
            dst.close();
            toFS.remove(tmp.c_str());
            return false;
        }
    }
    dst.close();
    src.close();
    if (toFS.exists(to)) {
        toFS.remove(to);
    }
    if (!toFS.rename(tmp.c_str(), to)) {
        if (err) {
            *err = "rename failed: ";
            *err += to;
        }
        toFS.remove(tmp.c_str());
        return false;
    }
    return true;
}

bool SdBackup::atomicWriteBytes(fs::FS &toFS, const char *to, const uint8_t *data, size_t len, String *err) {
    String tmp = String(to) + ".tmp";
    File dst = toFS.open(tmp.c_str(), "w");
    if (!dst) {
        if (err) {
            *err = "open dst failed: ";
            *err += tmp;
        }
        return false;
    }
    if (dst.write(data, len) != len) {
        if (err) {
            *err = "write failed: ";
            *err += tmp;
        }
        dst.close();
        toFS.remove(tmp.c_str());
        return false;
    }
    dst.close();
    if (toFS.exists(to)) {
        toFS.remove(to);
    }
    if (!toFS.rename(tmp.c_str(), to)) {
        if (err) {
            *err = "rename failed: ";
            *err += to;
        }
        toFS.remove(tmp.c_str());
        return false;
    }
    return true;
}

bool SdBackup::backupSettings(const Settings &settings, String *err) {
    if (!available()) {
        if (err) {
            *err = "SD not available";
        }
        return false;
    }
    if (!ensureBackupDir(err)) {
        return false;
    }
    DynamicJsonDocument doc(16384);
    JsonObject obj = doc.to<JsonObject>();
    settings.fillJson(obj);

    String tmp = String(kSettingsBackup) + ".tmp";
    File dst = SD_MMC.open(tmp.c_str(), "w");
    if (!dst) {
        if (err) {
            *err = "open settings tmp failed";
        }
        return false;
    }
    if (serializeJson(doc, dst) == 0) {
        if (err) {
            *err = "serialize failed";
        }
        dst.close();
        SD_MMC.remove(tmp.c_str());
        return false;
    }
    dst.close();
    if (SD_MMC.exists(kSettingsBackup)) {
        SD_MMC.remove(kSettingsBackup);
    }
    if (!SD_MMC.rename(tmp.c_str(), kSettingsBackup)) {
        if (err) {
            *err = "rename settings failed";
        }
        SD_MMC.remove(tmp.c_str());
        return false;
    }
    return true;
}

bool SdBackup::backupProfiles(fs::FS &fromFS, const char *profilesDir, String *err) {
    if (!available()) {
        if (err) {
            *err = "SD not available";
        }
        return false;
    }
    if (!ensureBackupDir(err)) {
        return false;
    }
    return atomicCopyDir(fromFS, profilesDir, SD_MMC, kProfilesBackupDir, kProfilesBackupTmpDir, err);
}

bool SdBackup::restoreSettings(Settings &settings, String *err) {
    if (!available()) {
        if (err) {
            *err = "SD not available";
        }
        return false;
    }
    if (!SD_MMC.exists(kSettingsBackup)) {
        if (err) {
            *err = "settings backup missing";
        }
        return false;
    }
    File src = SD_MMC.open(kSettingsBackup, "r");
    if (!src) {
        if (err) {
            *err = "open settings backup failed";
        }
        return false;
    }
    DynamicJsonDocument doc(16384);
    DeserializationError jsonErr = deserializeJson(doc, src);
    src.close();
    if (jsonErr) {
        if (err) {
            *err = "parse failed: ";
            *err += jsonErr.c_str();
        }
        return false;
    }
    JsonObject obj = doc.as<JsonObject>();
    settings.applyJson(obj);
    settings.save(true);
    return true;
}

bool SdBackup::restoreProfiles(fs::FS &toFS, const char *profilesDir, String *err) {
    if (!available()) {
        if (err) {
            *err = "SD not available";
        }
        return false;
    }
    if (!SD_MMC.exists(kProfilesBackupDir)) {
        if (err) {
            *err = "profiles backup missing";
        }
        return false;
    }
    String tmpDir = String(profilesDir) + "_new";
    if (!atomicCopyDir(SD_MMC, kProfilesBackupDir, toFS, profilesDir, tmpDir.c_str(), err)) {
        return false;
    }
    File root = toFS.open(profilesDir);
    if (!root || !root.isDirectory()) {
        if (err) {
            *err = "profiles dir missing after restore";
        }
        return false;
    }
    bool hasFile = false;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory() && file.size() > 0) {
            hasFile = true;
            break;
        }
        file = root.openNextFile();
    }
    root.close();
    if (!hasFile) {
        if (err) {
            *err = "profiles empty after restore";
        }
        return false;
    }
    return true;
}
