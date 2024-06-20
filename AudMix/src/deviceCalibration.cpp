#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "freertos/event_groups.h"
#include "device.hpp"

extern SemaphoreHandle_t displaysMutex;

void Device::clalibrate( void ){
    char msg[11];
    vTaskDelay(pdMS_TO_TICKS(100));

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
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Maximum slider position capture
    display->drawString("Calibration", 0, 0);
    display->drawString("Set slider", 0, 10);
    display->drawString("to MAXIMUM", 0, 20);
    display->drawString("position", 0, 30);

    for( uint32_t i = 8; i > 0; i-- ){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        sliders[0].display.drawString(msg, 0, 40);
        vTaskDelay(pdMS_TO_TICKS(1000));
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

    vTaskDelay(pdMS_TO_TICKS(2000));

    // Middle slider position capture
    display->drawString("Calibration", 0, 0);
    display->drawString("Set slider", 0, 10);
    display->drawString("to MIDLE", 0, 20);
    display->drawString("position", 0, 30);

    for( uint32_t i = 10; i > 0; i-- ){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        sliders[0].display.drawString(msg, 0, 40);
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    vTaskDelay(pdMS_TO_TICKS(3000));
}