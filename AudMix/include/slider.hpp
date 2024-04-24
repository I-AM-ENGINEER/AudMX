#pragma once

#include "SmartLeds.h"
#include "display.hpp"
#include "system.hpp"

//#include "config.hpp"


class Slider{
    adc_oneshot_unit_handle_t*    _adc_handle;
    adc_cali_handle_t               _adc_cali;
    
    // For calibration
    uint8_t _ico_buffer[64*64/8];
    uint16_t _ico_size_x = 0;
    uint16_t _ico_size_y = 0;
    bool _ico_display = false;

    struct calibration_t{
        float max_value;
        float min_value;
    };

    struct config_t{
        int16_t pin_scl                     =             -1;
        int16_t pin_sda                     =             -1;
        uint8_t i2c_addr                    =           0x3C;
        adc_channel_t adc_channel           =  ADC_CHANNEL_0;
        SmartLed *led_strip                 =        nullptr;
        uint16_t led_start                  =              0;
        uint16_t led_count                  =              0;
        bool double_leds                    =           true;
    };
    float readPercantage( void );
public:
    LGFX_SSD1306 display;
    
    //Slider( void ) {};
    const config_t& config( void ) const { return _cfg; }
    void config( const config_t& cfg );

    const calibration_t& getCalibration( void ) const { return _calibration; }
    void setCalibration( const calibration_t& calibration ) { _calibration = calibration; }

    int32_t init( adc_oneshot_unit_handle_t* adc_handle );
    int32_t setIcon( uint8_t *icon, uint32_t size_x, uint32_t size_y );
    bool displayIcon( bool display_icon );
    bool displayIcon( void );
    void update( void );
    void calibrate( void );
protected:
    config_t _cfg;
    calibration_t _calibration;
};
