
#include "esp_log.h"
#include "esp_console.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "esp_ws28xx.h"
#include "esp_log.h"
#include "device.hpp"

#define PROMPT_STR CONFIG_IDF_TARGET

CRGB *ws2812b_display_buffer;

void Device::adcInit( void ){
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = SLIDERS_ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &_sliders_adc));
}

void Device::update( void ){
    for( auto& slider : sliders ){
        slider.update();
    }
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
    char tmp[10];
    int i = 0;

    for( auto& slider : sliders ){
        sprintf(tmp, "test %d", i++);
        slider.display.setTextSize(1);
        slider.display.drawString(tmp, 0, 0);
    }
}

void Device::init( void ){
    ws2812b_display_buffer = _ws2812_buffer_a;
    configure();
    consoleInit();
    adcInit();

    gpio_config_t pullup_i2c = {
        .pin_bit_mask = (1 << GPIO_NUM_9),
        .mode = GPIO_MODE_OUTPUT,
    };

    gpio_config(&pullup_i2c);
    gpio_set_level(GPIO_NUM_9, 1);

    virtDispInit();

    for( auto& slider : sliders ){
        slider.init(&_sliders_adc);
        delay(1);
    }
}
