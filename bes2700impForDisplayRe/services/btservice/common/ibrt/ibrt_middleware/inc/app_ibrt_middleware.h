/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

#ifndef __APP_IBRT_MIDDLEWARE_H__
#define __APP_IBRT_MIDDLEWARE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/

/******************************macro defination*****************************/
#define TWS_SYNC_BUF_SIZE           APP_TWS_CTRL_BUFFER_MAX_LEN

// UI master is the one that communicates with mobile,
// it doesn't matter whether TWS is connected or not
typedef uint8_t TWS_UI_ROLE_E;
#define TWS_UI_MASTER       IBRT_MASTER
#define TWS_UI_SLAVE        IBRT_SLAVE
#define TWS_UI_UNKNOWN      IBRT_UNKNOW

#define  IBRT_UI_LONG_POLL_INTERVAL                         (0xD0)
#define  IBRT_UI_DEFAULT_POLL_INTERVAL                      (0x68)

#ifdef DUAL_MIC_RECORDING
#define  IBRT_UI_SHORT_POLL_INTERVAL                        (0x18)
#else
#define  IBRT_UI_SHORT_POLL_INTERVAL                        (0x34)
#endif
#define  IBRT_UI_EXTREMELY_SHORT_POLL_INTERVAL              (0x1A)

#define  IBRT_UI_DEFAULT_POLL_INTERVAL_IN_SCO               (0x9c)
#ifdef DUAL_MIC_RECORDING
#define  IBRT_UI_SHORT_POLL_INTERVAL_IN_SCO                 (0x27)
#else
#define  IBRT_UI_SHORT_POLL_INTERVAL_IN_SCO                 (0x4E)
#endif

#define  IBRT_TWS_BT_TPOLL_DEFAULT                          (80)

/******************************type defination******************************/
typedef enum {
    EAR_SIDE_UNKNOWN = 0,
    EAR_SIDE_LEFT    = 1,
    EAR_SIDE_RIGHT   = 2,
    EAR_SIDE_NUM,
} APP_TWS_SIDE_T;

enum {
    DISCONNECTED = 0,
    CONNECTED    = 1,

    CONNECTION_STATE_NUM,
};

typedef enum {
    TWS_SYNC_USER_BLE_INFO      = 0,
    TWS_SYNC_USER_OTA           = 1,
    TWS_SYNC_USER_AI_CONNECTION = 2,
    TWS_SYNC_USER_GFPS_INFO     = 3,
    TWS_SYNC_USER_AI_INFO       = 4,
    TWS_SYNC_USER_AI_MANAGER    = 5,
    TWS_SYNC_USER_DIP           = 6,
    TWS_SYNC_USER_LE_AUDIO      = 7,

    TWS_SYNC_USER_NUM,
} TWS_SYNC_USER_E;

typedef struct
{
    int32_t a2dp_local_volume;
    int32_t hfp_local_volume;
} TWS_VOLUME_SYNC_INFO_T;

typedef struct
{
    bt_bdaddr_t remote;
    uint8_t a2dp_local_volume;
    uint8_t hfp_local_volume;
} TWS_VOLUME_SYNC_INFO_T_V2;

typedef void (*TWS_SYNC_INFO_PREPARE_FUNC_T)(uint8_t *buf, uint16_t *totalLen, uint16_t *len, uint16_t expectLen);
typedef void (*TWS_INFO_SYNC_FUNC_T)(uint8_t *buf, uint16_t len, bool isContinueInfo);
typedef void (*TWS_SET_SIDE_FUNC_T)(APP_TWS_SIDE_T* twsSide);

typedef struct {
    TWS_SYNC_INFO_PREPARE_FUNC_T sync_info_prepare_handler;
    TWS_INFO_SYNC_FUNC_T sync_info_received_handler;
    TWS_SYNC_INFO_PREPARE_FUNC_T sync_info_prepare_rsp_handler;
    TWS_INFO_SYNC_FUNC_T sync_info_rsp_received_handler;
    TWS_INFO_SYNC_FUNC_T sync_info_rsp_timeout_handler;
} TWS_SYNC_USER_T;

typedef struct {
    TWS_SYNC_USER_E userId;
    bool continueInfo;
    uint16_t infoLen;     // length of the valid data in info[TWS_SYNC_BUF_SIZE-1]
    uint8_t info[TWS_SYNC_BUF_SIZE];
}TWS_SYNC_ENTRY_T;

#define TWS_SYNC_MAX_XFER_SIZE      sizeof(TWS_SYNC_ENTRY_T)

typedef struct {
    uint16_t totalLen;  // length of the valid data in content[TWS_SYNC_MAX_XFER_SIZE]
    uint8_t content[TWS_SYNC_MAX_XFER_SIZE];
}TWS_SYNC_DATA_T;

typedef struct {
    TWS_SYNC_USER_T syncUser[TWS_SYNC_USER_NUM];
    TWS_SYNC_DATA_T sync_data;
} TWS_ENV_T;

typedef enum
{
    TWS_SYNC_CONTINUE_FILLING   = 0,
    TWS_SYNC_BUF_FULL           = 1,
    TWS_SYNC_BUF_SEGMENTED      = 2,
    TWS_SYNC_NO_DATA_FILLED     = 3
} TWS_SYNC_FILL_RET_E;

