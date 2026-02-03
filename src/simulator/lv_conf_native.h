/**
 * @file lv_conf_native.h
 * LVGL configuration for native SDL simulator
 */

#if 1
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)

#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
  #define LV_MEM_SIZE (64U * 1024U)
  #define LV_MEM_ADR 0
  #undef LV_MEM_POOL_INCLUDE
  #undef LV_MEM_POOL_ALLOC
#else
  #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
  #define LV_MEM_CUSTOM_ALLOC malloc
  #define LV_MEM_CUSTOM_FREE free
  #define LV_MEM_CUSTOM_REALLOC realloc
#endif

#define LV_MEM_BUF_MAX_NUM 16
#define LV_MEMCPY_MEMSET_STD 0

#define LV_DISP_DEF_REFR_PERIOD 16
#define LV_INDEV_DEF_READ_PERIOD 30

#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
  #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
  #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

#define LV_DPI_DEF 130

#define LV_USE_LOG 0

#define LV_USE_USER_DATA 1

#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

#define LV_USE_FLEX 1
#define LV_USE_GRID 1

#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC 1
#define LV_USE_THEME_MONO 0

#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_BAR 1
#define LV_USE_SLIDER 1
#define LV_USE_TABLE 1
#define LV_USE_ARC 1
#define LV_USE_IMG 1
#define LV_USE_IMGBTN 1
#define LV_USE_LINE 1
#define LV_USE_SPINNER 1

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_FONT_DEFAULT &lv_font_montserrat_24

#define LV_USE_FONT_COMPRESSED 0
#define LV_USE_FONT_SUBPX 0
#define LV_USE_FONT_PLACEHOLDER 1

#define LV_USE_STDLIB_MALLOC 1

#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

#define LV_USE_SNAPSHOT 0

#define LV_USE_GIF 0
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_SJPG 0
#define LV_USE_FS_STDIO 0

#define LV_USE_USER_DATA 1

#endif /*LV_CONF_H*/
#endif /*End of "Content enable"*/
