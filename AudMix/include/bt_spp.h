#pragma once

#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#ifdef __cplusplus
extern "C" {
#endif

/* 16 Bit Service UUID */
#define BLE_SVC_BATTERY_UUID16                                      0x180F
#define BLE_SVC_VOLUME_CONTROL_UUID16                               0x18FF

/* 16 Bit Service Characteristics UUID */
#define BLE_SVC_BATTERY_CHR_UUID16                                  0x2A19
#define BLE_SVC_VOLUME_CONTROL_CHR_UUID16                           0x2FF0
#define BLE_SVC_NEW_TRANSACTION_CHR_UUID16                          0x2FF1
#define BLE_SVC_DISPLAY1_CHR_UUID16                                 0x2FF2
#define BLE_SVC_DISPLAY2_CHR_UUID16                                 0x2FF3
#define BLE_SVC_DISPLAY3_CHR_UUID16                                 0x2FF4
#define BLE_SVC_DISPLAY4_CHR_UUID16                                 0x2FF5
#define BLE_SVC_DISPLAY5_CHR_UUID16                                 0x2FF6
#define BLE_SVC_VOLUME_RACT_BOOL_CHR_UUID16                         0x2FF7
#define BLE_SVC_VOLUME_RACT_VAL_CHR_UUID16                          0x2FF8
#define BLE_SVC_BUTTON_1_CHR_UUID16                                 0x2FF9
#define BLE_SVC_BUTTON_2_CHR_UUID16                                 0x2FFA
#define BLE_SVC_BUTTON_3_CHR_UUID16                                 0x2FFB

#define CONFIG_EXAMPLE_IO_TYPE                              3

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

void ble_init( void );
bool ble_is_connected(void);
void ble_notification_volume_update( const char* str );
void ble_notification_button_update( uint8_t button_num, bool state );
void ble_notification_volreactive_update( bool state );
#ifdef __cplusplus
}
#endif
