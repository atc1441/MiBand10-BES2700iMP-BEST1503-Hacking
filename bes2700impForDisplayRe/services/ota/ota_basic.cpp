#include "hal_trace.h"
#include "string.h"
#include "cmsis_os.h"
#include "ota_basic.h"
#include "cmsis.h"
#include "cqueue.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "app_bt_func.h"
#include "ota_control.h"
#include "ota_spp.h"
#include "ota_ble.h"
#ifdef OTA_OVER_TOTA_ENABLED
#include "app_tota.h"
#include "app_tota_common.h"
#endif
#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ibrt_conn_api.h"
#include "app_tws_ctrl_thread.h"
#endif

#define APP_OTA_MAX_PACKET_SIZE     (RFCOMM_DEFAULT_MTU)
#define APP_OTA_EVENT_MAX_MAILBOX   (4)
#define APP_OTA_EVENT_BUF_SIZE      (APP_OTA_MAX_PACKET_SIZE*APP_OTA_EVENT_MAX_MAILBOX)
#define APP_OTA_BUFF_SIZE           (4096)

typedef struct {
    BES_OTA_RX_FLAG_E  flag;
    uint16_t len;
    bes_ota_event_param_t param;
} APP_OTA_EVENT_T;

typedef struct
{
    BES_OTA_PATH_TYPE_E connectedType;
    uint16_t            otaMtu;
    bool                otaFinished;
    uint8_t             conidx;
    uint8_t             packetStructureType;
    uint8_t             macAddress[OTA_BD_ADDR_LEN];
} APP_OTA_ENV_T;

static APP_OTA_ENV_T app_ota_env;
static uint8_t common_ota_buf[APP_OTA_BUFF_SIZE];
static uint8_t dataBuf[APP_OTA_MAX_PACKET_SIZE] = {0};
static uint8_t app_ota_event_buf[APP_OTA_EVENT_BUF_SIZE] = {0};
static CQueue ota_rx_cqueue;

static osThreadId app_ota_event_thread = NULL;
static void app_ota_event_handler_thread(const void *arg);
osThreadDef(app_ota_event_handler_thread, osPriorityAboveNormal, 1, 1024*3, "ota_rx");

osMutexId app_ota_buf_lock;
osMutexDef(app_ota_buf_lock);

osMailQDef (app_ota_event_mailbox, APP_OTA_EVENT_MAX_MAILBOX, APP_OTA_EVENT_T);
static osMailQId app_ota_event_mailbox_id = NULL;

static void app_ota_env_init(void)
{
    memset((uint8_t *)&app_ota_env, 0, sizeof(app_ota_env));
    app_ota_env.conidx = 0xff;

}

static int32_t app_ota_event_mailbox_init(void)
{
    app_ota_event_mailbox_id = osMailCreate(osMailQ(app_ota_event_mailbox), NULL);
    if (app_ota_event_mailbox_id == NULL) {
        TRACE(0, "Failed to Create app_ota_event_mailbox");
        return -1;
    }
    return 0;
}

static void app_ota_event_mailbox_free(APP_OTA_EVENT_T* rx_event)
{
    osStatus status;

    status = osMailFree(app_ota_event_mailbox_id, rx_event);
    ASSERT(osOK == status, "Free ble rx event mailbox failed!");
}

static int32_t app_ota_event_mailbox_get(APP_OTA_EVENT_T** rx_event)
{
    osEvent evt;
    evt = osMailGet(app_ota_event_mailbox_id, osWaitForever);
    if (evt.status == osEventMail) {
        *rx_event = (APP_OTA_EVENT_T *)evt.value.p;
        // TRACE(0, "flag %d len %d", (*rx_event)->flag,(*rx_event)->len);
        return 0;
    }
    return -1;
}

static void app_ota_event_handler_thread(void const *argument)
{
    while (true)
    {
        APP_OTA_EVENT_T* rx_event = NULL;
        if (app_ota_event_mailbox_get(&rx_event))
            return;

        bes_ota_event_param_t otaParam;
        TRACE(2, "%s ", __func__);
        osMutexWait(app_ota_buf_lock, osWaitForever);

        memcpy(&otaParam, &rx_event->param, sizeof(bes_ota_event_param_t));

        if (BES_OTA_RECEVICE_DATA == otaParam.event) {
            otaParam.param.receive_data.data = dataBuf;
            memset(otaParam.param.receive_data.data, 0, otaParam.param.receive_data.dataLen);
            DeCQueue(&ota_rx_cqueue, otaParam.param.receive_data.data, (int)otaParam.param.receive_data.dataLen);
            // TRACE(0, "[%s]datalen = %d data:", __func__, otaParam.param.receive_data.dataLen);
            // DUMP8("%x ",otaParam.param.receive_data.data, otaParam.param.receive_data.dataLen);
        }

        app_ota_data_handle(&otaParam, rx_event->flag);
        app_ota_event_mailbox_free(rx_event);
        osMutexRelease(app_ota_buf_lock);
    }
}

