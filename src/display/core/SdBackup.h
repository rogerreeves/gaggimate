#pragma once

#include <Arduino.h>
#include <FS.h>

class Settings;

namespace SdBackup {

bool begin();
bool available();
bool fileExists(const char *path);
bool atomicCopyFile(fs::FS &fromFS, const char *from, fs::FS &toFS, const char *to, String *err = nullptr);
bool atomicWriteBytes(fs::FS &toFS, const char *to, const uint8_t *data, size_t len, String *err = nullptr);

bool backupSettings(const Settings &settings, String *err = nullptr);
bool backupProfiles(fs::FS &fromFS, const char *profilesDir, String *err = nullptr);

bool restoreSettings(Settings &settings, String *err = nullptr);
bool restoreProfiles(fs::FS &toFS, const char *profilesDir, String *err = nullptr);

} // namespace SdBackup
