#pragma once

#include "esp_ws28xx.h"
#include <stdint.h>

typedef enum {
    STRIP_ANIMATION_RAINBOW = 0,
    STRIP_ANIMATION_STATIC,
} strip_animation_t;

class StripFrame{
    struct config_t{
        CRGB *led_strip                     =        nullptr;
        uint16_t led_start                  =              0;
        uint16_t led_count                  =              0;
        bool double_leds                    =           true;
    };

    struct animation_config_general_t {
        strip_animation_t animation =STRIP_ANIMATION_RAINBOW;
        uint8_t brightness_main             =             10;
        uint8_t brightness_background       =             10;
        uint8_t saturation_main             =            255;
        uint8_t saturation_background       =            255;
    };

    struct animation_config_rainbow_t {
        uint32_t period_ms                  =           3000;
        uint8_t phase_shift_pixel           =             20;
        uint8_t phase_zero                  =              0;
    };

    struct animation_config_static_t {
        uint8_t color_main                  =               0;
        uint8_t color_background            =               0;
    };
public:
    const config_t& config( void ) const { return _cfg; }
    void config( const config_t& cfg );
    void show( void );
    void brightnessSet( int16_t brightness, int16_t background_brightness = -1 );
    void saturationSet( int16_t saturation, int16_t background_saturation = -1 );
    void volumeSet( float volume ) { _volume_f = volume; };

    void animationSet( strip_animation_t animation ) { _animation = animation; };
    CRGB get_pixel_color( uint32_t pixelN );
    void color_update( void );
    void update( void );

    const animation_config_rainbow_t& animation_config_rainbow( void ) const { return _animation_config_rainbow; }
    void animation_config_rainbow( const animation_config_rainbow_t& config ) { _animation_config_rainbow = config; };

    const animation_config_static_t& animation_config_static( void ) const { return _animation_config_static; }
    void animation_config_static( const animation_config_static_t& config ) { _animation_config_static = config; };

protected:
    strip_animation_t _animation;

    animation_config_rainbow_t _animation_config_rainbow;
    animation_config_static_t _animation_config_static;

    uint8_t _hsv_saturation_main;
    uint8_t _hsv_saturation_background;
    uint8_t _hsv_brightness = 10;
    uint8_t _hsv_brightness_background = 10;
    
    //uint8_t _pixel_diff = 20;
    //uint32_t _period_ms = 10000;
    uint8_t _hsv_color;
    float _volume_f = 1.0f;
    int32_t _volume_i8 = 255;
    CRGB *_segment = nullptr;
    config_t _cfg;
};
