/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
#ifndef __GFPS_H__
#define __GFPS_H__
#ifdef GFPS_ENABLED

#define FP_TWS_MAX_LEN              (APP_TWS_CTRL_BUFFER_MAX_LEN)

#define FP_SERVICE_LEN                      (0x06)
#define FP_SERVICE_UUID                     (0x2CFE)
#define FP_DEVICE_MODEL_ID                  (0x2B677D)

#ifdef SPOT_ENABLED
#define FP_SPOT_SERVICE_UUID                (0xAAFE)
#define FP_SPOT_SERVICE_LEN                 (0x18)
#define FP_EID_FRAME_TYPE_WHEN_ENABLE_UTP   (0x41)
#define FP_EID_FRAME_TYPE_WHEN_DISABLE_UTP  (0x40)
#endif

#define FP_MESSAGE_RESERVED_LEN     (4)   // at least 4 bytes
// TODO: increase this value if needed
#define FP_MESSAGE_STREAM_MAX_ADDITIONAL_DATA_LEN   (64)
#define FP_MSG_MAX_LEN     (16)

#define IS_BT_DEVICE(id)  ((id) & 0x80)
#define GET_BT_ID(id)     ((id) & 0x7F)
#define SET_BT_ID(id)     ((id) | 0x80)

// The values is posted at FP spec 8/27/19 revision
#define FP_MSG_GROUP_BLUETOOTH_EVENT                0x01
#define FP_MSG_BT_EVENT_ENABLE_SILENCE_MODE         0x01
#define FP_MSG_BT_EVENT_DISABLE_SILENCE_MODE        0x02

#define FP_MSG_GROUP_COMPANION_APP_EVENT            0x02
#define FP_MSG_COMPANION_APP_LOG_BUF_FULL           0x01

#define FP_MSG_GROUP_DEVICE_INFO                    0x03
#define FP_MSG_DEVICE_INFO_MODEL_ID                 0x01
#define FP_MSG_DEVICE_INFO_BLE_ADD_UPDATED          0x02
#define FP_MSG_DEVICE_INFO_BATTERY_UPDATED          0x03
#define FP_MSG_DEVICE_INFO_REMAINING_BATTERY_TIME   0x04
#define FP_MSG_DEVICE_INFO_ACTIVE_COMPONENTS_REQ    0x05
#define FP_MSG_DEVICE_INFO_ACTIVE_COMPONENTS_RSP    0x06
#define FP_MSG_DEVICE_INFO_TELL_CAPABILITIES        0x07
#define FP_MSG_DEVICE_INFO_PLATFORM_TYPE            0x08
#define FP_MSG_DEVICE_INFO_FIRMWARE_VERSION         0x09
#define FP_MSG_DEVICE_INFO_SESSION_NONCE            0x0A
#define FP_MSG_DEVICE_INFO_EDD_IDENTIFIER           0x0B

#define FP_MSG_GROUP_DEVICE_ACTION                  0x04
#define FP_MSG_DEVICE_ACTION_RING                   0x01

#define FP_MSG_NEITHER_BUD_ACTIVE                   0x00
#define FP_MSG_RIGHT_BUD_ACTIVE                     0x01
#define FP_MSG_LEFT_BUD_ACTIVE                      0x02
#define FP_MSG_BOTH_BUDS_ACTIVE                     0x03

#define FP_MSG_GROUP_SASS                           0x07
#define FP_MSG_SASS_GET_CAPBILITY                   0x10
#define FP_MSG_SASS_NTF_CAPBILITY                   0x11
#define FP_MSG_SASS_SET_MULTIPOINT_STATE            0x12
#define FP_MSG_SASS_SET_SWITCH_PREFERENCE           0x20
#define FP_MSG_SASS_GET_SWITCH_PREFERENCE           0x21
#define FP_MSG_SASS_NTF_SWITCH_PREFERENCE           0x22
#define FP_MSG_SASS_SWITCH_ACTIVE_SOURCE            0x30
#define FP_MSG_SASS_SWITCH_BACK                     0x31
#define FP_MSG_SASS_NTF_SWITCH_EVT                  0x32
#define FP_MSG_SASS_GET_CONN_STATUS                 0x33
#define FP_MSG_SASS_NTF_CONN_STATUS                 0x34
#define FP_MSG_SASS_NTF_INIT_CONN                   0x40
#define FP_MSG_SASS_IND_INUSE_ACCOUNT_KEY           0x41
#define FP_MSG_SASS_SEND_CUSTOM_DATA                0x42
#define FP_MSG_SASS_SET_DROP_TGT                    0x43

