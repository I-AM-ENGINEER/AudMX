#pragma once

#include "esp_adc/adc_oneshot.h"
#include "SmartLeds.h"
//#include "config.hpp"
#include "slider.hpp"
#include "system.hpp"

#define SLIDERS_ADC_UNIT    ADC_UNIT_1
#define I2C_SDA_PIN         8
#define SLIDERS_COUNT       5
#define STRIP_TYPE          LED_WS2812B
#define STRIP_LED_COUNT     101
#define STRIP_PIN           6

class Device{
    adc_oneshot_unit_handle_t _sliders_adc;
    void adcInit( void );
    void consoleInit( void );
    void virtDispInit( void );
    void configure( void );
public:
    Battery battery;
    Slider sliders[SLIDERS_COUNT];
    //SmartLed strip;
    Device( void ); 
        //: strip(STRIP_TYPE, STRIP_LED_COUNT, STRIP_PIN) {}
    
    void init( void );
};