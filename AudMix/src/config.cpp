#include "device.hpp"

void Device::configure( void ){
    // Slider 1 config
    {
        auto cfg = sliders[0].config();
        cfg.adc_channel     = ADC_CHANNEL_0;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 10;
        cfg.led_start       = 1;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = &strip;
        sliders[0].config(cfg);
    }

    // Slider 2 config
    {
        auto cfg = sliders[1].config();
        cfg.adc_channel     = ADC_CHANNEL_1;
        cfg.i2c_addr        = 0x3D;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 10;
        cfg.led_start       = 21;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = &strip;
        sliders[1].config(cfg);
    }

    // Slider 3 config
    {
        auto cfg = sliders[2].config();
        cfg.adc_channel     = ADC_CHANNEL_2;
        cfg.i2c_addr        = 0x3D;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 20;
        cfg.led_start       = 41;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = &strip;
        sliders[2].config(cfg);
    }

    // Slider 4 config
    {
        auto cfg = sliders[3].config();
        cfg.adc_channel     = ADC_CHANNEL_3;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 20;
        cfg.led_start       = 61;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = &strip;
        sliders[3].config(cfg);
    }

    // Slider 5 config
    {
        auto cfg = sliders[4].config();
        cfg.adc_channel     = ADC_CHANNEL_4;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 21;
        cfg.led_start       = 81;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = &strip;
        sliders[4].config(cfg);
    }
}