#if defined(IBRT)&&(defined(IBRT_UI))
static OTA_BASIC_TWS_ROLE_E ota_basic_convert_uirole_to_ota_twsrole(uint8_t ui_role)
{
    TWS_UI_ROLE_E uiRole = (TWS_UI_ROLE_E)ui_role;
    
    if (TWS_UI_MASTER == uiRole)
    {
        return OTA_BASIC_TWS_MASTER;
    }
    else if (TWS_UI_SLAVE == uiRole)
    {
        return OTA_BASIC_TWS_SLAVE;
    }
    else if (TWS_UI_UNKNOWN == uiRole)
    {
        return OTA_BASIC_TWS_UNKNOWN;
    }
    else
    {
        return OTA_BASIC_TWS_INVALID_ROLE;
    }
}

OTA_BASIC_TWS_ROLE_E ota_basic_get_tws_role(void)
{
    TWS_UI_ROLE_E uiRole = app_ibrt_if_get_ui_role();
    return ota_basic_convert_uirole_to_ota_twsrole(uiRole);
}
#endif

uint8_t* app_ota_get_common_databuf(void)
{
    return (uint8_t*)common_ota_buf;
}

bool ota_basic_is_remap_enabled(void)
{
#ifdef FLASH_REMAP
    return true;
#else
    return false;
#endif

}

uint32_t ota_basic_get_remap_offset(void)
{
#ifdef FLASH_REMAP
    return ((uint32_t)OTA_REMAP_OFFSET);
#else
    return ((uint32_t)0x0);
#endif

}

void app_ota_push_rx_data(BES_OTA_RX_FLAG_E flag, bes_ota_event_param_t* ptr)
{
    if(ptr->event == BES_OTA_SEND_DONE)
        return;

    APP_OTA_EVENT_T* event = (APP_OTA_EVENT_T*)osMailAlloc(app_ota_event_mailbox_id, 0);
    if(!event){
        TRACE(0,"[%s] event is null!", __func__);
        return;
    }

    event->flag = flag;
    event->len = 0;

    if(ptr->event == BES_OTA_RECEVICE_DATA)
    {
        event->len = ptr->param.receive_data.dataLen;
        osMutexWait(app_ota_buf_lock, osWaitForever);
        int32_t ret = EnCQueue(&ota_rx_cqueue, ptr->param.receive_data.data, event->len);
        osMutexRelease(app_ota_buf_lock);

        if(CQ_OK != ret){
            TRACE(0,"[%s] rx buffer overflow! %d,%d", __func__,
                AvailableOfCQueue(&ota_rx_cqueue), event->len);
            event->len = 0;
        }
    }
    memcpy((uint8_t*)&(event->param), ptr, sizeof(bes_ota_event_param_t));

    osMailPut(app_ota_event_mailbox_id, event);
}

void app_ota_connected(BES_OTA_PATH_TYPE_E connType, uint8_t conidx)
{
    TRACE(0,"ota is connected.");
    app_ota_env.connectedType = connType;
    app_ota_env.conidx = conidx;
    app_ota_env.otaFinished = false;
}

void app_ota_disconnected(BES_OTA_PATH_TYPE_E connType)
{
    TRACE(0,"Ota is disconnected connType %d", connType);
    app_ota_env_init();

    Bes_exit_ota_state();
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
    if (DATA_PATH_BLE == connType) {
        tws_ctrl_send_cmd(APP_TWS_CMD_OTA_MOBILE_DISC_CMD, (uint8_t*)&connType, 1);
    }
#endif
}

BES_OTA_PATH_TYPE_E app_ota_get_connected_type(void)
{
    return app_ota_env.connectedType;
}

uint8_t app_ota_get_conidx(void)
{
    TRACE(2, "%s %d", __func__, app_ota_env.conidx);
    return app_ota_env.conidx;
}

