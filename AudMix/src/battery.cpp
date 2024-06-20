#include "battery.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void Battery::init( void ){
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = _adc_unit,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &_adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(_adc_handle, _adc_channel, &config));

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = _adc_unit,
        .chan = _adc_channel,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &_adc_cali));
}

/// @brief Read battery voltage
/// @param  
/// @return mv
uint16_t Battery::readVoltage( void ){
    int adc_val;
    int mV;
    adc_oneshot_read(_adc_handle, _adc_channel, &adc_val);
    adc_cali_raw_to_voltage(_adc_cali, adc_val, &mV);
    return mV;
}

/// @brief Read battery level 0.0...1.0
/// @param  
/// @return 0.0 - fully discharged, 1.0 - fully charged
float Battery::readLevel( void ){
    return 0.0f;
}
