#pragma once

#include "esp_ws28xx.h"
#include <stdint.h>

class StripFrame{
    struct config_t{
        CRGB *led_strip                     =        nullptr;
        uint16_t led_start                  =              0;
        uint16_t led_count                  =              0;
        bool double_leds                    =           true;     
    };
public:
    const config_t& config( void ) const { return _cfg; }
    void config( const config_t& cfg );
    void show( void );
    void setStaticColor( CRGB color );
    CRGB get_pixel_color( uint16_t pixelN );
    void color_update( void );
    void update( void );
protected:
    uint8_t _hsv_color;
    uint8_t _hsv_brightness;
    CRGB *_segment                           =        nullptr;
    config_t _cfg;
};
