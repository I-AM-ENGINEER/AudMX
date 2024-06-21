#include "bt_interface.h"
#include <math.h>
#include "device.hpp"

extern EventGroupHandle_t buttonsPressedEventGroup;
extern EventGroupHandle_t buttonsReleasedEventGroup;

void display_set_icon( uint8_t display, uint8_t* icon ){
    if(display >= SLIDERS_COUNT){
        return;
    }
    audMix.sliders[display].setIcon(icon, ICON_WIDTH, ICON_HEIGHT);
}

uint8_t battery_get_level( void ){
    float battery_level_f = audMix.battery.readLevel();
    uint8_t battery_level_u8 = (uint8_t)roundf(battery_level_f * 100.0f);
    return battery_level_u8;
}

void audioreactive_set_sliders( char* package ){
    float volumeValues[SLIDERS_COUNT];
    char *token;
    int index = 0;
    token = strtok((char *)package, "|");
    while (token != NULL && index < SLIDERS_COUNT) {
        volumeValues[index++] = atof(token);
        token = strtok(NULL, "|");
    }
    
    if(audMix.isAudioReactive()){
        for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
            audMix.sliders[i].strip.volumeSet(volumeValues[i]);
        }
    }
}

bool button_read( uint8_t button_num ){
    int8_t slider_num = -99;
    for(uint32_t i = 0; i < SLIDERS_COUNT; i++){
        if(audMix.sliders[i].readButtonNum() == button_num){
            slider_num = i;
            break;
        }
    }
    return audMix.sliders[slider_num].readButton();
}

void audioreactive_set( bool state ){
    audMix.isAudioReactive(state);
}
