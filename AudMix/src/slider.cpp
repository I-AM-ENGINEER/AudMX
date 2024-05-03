#include "slider.hpp"
#include "display.hpp"
#include "system.hpp"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include <stdio.h>
#include <string>

// Thanks chatgpt for this function
static  float exponentialToLinear( float value, float middleValue ) {
    if (value < middleValue) {
        return value / (2.0f * middleValue);
    } else {
        return 0.5f + (value - middleValue) / (2.0f * (1.0f - middleValue));
    }
}

// And this...
static float map( float value, float fromLow, float fromHigh, float toLow, float toHigh ) {
    if (fromHigh - fromLow == 0) {
        return 0.0;
    }

    float ratio = (value - fromLow) / (fromHigh - fromLow);
    return toLow + ratio * (toHigh - toLow);
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
    
    if(readButton()){
        display.drawString("Button!", 0, 20);
    }else{
        snprintf(msg, sizeof(msg), "%.1f    ", readPosition() * 100.0f);
        display.drawString("       ", 0, 20);
        display.drawString(msg, 0, 0);
    }
    

    if(_ico_display){
        //display.drawBitmap(0, 0, _ico_buffer, _ico_size_x, _ico_size_y, 0xFFFFFF, 0x000000);
    }
}

int32_t Slider::setIcon( const uint8_t *icon, uint32_t size_x, uint32_t size_y ){
    size_t icon_size = ((size_x+7)/8) * size_y;
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
    {
        auto cfg = strip.config();
        cfg.led_count = _cfg.led_count;
        cfg.led_start = _cfg.led_start;
        cfg.led_strip = _cfg.led_strip;
        cfg.double_leds = _cfg.double_leds;
        strip.config(cfg);
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
    
    setIcon(bitmap_test, 60, 44);
    displayIcon(true);
    return 0;
}

float Slider::adcRawRead( void ){
    const int sample_count = 4;
    int adc_val;
    int accumulator = 0;
    adc_oneshot_read(*_adc_handle, _cfg.adc_channel, &adc_val); // without this, some noise in output
    for(uint32_t i = 0; i < (uint32_t)sample_count; i++){
        adc_oneshot_read(*_adc_handle, _cfg.adc_channel, &adc_val);
        accumulator += adc_val;
    }
    
    float percentage = (float)accumulator / 4095.0f / (float)sample_count;
    return percentage;
}

float Slider::adcRawReadAccuracy( void ){
    const int sample_count = 16384;
    float accumulator = 0.0f;
    for(uint32_t i = 0; i < (uint32_t)sample_count; i++){
        accumulator += adcRawRead();
    }
    return accumulator / (float)sample_count;
}

bool Slider::readButton( void ){
    float adc_read = adcRawRead();
    if(adc_read > 0.99){
        return true;
    }
    return false;
}

float Slider::readPosition( void ){
    float adc_read = adcRawRead();
    float remaped = map(adc_read, _calibration.min_value, _calibration.max_value, 0.0f, 1.0f);
    float remaped_mid = map(_calibration.mid_value, _calibration.min_value, _calibration.max_value, 0.0f, 1.0f);
    float exponential = std::clamp(remaped, 0.0f, 1.0f);
    float linear = exponentialToLinear(exponential, remaped_mid);
    return linear;
}