/// Definition of the bits preventing the BTC from enter sniff
enum app_ibrt_if_prevent_sniff
{
    /// Flag indicating that the OTA process is ongoing
    OTA_ONGOING                       = 0x01,
    /// Flag indicating that vocie record is ongoing
    AI_VOICE_RECORD                   = 0x02,
    /// Flag indicating that AVRCP status change is ongoing
    AVRCP_STATUS_CHANING         = 0x04,
    /// Flag indicating that Call is on ongoing
    HFP_CALL_ONGOING = 0x08,
};

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    initialize the tws interface related parameter
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_init(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_role_switch_started_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler for role switch started event of tws system
 *
 * Parameters:
 *    void
 *
 * Return:
 *    voide
 */
void app_ibrt_middleware_role_switch_started_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_role_switch_complete_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function of role switch complete event for tws system
 *    NOTE: tws system include relay_tws and IBRT
 *
 * Parameters:
 *    newRole - current role of device after role switch complete
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_role_switch_complete_handler(uint8_t newRole);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_sync_info_received_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler for tws common info received event
 *    NOTE: see @ to get more info for common info received
 *
 * Parameters:
 *    rsp_seq - sequence number of response
 *    p_buff - pointer for received data
 *    lenght - length of received data
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_sync_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/*---------------------------------------------------------------------------
 *            app_tws_middleware_common_info_rsp_recieved_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    shared common info response received handler
 *
 * Parameters:
 *    rsp_seq - sequence number of response
 *    p_buff - pointer for received data
 *    lenght - length of received data
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_sync_info_rsp_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_sync_info_rsp_timeout_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    shared common info response timeout handler
 *
 * Parameters:
 *    rsp_seq - sequence number of response
 *    p_buff - pointer for received data
 *    lenght - length of received data
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_sync_info_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_mobile_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of tws connected with mobile phone
 *
 * Parameters:
 *    addr - connected mobile address
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_mobile_connected_handler(uint8_t *addr);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_mobile_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of tws disconnected with mobile phone
 *
 * Parameters:
 *    addr - disconnected mobile phone
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_mobile_disconnected_handler(uint8_t *addr);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ibrt_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of ibrt connected
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ibrt_connected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ibrt_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of ibrt disconnected
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ibrt_disconnected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ble_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of ble connected event for tws system
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ble_connected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ble_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    ble disconnected handler for tws system
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ble_disconnected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_tws_connected_sync_info
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    sync BLE\BISTO\AI info after tws connected
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_tws_connected_sync_info(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_tws_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of tws connected event
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_tws_connected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_tws_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of tws disconnected event
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_tws_disconnected_handler(void);


/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_handle_click
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handle the power key click event
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_handle_click(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_master_prepare_rs
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    master prepare for role switch for BISTO and AI
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_master_prepare_rs(uint8_t *p_buff, uint16_t length);

#ifdef IBRT
/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_slave_continue_rs
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    slave continue role switch progress
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_slave_continue_rs(uint8_t *p_buff, uint16_t length);
#endif

void app_tws_ai_send_cmd_to_peer(uint8_t *p_buff, uint16_t length);
void app_tws_ai_rev_peer_cmd_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_tws_ai_send_cmd_with_rsp_to_peer(uint8_t *p_buff, uint16_t length);
void app_tws_ai_send_cmd_rsp_to_peer(uint8_t *p_buff, uint16_t rsp_seq, uint16_t length);
void app_tws_ai_rev_peer_cmd_with_rsp_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_tws_ai_rev_cmd_rsp_from_peer_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_tws_ai_rev_cmd_rsp_timeout_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_if_prevent_sniff_set(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
void app_ibrt_if_prevent_sniff_clear(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
bool app_ibrt_if_customer_prevent_sniff(bt_bdaddr_t *p_mobile_addr);
void app_ibrt_middleware_ui_role_updated_handler(uint8_t newRole);
bool app_ibrt_middleware_checker_allow_role_switch_handler(void);

#ifdef GFPS_ENABLED
void app_ibrt_share_fastpair_info(uint8_t *p_buff, uint16_t length);
void app_ibrt_share_fastpair_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

APP_TWS_SIDE_T app_ibrt_middleware_get_side(void);

void app_ibrt_middleware_set_side(APP_TWS_SIDE_T side);

int app_ibrt_middleware_fill_debug_info(char* buf, unsigned int buf_len);

void app_ibrt_middleware_exit_sniff_with_mobile(uint8_t* mobileAddr);
void app_ibrt_reconect_mobile_after_factorty_test(void);

void app_ibrt_middleware_write_bt_local_address(uint8_t* btAddr);
void app_ibrt_middleware_write_ble_local_address(uint8_t* bleAddr);
uint8_t *app_ibrt_middleware_get_bt_local_address(void);
uint8_t *app_ibrt_middleware_get_ble_local_address(void);
uint8_t *app_ibrt_middleware_get_bt_peer_address(void);
void app_ibrt_middleware_big_little_switch(uint8_t *in, uint8_t *out, uint8_t len);
void app_ibrt_middleware_register_sync_user(TWS_SYNC_USER_E id, TWS_SYNC_USER_T *user);
void app_ibrt_middleware_deregister_sync_user(TWS_SYNC_USER_E id);
TWS_UI_ROLE_E app_ibrt_middleware_get_ui_role(void);
bool app_ibrt_middleware_is_ui_slave(void);
bool app_ibrt_middleware_is_ui_master(void);
bool app_ibrt_middleware_is_nv_master(void);
bool app_ibrt_middleware_is_left_side(void);
bool app_ibrt_middleware_is_right_side(void);
bool app_ibrt_middleware_unknown_side(void);
const char* app_ibrt_middleware_uirole2str(TWS_UI_ROLE_E uiRole);
void app_ibrt_middleware_register_set_tws_side_handler( TWS_SET_SIDE_FUNC_T handle);
void app_ibrt_middleware_prevent_sniff_set(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
void app_ibrt_middleware_reset_prevent_sniff_clear(const uint8_t *p_mobile_addr);
void app_ibrt_middleware_prevent_sniff_clear(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __APP_IBRT_MIDDLEWARE_H__ */
