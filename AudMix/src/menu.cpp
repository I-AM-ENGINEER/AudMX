#include "device.hpp"
#include <stdlib.h>
#include "nvs_flash.h"
#include "menu.h"
#include "bt_spp.h"

menu_item_t menu_item_animation_mode = {
    .name = "Mode",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 1,
    .i32_min = 1,
    .i32_max = 2,
};

menu_item_t menu_item_brightness = {
    .name = "Bright",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 10,
    .i32_min = 4,
    .i32_max = 20,
};

menu_item_t menu_item_brightness_background = {
    .name = "Bright bg",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = menu_item_brightness.i32_min,
    .i32_min = 0,
    .i32_max = menu_item_brightness.i32_max,
};

menu_item_t menu_item_saturation = {
    .name = "Satur",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 10,
    .i32_min = 0,
    .i32_max = 10,
};

menu_item_t menu_item_saturation_background = {
    .name = "Satur bg",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = menu_item_saturation.i32_max,
    .i32_min = menu_item_saturation.i32_min,
    .i32_max = menu_item_saturation.i32_max,
};

menu_item_t menu_item_volume_reactive = {
    .name = "Vol react",
    .enable = true,
    .type = MENU_ITEM_TYPE_BOOL,
    .b = false,
};

menu_item_t *menu_items[] = {
    &menu_item_animation_mode,
    &menu_item_brightness,
    &menu_item_brightness_background,
    &menu_item_saturation,
    &menu_item_saturation_background,
    &menu_item_volume_reactive,
    nullptr,
};

menu_item_t menu_item_speed = {
    .name = "Period",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 10,
    .i32_min = 1,
    .i32_max = 30,
};

menu_item_t menu_phase_shift_led = {
    .name = "Led shift",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 10,
    .i32_min = 1,
    .i32_max = 30,
};

menu_item_t menu_phase_shift_strip = {
    .name = "Str shift",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 0,
    .i32_min = 0,
    .i32_max = 35,
};

menu_item_t *menu_rainbow[] = {
    &menu_item_speed,
    &menu_phase_shift_led,
    &menu_phase_shift_strip,
    nullptr,
};

menu_item_t menu_color_main = {
    .name = "Color",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 0,
    .i32_min = 0,
    .i32_max = 35,
};

menu_item_t menu_color_background = {
    .name = "Color bg",
    .enable = true,
    .type = MENU_ITEM_TYPE_INT,
    .i32 = 0,
    .i32_min = 0,
    .i32_max = 35,
};

menu_item_t *menu_static[] = {
    &menu_color_main,
    &menu_color_background,
    nullptr,
};

menu_item_t *(*all_menus[])[] = {
    &menu_items,
    &menu_rainbow,
    &menu_static,
    nullptr,
};

#define MENU_NVS_STORAGE_TAG    "MENU"

static bool menu_nvs_load( menu_item_t* item ){
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(MENU_NVS_STORAGE_TAG, NVS_READONLY, &nvs_handle); 
    if(err != ESP_OK){
        return false;
    }

    char key[NVS_KEY_NAME_MAX_SIZE];
    memcpy(key, item->name, sizeof(item->name));
    size_t readed_len;
    nvs_get_blob(nvs_handle, key, item, &readed_len);
    nvs_close(nvs_handle);
    
    if(readed_len != sizeof(menu_item_t)){
        return false;
    }
    return true;
}

static bool menu_nvs_save( const menu_item_t* item ){
    menu_item_t readed_nvs;
    memcpy(readed_nvs.name, item->name, sizeof(item->name));
    if(menu_nvs_load(&readed_nvs) == true){
        if(memcmp(item, &readed_nvs, sizeof(menu_item_t)) == 0){
            return false;
        }
    }
    nvs_handle_t nvs_handle;
    nvs_open(MENU_NVS_STORAGE_TAG, NVS_READWRITE, &nvs_handle); 

    char key[NVS_KEY_NAME_MAX_SIZE];
    memcpy(key, item->name, sizeof(item->name));
    nvs_set_blob(nvs_handle, key, item, sizeof(menu_item_t));

    nvs_close(nvs_handle);
    return true;
}

void Device::menuUpdateSettings( void ){
    uint32_t i = 0;
    _is_audioreactive = menu_item_volume_reactive.b;
    ble_notification_volreactive_update(_is_audioreactive);
    for(auto &slider : sliders){
        slider.strip.brightnessSet(menu_item_brightness.i32, menu_item_brightness_background.i32);
        slider.strip.saturationSet((uint8_t)(((float)menu_item_saturation.i32)*25.5), (uint8_t)(((float)menu_item_saturation_background.i32)*25.5));
        slider.strip.animationSet((strip_animation_t)(menu_item_animation_mode.i32 - 1));
        {
            auto config = slider.strip.animation_config_rainbow();
            config.period_ms = menu_item_speed.i32 * 300;
            config.phase_shift_pixel = menu_phase_shift_led.i32;
            config.phase_zero = menu_phase_shift_strip.i32 * 2 * i++;
            slider.strip.animation_config_rainbow(config);
        }
        {
            auto config = slider.strip.animation_config_static();
            config.color_main = (uint8_t)((float)menu_color_main.i32 / (float)(menu_color_main.i32_max + 1) * 256.0f);
            config.color_background = (uint8_t)((float)menu_color_background.i32 / (float)(menu_color_background.i32_max + 1) * 256.0f);
            slider.strip.animation_config_static(config);
        }
    }
}

void Device::menuInit( void ){
    //menuSaveAll(); // For reset to default
    for(uint32_t i = 0; all_menus[i] != nullptr; i++){
        for(uint32_t j = 0; (*all_menus[i])[j] != nullptr; j++){
            if(!menu_nvs_load((*all_menus[i])[j])){
                menu_nvs_save((*all_menus[i])[j]);
            }
        }
    }
    menuUpdateSettings();
}

void Device::menuSaveAll( void ){
    for(uint32_t i = 0; all_menus[i] != nullptr; i++){
        for(uint32_t j = 0; (*all_menus[i])[j] != nullptr; j++){
            menu_nvs_save((*all_menus[i])[j]);
        }
    }
}
