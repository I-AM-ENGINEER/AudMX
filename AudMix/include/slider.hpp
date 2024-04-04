#pragma once

#include <SmartLeds.h>
#include "display.hpp"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc_cal.h"

class Slider{
    // For calibration
    uint16_t _potentiometer_max_value;
    uint16_t _potentiometer_min_value;

    struct config_t{
        int16_t pin_scl                     =             -1;
        int16_t pin_sda                     =             -1;
        uint8_t i2c_addr                    =           0x3C;
        adc_oneshot_unit_handle_t *adc_unit =        nullptr;
        adc_channel_t adc_channel           =  ADC_CHANNEL_0;
        SmartLed *led_strip                 =        nullptr;
        uint16_t led_start                  =              0;
        uint16_t led_count                  =              0;
        bool double_leds                    =           true;
    };

public:
    LGFX_SSD1306 display;
    
    Slider( void ){};
    const config_t& config( void ) const { return _cfg; }
    void config( const config_t& cfg );
    void init( void );
    float readPercantage( void );
protected:
    config_t _cfg;
};

void Slider::config( const config_t& cfg ) { 
    _cfg = cfg;
    {
        auto cfg = display.config();
        cfg.i2c_adress  =     _cfg.i2c_addr;
        cfg.pin_scl     =      _cfg.pin_scl;
        cfg.pin_sda     =      _cfg.pin_sda;
        display.config(cfg);
    }
}

void Slider::init( void ){
    display.init();
    // TODO:
    _potentiometer_min_value = 0;
    _potentiometer_max_value = 4000;
}

float Slider::readPercantage( void ){
    int adc_value;
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    // Maybe do this in init, not in runtime?
    ESP_ERROR_CHECK(adc_oneshot_config_channel(*_cfg.adc_unit, _cfg.adc_channel, &config));
    adc_oneshot_read(*_cfg.adc_unit, _cfg.adc_channel, &adc_value);
    float percentage = (float)adc_value / 4095.0f;
    return percentage;
}
