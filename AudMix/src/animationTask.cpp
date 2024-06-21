#include "animationTask.hpp"
#include "FreeRTOS/FreeRTOS.h"
#include "device.hpp"

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
    CRGB color_arr[STRIP_LED_COUNT][2];
    ESP_ERROR_CHECK_WITHOUT_ABORT(ws28xx_init(STRIP_PIN, STRIP_TYPE, STRIP_LED_COUNT, &ws2812_output_buffer));
    for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
        ws2812b_display_buffer[i].r = 0;
        ws2812b_display_buffer[i].g = 0;
        ws2812b_display_buffer[i].b = 0;
    }

    while (1){
        // Convert display buffer for interpolated steps
        // Add 1 extra bit for each color, but max brightness also div to 2
        for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
            for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
                stripConvertCRGB2br(ws2812b_display_buffer[i], &color_arr[i][0], &color_arr[i][1]);
            }
        }

        // Display colors
        for(uint32_t i = 0; i < 2; i++){
            for(uint32_t j = 0; j < STRIP_LED_COUNT; j++){
                ws2812_output_buffer[j] = color_arr[j][i];
            }
            ws28xx_update();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void stripAnimationTask( void *args ){
    while(1){
        vTaskDelay(pdMS_TO_TICKS(20));
		for( auto& slider : audMix.sliders ){
            slider.strip.update();
        }
    }
}
