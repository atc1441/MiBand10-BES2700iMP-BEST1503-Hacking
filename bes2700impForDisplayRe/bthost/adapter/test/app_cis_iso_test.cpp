#ifdef BLE_ISO_ENABLED
#include "app_trace_rx.h"
#include "bluetooth_ble_api.h"
#include "bes_aob_api.h"
#include "bap_service.h"
#include "aob_conn_api.h"
#include "bt_drv_interface.h"
#include "hci_i.h"
#include "ble_iso.h"
#include "app_ble.h"
#include "app_audio.h"
#include "gap_service.h"

#define ADV_DEMO_DATA0 "\x02\x01\x06\x03\x18\x04\xFE"
#define ADV_DEMO_DATA0_LEN (7)

const static uint8_t adv_addr_set[6]  = {0x66, 0x34, 0x33, 0x23, 0x22, 0x11};

POSSIBLY_UNUSED static uint8_t test_iso_data[6]  = {0x66, 0x34, 0x33, 0x23, 0x22, 0x11};
POSSIBLY_UNUSED static uint8_t aob_test_iso_data[6]  = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

uint16_t record_cis_handle = 0;
uint16_t record_acl_handle = 0;

typedef void (*app_cis_iso_test_cmd_handler_t)(void);


typedef struct {
    const char* string;
    app_cis_iso_test_cmd_handler_t function;
} app_cis_iso_test_handler_t;


typedef void (*app_cis_iso_test_cmd_with_param_handler_t)(const char* cmdParam, uint32_t cmdParamLen);

typedef struct {
    const char* string;
    app_cis_iso_test_cmd_with_param_handler_t function;
} app_cis_iso_test_with_param_handler_t;

uint16_t aob_conn_get_acl_handle()
{
    return record_acl_handle;
}

//this for application to malloc buff,for ex audio_player
POSSIBLY_UNUSED static void app_cis_test_media_buf_init()
{
    uint8_t* heapBufStartAddr = NULL;
    app_audio_mempool_init_with_specific_size(app_audio_mempool_size());

    uint32_t audioCacheHeapSize = 20*1024;

    app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
    app_iso_heap_init(heapBufStartAddr, audioCacheHeapSize);

}

static void app_cis_test_receive_data(uint16_t conhdl, GAF_ISO_PKT_STATUS_E pkt_status)
{

    gaf_media_data_t *p_decoder_frame = NULL;
    gaf_media_data_t recive_frame_info;

    while ((p_decoder_frame = (gaf_media_data_t *)bes_ble_iso_dp_itf_get_rx_data(conhdl, NULL)))
    {
             recive_frame_info = *p_decoder_frame;
             bes_ble_iso_rx_free_buff(p_decoder_frame);

             TRACE(0,"timestamp %u seq 0x%x pkt_status:%d data_len:%x",recive_frame_info.time_stamp,
                recive_frame_info.pkt_seq_nb,recive_frame_info.pkt_status,recive_frame_info.data_len);
             if(recive_frame_info.data_len>0)
             {
                 DUMP8("%02x ",recive_frame_info.sdu_data, recive_frame_info.data_len);
             }
    }

}

