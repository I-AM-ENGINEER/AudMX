#pragma once

#include "nvs.h"
#include "stdint.h"

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
    int32_t i32_min;
    int32_t i32_max;
} menu_item_t;
