#include <string.h>
#include <stdio.h>
#include <display.hpp>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "esp_event.h"
#include "esp_log.h"

#include "config.hpp"
#include "slider.hpp"
#include <SmartLeds.h>
#include <LovyanGFX.hpp>

extern "C" {
    void app_main(void);
}

adc_oneshot_unit_handle_t adc1_handle;

Slider sliders[SLIDERS_COUNT];
SmartLed strip(STRIP_TYPE, STRIP_LED_COUNT, STRIP_PIN);

void displayUpdate( void *args ){
    while(1){
        for( auto& slider : sliders ){
            char tmp[10];
            float slider_pos = slider.readPercantage();
            sprintf(tmp, "%.3f", (float)slider_pos);
            slider.display.startWrite();
            slider.display.drawString(tmp, 10, 10);
            slider.display.endWrite();
        }
    }
}

void app_main() {
    esp_log_level_set("*", ESP_LOG_DEBUG);		//DISABLE ESP32 LOGGING ON UART0

    // UART0 pins reset
	gpio_reset_pin(GPIO_NUM_20);
	gpio_reset_pin(GPIO_NUM_21);
    
	gpio_reset_pin(GPIO_NUM_0);
	gpio_reset_pin(GPIO_NUM_1);
	gpio_reset_pin(GPIO_NUM_2);
	gpio_reset_pin(GPIO_NUM_3);
	gpio_reset_pin(GPIO_NUM_4);
	gpio_reset_pin(GPIO_NUM_5);

    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_DISABLE);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_DISABLE);

    read_config();
    strip.begin();

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    for( auto& slider : sliders ){
        slider.init();
    }

    // This shit need for real displays work ok, if no this, last display work very unstable
    {
        static LGFX_SSD1306 virt_display;
        auto cfg = virt_display.config();
        cfg.pin_sda = I2C_SDA_PIN;
        cfg.pin_scl = 12;
        virt_display.config(cfg);
        virt_display.init();
    }
    
    xTaskCreate(displayUpdate, "test_task", 1000, NULL, 1000, NULL);
}
