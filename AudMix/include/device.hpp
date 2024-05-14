#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_ws28xx.h"
#include "slider.hpp"
#include "system.hpp"

#define SLIDERS_ADC_UNIT    ADC_UNIT_1
#define I2C_SDA_PIN         8
#define SLIDERS_COUNT       5
#define STRIP_TYPE          WS2812B
#define STRIP_LED_COUNT     101
#define STRIP_PIN           6
#define ICON_WIDTH          60
#define ICON_HEIGHT         44

void stripTask( void *args );

class Device{
    adc_oneshot_unit_handle_t _sliders_adc;
    void nvsInit( void );
    void adcInit( void );
    void consoleInit( void );
    void virtDispInit( void );
    void configure( void );
    void clalibrate( void );
    
    Battery battery;
    CRGB *_ws2812_output_buffer;
    CRGB _ws2812_buffer[STRIP_LED_COUNT];
    bool _displays_update = true;
public:
    uint8_t _buttons_pressed_for_calibration[SLIDERS_COUNT];
    Slider sliders[SLIDERS_COUNT];
    void init( void );
};
