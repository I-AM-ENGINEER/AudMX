#include "device.hpp"

menu_item_t menu_item_animation_mode = {
    .name = "Mode",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 1,
    .i32_min = 1,
    .i32_max = 10,
};

menu_item_t menu_item_brightness = {
    .name = "Bright",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 1,
    .i32_min = 1,
    .i32_max = 10,
};

menu_item_t menu_item_volume_reactive = {
    .name = "Vol react",
    .enable = true,
    .type = MENU_ITEM_TYPE_BOOL,
    .b = false,
};

menu_item_t menu_item_background_brightness = {
    .name = "Bg brig",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 0,
    .i32_min = menu_item_brightness.i32_min,
    .i32_max = menu_item_brightness.i32_max,
};

menu_item_t *menu_items[] = {
    &menu_item_animation_mode,
    &menu_item_brightness,
    &menu_item_volume_reactive,
    &menu_item_background_brightness,
    nullptr,
};
