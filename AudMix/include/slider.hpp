#pragma once

#include "display.hpp"
#include "system.hpp"
#include "stripFrame.hpp"

class HysteresisFilter {
private:
    float _threshold;
    float _min;
    float _max;
    float _previous;
public:
    HysteresisFilter(float threshold, float min, float max) : _threshold(threshold), _min(min), _max(max), _previous(min){}

    float filter(float input) {
        input = std::clamp(input, _min, _max);
        if(input < (_min + _threshold)){
            _previous = _min;
        }else if(input > (_max - _threshold)){
            _previous = _max;
        }
        
        if(std::abs(input - _previous) > _threshold) {
            _previous = input;
            return input;
        }else{
            return _previous;
        }
    }
};

class  Slider{
    adc_oneshot_unit_handle_t*    _adc_handle;
    adc_cali_handle_t               _adc_cali;
    
    // For calibration
    uint8_t _ico_buffer[64*64/8];
    uint16_t _ico_size_x = 0;
    uint16_t _ico_size_y = 0;
    bool _ico_display = false;

    float _position_filter_buff[11];
    size_t _position_filter_buff_idx = 0;

    int64_t _last_button_pressed_timestamp = 0;
    float _old_slider_pos;

    struct calibration_t{
        float max_value;
        float min_value;
        float mid_value;
    };

    struct config_t{
        int16_t pin_scl                     =             -1;
        int16_t pin_sda                     =             -1;
        uint8_t i2c_addr                    =           0x3C;
        adc_channel_t adc_channel           =  ADC_CHANNEL_0;
        CRGB *led_strip                     =        nullptr;
        uint16_t led_start                  =              0;
        uint16_t led_count                  =              0;
        bool double_leds                    =           true;
    };
    float adcRawReadAccuracy( void );
    HysteresisFilter histFilter;
public:
    Slider( void ) : histFilter(0.01f, 0.0f, 1.0f) {}

    LGFX_SSD1306 display;
    StripFrame strip;
    const config_t& config( void ) const { return _cfg; }
    void config( const config_t& cfg );

    calibration_t calibrationGet( void ) { return _calibration; }
    void calibrationSetMinPoint( calibration_t& calibration ) { calibration.min_value = adcRawReadAccuracy(); }
    void calibrationSetMidPoint( calibration_t& calibration ) { calibration.mid_value = adcRawReadAccuracy(); }
    void calibrationSetMaxPoint( calibration_t& calibration ) { calibration.max_value = adcRawReadAccuracy(); }
    void calibrationLoad( const calibration_t calibration ) { _calibration = calibration; }

    int32_t init( adc_oneshot_unit_handle_t* adc_handle );
    int32_t setIcon( const uint8_t *icon, uint32_t size_x, uint32_t size_y );
    bool displayIcon( bool display_icon );
    bool displayIcon( void );
    void updatePosition( void );
    void updateDisplay( void );

    float adcFilteredRead( void );
    float adcRawRead( void );
    float readPosition( void );
    bool readButton( void );
protected:
    config_t _cfg;
    calibration_t _calibration;
};