static int cis_sink_test_event_callback(uintptr_t group_id, bap_event_t event,bap_event_param_t param)
{
    TRACE(0,"%s %d, %d",__func__, group_id, event - BAP_EVENT_CIS_OPENED);

    gap_conn_item_t *p_acl_conn = NULL;

    if (event < BAP_EVENT_ISO_PATH_SETUP)
    {
        p_acl_conn =
            gap_get_conn_item(param.cis_opened->stream->connhdl);
    }
    else
    {
        p_acl_conn =
            gap_get_conn_item(param.iso_path_setup->stream->connhdl);
    }

    if (p_acl_conn == NULL)
    {
        TRACE(0,"0x%x", event < BAP_EVENT_ISO_PATH_SETUP ?
                  param.cis_opened->stream->connhdl :
                  param.iso_path_setup->stream->connhdl);
        return BT_STS_NO_LINK;
    }

    //uint8_t con_lid = GAP_2_GAF_CON_LID(p_acl_conn->con_idx);

    switch (event)
    {
        case BAP_EVENT_CIS_OPENED:
        {

            bap_iso_param_t iso_data_param = {0};

            iso_data_param.controller_delay_us = 0x06;

            //app_cis_test_media_buf_init();
            bes_ble_iso_dp_set_rx_dp_itf();
            bes_ble_iso_dp_itf_data_come_callback_register((void *)app_cis_test_receive_data);

            bap_setup_iso_tx_data_path(param.cis_open_req->stream->iso_handle, (const bap_iso_param_t *) &iso_data_param);
            bap_setup_iso_rx_data_path(param.cis_open_req->stream->iso_handle, (const bap_iso_param_t *) &iso_data_param);
            TRACE(0,"BAP_EVENT_CIS_OPENED ");
            if (param.cis_opened->error_code == HCI_ERROR_NO_ERROR)
            {
                TRACE(0,"cis conn group_id:%x stream_id:%x iso_handle:%x",
                    param.cis_opened->stream->group_id,
                    param.cis_opened->stream->stream_id,
                    param.cis_open_req->stream->iso_handle);
            }
            else
            {
                TRACE(0,"cis failed group_id:%x stream_id:%x error_code:%x",
                    param.cis_opened->stream->group_id,
                    param.cis_opened->stream->stream_id,
                    param.cis_opened->error_code);
            }
        }
        break;
        case BAP_EVENT_CIS_CLOSED:
        {
        }
        break;
        case BAP_EVENT_CIS_OPEN_REQ:
        {
            TRACE(0,"BAP_EVENT_CIS_OPEN_REQ ");
            bap_accept_cis_request(param.cis_open_req->stream->iso_handle, true);
        }
        break;
        case BAP_EVENT_CIS_REJECTED:
        {
        }
        break;
        case BAP_EVENT_ISO_PATH_SETUP:
        {
            POSSIBLY_UNUSED uint32_t cis_current_time = bt_syn_ble_bt_time_to_bts(btdrv_syn_get_curr_ticks(),624);
            bes_ble_iso_dp_send_data(param.iso_path_setup->stream->iso_handle,0,aob_test_iso_data,6,cis_current_time);
            TRACE(0,"ISO_PATH_SETUP group_id:%x iso_handle:%x tx_path_setup:%x rx_path_setup:%x",
                                             param.iso_path_setup->stream->group_id,
                                             param.iso_path_setup->stream->iso_handle,
                                             param.iso_path_setup->stream->iso_flag.tx_path_setup,
                                             param.iso_path_setup->stream->iso_flag.rx_path_setup);
        }
        break;
        case BAP_EVENT_ISO_PATH_REMOVED:
        {
        }
        break;
        case BAP_EVENT_CIG_OPENED:
        case BAP_EVENT_CIG_CLOSED:
        default:
            break;
    }

    return BT_STS_SUCCESS;
}

static void cis_iso_scan_data_report_callback(ble_event_t *event)
{
    ble_event_handled_t *ble_event = &event->p;
    BLE_ADV_DATA_T *particle;

    for (uint8_t offset = 0; offset < ble_event->scan_data_report_handled.length;)
    {
        /// get the particle of adv data
        particle = (BLE_ADV_DATA_T *)(ble_event->scan_data_report_handled.data + offset);
        bool ble_start_connect = false;
        bool isConnecting = app_ble_is_connection_on_by_addr(ble_event->scan_data_report_handled.trans_addr.addr);
        bool isConnected = false;//ble_audio_mobile_check_device_connected(ble_event->scan_data_report_handled.trans_addr.addr);

        if ((BLE_ADV_TYPE_COMPLETE_NAME == particle->type) && \
                (!memcmp(particle->value, app_ble_get_dev_name(), particle->length - 1)) &&
                (!isConnecting) && (!isConnected))
        {
            ble_start_connect = true;
        }

            /*
        if (!ble_start_connect)
        {
            // TODO: can set unicast server's name here

            for (uint16_t index = 0;index < particle->length;index++)
            {
                if (0 == memcmp(&(particle->value[index]), "test", 4))
                {
                    ble_start_connect = true;
                }
            }

            ble_start_connect = aob_test_match_peer_ble_addr(
                                    ble_event->scan_data_report_handled.trans_addr.addr);
        }
            */

            if (ble_start_connect)
            {
                TRACE(0,"cis_iso: found the device,start_conn");
                app_ble_stop_scan();
                app_ble_start_connect((ble_bdaddr_t *)&ble_event->scan_data_report_handled.trans_addr, APP_GAPM_STATIC_ADDR);
                break;
            }

            offset += (ADV_PARTICLE_HEADER_LEN + particle->length);
        }
}

