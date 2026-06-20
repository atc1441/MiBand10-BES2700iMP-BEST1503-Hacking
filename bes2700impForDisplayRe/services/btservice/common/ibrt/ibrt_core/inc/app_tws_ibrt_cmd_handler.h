/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __APP_TWS_IBRT_CMD_HANDLER__
#define __APP_TWS_IBRT_CMD_HANDLER__
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_cmd_audio_analysis.h"
#include "app_tws_ibrt_cmd_sync_hfp_status.h"
#include "app_tws_ibrt_cmd_sync_a2dp_status.h"
#include "app_ibrt_voice_report.h"

#ifndef APP_IBRT_CMD_RSP_MASK
#define APP_IBRT_CMD_RSP_MASK       0x80000000
#endif

#ifndef APP_IBRT_FULL_CMD_MASK
#define APP_IBRT_FULL_CMD_MASK      0x0000FFFF
#endif

#ifndef APP_IBRT_CMD_MASK
#define APP_IBRT_CMD_MASK           0x0F00
#endif

#ifndef APP_IBRT_CMD_PREFIX
#define APP_IBRT_CMD_PREFIX         0x0000
#endif

#ifndef APP_IBRT_CUSTOM_CMD_PREFIX
#define APP_IBRT_CUSTOM_CMD_PREFIX  0x0100
#endif

#define IBRT_DESC_SIZE                                    (8)

#define IBRT_WAIT_PROFILE_TIMEOUT                             (10000)
#define IBRT_NEW_PROFILE_WAIT_TIMEOUT                         (245)
#define IBRT_WAIT_PROFILE_EXCHANGE_COMPLETE_TIMEOUT           (500)

#define RSP_TIMEOUT_DEFAULT                     (5000)
#define RSP_TIMEOUT_FAST_ACK                    (1000)
#define APP_TWS_IBRT_MAX_DATA_SIZE              (672)
#define APP_TWS_IBRT_CMDCODE_SIZE               sizeof(uint16_t)
#define APP_TWS_IBRT_CMDSEQ_SIZE                sizeof(uint16_t)
#define APP_TWS_IBRT_CMDHEAD_SIZE               (APP_TWS_IBRT_CMDCODE_SIZE+APP_TWS_IBRT_CMDSEQ_SIZE)

#ifndef FREEMAN_ENABLED_STERO
#define IBRT_BESAUD_RX_BUFF_SIZE                (672)
#define IBRT_BESAUD_TX_BUFF_SIZE                (672*4)
#else
#define IBRT_BESAUD_RX_BUFF_SIZE                (0)
#define IBRT_BESAUD_TX_BUFF_SIZE                (0)
#endif
#define IBRT_TIMEOUT_INVALID                    (0)
#define app_ibrt_cmd_rsp_timeout_handler_null   (0)
#define app_ibrt_sync_dts_coc_data_handler_null (0)
#define app_ibrt_cmd_rsp_handler_null           (0)
#define app_ibrt_cmd_rx_handler_null            (0)
#define app_ibrt_cmd_tx_done_handler_null       (0)
#define RSP_TIMEOUT_VOICE_REPORT        (1000)

#define APP_TWS_CMD_PRIO_8       (0x8)
#define APP_TWS_CMD_PRIO_9       (0x9)
#define APP_TWS_CMD_PRIO_10      (0xA)
#define APP_TWS_CMD_PRIO_11      (0xB)
#define APP_TWS_CMD_PRIO_12      (0xC)
#define APP_TWS_CMD_PRIO_13      (0xD)
#define APP_TWS_CMD_PRIO_14      (0xE)
#define APP_TWS_CMD_PRIO_15      (0xF)

typedef int (*TWS_CMD_HANDLER_T)(void **cmd_tbl, void ***cmd_var_tbl, uint16_t *cmd_size);

enum CMD_ID_T
{
    TWS_CMD_IBRT = 0,
    TWS_CMD_CUSTOMER,
    TWS_CMD_NUM,
};

typedef enum
{
    APP_TWS_ACCEPT,
    APP_TWS_NOT_ACCEPT,
} app_tws_switch_e;

