#include <lvgl.h>
#include <sdl/sdl.h>

#include "display/core/PluginManager.h"
#include "display/core/ProfileManager.h"
#include "display/controller_api.h"
#include "display/ui/default/DefaultUI.h"
#include "display/main.h"

#include <chrono>
#include <thread>

Controller controller;

int main() {
    lv_init();
    sdl_init();

    static lv_color_t buf1[480 * 60];
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, 480 * 60);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 480;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    lv_indev_drv_register(&indev_drv);

    static lv_indev_drv_t wheel_drv;
    lv_indev_drv_init(&wheel_drv);
    wheel_drv.type = LV_INDEV_TYPE_ENCODER;
    wheel_drv.read_cb = sdl_mousewheel_read;
    lv_indev_drv_register(&wheel_drv);

    static lv_indev_drv_t key_drv;
    lv_indev_drv_init(&key_drv);
    key_drv.type = LV_INDEV_TYPE_KEYPAD;
    key_drv.read_cb = sdl_keyboard_read;
    lv_indev_drv_register(&key_drv);

    PluginManager pluginManager;
    ProfileManager profileManager;
    controller.setProfileManager(&profileManager);

    DefaultUI ui(&controller, nullptr, &pluginManager);
    controller.setUI(&ui);

    controller_api_init();
    ui.init();

    while (true) {
        controller_api_tick_16ms();
        ui.loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
