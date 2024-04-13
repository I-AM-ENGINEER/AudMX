#pragma once

// UNTESTED! TODO 

//#include "driver/adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class Battery{
    adc_oneshot_unit_handle_t   adc_handle;
    adc_unit_t                  adc_unit    =    ADC_UNIT_2;
    adc_channel_t               adc_channel = ADC_CHANNEL_0;
    adc_cali_handle_t           adc_cali;
    uint16_t mv;
public:
    Battery( void ){};
    uint16_t readVoltage();
    float readLevel( void );
    void init( void );
};

void delay( uint32_t ms ){
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void Battery::init( void ){
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = adc_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &config));

    //adc_cali

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = adc_unit,
        .chan = adc_channel,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali));
}

/// @brief Read battery voltage
/// @param  
/// @return mv
uint16_t Battery::readVoltage( void ){
    int adc_val;
    int mV;
    adc_oneshot_read(adc_handle, adc_channel, &adc_val);
    adc_cali_raw_to_voltage(adc_cali, adc_val, &mV);
    return mV;
}

/// @brief Read battery level 0.0...1.0
/// @param  
/// @return 0.0 - fully discharged, 1.0 - fully charged
float Battery::readLevel( void ){
    return 0.0f;
}
