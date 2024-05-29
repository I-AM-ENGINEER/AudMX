
#include "esp_log.h"
#include "esp_console.h"
#include "esp_adc/adc_oneshot.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "esp_ws28xx.h"
#include "esp_log.h"
#include "device.hpp"

#define PROMPT_STR CONFIG_IDF_TARGET

CRGB *ws2812b_display_buffer;
extern EventGroupHandle_t buttonsPressedEventGroup;
extern EventGroupHandle_t buttonsReleasedEventGroup;
extern SemaphoreHandle_t displaysMutex;

extern menu_item_t menu_item_animation_mode;
extern menu_item_t menu_item_brightness;
extern menu_item_t menu_item_volume_reactive;
extern menu_item_t menu_item_brightness_background;
extern menu_item_t menu_item_saturation;
extern menu_item_t menu_item_saturation_background;
extern menu_item_t *menu_items[];
extern menu_item_t *menu_rainbow[];
extern menu_item_t *menu_static[];

void Device::adcInit( void ){
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = SLIDERS_ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &_sliders_adc));
}

void Device::consoleInit( void ){
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 32;

    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));
}

void Device::virtDispInit( void ){
    static LGFX_SSD1306 virt_display;
    auto cfg = virt_display.config();
    cfg.pin_sda = I2C_SDA_PIN;
    cfg.pin_scl = 12;
    virt_display.config(cfg);
    virt_display.init();
}

void Device::clalibrate( void ){
    char msg[11];
    delay(100);

    xSemaphoreTake(displaysMutex, portMAX_DELAY);
    LGFX_SSD1306 *display = &sliders[0].display;
    
    // Minimum slider position capture
    display->setTextSize(1.0f);
    display->drawString("Calibration", 0, 0);
    display->drawString("Set slider", 0, 10);
    display->drawString("to MINIMUM", 0, 20);
    display->drawString("position", 0, 30);

    for( uint32_t i = 8; i > 0; i-- ){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        sliders[0].display.drawString(msg, 0, 40);
        delay(1000);
    }

    display->clear(TFT_BLACK);
    display->drawString("Sampling...", 0, 0);

    for( auto& slider : sliders ){
        auto calibration = slider.calibrationGet();
        slider.calibrationSetMinPoint(calibration);
        slider.calibrationLoad(calibration);
        snprintf(msg, sizeof(msg), "min:%.3f", calibration.min_value);
        slider.display.drawString(msg, 0, 20);
    }
    delay(2000);

    // Maximum slider position capture
    display->drawString("Calibration", 0, 0);
    display->drawString("Set slider", 0, 10);
    display->drawString("to MAXIMUM", 0, 20);
    display->drawString("position", 0, 30);

    for( uint32_t i = 8; i > 0; i-- ){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        sliders[0].display.drawString(msg, 0, 40);
        delay(1000);
    }

    {
        display->clear(TFT_BLACK);
        display->drawString("Sampling...", 0, 0);
        auto calibration = sliders[0].calibrationGet();
        snprintf(msg, sizeof(msg), "min:%.3f", calibration.min_value);
        display->drawString(msg, 0, 20);
    }

    for( auto& slider : sliders ){
        auto calibration = slider.calibrationGet();
        slider.calibrationSetMaxPoint(calibration);
        slider.calibrationLoad(calibration);
        snprintf(msg, sizeof(msg), "min:%.3f", calibration.min_value);
        slider.display.drawString(msg, 0, 20);
        snprintf(msg, sizeof(msg), "max:%.3f", calibration.max_value);
        slider.display.drawString(msg, 0, 30);
    }

    delay(2000);

    // Middle slider position capture
    display->drawString("Calibration", 0, 0);
    display->drawString("Set slider", 0, 10);
    display->drawString("to MIDLE", 0, 20);
    display->drawString("position", 0, 30);

    for( uint32_t i = 10; i > 0; i-- ){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        sliders[0].display.drawString(msg, 0, 40);
        delay(1000);
    }  

    {
        display->clear(TFT_BLACK);
        display->drawString("Sampling...", 0, 0);
        auto calibration = sliders[0].calibrationGet();
        snprintf(msg, sizeof(msg), "min:%.3f", calibration.min_value);
        display->drawString(msg, 0, 20);
        snprintf(msg, sizeof(msg), "min:%.3f", calibration.max_value);
        display->drawString(msg, 0, 30);
    }

    for( auto& slider : sliders ){
        auto calibration = slider.calibrationGet();
        slider.calibrationSetMidPoint(calibration);
        slider.calibrationLoad(calibration);
        snprintf(msg, sizeof(msg), "min:%.3f", calibration.min_value);
        slider.display.drawString(msg, 0, 20);
        snprintf(msg, sizeof(msg), "max:%.3f", calibration.max_value);
        slider.display.drawString(msg, 0, 30);
        snprintf(msg, sizeof(msg), "mid:%.3f", calibration.mid_value);
        slider.display.drawString(msg, 0, 40);
    }

    display->drawString("Calibration", 0, 0);
    display->drawString("complete", 0, 10);


    nvs_handle_t nvs_handle;
    nvs_open("CAL", NVS_READWRITE, &nvs_handle); 

    uint32_t i = 0;
    for( auto& slider : sliders ){
        char tmp[10];
        i++;
        snprintf(tmp, sizeof(tmp), "CH%lu", i);
        auto calibration = slider.calibrationGet();
        nvs_set_blob(nvs_handle, tmp, &calibration, sizeof(calibration));
    }
    nvs_close(nvs_handle);
    

    xSemaphoreGive(displaysMutex);
    delay(3000);
}

