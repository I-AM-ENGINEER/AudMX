#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"
#include <LovyanGFX.hpp>

class LGFX_SSD1306  : public lgfx::LGFX_Device{    
    lgfx::Panel_SSD1306     _panel_instance;
    lgfx::Bus_I2C           _bus_instance;

    struct config_t{
        int16_t pin_scl         =   -1;
        int16_t pin_sda         =   -1;
        uint8_t screen_height   =   48;
        uint8_t screen_width    =   64;
        uint8_t i2c_adress      =   0x3C;
    };

    void select( void );
    void unselect( void );
public:
    LGFX_SSD1306( void ){}
    void startWrite( bool transaction = true ); // override
    void endWrite( void ); // override
    const config_t& config(void) const { return _cfg; }
    void config(const config_t& cfg);
private:
    config_t _cfg;
};

void LGFX_SSD1306::config(const config_t& cfg){
    _cfg = cfg;
    {
        auto cfg = _bus_instance.config();
        cfg.i2c_port    = 0;
        cfg.freq_write  = 400000; // Frequency here not real
        cfg.freq_read   = 400000;
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
        cfg.offset_x         =                   0;
        cfg.offset_y         =                   0;
        cfg.offset_rotation  =                   2;
        cfg.invert           =               false;

        _panel_instance.config(cfg);
        setPanel(&_panel_instance);
    }
}

void LGFX_SSD1306::select( void ){
    i2c_set_pin(I2C_NUM_0, _cfg.pin_sda, _cfg.pin_scl, true, true, I2C_MODE_MASTER);
}

void LGFX_SSD1306::startWrite( bool transaction ){
    select();
    _panel->startWrite(transaction);
}

void LGFX_SSD1306::endWrite( void ){
    _panel->endWrite();
    unselect();
}

void LGFX_SSD1306::unselect( void ){
    gpio_reset_pin((gpio_num_t)this->_cfg.pin_scl);
}