typedef enum
{
    APP_TWS_CMD_PROFILE_DATA_EXCHANGE               = 0x8001,
    APP_TWS_CMD_PROFILE_DATA_EXCHANGE_DONE          = 0x8002,
    APP_TWS_CMD_RSP                                 = 0x8003,
    APP_TWS_CMD_SWITCH_ROLE                         = 0x8004,
    APP_TWS_CMD_SEND_PLAYBACK_INFO                  = 0x8005,
    APP_TWS_CMD_SET_TRIGGER_TIME                    = 0x8006,
    APP_TWS_CMD_NEED_RETRIGGER                      = 0x8007,
    APP_TWS_CMD_SYNC_TUNE                           = 0x8008,
    APP_TWS_CMD_SET_LATENCYFACTOR                   = 0x8009,
    APP_TWS_CMD_STOP_IBRT                           = 0x800B,
    APP_TWS_CMD_SET_ENV                             = 0x800C,
    APP_TWS_CMD_PROFILE_DATA_REQ                    = 0x800D,
    APP_TWS_CMD_FAST_ACK_REQ                        = 0x800E,
    APP_TWS_CMD_A2DP_STATUS_SYNC                    = 0x800F,
    APP_TWS_CMD_HFP_STATUS_SYNC                     = 0x8010,
    APP_TWS_CMD_VOICE_REPORT_REQUEST                = 0x8011,
    APP_TWS_CMD_VOICE_REPORT_START                  = 0x8012,
    APP_TWS_CMD_KEYBOARD_REQUEST                    = 0x8013,
    APP_TWS_CMD_GET_PEER_MOBILE_RSSI                = 0x8015,
    APP_TWS_CMD_SET_LINK_POLICY                     = 0x8016,
    APP_TWS_CMD_SEND_CONTROLLER_PROFILE             = 0x8017,
    APP_TWS_CMD_SYNC_VOLUME_INFO                    = 0x8018,
    APP_TWS_CMD_PERFORM_ACTION                      = 0x8019,
    APP_TWS_CMD_EXIT_MOBILE_SNIFF_MODE              = 0x801A,
    APP_TWS_CMD_STOP_IBRT_FAILED                    = 0x801B,
    APP_TWS_CMD_SHARE_COMMON_INFO                   = 0x801C,
    APP_TWS_CMD_SYNC_MIX_PROMPT_REQ                 = 0x801D,
    APP_TWS_CMD_STOP_PEER_PROMPT_REQ                = 0x801E,
    APP_TWS_CMD_LET_PEER_PLAY_PROMPT                = 0x801F,
    APP_TWS_CMD_LET_MASTER_PREPARE_RS               = 0x8020, // for slave triggered role switch, BISTO need the master to control the gsound role switch
    APP_TWS_CMD_LET_SLAVE_CONTINUE_RS               = 0x8021, // for slave triggered role switch, tell the slave gsound role switch has completed
    APP_TWS_CMD_AI_SEND_CMD_TO_PEER                 = 0x8022, // ai send cmd to peer whitout rsp
    APP_TWS_CMD_AI_SEND_CMD_TO_PEER_WITH_RSP        = 0x8023, // ai send cmd to peer whit rsp

    APP_TWS_CMD_MOBILE_LINK_PLAYBACK_INFO           = 0x8024,
    APP_TWS_CMD_SEND_IBRT_MGR_INFO                  = 0x8026,
    APP_TWS_CMD_NOTIFY_RUNNING_INFO                 = 0x8027,
    APP_TWS_CMD_CONN_PROFILE_REQ                    = 0x8028,
    APP_TWS_CMD_DISC_PROFILE_REQ                    = 0x8029,
    APP_TWS_CMD_DISC_RFCOMM_REQ                     = 0x802A,
    APP_TWS_CMD_SYNC_TOTA_ENCODE_STATUS             = 0x802B,
    APP_TWS_CMD_SEND_CUSTOM_PLAY_SPEED_TUNING_REQ   = 0x802C,
    APP_TWS_CMD_SYNC_TARGET_BUF_CNT_REQ             = 0x802D,
    APP_TWS_CMD_START_IBRT_FAILED                   = 0x802E,
    APP_TWS_CMD_SET_SYNC_TIME                       = 0x802F,
    APP_TWS_CMD_LET_MASTER_SEND_AT_CHLD             = 0x8030,
    APP_TWS_CMD_ENHANCED_ROLESWITCH                 = 0x8031,
    APP_TWS_CMD_SYNC_TXRX_CREDIT                    = 0x8032,
    APP_TWS_CMD_SYNC_INFO                           = 0x8033,
    APP_TWS_CMD_DESTROY_DEVICE                      = 0x8034,
    APP_TWS_CMD_NOTIFY_UI_INFO                      = 0x8035,
    APP_TWS_CMD_COMMON_CHNL                         = 0X8036,
    APP_TWS_CMD_NOTIFY_BT_NV_RECORED_CHANGED        = 0x8037,
    APP_TWS_CMD_SYNC_REMOTE_SMP_DEV                 = 0x8038,
    APP_TWS_CMD_SEND_SASS_INFO                      = 0x8039,
    APP_TWS_CMD_SWITCH_BACKGROUND                   = 0x803A,
    APP_TWS_CMD_REFILL_FRAMES                       = 0x803B,
    APP_TWS_CMD_SHARE_BLE_CTKD_INFO                 = 0x803C,
    APP_TWS_CMD_FORWARD_PROMPT_PLAYING_REQ          = 0x803D,
    APP_TWS_CMD_SNED_GFPS_FLAG_INFO                 = 0x803E,
    APP_TWS_CMD_SNED_GFPS_PASSKEY                   = 0x803F,
    APP_TWS_CMD_SEND_GFPS_RING_INFO                 = 0x8040,
    APP_TWS_CMD_SNED_GFPS_BLE_DISC_CMD              = 0x8041,
    APP_TWS_CMD_SEND_DEV_MGR_INFO                   = 0x8042,
    APP_TWS_CMD_SEND_LEA_ADDR_MGR                   = 0x8043,
    APP_TWS_CMD_SEND_BIS_SELFSCAN_INFO              = 0x8044,
    APP_TWS_CMD_SEND_DEMO_APP_INFO                  = 0x8045,
    APP_TWS_CMD_EXCH_BLE_AUDIO_INFO                 = 0x8046,
    APP_TWS_CMD_SYNC_DEV_INFO                       = 0x8047,
    APP_TWS_CMD_SHARE_SERVICE_INFO                  = 0x8048,
    APP_TWS_CMD_REQ_TRIGGER_SYNC_CAPTURE            = 0x8049,
    APP_TWS_CMD_CAPTURE_US_SINCE_LATEST_ANCHOR      = 0x804A,
    APP_TWS_CMD_SHARE_FASTPAIR_INFO                 = 0x804B,
    APP_TWS_CMD_GMA_SECRET_KEY                      = 0x804C,
    APP_TWS_CMD_BISTO_DIP_SYNC                      = 0x804D,
    APP_TWS_CMD_DMA_AUDIO                           = 0x804E,
    APP_TWS_CMD_UPDATE_BITRATE                      = 0x804F,
    APP_TWS_CMD_REPORT_BUF_LVL                      = 0x8050,
    APP_TWS_CMD_SYNC_ANC_STATUS                     = 0x8051,
    APP_TWS_CMD_SYNC_PSAP_STATUS                    = 0x8052,
    APP_TWS_CMD_SYNC_ANC_ASSIST_STATUS              = 0x8053,
    APP_TWS_CMD_SYNC_AUDIO_PROCESS                  = 0x8054,
    APP_TWS_CMD_NOTIFY_SLAVE_MAX_LINK_CHANGE        = 0x8055,
    APP_TWS_CMD_CONTROL_SBM                         = 0x8056,
    APP_TWS_CMD_SYNC_BIXBY_STATE                    = 0x8057,
    APP_TWS_CMD_SHARE_LINK_INFO                     = 0x8058,
    APP_TWS_CMD_ROLE_SWITCH_MONITOR                 = 0x8059,
    APP_TWS_CMD_SYNC_TOTA_FACTORY_RESET             = 0x805A,
    APP_TWS_CMD_SYNC_TOTA_LEAK_DETECT               = 0x805B,
    APP_TWS_CMD_TOTA_SEND_LEAK_DETECT_STATUS        = 0x805C,
    APP_TWS_CMD_SYNC_TOTA_AUDIO_EQ                  = 0x805D,
    APP_TWS_CMD_SYNC_TOTA_BUTTON_SETTINGS_CONTROL   = 0x805E,
    APP_TWS_CMD_SYNC_CODEC_INFO                     = 0x805F,
    APP_TWS_CMD_RECORD_INFO                         = 0x8060,
    APP_TWS_CMD_SPA_SENS_DATA_SYNC                  = 0x8061,
    APP_TWS_CMD_OTA_UPDATE_NOW                      = 0x8062,
    APP_TWS_CMD_UPDATE_SECTION                      = 0x8063,
    APP_TWS_CMD_CHECK_UPDATE_INFO                   = 0x8064,
    APP_TWS_CMD_CHECK_UPDATE_INFO2                  = 0x8065,
    APP_TWS_CMD_SYNC_BREAKPIONT                     = 0x8066,
    APP_TWS_CMD_VALIDATION_DONE                     = 0x8067,
    APP_TWS_CMD_OTA_GET_VERSION_CMD                 = 0x8068,
    APP_TWS_CMD_OTA_SELECT_SIDE_CMD                 = 0x8069,
    APP_TWS_CMD_OTA_BP_CHECK_CMD                    = 0x806A,
    APP_TWS_CMD_OTA_START_OTA_CMD                   = 0x806B,
    APP_TWS_CMD_OTA_OTA_CONFIG_CMD                  = 0x806C,
    APP_TWS_CMD_OTA_SEGMENT_CRC_CMD                 = 0x806D,
    APP_TWS_CMD_OTA_IMAGE_CRC_CMD                   = 0x806E,
    APP_TWS_CMD_OTA_IMAGE_OVERWRITE_CMD             = 0x806F,
    APP_TWS_CMD_OTA_SET_USER_CMD                    = 0x8070,
    APP_TWS_CMD_OTA_GET_OTA_VERSION_CMD             = 0x8071,
    APP_TWS_CMD_OTA_ROLE_SWITCH_CMD                 = 0x8072,
    APP_TWS_CMD_OTA_MOBILE_DISC_CMD                 = 0x8073,
    APP_TWS_CMD_OTA_UPDATE_RD_CMD                   = 0x8074,
    APP_TWS_CMD_OTA_IMAGE_BUFF                      = 0x8075,
    APP_TWS_CMD_GMA_OTA                             = 0X8076,
    APP_TWS_CMD_COMMON_OTA                          = 0X8077,
    APP_TWS_CMD_BLE_ROLE_SWITCH_SHARE_INFO          = 0x8078,
    APP_TWS_CMD_SNED_STREAMING_STATE_CMD            = 0x8079,
    APP_TWS_CMD_SPECIAL_ALL_CANCEL                  = 0xffff,
} app_tws_cmd_code_e;

