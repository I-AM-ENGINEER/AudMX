#pragma once

#include "esp_crc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <iostream>
//#include "config.hpp"
#include <LovyanGFX.hpp>

struct Bus_I2C_Mod : public lgfx::Bus_I2C{
    void beginTransaction( void ) override{
        i2c_set_pin(I2C_NUM_0, _cfg.pin_sda, _cfg.pin_scl, true, true, I2C_MODE_MASTER);
        lgfx::Bus_I2C::beginTransaction();
    };

    void endTransaction( void ) override{
        lgfx::Bus_I2C::endTransaction();
        gpio_reset_pin((gpio_num_t)this->_cfg.pin_scl);
    }
};

class LGFX_SSD1306  : public lgfx::LGFX_Device{    
    lgfx::Panel_SSD1306         _panel_instance;
    Bus_I2C_Mod                 _bus_instance;

    struct config_t{
        int16_t pin_scl         =     -1;
        int16_t pin_sda         =     -1;
        uint8_t screen_height   =     48;
        uint8_t screen_width    =     64;
        uint8_t i2c_adress      =   0x3C;
    };

public:
    LGFX_SSD1306( void ){}
    const config_t& config(void) const { return _cfg; }
    void config( const config_t& cfg ){
        _cfg = cfg;
        {
            auto cfg = _bus_instance.config();
            cfg.i2c_port    = 0;
            cfg.freq_write  = 100000; // Frequency here not real
            cfg.freq_read   = 100000;
            cfg.pin_scl     = _cfg.pin_scl;
            cfg.pin_sda     = _cfg.pin_sda;
            cfg.i2c_addr    = _cfg.i2c_adress;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();

            cfg.panel_width      =   _cfg.screen_width;
            cfg.panel_height     =  _cfg.screen_height;
            cfg.memory_width     =   _cfg.screen_width;
            cfg.memory_height    =  _cfg.screen_height;
            cfg.offset_x         =                   32;
            cfg.offset_y         =                   0;
            cfg.offset_rotation  =                   0;
            cfg.invert           =               false;

            _panel_instance.config(cfg);
            setPanel(&_panel_instance);
        }
    }
protected:
    config_t _cfg;
};


























































































































































































































































































































































































































