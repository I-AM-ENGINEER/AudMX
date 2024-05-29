#include <string>
#include <iostream>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "device.hpp"
#include "nvs_flash.h"
#include "AudMX_logo.h"

#define PROMPT_STR CONFIG_IDF_TARGET
#define CMD_SET_ICON    "SET_ICON"
#define CMD_VOL_UPDATE  "VOL"

extern "C" {
    void app_main(void);
}


SemaphoreHandle_t displaysMutex;
EventGroupHandle_t buttonsPressedEventGroup;
EventGroupHandle_t buttonsReleasedEventGroup;

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
            delay(1);
        }
    }
}

void readTask( void *args ){
    uint16_t positions_old[SLIDERS_COUNT] = {0};
	bool buttons_old[SLIDERS_COUNT] = {false};
    int64_t timestamp = 3'000'000;

    while (1){
        bool need_positions_send = false;
        // Check updated value
        for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
            audMix.sliders[i].updatePosition();
            float position_current_f = audMix.sliders[i].readPosition();
            uint16_t position_current = (uint16_t)std::round(position_current_f * 1023.0f);
            if(!audMix.isAudioReactive()){
                audMix.sliders[i].strip.volumeSet(position_current_f);
            }
            if(positions_old[i] != position_current){
                need_positions_send = true;
                positions_old[i] = position_current;
            }

			bool button = audMix.sliders[i].readButton();
			int button_num = (int)audMix.sliders[i].readButtonNum();
			if(buttons_old[i] != button){
				if(button){
					xEventGroupSetBits(buttonsPressedEventGroup, (1 << i));
				}else{
					xEventGroupSetBits(buttonsReleasedEventGroup, (1 << i));
				}
				buttons_old[i] = button;
			}
			if(button_num >= 0){
				EventBits_t bits;
				bits = xEventGroupClearBits(buttonsPressedEventGroup, (1 << i));
				if(bits & (1 << i)){
					std::cout << "BUTTON:pressed|" << button_num << std::endl;
				}
				bits = xEventGroupClearBits(buttonsReleasedEventGroup, (1 << i));
				if(bits & (1 << i)){
					std::cout << "BUTTON:released|" << button_num << std::endl;
				}
			}
        }

        // Send every second send slider position
        if((esp_timer_get_time() - timestamp) >= 1000000LL){
            need_positions_send = true;
        }

		// Send position no faster than 100ms period
        if(((esp_timer_get_time() - timestamp) >= 100000LL) && need_positions_send){
            timestamp = esp_timer_get_time();
			for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
                std::cout << (int)positions_old[i];
                if(i < (SLIDERS_COUNT - 1)){
                    std::cout << "|";
                }
            }
            std::cout << std::endl;
        }
        delay(2);
    }
}

void displayTask( void *args ){
    for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
        audMix.sliders[i].display.setBrightness(255);
        audMix.sliders[i].setIcon(AudMX_bitmap[i], 60, 44);
        audMix.sliders[i].displayIcon(true);
        audMix.sliders[i].updateDisplay();
    }
    delay(1000);
    xSemaphoreTake(displaysMutex, portMAX_DELAY);
    for(uint32_t i = 255; i > 20; i-=2){
        for( auto& slider : audMix.sliders ){
            slider.display.setBrightness(i);
        }
        delay(5);
    }
    xSemaphoreGive(displaysMutex);
    
    while(1){
        for( auto& slider : audMix.sliders ){
            slider.updateDisplay();
        }
        delay(100);
    }
}

void stripAnimation( void *args ){
    while(1){
        delay(20);
		for( auto& slider : audMix.sliders ){
            slider.strip.update();
        }
    }
}

void consoleTask( void *args ){
    std::string str = "";

    while (1){
        int res = 0;
        std::getline(std::cin, str);

        if(str.compare(0, strlen(CMD_SET_ICON), CMD_SET_ICON) == 0){
            uint8_t icon_array[((ICON_WIDTH+7)/8)*ICON_HEIGHT];
			size_t prefixLen = strlen(CMD_SET_ICON);
			if (str.size() <= prefixLen + 1) {
                res = -2;
				goto exit;
            }
            std::string afterPrefix = str.substr(prefixLen+1);
            int display_num = std::atoi(afterPrefix.c_str());
            if((display_num < 0) || (display_num >= SLIDERS_COUNT)){
                res = 1;
                goto exit;
            }
            std::cout << "Send " << sizeof(icon_array) << " bytes" << std::endl;
            
            std::cin.read((char*)icon_array, sizeof(icon_array));
            audMix.sliders[display_num].setIcon(icon_array, ICON_WIDTH, ICON_HEIGHT);
            audMix.sliders[display_num].displayIcon(true);
        }else if(str.compare(0, strlen(CMD_VOL_UPDATE), CMD_VOL_UPDATE) == 0){
            float volumeValues[5];
            const char *tokens = str.c_str() + 4; // Skip "VOL:"
            char *token;
            int index = 0;
            token = strtok((char *)tokens, "|");
            while (token != NULL && index < 5) {
                volumeValues[index++] = atof(token);
                token = strtok(NULL, "|");
            }
            
            if(audMix.isAudioReactive()){
                for(uint32_t i = 0; i < 5; i++){
                    audMix.sliders[i].strip.volumeSet(volumeValues[i]);
                }
            }
        }else{
            res = -1;
        }
exit:
        if(res == 0){
            std::cout << "OK" << std::endl;
        }else{
            std::cerr << "ERROR: " << res << std::endl;
        }
        delay(1);
    }
}

void device_task( void *args ){
    audMix.update();
}

void app_main() {
    displaysMutex = xSemaphoreCreateMutex();
	buttonsPressedEventGroup = xEventGroupCreate();
	buttonsReleasedEventGroup = xEventGroupCreate();
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_NONE);

    audMix.init();


    xTaskCreate(consoleTask, 	"console_task", 	8000, NULL, 1500, NULL);
    xTaskCreate(device_task, 	"device_task", 	    3000, NULL, 3000, NULL);
    xTaskCreate(stripAnimation, "strip_ani_task", 	1000, NULL, 1900, NULL);
    xTaskCreate(readTask, 		"analog_task", 		3000, NULL, 1200, NULL);
    xTaskCreate(displayTask,	"displays_task", 	3000, NULL, 1000, NULL);
    xTaskCreate(stripTask, 		"strip_task", 		3000, NULL, 2000, NULL);
}
