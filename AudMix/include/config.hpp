#pragma once

#define SLIDERS_COUNT   5

#define STRIP_TYPE      LED_WS2812B
#define STRIP_LED_COUNT 101
#define STRIP_PIN       6
#define I2C_SDA_PIN     8
#define ICON_SIZE_PX    48

#define DISPLAY_WIDTH   64
#define DISPLAY_HEIGHT  48


#include <slider.hpp>

extern Slider sliders[SLIDERS_COUNT];
extern adc_oneshot_unit_handle_t adc1_handle;

void read_config( void ){
    // Slider 1 config
    {
        auto cfg = sliders[0].config();
        cfg.adc_unit        = &adc1_handle;
        cfg.adc_channel     = ADC_CHANNEL_0;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 9;
        cfg.led_start       = 1;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        sliders[0].config(cfg);
    }

    // Slider 2 config
    {
        auto cfg = sliders[1].config();
        cfg.adc_unit        = &adc1_handle;
        cfg.adc_channel     = ADC_CHANNEL_1;
        cfg.i2c_addr        = 0x3D;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 9;
        cfg.led_start       = 21;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        sliders[1].config(cfg);
    }

    // Slider 3 config
    {
        auto cfg = sliders[2].config();
        cfg.adc_unit        = &adc1_handle;
        cfg.adc_channel     = ADC_CHANNEL_2;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 10;
        cfg.led_start       = 41;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        sliders[2].config(cfg);
    }

    // Slider 4 config
    {
        auto cfg = sliders[3].config();
        cfg.adc_unit        = &adc1_handle;
        cfg.adc_channel     = ADC_CHANNEL_3;
        cfg.i2c_addr        = 0x3D;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 10;
        cfg.led_start       = 61;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        sliders[3].config(cfg);
    }

    // Slider 5 config
    {
        auto cfg = sliders[4].config();
        cfg.adc_unit        = &adc1_handle;
        cfg.adc_channel     = ADC_CHANNEL_4;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 20;
        cfg.led_start       = 81;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        sliders[4].config(cfg);
    }
}
