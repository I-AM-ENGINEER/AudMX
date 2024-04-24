#pragma once

// UNTESTED! TODO 

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

class Battery{
    adc_oneshot_unit_handle_t   _adc_handle;
    adc_unit_t                  _adc_unit    =    ADC_UNIT_2;
    adc_channel_t               _adc_channel = ADC_CHANNEL_0;
    adc_cali_handle_t           _adc_cali;
    uint16_t                    _mv;
public:
    Battery( void ){};
    uint16_t readVoltage();
    float readLevel( void );
    void init( void );
};

void delay( uint32_t ms );
