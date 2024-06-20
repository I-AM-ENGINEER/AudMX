#include "stripFrame.hpp"
#include "esp_timer.h"
#include <stdlib.h>
#include <math.h>

#define LED_RAINBOW_PIXEL_DIF			(20)

static CRGB hsv2rgb(uint8_t h, uint8_t s, uint8_t v) {    
	CRGB rgb;

	uint32_t tmp_val = ((uint32_t)v * (uint32_t)s) / 256; // Should be /255, but /256 faster
    uint32_t vmin = (uint32_t)v - tmp_val;
    uint32_t a = tmp_val * ((uint32_t)h * 24 / 17 % 60) / 60;
    uint32_t vinc = vmin + a;
    uint32_t vdec = (uint32_t)v - a;
	uint32_t zone = ((24 * (uint32_t)h / 17) / 60) % 6;

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

static CRGB combineRGB(CRGB a, CRGB b, float k) {
    CRGB result;
    result.r = (uint8_t)((1 - k) * a.r + k * b.r);
    result.g = (uint8_t)((1 - k) * a.g + k * b.g);
    result.b = (uint8_t)((1 - k) * a.b + k * b.b);
    return result;
}

CRGB StripFrame::get_pixel_color( uint32_t pixelN ){
    float perc = (float)pixelN / (float)_cfg.led_count;
    uint32_t pixel_color = 0;
    uint8_t brightness;
    uint8_t saturation;
    if(_animation == STRIP_ANIMATION_RAINBOW){
        pixel_color = _hsv_color + pixelN*(uint32_t)_animation_config_rainbow.phase_shift_pixel + _animation_config_rainbow.phase_zero;
    }else if(_animation == STRIP_ANIMATION_STATIC){
        if(perc > (1.0f - _volume_f)){
            pixel_color = _animation_config_static.color_main;
        }else{
            pixel_color = _animation_config_static.color_background;
        }
    }
    
    CRGB RGB;
    if(fabsf(perc - (1.0f - _volume_f)) < 0.05f){
        float bk = ((((1.0f - _volume_f) - perc + 0.05f) * 10.0f));
	    CRGB RGB_main = hsv2rgb(pixel_color, _hsv_saturation_main, _hsv_brightness);
	    CRGB RGB_bg   = hsv2rgb(pixel_color, _hsv_saturation_background, _hsv_brightness_background);
        RGB = combineRGB(RGB_main, RGB_bg, bk);
    }else{
        if(perc > (1.0f - _volume_f)){
            brightness = _hsv_brightness;
            saturation = _hsv_saturation_main;
        }else{
            brightness = _hsv_brightness_background;
            saturation = _hsv_saturation_background;
        }
        if(brightness < 4){
            pixel_color = 0;
            saturation  = 0;
        }
        RGB = hsv2rgb(pixel_color, saturation, brightness);
    }
	return RGB;
}

void StripFrame::color_update( void ){
	_hsv_color = (uint8_t)((((esp_timer_get_time()/1024) % (int64_t)_animation_config_rainbow.period_ms) * 255) / (uint32_t)_animation_config_rainbow.period_ms);
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

/*
void StripFrame::setStaticColor( CRGB color ){
    if(_cfg.led_strip == nullptr){
        return;
    }
    for(uint32_t i = 0; i < _cfg.led_count; i++){
        _segment[i] = color;
    }
    show();
}*/

void StripFrame::brightnessSet( int16_t brightness, int16_t background_brightness ) { 
    if(brightness < 0){ 
        return; 
    } 
    _hsv_brightness = (uint8_t)brightness; 
    if(background_brightness < 0){ 
        _hsv_brightness_background = _hsv_brightness;
    }else{
        _hsv_brightness_background = background_brightness;
    }
}

/*void StripFrame::phaseShiftSer( uint16_t phase_shift ) {
    _pixel_diff = pixel_diff;
}

void StripFrame::pixelDiffSet( uint16_t pixel_diff ) { 
    _pixel_diff = pixel_diff;
}

void StripFrame::periodSet( uint32_t period_ms ) { 
    if(period_ms < 100){ 
        return; 
    }
    _period_ms = period_ms;
}*/

void StripFrame::saturationSet( int16_t saturation, int16_t background_saturation ) { 
    if(saturation < 0){ 
        return; 
    } 
    _hsv_saturation_main = (uint8_t)saturation; 
    if(background_saturation < 0){ 
        _hsv_saturation_background = _hsv_saturation_main;
    }else{
        _hsv_saturation_background = background_saturation;
    }
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
