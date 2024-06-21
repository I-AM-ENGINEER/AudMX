#include "menuTask.hpp"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "freertos/event_groups.h"
#include "device.hpp"
#include "menu.h"

extern SemaphoreHandle_t displaysMutex;
extern EventGroupHandle_t buttonsPressedEventGroup;
extern EventGroupHandle_t buttonsReleasedEventGroup;

extern menu_item_t menu_item_animation_mode;
extern menu_item_t menu_item_brightness;
extern menu_item_t menu_item_volume_reactive;
extern menu_item_t menu_item_brightness_background;
extern menu_item_t menu_item_saturation;
extern menu_item_t menu_item_saturation_background;
extern menu_item_t *menu_items[];
extern menu_item_t *menu_rainbow[];
extern menu_item_t *menu_static[];

void menuTask( void *args ){
    audMix.update();
}

static bool isBtnPressed( uint8_t btn ){
    EventBits_t bits;
	bits = xEventGroupClearBits(buttonsPressedEventGroup, (1 << btn));
    return (bits & (1 << btn));
}

static bool isBtnReleased( uint8_t btn ){
    EventBits_t bits;
	bits = xEventGroupClearBits(buttonsReleasedEventGroup, (1 << btn));
    return (bits & (1 << btn));
}

void Device::update( void ){
    Slider &slider = sliders[0];
    LGFX_SSD1306 &display = sliders[0].display;
    uint8_t start_brightness;
    uint32_t selected_item = 0; 

    while (1){
        EventBits_t bits;
		bits = xEventGroupClearBits(buttonsPressedEventGroup, (1 << _button_for_stip_config));
        int64_t select_btn_pressed_timestamp = 0;
        if(bits & (1 << _button_for_stip_config)){
            int64_t start_timestamp = esp_timer_get_time();
            slider.displayIcon(false);
            
            display.setTextColor(TFT_WHITE, TFT_BLACK);
            display.setTextSize(1.0f);
            
            xSemaphoreTake(displaysMutex, portMAX_DELAY);
            start_brightness = display.getBrightness();
            display.clear(TFT_BLACK);
            xSemaphoreGive(displaysMutex);

            menu_item_t **current_menu = menu_items;

            while((esp_timer_get_time() - start_timestamp) < 10'000'000){
                xSemaphoreTake(displaysMutex, portMAX_DELAY);
                
                menu_item_brightness_background.i32_max = menu_item_brightness.i32;
                uint32_t displayed_items = 0;

                display.setBrightness(200);
                display.setCursor(0,0);

                int32_t cursor_y = 0;
                
                for(uint32_t i = 0; current_menu[i] != nullptr; i++){
                    menu_item_t &menu_item = *current_menu[i];

                    if(menu_item.type == MENU_ITEM_TYPE_INT){
                        if(menu_item.i32 > menu_item.i32_max){
                            menu_item.i32 = menu_item.i32_min;
                            menuUpdateSettings();
                            display.setCursor(40, cursor_y);
                            display.print("   ");
                        }
                    }
                    display.setCursor(0,cursor_y);

                    if(menu_item.enable){
                        // Selected - invert color
                        if(selected_item == i){
                            display.setTextColor(TFT_BLACK, TFT_WHITE);
                        }else{
                            display.setTextColor(TFT_WHITE, TFT_BLACK);
                        }

                        display.print(menu_item.name);

                        display.setTextColor(TFT_WHITE, TFT_BLACK);

                        if(menu_item.type == MENU_ITEM_TYPE_INT){
                            if((menu_item.i32 < -99) || (menu_item.i32 > 999)){
                                display.setCursor(40,cursor_y);
                            }else if((menu_item.i32 < -9) || (menu_item.i32 > 99)){
                                display.setCursor(46,cursor_y);
                            }else if((menu_item.i32 < 0) || (menu_item.i32 > 9)){
                                display.setCursor(52,cursor_y);
                            }else{
                                display.setCursor(58,cursor_y);
                            }
                            display.print(menu_item.i32);
                        }else{
                            display.setCursor(58,cursor_y);
                            if(menu_item.b){
                                display.print("T");
                            }else{
                                display.print("F");
                            }
                        }
                        displayed_items++;
                        cursor_y += 8;
                    }else if(selected_item == i){
                        selected_item++;
                    }
                }

                xSemaphoreGive(displaysMutex);
                if(isBtnPressed(_button_select)){
                    select_btn_pressed_timestamp = esp_timer_get_time();
                }
                if(isBtnReleased(_button_select)){
                    select_btn_pressed_timestamp = 0;
                }
                if(isBtnPressed(_button_next)){
                    start_timestamp = esp_timer_get_time();
                    selected_item++;
                }
                if(displayed_items == selected_item){
                    selected_item = 0;
                    xSemaphoreTake(displaysMutex, portMAX_DELAY);
                    display.clear(TFT_BLACK);
                    xSemaphoreGive(displaysMutex);

                    if(current_menu == menu_items){
                        switch (menu_item_animation_mode.i32 - 1){
                            case STRIP_ANIMATION_RAINBOW:       current_menu = menu_rainbow;    break;
                            case STRIP_ANIMATION_STATIC:        current_menu = menu_static;     break;
                            default: goto exit;
                        }
                    }else{
                        goto exit;
                    }
                }

                static uint32_t btn_step = 0;
                if(select_btn_pressed_timestamp != 0){
                    start_timestamp = esp_timer_get_time();
                    uint32_t select_btn_pressed_ms = (esp_timer_get_time() - select_btn_pressed_timestamp)/1000;
                    if((select_btn_pressed_ms > 10) && ((btn_step == 0) || (btn_step == 2))){
                        if(current_menu[selected_item]->type == MENU_ITEM_TYPE_BOOL){
                            current_menu[selected_item]->b = !current_menu[selected_item]->b;
                        }else if(current_menu[selected_item]->type == MENU_ITEM_TYPE_INT){
                            current_menu[selected_item]->i32 += 1;
                        }
                        menuUpdateSettings();
                        if(btn_step == 0){
                            btn_step++;
                        }else if(btn_step == 2){
                            select_btn_pressed_timestamp = esp_timer_get_time();
                        }
                    }
                    if(select_btn_pressed_ms > 1000){
                        btn_step = 2;
                        select_btn_pressed_timestamp = esp_timer_get_time();
                    }
                }else{
                    btn_step = 0;
                }
                vTaskDelay(pdMS_TO_TICKS(20));
            }
exit:
            xSemaphoreTake(displaysMutex, portMAX_DELAY);
            display.clear();
            display.setBrightness(start_brightness);
            xSemaphoreGive(displaysMutex);
            menuSaveAll();
            slider.displayIcon(true);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}