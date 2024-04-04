#pragma once

// UNTESTED! TODO 

#include "driver/adc.h"
#include "esp_adc_cal.h"

class Battery{
    uint16_t mv;
public:
    Battery( void ){};
    void init( void );
};

void Battery::init( void ){
    adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_DB_11);
    esp_adc_cal_characteristics_t adc2_chars;
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, ESP_ADC_CAL_VAL_DEFAULT_VREF, &adc2_chars);
    int adc2_value;
    adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &adc2_value);
    uint32_t mV = esp_adc_cal_raw_to_voltage(adc2_value, &adc2_chars);

}

