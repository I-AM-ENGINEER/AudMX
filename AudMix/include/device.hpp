#pragma once

#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_ws28xx.h"
#include "slider.hpp"
#include "system.hpp"

#define SLIDERS_ADC_UNIT    ADC_UNIT_1
#define I2C_SDA_PIN         8
#define SLIDERS_COUNT       5
#define STRIP_TYPE          WS2812B
#define STRIP_LED_COUNT     101
#define STRIP_PIN           6
#define ICON_WIDTH          60
#define ICON_HEIGHT         44

typedef enum {
    MENU_ITEM_TYPE_BOOL,
    MENU_ITEM_TYPE_INT,
} menu_item_type_t;

typedef struct {
    char name[NVS_KEY_NAME_MAX_SIZE];
    bool enable;
    menu_item_type_t type;
    union{
        int32_t i32;
        bool b;
    };
    int32_t i32_min = 0;
    int32_t i32_max = 1;
} menu_item_t;

class Device{
    adc_oneshot_unit_handle_t _sliders_adc;
    void nvsInit( void );
    void adcInit( void );
    void consoleInit( void );
    void virtDispInit( void );
    void configure( void );
    void clalibrate( void );
    void menuInit( void );
    void menuSaveAll( void );
    void menuUpdateSettings( void );

    Battery battery;
    CRGB *_ws2812_output_buffer;
    CRGB _ws2812_buffer[STRIP_LED_COUNT];
    bool _displays_update = true;
    bool _buttons_pressed_for_calibration[SLIDERS_COUNT] = {false};
    uint8_t _button_for_stip_config = -1;
    uint8_t _button_for_bluetooth_config = -1;
    uint8_t _button_next = -1;
    uint8_t _button_select = -1;
public:
    Slider sliders[SLIDERS_COUNT];
    void init( void );
    void update( void );
};
