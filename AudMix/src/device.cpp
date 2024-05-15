
#include "esp_log.h"
#include "esp_console.h"
#include "esp_adc/adc_oneshot.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "esp_ws28xx.h"
#include "esp_log.h"
#include "device.hpp"

#define PROMPT_STR CONFIG_IDF_TARGET

CRGB *ws2812b_display_buffer;
extern EventGroupHandle_t buttonsPressedEventGroup;

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

void Device::update( void ){

    while (1){
        delay(10);
        EventBits_t bits;
		bits = xEventGroupClearBits(buttonsPressedEventGroup, (1 << _button_for_stip_config));
        if(bits & (1 << _button_for_stip_config)){
            sliders[0].displayIcon(false);
            sliders[0].display.clear(TFT_BLACK);//display->drawString("Calibration", 0, 0);
            delay(1000);
            sliders[0].displayIcon(true);
        }
    }
    
}

void Device::init( void ){
    ws2812b_display_buffer = _ws2812_buffer;
    configure();
    nvsInit();
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