void ota_basic_mtu_set(uint16_t otaMtu)
{
    // remove the 5 bytes to accommodate i6s write command error
#if defined(OTA_OVER_TOTA_ENABLED)
    app_ota_env.otaMtu = (otaMtu / 16) * 16 - 5;
#endif
    app_ota_env.otaMtu = otaMtu - 5;
}

uint16_t ota_basic_mtu_get(void)
{
    return app_ota_env.otaMtu;
}

void app_ota_bt_mac_addr_set(const uint8_t* macAddr)
{
    memcpy(app_ota_env.macAddress, macAddr, OTA_BD_ADDR_LEN);
}

uint8_t* ota_basic_get_mac_addr(void)
{
    return app_ota_env.macAddress;
}

uint32_t ota_basic_get_max_image_size(void)
{
#ifdef FLASH_REMAP
    return ((uint32_t)OTA_REMAP_OFFSET);
#else
    return ((uint32_t)(NEW_IMAGE_FLASH_OFFSET - OTA_CODE_OFFSET));
#endif
}

void app_ota_data_handle(bes_ota_event_param_t *param, BES_OTA_RX_FLAG_E flag)
{
    ASSERT(param,"ota data is null.");

    switch(param->event)
    {
    case BES_OTA_CONN:{
        TRACE(0,"[%s] event BES_OTA_CONN path type:%d",__func__, param->pathType);
        if (NO_OTA_CONNECTION != app_ota_get_connected_type()) {
            break;
        }
        app_ota_connected(param->pathType, param->conidx);
        if (DATA_PATH_SPP == param->pathType) {
            app_ota_bt_mac_addr_set(param->param.address);
        }

        if (SPP_RX_DATA_SELF_OTA == flag) {
            #ifndef OTA_OVER_TOTA_ENABLED
            ota_control_register_transmitter(app_ota_spp_send_data);
            #endif
            ota_basic_mtu_set(APP_OTA_MAX_PACKET_SIZE);
        } else if(BLE_RX_DATA_SELF_OTA == flag){
            #ifdef __IAG_BLE_INCLUDE__
            #ifndef OTA_OVER_TOTA_ENABLED
            ota_control_register_transmitter(ota_ble_send_notification);
            #endif
            #endif
        } else if(BLE_RX_DATA_SELF_OTA_OVER_TOTA == flag){
            #ifdef OTA_OVER_TOTA_ENABLED
            ota_control_register_transmitter(ota_tota_send_notification);
            #endif
        } else if(SPP_RX_DATA_SELF_OTA_OVER_TOTA == flag){
            #ifdef OTA_OVER_TOTA_ENABLED
            ota_control_register_transmitter(ota_spp_tota_send_data);
            ota_basic_mtu_set(APP_OTA_MAX_PACKET_SIZE - TOTA_PACKET_VERIFY_SIZE);
            #endif
        }
        break;
    }
    case BES_OTA_DISCONN:{
        TRACE(0,"[%s] event BES_OTA_CONN path type:%d",__func__, param->pathType);
        app_ota_disconnected(param->pathType);
        break;        
    }
    case BES_OTA_RECEVICE_DATA:{
        bool isViaBle = (param->pathType==DATA_PATH_BLE)?true:false;
        ota_control_handle_received_data((uint8_t*)param->param.receive_data.data, 
                                        isViaBle, 
                                        param->param.receive_data.dataLen);
        break;        
    }
    case BES_OTA_MTU_UPDATE:{
        ota_basic_mtu_set(param->param.mtu);
        break;
    }
    case BES_OTA_SEND_DONE:
        break;
    default:
        TRACE(0,"[%s]unknown event %02x", __func__, param->event);
        break;
    }
}

void app_ota_rx_thread_init(void)
{
    TRACE(0,"[%s]",__func__);

    InitCQueue(&ota_rx_cqueue, APP_OTA_EVENT_BUF_SIZE, ( CQItemType * )app_ota_event_buf);
    app_ota_event_mailbox_init();

    app_ota_event_thread = osThreadCreate(osThread(app_ota_event_handler_thread), NULL);

    app_ota_buf_lock = osMutexCreate(osMutex(app_ota_buf_lock));
    if (app_ota_buf_lock == NULL) {
        TRACE(1, "Failed to Create ota buf lock\n");
        return;
    }

}

void ota_basic_env_init(void)
{
#if defined(BES_OTA) && !defined(OTA_OVER_TOTA_ENABLED)
    app_ota_spp_init();
#ifdef __IAG_BLE_INCLUDE__
    ota_ble_init();
#endif
#endif
    app_ota_rx_thread_init();
}