#define IBRT_TWS_PERFORM_USER_ACITON                    0xff
#define IBRT_TWS_PERFORM_CONNECT_SECOND_MOBILE          0x01
#define IBRT_TWS_PERFORM_DISCONNECT_MOBILE_TWS_LINK     0x02
#define IBRT_TWS_PERFORM_PEER_TWS_SWITCH_DONE           0x03
#define IBRT_TWS_PERFORM_CHOICE_MOBILE_CONNECT          0x04
#define IBRT_TWS_PERFORM_AVRCP_NOTIFY_REGISTER          0x05
#define IBRT_TWS_PERFORM_COMMON_SM_WITH_MOBILE          0x06
#define IBRT_TWS_PERFORM_A2DP_STREAM_COMMAND_RSP_ACTION 0x07
#define IBRT_TWS_PERFORM_A2DP_STREAM_COMMAND_RSP_DONE   0x08
#define IBRT_TWS_PERFORM_SCO_CODEC_INFO_SYNC            0x09
#define IBRT_TWS_PERFORM_DELETE_PEER_NV_ADDR            0x0A
#define IBRT_TWS_PERFORM_NEW_MASTER_READY_REQ           0x0B
#define IBRT_TWS_PERFORM_NEW_MASTER_READY_RSP           0x0C
#define IBRT_TWS_PERFORM_RELEASE_SPP_DLC_CONNECTION     0x0D
#define IBRT_TWS_PERFORM_ACRCP_CT_SDP_INFO              0x0E
#define IBRT_TWS_PERFORM_SET_TWS_LINK_ID_REQ            0x0F
#define IBRT_TWS_PERFORM_SET_TWS_LINK_ID_RSP            0x10
#define IBRT_TWS_PERFORM_NO_PROFILE_NOTIFY              0x11

