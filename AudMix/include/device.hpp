#pragma once

#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_ws28xx.h"
#include "slider.hpp"
#include "battery.hpp"

#define SLIDERS_ADC_UNIT    ADC_UNIT_1
#define I2C_SDA_PIN         8
#define SLIDERS_COUNT       5
#define STRIP_TYPE          WS2812B
#define STRIP_LED_COUNT     101
#define STRIP_PIN           6
#define ICON_WIDTH          60
#define ICON_HEIGHT         44

class Device{
    adc_oneshot_unit_handle_t _sliders_adc;
    void nvsInit( void );
    void adcInit( void );
    void consoleInit( void );
    void virtDispInit( void );
    void configure( void );
    void clalibrate( void );
    void menuInit( void );
    void menuSaveAll( void );
    void menuUpdateSettings( void );

    CRGB *_ws2812_output_buffer;
    CRGB _ws2812_buffer[STRIP_LED_COUNT];
    bool _displays_update = true;
    bool _buttons_pressed_for_calibration[SLIDERS_COUNT] = {false};
    uint8_t _button_for_stip_config = -1;
    uint8_t _button_for_bluetooth_config = -1;
    uint8_t _button_next = -1;
    uint8_t _button_select = -1;
    bool _is_audioreactive = false;
public:
    Battery battery;
    Slider sliders[SLIDERS_COUNT];
    bool isAudioReactive( void ){
        return _is_audioreactive;
    }
    bool isAudioReactive( bool state );
    void init( void );
    void update( void );
};

extern Device audMix;
