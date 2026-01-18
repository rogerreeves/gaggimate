# Work Log (Reference)

This file is a running summary of recent changes and decisions so we can track what was done without re-reading chat history.

## Branches and Remotes
- Created branch `rancilio-minimal` (reskin work). Tracking `github/rancilio-minimal`.
- `custom-build` remains the shipping branch.
- Added `upstream` remote for `https://github.com/jniebuhr/gaggimate.git`, push disabled.

## Single-Dose (Grind) Screen Split
- Goal: separate custom single-dose UI from the official grind UI.
- New single-dose screen file: `src/display/ui/default/lvgl/screens/ui_GrindScreen_SingleDose.c`.
- Official grind UI restored in `src/display/ui/default/lvgl/screens/ui_GrindScreen.c`.
- Screen selection now depends on the WebUI toggle (dose measure):
  - enabled -> single-dose screen init
  - disabled -> official grind screen init
- Wiring updates:
  - `src/display/ui/default/lvgl/ui_events.cpp`
  - `src/display/ui/default/DefaultUI.cpp`
  - build lists: `src/display/ui/default/lvgl/CMakeLists.txt`, `src/display/ui/default/lvgl/filelist.txt`
  - header updates: `src/display/ui/default/lvgl/screens/ui_GrindScreen.h`

## Screensaver Overlay
- Overlay made permanent and opacity increased to 50% (LVGL opacity 128).
- File: `src/display/ui/default/lvgl/screens/ui_ScreensaverScreen.c`.

## Standby -> Menu Wake Behavior
- To keep the boiler target active while landing on Menu, mode is set to `MODE_BREW` before changing to Menu.
- File: `src/display/ui/default/lvgl/ui_events.cpp` (onWakeup path).

## SD Backup
- Profile backup path fix: profile file copy now prefixes the source path when file names are relative.
- File: `src/display/core/SdBackup.cpp`.
- WebUI backup crash fix: move profile SD backup to a dedicated FreeRTOS task instead of running in async webserver task.
- File: `src/display/plugins/WebUIPlugin.cpp`.

## WebUI / Workflow Notes
- SD backup routes:
  - list: `/api/sd/backup` (GET)
  - save settings: `/api/sd/backup/settings` (POST)
  - save profiles: `/api/sd/backup/profiles` (POST)
- Webflasher entries are created from CI artifacts and added to `gaggimate-display-webflash-v2`.

## Reskin Template (rancilio-minimal)
- Added code-based layout main component (since SquareLine project file not found in the repo):
  - `src/display/ui/default/lvgl/components/ui_comp_layout_main.c`
  - `src/display/ui/default/lvgl/components/ui_comp_layout_main.h`
- Template defines named containers at exact positions/sizes (no fill/outline; centered alignment).
- Build lists updated to include the new component.
- Added settings layout component:
  - `src/display/ui/default/lvgl/components/ui_comp_layout_settings.c`
  - `src/display/ui/default/lvgl/components/ui_comp_layout_settings.h`
  - Build lists updated to include the new component.

## Fonts (rancilio-minimal)
- Generated LVGL fonts from `ui/assets/SFPRODISPLAYBOLD.OTF` at 18/24/45pt, 8bpp, ASCII + degree:
  - `src/display/ui/default/lvgl/fonts/ui_font_sfprodisplaybold_18.c`
  - `src/display/ui/default/lvgl/fonts/ui_font_sfprodisplaybold_24.c`
  - `src/display/ui/default/lvgl/fonts/ui_font_sfprodisplaybold_45.c`
- Declarations added to `src/display/ui/default/lvgl/ui.h`.

## Standard Buttons (rancilio-minimal)
- Added reusable text-only button components:
  - `ui_comp_button_menu` (18px, color 0x727373, links to menu)
  - `ui_comp_button_profiles` (18px, color 0x727373, links to profile screen)
  - `ui_comp_button_confirm` (24px, color 0x727373, stub handler)
  - `ui_comp_button_save` (24px, color 0x727373, stub handler)

## Target Temp Template (rancilio-minimal)
- Added `ui_comp_layout_settings_single` with Title/Main/Left/Right slots for the target temp page.
- Added `BASE_TOP` slot to `ui_comp_layout_settings_single`.

## Target Temp Screen (rancilio-minimal)
- New `ui_TargetTempScreen` using the single settings template.
- Title/value/+/−/save-back labels styled with SF Pro (24/45) and specified colors.
- Steam target temp label on the steam screen opens the target temp screen.
- Save/back returns to the steam screen; text swaps to "back" when unchanged.

## Background (rancilio-minimal)
- Converted `ui/assets/background_minimal.png` to `ui_img_background_minimal.c` (RGB565).
- Added as the bottom-most image on all screens.

## Minimal Tick Dials (rancilio-minimal)
- Added RGB565 indicator assets: `ui_img_minimal_indicator_temp.c`, `ui_img_minimal_indicator_pressure.c`.
- Replaced arc gauges with indicator images + 2px LVGL target lines (temp red, pressure blue).
- Implemented 2°-snapped angle mapping and dead-gap masking (256°–284°).

## Standby Screen (rancilio-minimal)
- Standby now uses `layout_main` with clock in TOP (18px, 0x727373).
- Wifi icon in temp container (mid-left, right-aligned), Bluetooth icon in pressure container (mid-right, left-aligned), recolored 0x727373.

## Steam Screen (rancilio-minimal)
- `ui_SimpleProcessScreen` now uses `ui_comp_layout_main`.
- Menu text button placed in `TOP`, main label in `MAIN` (45px SF Pro, white), target temp label in `BASE_BOTTOM` (18px SF Pro, 0x727373).
- Temp up/down buttons moved to `MAIN_LEFT`/`MAIN_RIGHT`; water start/stop label lives in `BASE_TOP`.
- Steam label alternates "Steam" / "Heating..." every 3s until target, shows "Ready..." for 2s when target hit, then "Steam".
- Temp +/- controls are now text labels (45px, 0x727374) aligned right/left in left/right containers.

## Current State (high level)
- `custom-build` contains the single-dose split and SD backup fixes.
- `rancilio-minimal` has the reskin layout template ready to apply to screens.
