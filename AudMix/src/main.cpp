#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "device.hpp"
#include "displayTask.hpp"
#include "menuTask.hpp"
#include "sliderPosTask.hpp"
#include "animationTask.hpp"
#include "commTask.hpp"
#include "bt_spp.h"
#include "driver/spi_master.h"

extern "C" {
    void app_main(void);
}

extern CRGB *ws2812b_display_buffer;

static int64_t last_ping;

SemaphoreHandle_t displaysMutex;
EventGroupHandle_t buttonsPressedEventGroup;
EventGroupHandle_t buttonsReleasedEventGroup;

Device audMix; // I dont know, why i create class for entire device...

TaskHandle_t stripAnimationTaskHandle;
TaskHandle_t stripTaskHandle;
TaskHandle_t displayTaskHandle;
TaskHandle_t readPotentiometersButtonsTaskHandle;
TaskHandle_t menuTaskHandle;

void sleepTask( void *args );
void batteryCheckTask( void *args );

void app_main() {
    displaysMutex = xSemaphoreCreateMutex();
	buttonsPressedEventGroup = xEventGroupCreate();
	buttonsReleasedEventGroup = xEventGroupCreate();
    esp_log_level_set("*", ESP_LOG_NONE);
    //esp_log_level_set("NimBLE", ESP_LOG_INFO);

    audMix.init();

    ble_init();
    xTaskCreate(sleepTask, 		                    "sleepTask", 		3000, NULL, 1,  NULL);
    xTaskCreate(batteryCheckTask,                   "batteryTask", 	    1000, NULL, 2,  NULL);
    xTaskCreate(communicationTask,                  "consoleTask", 	    8000, NULL, 15, NULL);
    xTaskCreate(menuTask, 	                        "menuTask", 	    3000, NULL, 5,  &menuTaskHandle);
    xTaskCreate(stripAnimationTask,                 "stripAniTask", 	1000, NULL, 25, &stripAnimationTaskHandle);
    xTaskCreate(readPotentiometersButtonsTask, 		"analogTask", 		3000, NULL, 20, &readPotentiometersButtonsTaskHandle);
    xTaskCreate(displayTask,	                    "displaysTask", 	3000, NULL, 10, &displayTaskHandle);
    xTaskCreate(stripTask, 		                    "stripTask", 		3000, NULL, 30, &stripTaskHandle);
}

void batteryCheckTask( void *args ){
    gpio_config_t pin_cfg = {
        .pin_bit_mask = (1 << GPIO_NUM_7),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&pin_cfg);
    
    while (1){
        float battery_level = audMix.battery.readLevel();
        bool low_charge = battery_level < 0.2;
        bool charger_connected = gpio_get_level(GPIO_NUM_7) == 1 ? true : false;
        if(low_charge && !charger_connected){
            ws2812b_display_buffer[0].r = 127;
            ws2812b_display_buffer[0].g = 0;
            ws2812b_display_buffer[0].b = 0;
            vTaskDelay(100);
        }
        ws2812b_display_buffer[0].r = 0;
        ws2812b_display_buffer[0].g = 0;
        ws2812b_display_buffer[0].b = 0;
        vTaskDelay(3000);
    }
}

void sleepPing( void ){
    last_ping = esp_timer_get_time();
}

void sleepTask( void *args ){
    sleepPing();
    while (1){
        int64_t current_time = esp_timer_get_time();
        if(ble_is_connected()){
            sleepPing();
        }
        if((current_time - last_ping) > 3000000'000'000){
            vTaskSuspend(menuTaskHandle);
            vTaskSuspend(readPotentiometersButtonsTaskHandle);
            vTaskSuspend(stripAnimationTaskHandle);
            vTaskSuspend(displayTaskHandle);
            for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
                audMix.sliders[i].display.sleep();
            }
            for(uint32_t i = 0; i < STRIP_LED_COUNT; i++){
                ws2812b_display_buffer[i].r = 0;
                ws2812b_display_buffer[i].g = 0;
                ws2812b_display_buffer[i].b = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            vTaskSuspend(stripTaskHandle);

            while((current_time - last_ping) > 30'000'000){
                if(ble_is_connected()){
                    sleepPing();
                }
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            
            for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
                audMix.sliders[i].display.wakeup();
            }
            
            vTaskResume(menuTaskHandle);
            vTaskResume(readPotentiometersButtonsTaskHandle);
            vTaskResume(stripAnimationTaskHandle);
            vTaskResume(displayTaskHandle);
            vTaskResume(stripTaskHandle);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