void ble_cis_iso_event_callback(ble_event_t *event)
{
    ble_event_handled_t *ble_event = &event->p;
    uint8_t con_lid = ble_event->connect_handled.conidx;

    TRACE(0,"%s connhdl:%x event:%x",__func__,con_lid,event->evt_type);

    switch (event->evt_type)
    {
        case BLE_LINK_CONNECTED_EVENT:
            TRACE(0,"ble acl conn recored acl_handle:%x",ble_event->connect_handled.connhdl);
            record_acl_handle = ble_event->connect_handled.connhdl;
            bap_register_cis_callback(record_acl_handle,cis_sink_test_event_callback);

            bes_ble_gap_sec_send_security_req(con_lid,0x03);

            break;
        case BLE_CONNECT_BOND_EVENT:
            break;
        case BLE_CONNECT_ENCRYPT_EVENT:
            break;
        case BLE_CONNECT_BOND_FAIL_EVENT:
            break;
        case BLE_CONNECTING_STOPPED_EVENT:
            break;
        case BLE_CONNECTING_FAILED_EVENT:
            break;
        case BLE_DISCONNECT_EVENT:
            break;
        case BLE_CONN_PARAM_UPDATE_REQ_EVENT:
            break;
        case BLE_CONN_PARAM_UPDATE_FAILED_EVENT:
            break;
        case BLE_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT:
            break;
        case BLE_SET_RANDOM_BD_ADDR_EVENT:
            break;
        case BLE_ADV_STARTED_EVENT:
            break;
        case BLE_ADV_STARTING_FAILED_EVENT:
            break;
        case BLE_ADV_STOPPED_EVENT:
            break;
        case BLE_SCAN_STARTED_EVENT:
            break;
        case BLE_SCAN_DATA_REPORT_EVENT:
            cis_iso_scan_data_report_callback(event);
            break;
        case BLE_SCAN_STARTING_FAILED_EVENT:
            break;
        case BLE_SCAN_STOPPED_EVENT:
            break;
        case BLE_CREDIT_BASED_CONN_REQ_EVENT:
            break;
        case BLE_RPA_ADDR_PARSED_EVENT:
            break;
        case BLE_GET_TX_PWR_LEVEL:
            break;
        case BLE_TX_PWR_REPORT_EVENT:
            break;
        case BLE_PATH_LOSS_REPORT_EVENT:
            break;
        case BLE_SUBRATE_CHANGE_EVENT:
            break;
        case BLE_MTU_EXECHANGE_EVENT:
            break;
        default:
            break;
    }
}

static int cig_src_test_event_callback(uintptr_t group_id, bap_event_t event,bap_event_param_t param)
{
    TRACE(0,"cig_src_test_event_callback %d, evevt %d", group_id, event - BAP_EVENT_CIG_OPENED);

    switch (event)
    {
        case BAP_EVENT_CIG_OPENED:
        case BAP_EVENT_CIG_UPDATE:
        {
            TRACE(0,"cig created cig_id:%x cis_count:%x cis_id:%x cis_handle:%x",
                    param.cig_opened->cig_id,
                    param.cig_opened->cis_count,
                    param.cig_opened->cis_id[0],
                    param.cig_opened->cis_handle[0]);

                    record_cis_handle = param.cig_opened->cis_handle[0];
        }
        break;
        case BAP_EVENT_CIG_CLOSED:
        {
            TRACE(0,"cig removed cig_id:%x",param.cig_closed->cig_id);
        }
        break;
        case BAP_EVENT_CIS_OPENED:
        {
            if (param.cis_opened->error_code == HCI_ERROR_NO_ERROR)
            {
                bap_iso_param_t iso_data_param = {0};

                iso_data_param.controller_delay_us = 0x06;

                //app_cis_test_media_buf_init();
                bes_ble_iso_dp_set_rx_dp_itf();
                bes_ble_iso_dp_itf_data_come_callback_register((void *)app_cis_test_receive_data);
                bap_setup_iso_tx_data_path(param.cis_open_req->stream->iso_handle, (const bap_iso_param_t *) &iso_data_param);
                bap_setup_iso_rx_data_path(param.cis_open_req->stream->iso_handle, (const bap_iso_param_t *) &iso_data_param);

                TRACE(0,"cis conn group_id:%x stream_id:%x iso_handle:%x",
                    param.cis_opened->stream->group_id,
                    param.cis_opened->stream->stream_id,
                    param.cis_open_req->stream->iso_handle);
            }
            else
            {
                TRACE(0,"cis failed group_id:%x stream_id:%x error_code:%x",
                    param.cis_opened->stream->group_id,
                    param.cis_opened->stream->stream_id,
                    param.cis_opened->error_code);
            }
        }
        break;
        case BAP_EVENT_CIS_CLOSED:
        {
            TRACE(0,"cis discon group_id:%x stream_id:%x iso_handle:%x error_code:%x",
                        param.cis_closed->stream->group_id,
                        param.cis_opened->stream->stream_id,
                        param.cis_closed->stream->iso_handle,
                        param.cis_closed->error_code);
        }
        break;
        case BAP_EVENT_CIS_REJECTED:
        {
            // This event will only use in uc srv
        }
        break;
        case BAP_EVENT_ISO_PATH_SETUP:
        {
            POSSIBLY_UNUSED uint32_t cis_current_time = bt_syn_ble_bt_time_to_bts(btdrv_syn_get_curr_ticks(),624);
            bes_ble_iso_dp_send_data(param.iso_path_setup->stream->iso_handle,0,test_iso_data,6,cis_current_time);
            TRACE(0,"ISO_PATH_SETUP group_id:%x iso_handle:%x tx_path_setup:%x rx_path_setup:%x",
                                             param.iso_path_setup->stream->group_id,
                                             param.iso_path_setup->stream->iso_handle,
                                             param.iso_path_setup->stream->iso_flag.tx_path_setup,
                                             param.iso_path_setup->stream->iso_flag.rx_path_setup);
        }
        break;
        case BAP_EVENT_ISO_PATH_REMOVED:
        {

        }
        break;
        default:
            break;
    }

    return BT_STS_SUCCESS;

}

