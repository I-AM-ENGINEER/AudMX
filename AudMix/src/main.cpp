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

extern "C" {
    void app_main(void);
}

SemaphoreHandle_t displaysMutex;
EventGroupHandle_t buttonsPressedEventGroup;
EventGroupHandle_t buttonsReleasedEventGroup;

Device audMix; // I dont know, why i create class for entire device...

void app_main() {
    displaysMutex = xSemaphoreCreateMutex();
	buttonsPressedEventGroup = xEventGroupCreate();
	buttonsReleasedEventGroup = xEventGroupCreate();
    esp_log_level_set("*", ESP_LOG_NONE);
    //esp_log_level_set("NimBLE", ESP_LOG_INFO);

    audMix.init();

    ble_init();

    xTaskCreate(communicationTask,                  "consoleTask", 	    8000, NULL, 15, NULL);
    xTaskCreate(menuTask, 	                        "menuTask", 	    3000, NULL, 5,  NULL);
    xTaskCreate(stripAnimationTask,                 "stripAniTask", 	1000, NULL, 25, NULL);
    xTaskCreate(readPotentiometersButtonsTask, 		"analogTask", 		3000, NULL, 20, NULL);
    xTaskCreate(displayTask,	                    "displaysTask", 	3000, NULL, 10, NULL);
    xTaskCreate(stripTask, 		                    "stripTask", 		3000, NULL, 30, NULL);
}
