#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void display_set_icon( uint8_t display, uint8_t* icon );
void audioreactive_set_sliders( char* package );
uint8_t battery_get_level( void );
bool button_read( uint8_t button_num );
void ble_notification_volreactive_update( bool state );
void audioreactive_set( bool state );

#ifdef __cplusplus
}
#endif
