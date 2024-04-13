#pragma once

#include "config.hpp"
#include <SmartLeds.h>
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc_cal.h"
#include "display.hpp"
#include "system.hpp"
#include <stdio.h>
#include <string>

class Slider{
    // For calibration
    uint16_t _potentiometer_max_value;
    uint16_t _potentiometer_min_value;
    uint8_t _ico_buffer[DISPLAY_HEIGHT*DISPLAY_WIDTH/8];
    uint16_t _ico_size_x = 0;
    uint16_t _ico_size_y = 0;
    bool _ico_display = false;

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
    int32_t init( void );
    int32_t setIcon( uint8_t *icon, uint32_t size_x, uint32_t size_y );
    bool displayIcon( bool display_icon );
    bool displayIcon( void );
    void update( void );
    void calibrate( void );
    float readPercantage( void );
protected:
    config_t _cfg;
};

void Slider::calibrate( void ){
    char msg[11];
    display.setTextSize(1.0f);
    display.drawString("Calibration", 0, 0);
    display.drawString("Set slider", 0, 10);
    display.drawString("to minimum", 0, 20);
    display.drawString("position", 0, 30);
    for(uint32_t i = 3; i > 0; i--){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        display.drawString(msg, 0, 40);
        delay(1000);
    }
    
    display.clear(TFT_BLACK);
    display.setTextSize(2.0f);
    display.drawString("OK", 0, 0);
    

    //delay(1000);
    //display.endWrite();
}

bool Slider::displayIcon( bool display_icon ){
    _ico_display = display_icon;
    return _ico_display;
}

bool Slider::displayIcon( void ){
    return _ico_display;
}

void Slider::update( void ){
    display.startWrite();
    //display.draw_bitmap(0,0,_ico_buffer,_ico_size_x,_ico_size_y,0,TFT_BLACK);
    display.drawCircle(0,0, 10);
    display.drawString("123", 0, 0);
    //display.draw_xbitmap
    //if(_ico_display){
        
    //}
    display.endWrite();
}

int32_t Slider::setIcon( uint8_t *icon, uint32_t size_x, uint32_t size_y ){
    size_t icon_size = size_x*size_y/8;
    if(sizeof(_ico_buffer) < icon_size){
        return 1;
    }
    _ico_size_x = size_x;
    _ico_size_y = size_y;
    memcpy(_ico_buffer, icon, icon_size);
    return 0;
}

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

int32_t Slider::init( void ){
    display.init();
    // TODO:
    _potentiometer_min_value = 0;
    _potentiometer_max_value = 4000;

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(*_cfg.adc_unit, _cfg.adc_channel, &config));
    return 0;
}

float Slider::readPercantage( void ){
    int adc_value;
    adc_oneshot_read(*_cfg.adc_unit, _cfg.adc_channel, &adc_value);
    float percentage = (float)adc_value / 4095.0f;
    return percentage;
}