#define FP_HEARABLE_CONTROL_VERSION                 0x01
#define FP_MSG_GROUP_HEARABLE_CONTROL               0x08
#define FP_MSG_HC_GET_ANC_STATE                     0x11
#define FP_MSG_HC_SET_ANC_STATE                     0x12
#define FP_MSG_HC_NTF_ANC_STATE                     0x13

#define FP_MSG_GROUP_ACKNOWLEDGEMENT                0xFF
#define FP_MSG_ACK                                  0x01
#define FP_MSG_NAK                                  0x02

#define FP_MSG_NAK_REASON_NOT_SUPPORTED             0x00
#define FP_MSG_NAK_REASON_DEVICE_BUSY               0x01
#define FP_MSG_NAK_REASON_NOT_ALLOWED               0x02
#define FP_MSG_NAK_REASON_REDUNDANT_ACTION          0x04

#define FP_MSG_GROUP_DEVICE_CAPABLITY_SYNC          0x06
#define FP_MSG_DEVICE_CAPABLITY_CAP_UPDATE_REQ      0x01
#define FP_MSG_DEVICE_CAPABLITY_DYN_BUF_SIZE        0x02
#define FP_MSG_DEVICE_CAPABLITY_ENABLE_EDD_TRACK    0x03

#define FIND_MY_BUDS_STATUS_SLAVE_MASK              0x01
#define FIND_MY_BUDS_STATUS_MASTER_MASK             0x02

#define GFPS_FIND_MY_BUDS_CMD_STOP                  0x00
#define GFPS_FIND_MY_BUDS_CMD_START                 0x01

#define GFPS_FIND_MY_BUDS_CMD_STOP_DUAL             0x00
#define GFPS_FIND_MY_BUDS_CMD_START_MASTER_ONLY     0x01
#define GFPS_FIND_MY_BUDS_CMD_START_SLAVE_ONLY      0x02
#define GFPS_FIND_MY_BUDS_CMD_START_DUAL            0x03

typedef enum
{
    BATTERY_NOT_CHARGING = 0,
    BATTERY_CHARGING = 1,    
} GFPS_BATTERY_STATUS_E;

typedef enum
{
    HIDE_SUBSEQUENT_INDICATION = 2,
    SHOW_UI_INDICATION = 3,
    HIDE_UI_INDICATION = 4
} GFPS_BATTERY_DATA_TYPE_E;

typedef enum
{
    GFPS_MUSIC_OFF_FIND_OFF,
    GFPS_MUSIC_OFF_FIND_ON,
    GFPS_MUSIC_ON_FIND_OFF,
    GFPS_MUSIC_ON_FIND_ON,
} GFPS_FIND_BUDS_STATUS;

typedef enum {
    GFPS_RING_MODE_BOTH_OFF,
    GFPS_RING_MODE_LEFT_ON,
    GPFS_RING_MODE_RIGHT_ON,
    GFPS_RING_MODE_BOTH_ON,
} GFPS_RING_MODE;

typedef union
{
    struct
    {
        uint8_t     isCompanionAppInstalled :   1;
        uint8_t     isSilentModeSupported   :   1;
        uint8_t     reserve                 :   6;
    } env;
    uint8_t content;
} FpCapabilitiesEnv_t;

typedef struct
{
    uint8_t isRightRinging  :   1;
    uint8_t isLeftRinging   :   1;
    uint8_t reserve         :   6;
} FpRingStatus_t;

typedef struct
{
    uint8_t version;
    uint8_t ui_flags_reserved        :  3;
    uint8_t ui_anc                   :  1;
    uint8_t ui_reserved_bit3         :  1;
    uint8_t ui_off                   :  1;
    uint8_t ui_reserved_bit1         :  1;
    uint8_t ui_transparent           :  1;

    uint8_t settable_flags_reserved  :  3;
    uint8_t settable_anc             :  1;
    uint8_t settable_reserved_bit3   :  1;
    uint8_t settable_off             :  1;
    uint8_t settable_reserved_bit1   :  1;
    uint8_t settable_transparent     :  1;

    uint8_t current_state_reserved1  :  5;
    uint8_t current_state_info       :  1;
    uint8_t current_state_reserved   :  2;
} FpANCState_t;

