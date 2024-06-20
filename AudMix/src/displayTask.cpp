#include "displayTask.hpp"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "device.hpp"
#include "AudMX_logo.h"

extern SemaphoreHandle_t displaysMutex;

void displayTask( void *args ){
    for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
        audMix.sliders[i].display.setBrightness(255);
        audMix.sliders[i].setIcon(AudMX_bitmap[i], 60, 44);
        audMix.sliders[i].displayIcon(true);
        audMix.sliders[i].updateDisplay();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    xSemaphoreTake(displaysMutex, portMAX_DELAY);
    for(uint32_t i = 255; i > 20; i -= 2){
        for( auto& slider : audMix.sliders ){
            slider.display.setBrightness(i);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    xSemaphoreGive(displaysMutex);
    
    while(1){
        for( auto& slider : audMix.sliders ){
            slider.updateDisplay();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
