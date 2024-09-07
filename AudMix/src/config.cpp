#include "device.hpp"

extern CRGB *ws2812b_display_buffer;

void Device::configure( void ){
    // Long press button 0 and 1 during boot for start calibration
    _buttons_pressed_for_calibration[0] = true;
    _buttons_pressed_for_calibration[1] = true;

    _button_for_stip_config = 0;
    _button_for_bluetooth_config = 1;
    _button_next = 0;
    _button_select = 1;

    // Slider 1 config
    {
        auto cfg = sliders[0].config();
        cfg.adc_channel     = ADC_CHANNEL_0;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 10;
        cfg.led_start       = 81;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = ws2812b_display_buffer;
        cfg.button_number   = 1; // -1 - system button, no send buttons events to PC, > 0 - sending if pressed
        sliders[0].config(cfg);
    }

    // Slider 2 config
    {
        auto cfg = sliders[1].config();
        cfg.adc_channel     = ADC_CHANNEL_1;
        cfg.i2c_addr        = 0x3D;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 10;
        cfg.led_start       = 61;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = ws2812b_display_buffer;
        cfg.button_number   = 2;
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
        cfg.led_strip       = ws2812b_display_buffer;
        cfg.button_number   = 0;
        sliders[2].config(cfg);
    }

    // Slider 4 config
    {
        auto cfg = sliders[3].config();
        cfg.adc_channel     = ADC_CHANNEL_3;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 20;
        cfg.led_start       = 21;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = ws2812b_display_buffer;
        cfg.button_number   = -1;
        sliders[3].config(cfg);
    }

    // Slider 5 config
    {
        auto cfg = sliders[4].config();
        cfg.adc_channel     = ADC_CHANNEL_4;
        cfg.i2c_addr        = 0x3C;
        cfg.pin_sda         = I2C_SDA_PIN;
        cfg.pin_scl         = 21;
        cfg.led_start       = 1;
        cfg.led_count       = 10;
        cfg.double_leds     = true;
        cfg.led_strip       = ws2812b_display_buffer;
        cfg.button_number   = -1;
        sliders[4].config(cfg);
    }
}
