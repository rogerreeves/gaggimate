/**
 * @file lv_drv_conf.h
 * Configuration file for lv_drivers (SDL)
 */

#if 1
#ifndef LV_DRV_CONF_H
#define LV_DRV_CONF_H

#include "lv_conf.h"

#define USE_SDL 1
#define USE_SDL_GPU 0

#if USE_SDL || USE_SDL_GPU
#  define SDL_HOR_RES 480
#  define SDL_VER_RES 480
#  define SDL_ZOOM 1
#  define SDL_DOUBLE_BUFFERED 0
#  define SDL_INCLUDE_PATH <SDL2/SDL.h>
#  define SDL_DUAL_DISPLAY 0
#endif

#define USE_MONITOR 0
#define USE_WINDOWS 0
#define USE_WIN32DRV 0
#define USE_GTK 0
#define USE_WAYLAND 0
#define USE_SSD1963 0

#endif // LV_DRV_CONF_H
#endif
