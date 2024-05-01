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

void displayTask( void *args ){
    while(1){
        audMix.update();
        vTaskDelay(1);
    }
}

void stripAnimation( void *args ){
    while(1){
        for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
            audMix.sliders[i].strip.update();
        }
        vTaskDelay(20);
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
    xTaskCreate(stripAnimation, "animation", 1000, NULL, 500, NULL);
    xTaskCreate(displayTask, "test_task", 8000, NULL, 1000, NULL);
    xTaskCreate(stripTask, "strip_", 3000, NULL, 2000, NULL);
    //xTaskCreate(consoleTask, "console_task", 5000, NULL, 2000, NULL);
}
