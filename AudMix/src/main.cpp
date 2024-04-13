#include <string>
#include <iostream>
#include <display.hpp>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_adc/adc_oneshot.h"
#include "nvs_flash.h"


#include "config.hpp"
#include "slider.hpp"
#include <SmartLeds.h>
#include <LovyanGFX.hpp>

#define PROMPT_STR CONFIG_IDF_TARGET
#define CMD_SET_ICON    "SET_ICON"

extern "C" {
    void app_main(void);
}

adc_oneshot_unit_handle_t adc1_handle;

Slider sliders[SLIDERS_COUNT];

SmartLed strip(STRIP_TYPE, STRIP_LED_COUNT, STRIP_PIN);

void displayTask( void *args ){
    //char tmp[10];
    while(1){
        for( auto& slider : sliders ){
            //float slider_pos = slider.readPercantage();
            //sprintf(tmp, "%.3f", (float)slider_pos);
            //slider.display.drawString(tmp, 10, 10);
            //if(slider.display)
            slider.update();
        }
        vTaskDelay(1);
    }
}


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
}

void adc_init( void ){
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = SLIDERS_ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
}

void virtdisp_init( void ){
    static LGFX_SSD1306 virt_display;
    auto cfg = virt_display.config();
    cfg.pin_sda = I2C_SDA_PIN;
    cfg.pin_scl = 12;
    virt_display.config(cfg);
    virt_display.init();
}

void consoleInit( void ){
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 32;

    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));
}

void app_main() {
    //esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);

    consoleInit();

    // UART0 pins reset, maybe useless?
	gpio_reset_pin(GPIO_NUM_20);
	gpio_reset_pin(GPIO_NUM_21);

    read_config();
    //strip.begin();

    adc_init();
    for( auto& slider : sliders ){
        slider.init(&adc1_handle);
        slider.calibrate();
        //auto calibration = slider.getCalibration();
        //sizeof(calibration);
    }

    //sliders[0].calibrate();
    
    delay(5000);
    // This shit need for real displays work ok, if no this, last display work very unstable
    virtdisp_init();

    xTaskCreate(displayTask, "test_task", 3000, NULL, 1000, NULL);
    xTaskCreate(consoleTask, "console_task", 5000, NULL, 2000, NULL);
}