typedef void (*app_tws_cmd_send_handler_t)(uint8_t*, uint16_t);
typedef void (*app_tws_cmd_receivd_handler_t)(uint16_t, uint8_t*, uint16_t);
typedef void (*app_tws_rsp_timeout_handle_t)(uint16_t, uint8_t*, uint16_t);
typedef void (*app_tws_rsp_handle_t)(uint16_t, uint8_t*, uint16_t);
typedef void (*app_tws_cmd_tx_done_handler_t) (uint16_t, uint16_t, uint8_t*, uint16_t);
typedef void (*app_tws_cmd_send_via_ble_t)(uint8_t*, uint16_t);
typedef bool (*app_tws_exchange_profile_immediate)(void *remote);
typedef struct
{
    void                            *msg_p;
    int32_t                         timeout_ms;
} app_tws_cmd_timer_instance_t;

typedef struct
{
    uint32_t                        cmdcode;
    const char                      *log_cmd_code_str;
    app_tws_cmd_send_handler_t      tws_cmd_send;
    app_tws_cmd_receivd_handler_t   cmdhandler;             /**< command handler function */
    uint32_t                        timeout_ms;
    app_tws_rsp_timeout_handle_t    app_tws_rsp_timeout_handle;
    app_tws_rsp_handle_t            app_tws_rsp_handle;
    app_tws_cmd_tx_done_handler_t   app_tws_cmd_tx_done_handler;
    uint8_t                         prio;
} __attribute__((packed)) app_tws_cmd_instance_t;


