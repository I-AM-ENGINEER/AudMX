#include "stripFrame.hpp"
#include "esp_timer.h"
#include <stdlib.h>

#define LED_RAINBOW_PIXEL_DIF			(20)
#define LED_RAINBOW_PERIOD				(2048)

static CRGB hsv2rgb(uint8_t h, uint8_t s, uint8_t v) {    
	CRGB rgb;

	uint8_t tmp_val = ((uint16_t)v * (uint16_t)s) / 256; // Should be /255, but /256 faster
    uint8_t vmin = (uint16_t)v - tmp_val;
    uint8_t a = tmp_val * ((uint16_t)h * 24 / 17 % 60) / 60;
    uint8_t vinc = vmin + a;
    uint8_t vdec = v - a;
	uint8_t zone = ((24 * (uint16_t)h / 17) / 60) % 6;

    switch (zone) {
		case 0: rgb.r = v; rgb.g = vinc; rgb.b = vmin; break;
		case 1: rgb.r = vdec; rgb.g = v; rgb.b = vmin; break;
		case 2: rgb.r = vmin; rgb.g = v; rgb.b = vinc; break;
		case 3: rgb.r = vmin; rgb.g = vdec; rgb.b = v; break;
		case 4: rgb.r = vinc; rgb.g = vmin; rgb.b = v; break;
		case 5: rgb.r = v; rgb.g = vmin; rgb.b = vdec; break;
    }
    return rgb;
}

CRGB StripFrame::get_pixel_color(uint16_t pixelN){
	uint8_t pixel_color = _hsv_color + pixelN*LED_RAINBOW_PIXEL_DIF;
	CRGB RGB = hsv2rgb(pixel_color, 255, 10);
	return RGB;
}

void StripFrame::color_update( void ){
	_hsv_color = (uint8_t)((((esp_timer_get_time()/1024) % (int64_t)LED_RAINBOW_PERIOD) * 255) / (uint32_t)LED_RAINBOW_PERIOD);
	uint16_t brightness = (uint16_t)((float)255);
	_hsv_brightness = brightness;
}

void StripFrame::update( void ){
    color_update();
    if(_segment == nullptr){
        return;
    }
    for(uint32_t i = 0; i < _cfg.led_count; i++){
        _segment[i] = get_pixel_color(i);
    }
    show();
}

void StripFrame::config( const config_t& cfg ){
    _cfg = cfg;
    if(_segment != nullptr){
        free(_segment);
    }
    _segment = (CRGB*)malloc(sizeof(CRGB) * _cfg.led_count);
}

void StripFrame::setStaticColor( CRGB color ){
    if(_cfg.led_strip == nullptr){
        return;
    }
    for(uint32_t i = 0; i < _cfg.led_count; i++){
        _segment[i] = color;
    }
    show();
}

void StripFrame::show( void ){
    if(_cfg.led_strip == nullptr){
        return;
    }
    for(uint32_t i = 0; i < _cfg.led_count; i++){
        _cfg.led_strip[i+_cfg.led_start] = _segment[i];
        if(_cfg.double_leds){
            _cfg.led_strip[_cfg.led_start + _cfg.led_count*2 - 1 - i] = _segment[i];
        }
    }
}
