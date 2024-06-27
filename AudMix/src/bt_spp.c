#include "bt_spp.h"
#include "esp_log.h"
#include "nvs_flash.h"
/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "bt_interface.h"
#include "menu.h"

static int ble_spp_server_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;
int gatt_svr_register(void);
static bool conn_handle_subs[CONFIG_BT_NIMBLE_MAX_CONNECTIONS + 1] = {false};
static uint16_t ble_spp_svc_gatt_read_val_handle;
static uint16_t ble_spp_svc_gatt_read_val_handle3;
static uint16_t ble_spp_svc_gatt_read_val_handle2;

static uint16_t ble_spp_svc_gatt_volreact_handle;
static uint16_t ble_spp_svc_gatt_button1_handle;
static uint16_t ble_spp_svc_gatt_button2_handle;
static uint16_t ble_spp_svc_gatt_button3_handle;

extern menu_item_t menu_item_volume_reactive;

static void ble_spp_server_advertise(void){
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(BLE_SVC_VOLUME_CONTROL_UUID16),
        BLE_UUID16_INIT(BLE_SVC_BATTERY_UUID16),
    };
    fields.num_uuids16 = 2;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, ble_spp_server_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * ble_spp_server uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  ble_spp_server.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int ble_spp_server_gap_event(struct ble_gap_event *event, void *arg){
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
        }
        MODLOG_DFLT(INFO, "\n");
        if (event->connect.status != 0 || CONFIG_BT_NIMBLE_MAX_CONNECTIONS > 1) {
            /* Connection failed or if multiple connection allowed; resume advertising. */
            ble_spp_server_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        MODLOG_DFLT(INFO, "\n");

        conn_handle_subs[event->disconnect.conn.conn_handle] = false;

        /* Connection terminated; resume advertising. */
        ble_spp_server_advertise();
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        MODLOG_DFLT(INFO, "connection updated; status=%d ",
                    event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == 0);
        MODLOG_DFLT(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                    event->adv_complete.reason);
        ble_spp_server_advertise();
        return 0;

    case BLE_GAP_EVENT_MTU:
        MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        MODLOG_DFLT(INFO, "subscribe event; conn_handle=%d attr_handle=%d "
                    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);
        conn_handle_subs[event->subscribe.conn_handle] = true;
        return 0;

    default:
        return 0;
    }
}

static void ble_spp_server_on_reset(int reason){
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

static void ble_spp_server_on_sync(void){
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    /* Begin advertising. */
    ble_spp_server_advertise();
}

void ble_spp_server_host_task(void *param){
    MODLOG_DFLT(INFO, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

/* Callback function for custom service */
static int  ble_svc_gatt_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        //printf("Callback for read,conn_handle = %x,attr_handle = %x", conn_handle, attr_handle);
        MODLOG_DFLT(INFO, "Callback for read");
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        //printf("Data received in write event,conn_handle = %x,attr_handle = %x", conn_handle, attr_handle);
        MODLOG_DFLT(INFO, "Data received in write event,conn_handle = %x,attr_handle = %x", conn_handle, attr_handle);
        break;

    default:
        MODLOG_DFLT(INFO, "\nDefault Callback");
        break;
    }
    return 0;
}

static int  ble_svc_gatt_battery_reader_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    if(ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR){
        return 0;
    }
    
    uint8_t battery_level = battery_get_level();
    os_mbuf_append(ctxt->om, &battery_level, 1);
    return 0;
}

static int  ble_svc_gatt_btn_read_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR){

    }
    
    uint8_t battery_level = battery_get_level();
    os_mbuf_append(ctxt->om, &battery_level, 1);
    return 0;
}

static int  ble_svc_gatt_volreact_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    if((int32_t)arg == 0){
        if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR){
            uint8_t vol_react = menu_item_volume_reactive.b ? 1 : 0;
            os_mbuf_append(ctxt->om, &vol_react, 1);
        }else if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR){
            if(ctxt->om->om_len != 1){
                return 0;
            }
            audioreactive_set((bool)(*ctxt->om->om_databuf));
        }
    }else if((int32_t)arg == 1){
        if(ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR){
            return 0;
        }
        size_t str_len = ctxt->om->om_len;
        char* str = malloc(str_len + 1);
        str[str_len] = '\0';
        memcpy(str, ctxt->om->om_data, str_len);
        audioreactive_set_sliders(str);
        free(str);
    }
    return 0;
}

static int  ble_svc_gatt_display_write_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg){
    if(ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR){
        return 0;
    }
    static uint8_t arr[(64*44+7)/8];
    static size_t receive_len;

    int32_t disp = (int32_t)arg;

    if(disp == -1){
        receive_len = 0;
    }else if((receive_len + ctxt->om->om_len) <= sizeof(arr)){
        memcpy(&arr[receive_len], ctxt->om->om_data, ctxt->om->om_len);
        receive_len += ctxt->om->om_len;
        
        //printf("size: %u\r\n", receive_len);
        if(receive_len == sizeof(arr)){
            display_set_icon(disp, arr);
        }
    }else{
        receive_len = 0;
    }
    
    return 0;
}