typedef struct
{
    uint8_t current_state_reserved1  :  5;
    uint8_t current_state_info       :  1;
    uint8_t current_state_reserved   :  2;

    uint8_t message_nonce[8];
    uint8_t message_auth_code[8];
} FpSetANCState_t;

typedef enum
{
    FP_SRV_EVENT_CONNECTED = 0,
    FP_SRV_EVENT_DISCONNECTED,
    FP_SRV_EVENT_RFCOMM_DATA_IND,
    FP_SRV_EVNET_L2CAP_DATA_IND,
    FP_SRV_EVENT_SENT_DONE,
}GFPS_SRV_EVENT_TYPE_T;

typedef enum  {
    GFPS_EVENT_CONNECTION = 0,
    GFPS_EVENT_PROFILE,
    GFPS_EVENT_FROM_RFCOMM_SEEKER,
    GFPS_EVENT_FROM_L2CAP_SEEKER,
} GFPS_EVENT_TYPE_E;

typedef enum  {
    GFPS_EVENT_LINK_CONNECTED,
    GFPS_EVENT_LINK_DISCONNECTED,
    GFPS_EVENT_LINK_DESTORY,
    GFPS_EVENT_INVALID,
} GFPS_EVENT_CONNECTION_TYPE_E;

typedef enum
{
    GFPS_PROFILE_A2DP = 1,
    GFPS_PROFILE_AVRCP,
    GFPS_PROFILE_HFP,
    GFPS_PROFILE_LEA,
    GFPS_PROFILE_INVALID = 0xFF,
} GFPS_PROFILE_ID_E;

typedef struct {
    GFPS_EVENT_CONNECTION_TYPE_E     event;
    uint8_t                          reason;
    bt_bdaddr_t                      addr;
} GFPS_CONNECTION_EVENT;

typedef struct {
    GFPS_PROFILE_ID_E     pro;
    bt_bdaddr_t           btAddr;
    uint8_t               btEvt;
    uint16_t              len;
    uint8_t               param[FP_MSG_MAX_LEN];
} GFPS_SASS_PROFILE_EVENT;

typedef struct {
    GFPS_EVENT_CONNECTION_TYPE_E     event;
    uint8_t                          reason;
    bt_bdaddr_t                      addr;
} GFPS_SRV_EVENT;

typedef struct
{
    uint8_t *pBuf;
    uint16_t len;
} GFPS_PACKET_T;

typedef struct
{
    uint8_t pBuf[FP_MESSAGE_STREAM_MAX_ADDITIONAL_DATA_LEN];
    uint16_t len;
} GFPS_L2CAP_PACKET_T;

typedef struct
{
    uint8_t event;
    union{
        bt_bdaddr_t      addr;
        GFPS_PACKET_T    data;
    }p;
}GFPS_SRV_EVENT_RFCOM_PARAM_T;

typedef struct
{
    uint8_t event;
    union{
        bt_bdaddr_t      addr;
        GFPS_L2CAP_PACKET_T    data;
    }p;
}GFPS_SRV_EVENT_L2CAP_PARAM_T;

typedef struct {
    uint8_t     devId;
    uint8_t     event;
    uint16_t    len;
    union
    {
        GFPS_SASS_PROFILE_EVENT proParam;
        GFPS_CONNECTION_EVENT   conParam;
        GFPS_SRV_EVENT_RFCOM_PARAM_T  srvRfParam;
        GFPS_SRV_EVENT_L2CAP_PARAM_T  srvl2capParam;
    } p;
} GFPS_MESSAGE_BLOCK;

typedef void (*gfps_enter_pairing_mode_cb)(void);
typedef uint8_t (*gfps_bt_io_cap_set_cb)(uint8_t mode);
typedef uint8_t (*gfps_bt_io_authrequirements_set_cb)(uint8_t authrequirements);

typedef void (*gfps_get_battery_info_cb)(uint8_t* batteryValueCount, uint8_t* batteryValue);

typedef struct {
  uint32_t model_id;
  uint8_t public_anti_spoofing_key[64];
  uint8_t private_anti_spoofing_key[32];
} GFPS_KEY_INFO_T;

typedef struct {
  bt_bdaddr_t devAddr;
  uint8_t state;
} GFPS_STREAMING_STATE_T;

