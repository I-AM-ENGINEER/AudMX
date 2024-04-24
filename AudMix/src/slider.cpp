#include "slider.hpp"
#include "display.hpp"
#include "system.hpp"

#include <stdio.h>
#include <string>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

void Slider::calibrate( void ){
    char msg[11];
    float accumulator = 0;
    float raw_value;
    float min_position;
    float max_position;

    // Minimum slider position set message
    display.setTextSize(1.0f);
    display.drawString("Calibration", 0, 0);
    display.drawString("Set slider", 0, 10);
    display.drawString("to MINIMUM", 0, 20);
    display.drawString("position", 0, 30);
    for(uint32_t i = 3; i > 0; i--){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        display.drawString(msg, 0, 40);
        delay(1000);
    }
    
    // Minimum slider position capture
    display.clear(TFT_BLACK);
    display.drawString("Sampling...", 0, 0);
    display.setTextSize(2.0f);
    for(uint32_t i = 0; i < 1024; i++){
        raw_value = readPercantage();
        accumulator += raw_value;
        snprintf(msg, sizeof(msg), "%.3f", raw_value);
        display.drawString(msg, 0, 10);
    }
    min_position = accumulator/1024.0f;
    accumulator = 0;

    // Maximum slider position set message
    display.clear(TFT_BLACK);
    display.setTextSize(1.0f);
    display.drawString("Calibration", 0, 0);
    display.drawString("Set slider", 0, 10);
    display.drawString("to MAXIMUM", 0, 20);
    display.drawString("position", 0, 30);
    for(uint32_t i = 3; i > 0; i--){
        snprintf(msg, sizeof(msg), "in %lu ...", i);
        display.drawString(msg, 0, 40);
        delay(1000);
    }

    // Maxomum slider position capture
    display.clear(TFT_BLACK);
    display.setTextSize(1.0f);
    display.drawString("Sampling...", 0, 0);
    display.setTextSize(2.0f);
    for(uint32_t i = 0; i < 1024; i++){
        raw_value = readPercantage();
        accumulator += raw_value;
        snprintf(msg, sizeof(msg), "%.3f", raw_value);
        display.drawString(msg, 0, 10);
    }
    max_position = accumulator/1024.0f;
    
    display.clear(TFT_BLACK);
    display.setTextSize(1.0f);
    display.drawString("Calibration", 0, 0);
    display.drawString("complete!", 0, 10);
    snprintf(msg, sizeof(msg), "min: %.3f", min_position);
    display.drawString(msg, 0, 20);
    snprintf(msg, sizeof(msg), "max: %.3f", max_position);
    display.drawString(msg, 0, 30);

    _calibration.max_value = max_position;
    _calibration.min_value = min_position;
}

bool Slider::displayIcon( bool display_icon ){
    _ico_display = display_icon;
    return _ico_display;
}

bool Slider::displayIcon( void ){
    return _ico_display;
}

void Slider::update( void ){
    display.setTextSize(1.0f);
    char msg[11];
    //display.draw_bitmap(0,0,_ico_buffer,_ico_size_x,_ico_size_y,0,TFT_BLACK);
    snprintf(msg, sizeof(msg), "%.0f", readPercantage() * 4095.0f);
    display.drawString(msg, 0, 0);
    //display.draw_xbitmap
    //if(_ico_display){
        
    //}
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

int32_t Slider::init( adc_oneshot_unit_handle_t* adc_handle ){
    _adc_handle = adc_handle;

    display.init();
    
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(*_adc_handle, _cfg.adc_channel, &config));
    return 0;
}

float Slider::readPercantage( void ){
    int adc_val;
    int accumulator = 0;
    for(uint32_t i = 0; i < 4; i++){
        adc_oneshot_read(*_adc_handle, _cfg.adc_channel, &adc_val);
        accumulator += adc_val;
    }
    
    float percentage = (float)accumulator / 4.0f / 4095.0f;
    return percentage;
}