/* Define new custom service */
static const struct ble_gatt_svc_def new_ble_svc_gatt_defs[] = {
    {
        /*** Service: SPP */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_VOLUME_CONTROL_UUID16),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_VOLUME_CONTROL_CHR_UUID16),
                .access_cb = ble_svc_gatt_handler,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &ble_spp_svc_gatt_read_val_handle3,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_NEW_TRANSACTION_CHR_UUID16),
                .access_cb = ble_svc_gatt_display_write_handler,
                .arg = (void*)-1,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_DISPLAY1_CHR_UUID16),
                .access_cb = ble_svc_gatt_display_write_handler,
                .arg = (void*)0,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_DISPLAY2_CHR_UUID16),
                .access_cb = ble_svc_gatt_display_write_handler,
                .arg = (void*)1,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_DISPLAY3_CHR_UUID16),
                .access_cb = ble_svc_gatt_display_write_handler,
                .arg = (void*)2,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_DISPLAY4_CHR_UUID16),
                .access_cb = ble_svc_gatt_display_write_handler,
                .arg = (void*)3,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_DISPLAY5_CHR_UUID16),
                .access_cb = ble_svc_gatt_display_write_handler,
                .arg = (void*)4,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_VOLUME_RACT_BOOL_CHR_UUID16),
                .access_cb = ble_svc_gatt_volreact_handler,
                .arg = (void*)0,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &ble_spp_svc_gatt_volreact_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_VOLUME_RACT_VAL_CHR_UUID16),
                .access_cb = ble_svc_gatt_volreact_handler,
                .arg = (void*)1,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_spp_svc_gatt_read_val_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_BUTTON_1_CHR_UUID16),
                .access_cb = ble_svc_gatt_btn_read_handler,
                .arg = (void*)1,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &ble_spp_svc_gatt_button1_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_BUTTON_2_CHR_UUID16),
                .access_cb = ble_svc_gatt_btn_read_handler,
                .arg = (void*)2,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &ble_spp_svc_gatt_button2_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_BUTTON_3_CHR_UUID16),
                .access_cb = ble_svc_gatt_btn_read_handler,
                .arg = (void*)3,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &ble_spp_svc_gatt_button3_handle,
            },
            {
                0, /* No more characteristics */
            }
        },
    },
    {
        /*** Service: SPP */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_BATTERY_UUID16),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                .uuid = BLE_UUID16_DECLARE(BLE_SVC_BATTERY_CHR_UUID16),
                .access_cb = ble_svc_gatt_battery_reader_handler,
                .val_handle = &ble_spp_svc_gatt_read_val_handle2,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                0, /* No more characteristics */
            }
        },
    },
    {
        0, /* No more services. */
    },
};

static void
gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int gatt_svr_init(void){
    int rc = 0;
    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(new_ble_svc_gatt_defs);

    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(new_ble_svc_gatt_defs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

void ble_notification_volume_update( const char* str ){
    if(!ble_is_connected()){
        return;
    }
    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
        //Check if client has subscribed to notifications
        if (conn_handle_subs[i]) {
            struct os_mbuf *txom;

            txom = ble_hs_mbuf_from_flat(str, strlen(str));
            ble_gatts_notify_custom(i, ble_spp_svc_gatt_read_val_handle3, txom);
        }
    }
}

void ble_notification_volreactive_update( bool state ){
    if(!ble_is_connected()){
        return;
    }
    struct os_mbuf *txom;
    uint8_t state_u8 = state ? 1 : 0;
    
    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
        txom = ble_hs_mbuf_from_flat(&state_u8, 1);
        ble_gatts_notify_custom(i, ble_spp_svc_gatt_volreact_handle, txom);
    }
}

void ble_notification_button_update( uint8_t button_num, bool state ){
    if(!ble_is_connected()){
        return;
    }
    struct os_mbuf *txom;
    uint8_t btn_state_u8 = state ? 1 : 0;
    
    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
        txom = ble_hs_mbuf_from_flat(&btn_state_u8, 1);
        switch (button_num){
            case 0: ble_gatts_notify_custom(i, ble_spp_svc_gatt_button1_handle, txom); break;
            case 1: ble_gatts_notify_custom(i, ble_spp_svc_gatt_button2_handle, txom); break;
            case 2: ble_gatts_notify_custom(i, ble_spp_svc_gatt_button3_handle, txom); break;
            default: break;
        }
    }
}

bool ble_is_connected(void){
    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
        if (conn_handle_subs[i]) {
            return true;
        }
    }
    return false;
}

void ble_init(void){
    int rc;
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
        return;
    }

    /* Initialize connection_handle array */
    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
        conn_handle_subs[i] = false;
    }

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = ble_spp_server_on_reset;
    ble_hs_cfg.sync_cb = ble_spp_server_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
    ble_hs_cfg.sm_sc = 0;

    /* Register custom service */
    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("AudMX");
    assert(rc == 0);

    nimble_port_freertos_init(ble_spp_server_host_task);
}
