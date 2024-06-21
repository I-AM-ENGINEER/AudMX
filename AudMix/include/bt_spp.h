#pragma once

#include <stdbool.h>
#include "nimble/ble.h"
#include "modlog/modlog.h"
#ifdef __cplusplus
extern "C" {
#endif

/* 16 Bit SPP Service UUID */
#define BLE_SVC_SPP_UUID16                                  0xABF0

/* 16 Bit SPP Service Characteristic UUID */
#define BLE_SVC_SPP_CHR_UUID16                              0xABF1

#define CONFIG_EXAMPLE_IO_TYPE                              3

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;



/* Console */
int scli_init(void);
int scli_receive_key(int *key);

/** Misc. */
void print_bytes(const uint8_t *bytes, int len);
void print_addr(const void *addr);
char *addr_str(const void *addr);
void print_mbuf(const struct os_mbuf *om);

void bluetooth_init(void);

void ble_send_volume(const char* str);

#ifdef __cplusplus
}
#endif