void cis_test_set_cig_parameters()
{
    bap_cig_param_t test_cig = {0};
    bap_cis_param_t test_cis = {0};

    //cig param as customer
    test_cig.sdu_interval_c2p_us = 0x4e20;
    test_cig.sdu_interval_p2c_us = 0x4e20;
    test_cig.test_ft_c2p = 0x01;
    test_cig.test_ft_p2c = 0x01;
    test_cig.test_iso_interval_1_25ms = 0x10;
    test_cig.worst_case_sca = 0;
    test_cig.packing = 0x01;
    test_cig.framing = 0x00;

    test_cis.cis_id = 0x00;
    test_cis.test_nse = 0x03;
    test_cis.max_sdu_c2p = 0x30;
    test_cis.max_sdu_p2c = 0x30;
    test_cis.test_max_pdu_c2p = 0x30;
    test_cis.test_max_pdu_p2c = 0x30;
    test_cis.phy_bits_c2p = 0x01;
    test_cis.phy_bits_p2c = 0x01;
    test_cis.test_bn_c2p = 0x01;
    test_cis.test_bn_p2c = 0x01;

    bap_set_cig_parameters(cig_src_test_event_callback,(const bap_cig_param_t *)&test_cig,(const bap_cis_param_t *)&test_cis,1,true);
}


void cis_test_establish()
{
    uint16_t acl_handle= aob_conn_get_acl_handle();
    bap_create_cis(record_cis_handle,acl_handle);
}


void iso_test_start_adv()
{
    TRACE(0,"%s", __func__);
    app_ble_custom_init();
    app_ble_custom_adv_write_data(BLE_ADV_ACTIVITY_USER_2,
                            true,
                            BLE_ADV_PUBLIC_STATIC,
                            (uint8_t *)adv_addr_set,
                            NULL,
                            160,
                            ADV_TYPE_CONN_EXT_ADV,
                            ADV_MODE_EXTENDED,
                            12,
                            (uint8_t *)ADV_DEMO_DATA0, ADV_DEMO_DATA0_LEN,
                            NULL, 0);

    app_ble_custom_adv_start(BLE_ADV_ACTIVITY_USER_2);
}

void iso_test_stop_adv()
{
    TRACE(0,"%s", __func__);
    app_ble_custom_adv_stop(BLE_ADV_ACTIVITY_USER_2);
}

static void iso_test_start_advertising(void)
{
    iso_test_start_adv();
}

static void iso_test_stop_advertising(void)
{
    iso_test_stop_adv();
}

static void iso_test_scan_start_handler(void)
{
    BLE_SCAN_PARAM_T scan_param = {0};

    scan_param.scanFolicyType = BLE_DEFAULT_SCAN_POLICY;
    scan_param.scanWindowMs   = 20;
    scan_param.scanIntervalMs = 50;
    app_ble_start_scan(&scan_param);
}



