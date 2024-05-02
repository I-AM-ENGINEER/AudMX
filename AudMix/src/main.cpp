#include <string>
#include <iostream>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "device.hpp"

#define PROMPT_STR CONFIG_IDF_TARGET
#define CMD_SET_ICON    "SET_ICON"

extern "C" {
    void app_main(void);
}

Device audMix;

extern CRGB *ws2812b_display_buffer;

static inline void stripConvert2br(uint8_t input, uint8_t *out_a, uint8_t *out_b) {
    *out_a = input/2;
    if(input & 0x01){
        *out_b = input/2 + 1;
    }else{
        *out_b = input/2;
    }
}

static inline void stripConvertCRGB2br( CRGB color, CRGB *color_a, CRGB *color_b ){
    stripConvert2br(color.r, &color_a->r, &color_b->r);
    stripConvert2br(color.g, &color_a->g, &color_b->g);
    stripConvert2br(color.b, &color_a->b, &color_b->b);
}

void stripTask( void *args ){
    CRGB *ws2812_output_buffer;
    ESP_ERROR_CHECK_WITHOUT_ABORT(ws28xx_init(STRIP_PIN, STRIP_TYPE, STRIP_LED_COUNT, &ws2812_output_buffer));
    for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
        ws2812b_display_buffer[i].r = 0;
        ws2812b_display_buffer[i].g = 0;
        ws2812b_display_buffer[i].b = 0;
    }

    CRGB color_arr[STRIP_LED_COUNT][2];
    while (1){
        // Convert display buffer for interpolated steps
        // Add 1 extra bit for each color, but max brightness also div to 2
        for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
            for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
                stripConvertCRGB2br(ws2812b_display_buffer[i], &color_arr[i][0], &color_arr[i][1]);
                // Divider for 4 work, but 66Hz flick visible
                //stripConvertCRGB2br(ws2812b_display_buffer[i], &color_tmp[0], &color_tmp[1]);
                //stripConvertCRGB2br(color_tmp[0], &color_arr[i][0], &color_arr[i][1]);
                //stripConvertCRGB2br(color_tmp[1], &color_arr[i][3], &color_arr[i][2]);
            }
        }

        // Display colors
        for(uint32_t i = 0; i < 2; i++){
            for(uint32_t j = 0; j < STRIP_LED_COUNT; j++){
                ws2812_output_buffer[j] = color_arr[j][i];
            }
            ws28xx_update();
            vTaskDelay(1);
        }
    }
}

void displayTask( void *args ){
    while(1){
        audMix.update();
        vTaskDelay(50);
    }
}

void stripAnimation( void *args ){
    while(1){
        vTaskDelay(20);
        for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
            audMix.sliders[i].strip.update();
        }
    }
}

/*
void consoleTask( void *args ){
    std::string str = "";

    while (1){
        int res = 0;
        std::cin >> str;

        if(str.compare(0, strlen(CMD_SET_ICON), CMD_SET_ICON) == 0){
            uint8_t icon_array[ICON_SIZE_PX*ICON_SIZE_PX/8];

            std::string afterPrefix = str.substr(strlen(CMD_SET_ICON)+1);
            int display_num = std::atoi(afterPrefix.c_str());
            if((display_num < 0) || (display_num >= SLIDERS_COUNT)){
                res = 1;
                goto exit;
            }
            
            std::cin.read((char*)icon_array, sizeof(icon_array));
            sliders[display_num].setIcon(icon_array, ICON_SIZE_PX, ICON_SIZE_PX);
            sliders[display_num].displayIcon(true);
        }else{
            res = -1;
        }
exit:
        if(res == 0){
            std::cout << "OK" << std::endl;
        }else{
            std::cout << "ERROR: " << res << std::endl;
        }
        vTaskDelay(1);
    }
}*/

void app_main() {
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_ERROR);

    audMix.init();
    xTaskCreate(stripAnimation, "animation", 3000, NULL, 1900, NULL);
    xTaskCreate(displayTask, "test_task", 8000, NULL, 1000, NULL);
    xTaskCreate(stripTask, "strip_", 3000, NULL, 2000, NULL);
    //xTaskCreate(consoleTask, "console_task", 5000, NULL, 2000, NULL);
}