typedef struct
{
    uint8_t    isBatteryInfoIncluded;
    uint8_t    bt_iocap;
    uint8_t    bt_authrequirements;
    uint8_t    isLastResponsePending;
    uint8_t    pendingLastResponse[16];
    bool       isFastPairMode;
    bool       findPeer;
    uint8_t    ringMode;
    uint8_t    findState;
    GFPS_KEY_INFO_T                    keyInfo;
    FpCapabilitiesEnv_t                fpCap;
    gfps_enter_pairing_mode_cb         enterPairingMode;
    gfps_bt_io_cap_set_cb              btSetIocap;
    gfps_bt_io_authrequirements_set_cb btSetAuthrequirements;
    gfps_get_battery_info_cb           getBatteryHandler;
    GFPS_BATTERY_DATA_TYPE_E           batteryDataType;
} GFPSEnv_t;

typedef struct
{
    uint8_t messageGroup;
    uint8_t messageCode;
    uint8_t dataLenHighByte;
    uint8_t dataLenLowByte;
    uint8_t data[FP_MESSAGE_STREAM_MAX_ADDITIONAL_DATA_LEN];
}  __attribute__((packed)) FP_MESSAGE_STREAM_T;


#ifdef __cplusplus
extern "C" {
#endif

void gfps_init(void);

void gfps_disconnect(uint8_t devId);

void gfps_link_connect_handler(uint8_t devId, const bt_bdaddr_t *addr);

void gfps_link_disconnect_handler(uint8_t devId, const bt_bdaddr_t *addr, uint8_t errCode);

void gfps_link_destory_handler(void);

uint32_t gfps_get_model_id(void);

void gfps_tws_sync_init(void);

void gfps_role_switch_prepare(void);

void gfps_sync_info(void);

void gfps_send_battery_levels(uint8_t devId);

void gfps_send_ble_addr(uint8_t devId);

void gfps_set_multi_status(const bt_bdaddr_t *addr, bool enable_multipoint);

void gfps_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void gfps_enter_connectable_mode_req_handler(uint8_t *response);

void gfps_set_battery_datatype(GFPS_BATTERY_DATA_TYPE_E batteryDataType);

GFPS_BATTERY_DATA_TYPE_E gfps_get_battery_datatype(void);

void gfps_get_battery_levels(uint8_t *pCount, uint8_t *pBatteryLevel);

void gfps_reg_battery_handler(gfps_get_battery_info_cb cb);

bool gfps_is_battery_enabled(void);

uint8_t gfps_send(uint8_t devId, uint8_t *ptrData, uint32_t length);

void gfps_send_msg_ack(uint8_t devId, uint8_t msgGroup, uint8_t msgCode);

void gfps_send_msg_nak(uint8_t devId, uint8_t reason, uint8_t msgGroup, uint8_t msgCode);

bool gfps_is_last_response_pending(void);

uint8_t *gfps_get_last_response(void);

void gfps_ntf_ble_bond_over_bt(uint8_t devId, bool bond);

void gfps_enter_fastpairing_mode(void);

bool gfps_is_in_fastpairing_mode(void);

void gfps_set_in_fastpairing_mode_flag(bool isEnabled);

void gfps_exit_fastpairing_mode(void);

int gfps_mailbox_put(uint8_t devId, uint8_t event, uint8_t *param, uint16_t len);

uint16_t gfps_data_handler(uint8_t devId, uint8_t* ptr, uint16_t len);

void gfps_start_find_ringtone(void);

void gfps_stop_find_ringtone(void);

void app_tws_send_fastpair_info_to_slave(void);

void app_ibrt_share_fastpair_info(uint8_t *p_buff, uint16_t length);

void app_ibrt_share_fastpair_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

uint16_t gfps_event_rfcomm_callback(uint8_t devId, GFPS_SRV_EVENT_RFCOM_PARAM_T *param);

uint16_t gfps_event_l2cap_callback(uint8_t devId, GFPS_SRV_EVENT_L2CAP_PARAM_T *param);

void gfps_lea_event_handler(void *param);

void gfps_send_ble_disconnect_cmd_to_slave(uint8_t *buf, uint16_t length);

void gfps_send_streaming_state_to_master(bt_bdaddr_t *addr, uint8_t state);

void gfps_rec_peer_streaming_state_handler(uint8_t *p_buff, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif

#endif  // __GFPS_H__