typedef enum
{
    HANDLE_BASED,
    ADDR_BASED,
} cmd_type_e;

typedef struct
{
    uint16_t                        opcode;
    cmd_type_e                      cmd_type;
    uint8_t                         type_pos;
    const char                      *log_cmd_code_str;
} __attribute__((packed)) app_ibrt_cmd_filter_t;

typedef struct
{
    uint16_t  cmdcode;
    uint16_t  cmdseq;
    uint8_t   content[APP_TWS_IBRT_MAX_DATA_SIZE];
} __attribute__((packed)) app_tws_ibrt_cmd_t;

#ifdef __cplusplus
extern "C" {
#endif

void app_ibrt_cmd_handler_init(void);
void app_ibrt_cmd_handler_register_cmd_tx_done_cb(app_tws_cmd_tx_done_handler_t handler);
int app_ibrt_cmd_table_get(void **cmd_tbl,  void ***cmd_var_tbl, uint16_t *cmd_size);
void app_ibrt_init_cmd_timer_table(enum CMD_ID_T cmd_id);
void app_ibrt_clear_cmd_mailbox(void);
int app_ibrt_set_cmdhandle(enum CMD_ID_T cmd_id, TWS_CMD_HANDLER_T handler);
void app_ibrt_send_cmd_via_ble_register(app_tws_cmd_send_via_ble_t func);
void app_ibrt_cmd_rx_handler(uint8_t* p_data_buff, uint16_t length);
void app_ibrt_data_send_handler(void);
void app_ibrt_data_receive_handler(void);
void app_ibrt_cmd_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
bool app_ibrt_send_cmd_without_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_cmd_with_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_ibrt_profile_data_exchange_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_wait_profile_exchange_complete_timer_cb(void const *param);
void app_ibrt_set_codec_type(uint8_t *p_buff, uint16_t length);
void app_ibrt_set_codec_type_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_profiles(ibrt_mobile_info_t* p_mobile_info);
void app_ibrt_send_tws_switch_cmd(uint8_t *p_buff, uint16_t length);
void app_ibrt_tws_switch_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_cmd_rsp(uint8_t *p_buff, uint16_t length);
bool app_ibrt_send_cmd_process_timeout(uint16 ms);
void app_ibrt_inform_stop_ibrt_mode(uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_mode_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_codec_type_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_codec_type_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_data_exchange_rsp_timeout_handler(uint8_t *p_buff, uint16_t length);
void app_ibrt_data_exchange_done(uint8_t *p_buff, uint16_t length);
void app_ibrt_data_exchange_done_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd(uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_profile_data_req_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_profile_data_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_wait_profile_ready_timer_cb(void const *n);
void app_ibrt_delay_profile_send_timer_cb(void const *n);
void app_ibrt_delay_profile_send_timer(uint32_t millisec);
void app_ibrt_wait_profile_ready_timeout_handler(void);
void app_ibrt_wait_profile_exchange_complete_timer(ibrt_mobile_info_t *p_mobile_info);
void app_ibrt_cancel_profile_send_procedure(ibrt_mobile_info_t *p_mobile_info);
void app_ibrt_send_conn_profile_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_conn_profile_req_handler(uint16_t req_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_conn_profile_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_disc_profile_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_disc_profile_req_handler(uint16_t req_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_disc_profile_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_disc_rfcomm_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_disc_rfcomm_req_handler(uint16_t req_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_disc_rfcomm_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
int app_ibrt_pack_a2dp_command_accept_event(void* p_remote, uint8_t transaction, uint8_t signal_id);
void app_ibrt_new_master_is_ready(const bt_bdaddr_t *remote);
void app_ibrt_sync_profile_init_status(ibrt_mobile_info_t *p_mobile_info);
void app_ibrt_exchange_profile_timer_init(ibrt_mobile_info_t *p_mobile_info);
void app_ibrt_exchange_profile_timer_deinit(ibrt_mobile_info_t *p_mobile_info);
uint8_t app_ibrt_send_profile_data(ibrt_mobile_info_t *p_mobile_info,uint64_t profile_mask,bool tx_silence);
void app_ibrt_profile_ready_notify(ibrt_mobile_info_t *p_mobile_info,uint64_t profile);
uint8_t app_ibrt_profile_check_and_resync(void *param0);
void app_ibrt_fast_ack_req_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_fast_ack_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_ibrt_fast_ack_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_fast_ack_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
uint16_t app_ibrt_find_cmd_table_index(uint16_t cmdcode, app_tws_cmd_instance_t **cmd_tbl, app_tws_cmd_timer_instance_t ***cmd_var_tbl);
void app_ibrt_sync_tota_encode_status_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_tota_encode_status(uint8_t *p_buff, uint16_t length);
void app_ibrt_perform_action(uint8_t *p_buff, uint16_t length);
void app_ibrt_perform_action_handler_v2(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_init_tws_trx_cmd_queue(void);
void app_ibrt_reset_tws_trx_cmd_queue(void);
void app_ibrt_notify_link_policy(uint8_t *p_buff, uint16_t length);
void app_ibrt_notify_link_policy_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ui_debug_controller_timer_cb(void const *current_evt);
void app_ibrt_recieve_bt_controller_profile_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_bt_controller_profile(uint8_t *p_buff, uint16_t input_length);
void app_ibrt_sync_volume_info_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_get_volume_info_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_exit_mobile_sniff(uint8_t *p_buff, uint16_t length);
void app_ibrt_exit_mobile_sniff_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_failed(uint8_t *p_buff, uint16_t length);
void app_ibrt_start_ibrt_failed(uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_failed_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_start_ibrt_failed_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_share_common_info(uint8_t *p_buff, uint16_t length);
void app_ibrt_share_common_info_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_share_common_info_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_share_common_info_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_mix_prompt_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_mix_prompt_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_prompt_play_req_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_prompt_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_peer_prompt_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_peer_prompt_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_let_master_prepare_rs(uint8_t *p_buff, uint16_t length);
void app_ibrt_master_prepare_rs(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_let_slave_continue_rs(uint8_t *p_buff, uint16_t length);
void app_ibrt_slave_continue_rs(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ai_send_cmd_to_peer(uint8_t *p_buff, uint16_t length);
void app_ibrt_ai_rev_peer_cmd_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_mobile_link_playback_info(uint8_t *p_buff, uint16_t length);
void app_ibrt_send_mobile_link_playback_info_handler(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
void app_ibrt_ai_send_cmd_with_rsp_to_peer(uint8_t *p_buff, uint16_t length);
void app_ibrt_ai_rev_cmd_with_rsp_to_peer_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ai_rev_cmd_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ai_rev_cmd_rsp_from_peer_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_send_custom_play_speed_tuning_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_custom_play_speed_tuning_req_handler(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
void app_ibrt_custom_play_speed_tuning_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_custom_play_speed_tuning_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_sync_target_buf_cnt_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_target_buf_cnt_req_handler(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
void app_ibrt_sync_target_buf_cnt_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_target_buf_cnt_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_set_sync_time_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_sync_time(uint8_t *p_buff, uint16_t length);
void app_ibrt_set_sync_time_done(uint16_t cmdcode, uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);

void app_ibrt_send_switch_background_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_switch_background_handler(uint16_t req_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_switch_background_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

#define APP_TWS_EXT_CMDTYPE_MASK 0xe800

#define APP_TWS_EXT_CMDCODE_MASK 0xefff

#define APP_TWS_EXT_CMD_RSP_BIT  0x1000

#define APP_TWS_BESAPP_EXT_CMD_PREFIX 0xe100

#define APP_TWS_CUSTOM_EXT_CMD_PREFIX 0xe200

typedef struct
{
    uint16_t                        ext_cmdcode;
    uint16_t                        ext_cmdseq;
} app_tws_ext_cmd_head_t;

typedef void (*app_tws_ext_cmd_rx_handler)(bool is_response, app_tws_ext_cmd_head_t *extcmd, uint32_t length);

typedef struct
{
    uint16_t                        ext_cmdcode;
    const char                      *ext_cmdcode_str;
    app_tws_ext_cmd_rx_handler    ext_cmd_rx_handler;
} app_tws_ext_cmd_handler_t;

typedef struct
{
    uint16_t                        ext_cmdtype;
    uint16_t                        ext_cmd_count;
    const app_tws_ext_cmd_handler_t *first_ext_cmd_handler;
} app_tws_ext_cmd_table;

bool app_ibrt_register_ext_cmd_table(const app_tws_ext_cmd_handler_t* first_cmd, uint16_t cmd_count);

void app_ibrt_tws_send_ext_cmd(uint16_t cmdcode, app_tws_ext_cmd_head_t *cmd, uint32_t length);

void app_ibrt_tws_send_ext_cmd_rsp(uint16_t cmdcode, app_tws_ext_cmd_head_t *cmd, uint32_t length);

void app_tws_ibrt_allow_send_profile_init(app_tws_exchange_profile_immediate allow_send_profile_immediatelly);

#ifdef __cplusplus
}
#endif

void app_ibrt_sync_tota_leak_deteck(uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_tota_leak_deteck_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
uint8_t app_ibrt_tota_get_leak_detect_status();
void app_ibrt_tota_send_leak_deteck_status(uint8_t *p_buff, uint16_t length);
void app_ibrt_tota_send_leak_detect_status_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_tota_audio_eq(uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_tota_audio_eq_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_tota_button_settings_control(uint8_t *p_buff, uint16_t length);
void app_ibrt_sync_tota_button_settings_control_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

#endif/*__APP_TWS_IBRT_CMD_HANDLER__*/