void Device::nvsInit( void ){
    esp_err_t err = nvs_flash_init();
    if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_LOGW("NVS", "Erasing NVS partition...");
        nvs_flash_erase();
        err = nvs_flash_init();
    };
    if (err == ESP_OK) {
        ESP_LOGI("NVS", "NVS partition initilized");
    } else {
        ESP_LOGE("NVS", "NVS partition initialization error: %d (%s)", err, esp_err_to_name(err));
    };
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
                delay(20);
            }
exit:
            xSemaphoreTake(displaysMutex, portMAX_DELAY);
            display.clear();
            display.setBrightness(start_brightness);
            xSemaphoreGive(displaysMutex);
            menuSaveAll();
            slider.displayIcon(true);
        }
        delay(10);
    }
}

void Device::init( void ){
    ws2812b_display_buffer = _ws2812_buffer;
    configure();
    nvsInit();
    menuInit();
    consoleInit();
    adcInit();

    gpio_config_t pullup_i2c = {
        .pin_bit_mask = (1 << GPIO_NUM_9),
        .mode = GPIO_MODE_OUTPUT,
    };

    gpio_config(&pullup_i2c);
    gpio_set_level(GPIO_NUM_9, 1);


    for( auto& slider : sliders ){
        slider.init(&_sliders_adc);
        delay(1);
    }
    virtDispInit(); // This must be here for normal real displays work
    
    bool calibration = true;
    for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
        if(_buttons_pressed_for_calibration[i]){
            if(sliders[i].adcRawRead() < 0.99){
                calibration = false;
                break;
            }
        }
    }
    
    if(calibration){
        clalibrate();
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("CAL", NVS_READONLY, &nvs_handle); 
    
    if(err != ESP_OK){
        nvs_close(nvs_handle);
        clalibrate();
    }else{
        uint32_t i = 0;
        for( auto& slider : sliders ){
            char tmp[10];
            i++;
            snprintf(tmp, sizeof(tmp), "CH%lu", i);
            auto calibration = slider.calibrationGet();
            size_t size = sizeof(calibration);
            nvs_get_blob(nvs_handle, tmp, &calibration, &size);
            if(size == sizeof(calibration)){
                slider.calibrationLoad(calibration);
            }
        }
        nvs_close(nvs_handle);
    }
}