static const app_cis_iso_test_with_param_handler_t g_app_cis_iso_test_cmd_with_param_table[] =
{

};


static const app_cis_iso_test_handler_t g_app_cis_iso_test_cmd_table[] =
{
    {"start_adv",                   iso_test_start_advertising},
    {"close_adv",                   iso_test_stop_advertising},
    {"set_cig_param",               cis_test_set_cig_parameters},
    {"establish_cis",               cis_test_establish},

    {"start_scan",                   iso_test_scan_start_handler},

};




static app_cis_iso_test_cmd_with_param_handler_t app_cis_iso_test_cmd_with_param_get_handler(char* buf)
{
    app_cis_iso_test_cmd_with_param_handler_t p = NULL;
    for(uint32_t i = 0; i < ARRAY_SIZE(g_app_cis_iso_test_cmd_with_param_table); i++)
    {
        if (strncmp((char*)buf, g_app_cis_iso_test_cmd_with_param_table[i].string, strlen((char*)buf))==0 ||
            strstr(g_app_cis_iso_test_cmd_with_param_table[i].string, (char*)buf))
        {
            TRACE(1, "cis_iso cmd:%s", g_app_cis_iso_test_cmd_with_param_table[i].string);
            p = g_app_cis_iso_test_cmd_with_param_table[i].function;
            break;
        }
    }
    return p;
}

static int app_cis_iso_test_cmd_with_param_handler(char* cmd, uint32_t cmdLen, char* cmdParam, uint32_t cmdParamLen)
{
    int ret = 0;

    app_cis_iso_test_cmd_with_param_handler_t handl_function = app_cis_iso_test_cmd_with_param_get_handler(cmd);
    if(handl_function)
    {
        handl_function(cmdParam, cmdParamLen);
    }
    else
    {
        ret = -1;
        TRACE(0,"can not find handle function");
    }
    return ret;
}

static app_cis_iso_test_cmd_handler_t app_cis_iso_test_cmd_get_handler(unsigned char* buf)
{
    app_cis_iso_test_cmd_handler_t p = NULL;
    for(uint32_t i = 0; i < ARRAY_SIZE(g_app_cis_iso_test_cmd_table); i++)
    {
        if (strncmp((char*)buf, g_app_cis_iso_test_cmd_table[i].string, strlen((char*)buf))==0 ||
            strstr(g_app_cis_iso_test_cmd_table[i].string, (char*)buf))
        {
            TRACE(1, "cis_iso cmd:%s", g_app_cis_iso_test_cmd_table[i].string);
            p = g_app_cis_iso_test_cmd_table[i].function;
            break;
        }
    }
    return p;
}

static int app_cis_iso_test_cmd_handler(unsigned char *buf, unsigned int length)
{
    int ret = 0;

    app_cis_iso_test_cmd_handler_t handl_function = app_cis_iso_test_cmd_get_handler(buf);
    if(handl_function)
    {
        //handl_function();
        bt_defer_call_func_0(handl_function);
    }
    else
    {
        ret = -1;
        TRACE(0,"can not find handle function");
    }
    return ret;
}

static unsigned int app_cis_iso_test_cmd_callback(unsigned char *cmd, unsigned int cmd_length)
{
    int param_len = 0;
    char* cmd_param = NULL;
    char* cmd_end = (char *)cmd + cmd_length;

    cmd_param = strstr((char*)cmd, (char*)"|");

    if (cmd_param)
    {
        *cmd_param = '\0';
        cmd_length = cmd_param - (char *)cmd;
        cmd_param += 1;

        param_len = cmd_end - cmd_param;

        app_cis_iso_test_cmd_with_param_handler((char *)cmd, cmd_length, cmd_param, param_len);
    }
    else
    {
        app_cis_iso_test_cmd_handler((unsigned char*)cmd, strlen((char*)cmd));
    }

    return 0;
}

void app_cis_iso_ble_init(void)
{
    app_ble_core_evt_cb_register(ble_cis_iso_event_callback);
}

void app_cis_iso_test_cmd_init(void)
{
#ifdef APP_TRACE_RX_ENABLE
    TRACE(0,"app_cis_iso_test_cmd_init");
    app_trace_rx_register("CIS", app_cis_iso_test_cmd_callback);
#endif
    app_cis_iso_ble_init();
}
#endif