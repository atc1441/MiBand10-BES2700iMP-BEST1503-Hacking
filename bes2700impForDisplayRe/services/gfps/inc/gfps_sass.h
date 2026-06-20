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
/**
 ****************************************************************************************
 * @addtogroup GFPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#ifndef __GFPS_SASS_H__
#define __GFPS_SASS_H__

#include "bluetooth.h"
#include "me_api.h"

#define SASS_SECURE_ENHACEMENT

// there will be 3 connected devices during accept 3rd dev and disconnecting the former dev.
#if BLE_AUDIO_ENABLED
#define SASS_DEVICE_NUM      (3)
#else
#define SASS_DEVICE_NUM      (2)
#endif
#define SASS_MAX_DEVICE_NUM  (2)

#define COD_TYPE_LAPTOP      (BTIF_COD_MAJOR_COMPUTER | BTIF_COD_MINOR_COMP_LAPTOP)
#define COD_TYPE_TABLET      (BTIF_COD_MAJOR_COMPUTER | BTIF_COD_MINOR_COMP_TABLET)
#define COD_TYPE_PHONE       (BTIF_COD_TELEPHONY | BTIF_COD_MAJOR_PHONE)
#define COD_TYPE_TV          (BTIF_COD_MAJOR_IMAGING | BTIF_COD_MINOR_IMAGE_DISPLAY)

#define SASS_CONN_STATE_TYPE (0x05)
#ifdef SASS_SECURE_ENHACEMENT
#define SASS_VERSION         (0x0102)
#else
#define SASS_VERSION         (0x0101)
#endif

#define SASS_MSG_MAX_LEN     (16)
#define NTF_CAP_LEN          (4)

/**
 * @brief sass capability state
 *
 */
#define SASS_STATE_ON_BIT                 (1 << 15)
//1: if the device supports multipoint and it can be switched between on and off
//0: otherwise (does not support multipoint or multipoint is always on)
#define SASS_MULTIPOINT_BIT               (1 << 14)
//1: if multipoint on; 0: otherwise
#define SASS_MULTIPOINT_ON_BIT            (1 << 13)
//1: support on-head detection; 0: otherwise
#define SASS_ON_HEAD_BIT                  (1 << 12)
//1: on-head detection is turned on; 0: do not support on-head detection or is disabled
#define SASS_ON_HEAD_ON_BIT               (1 << 11)

/**
 * @brief sass switch preference, new profile request VS current active profile
 *
 */
#define SASS_A2DP_VS_A2DP_BIT             (1 << 7)  //default be 0
#define SASS_HFP_VS_HFP_BIT               (1 << 6)  //default be 0
#define SASS_A2DP_VS_HFP_BIT              (1 << 5)  //default be 0
#define SASS_HFP_VS_A2DP_BIT              (1 << 4)  //default be 1

/**
 * @brief sass switch audio source event type
 *
 */
#define SASS_SWITCH_TO_CURR_DEV_BIT       (1 << 7)
#define SASS_RESUME_ON_SWITCH_DEV_BIT     (1 << 6)
#define SASS_REJECT_SCO_ON_AWAY_DEV_BIT   (1 << 5)
#define SASS_DISCONN_ON_AWAY_DEV_BIT      (1 << 4)

#define HISTORY_DEV_NUM                   (2)
#define SASS_ACCOUNT_KEY_SIZE             (16)
#define SESSION_NOUNCE_NUM                (8)

/**
 * @brief The head state
 *
 */
#define SASS_STATE_BIT_HEAD_ON          (7)
#define SASS_STATE_BIT_CONN_AVAILABLE   (6)
#define SASS_STATE_BIT_FOCUS_MODE       (5)
#define SASS_STATE_BIT_AUTO_RECONN      (4)
#define SASS_STATE_BIT_CONN_STATE       (0xF)

#define SET_SASS_STATE(S, B, P)       (S = ((S & ~(1 << SASS_STATE_BIT_##B)) | (P << SASS_STATE_BIT_##B)))
#define SET_SASS_CONN_STATE(S, B, P)  (S = ((S & ~(SASS_STATE_BIT_##B)) | P))


/**
 * @brief The first byte of account key
 *
 */
#define SASS_NOT_IN_USE_ACCOUNT_KEY         (0x04)
#define SASS_RECENTLY_USED_ACCOUNT_KEY      (0x05)
#define SASS_IN_USE_ACCOUNT_KEY             (0x06)

#define NODE_BD_ADDR(n) ((bt_bdaddr_t *)((uint8_t *)n + sizeof(struct list_node)))

#define SASS_PROFILE_STATE_BIT_CONNECTION       (0) //bit0, 1: connected; 0: disconnected
#define SASS_PROFILE_STATE_BIT_AUDIO            (1) //bit1, 1: audio playing; 0: paused or stopped

#define SASS_PROFILE_A2DP_CONNECTION_POS    (0)
#define SASS_PROFILE_A2DP_CONNECTION_BIT    (0x0001)
#define SASS_PROFILE_A2DP_AUDIO_POS         (1)
#define SASS_PROFILE_A2DP_AUDIO_BIT         (0x0002)
#define SASS_PROFILE_AVRCP_CONNECTION_POS   (2)
#define SASS_PROFILE_AVRCP_CONNECTION_BIT   (0x0004)
#define SASS_PROFILE_AVRCP_AUDIO_POS        (3)
#define SASS_PROFILE_AVRCP_AUDIO_BIT        (0x0008)
#define SASS_PROFILE_HFP_CONNECTION_POS     (4)
#define SASS_PROFILE_HFP_CONNECTION_BIT     (0x0010)
#define SASS_PROFILE_HFP_AUDIO_POS          (5)
#define SASS_PROFILE_HFP_AUDIO_BIT          (0x0020)

#define SASS_PROFILE_LEA_CONNECTION_POS     (6)
#define SASS_PROFILE_LEA_CONNECTION_BIT     (0x0040)
#define SASS_PROFILE_LEA_GAME_POS           (7)
#define SASS_PROFILE_LEA_GAME_BIT           (0x0080)
#define SASS_PROFILE_LEA_MUSIC_POS          (8)
#define SASS_PROFILE_LEA_MUSIC_BIT          (0x0100)
#define SASS_PROFILE_LEA_CALL_POS           (9)
#define SASS_PROFILE_LEA_CALL_BIT           (0x0200)

#define SET_PROFILE_STATE(r, s, p, v)   (r = ((r & ~(SASS_PROFILE_## p ##_## s ##_BIT)) | ( v << SASS_PROFILE_## p ##_## s ##_POS)))
#define GET_PROFILE_STATE(r, s, p)      (( (r) & (SASS_PROFILE_## p ##_## s ##_BIT) ) >> (SASS_PROFILE_## p ##_## s ##_POS))

#define SASS_LINK_SWITCH_TO_SINGLE_POINT   (1)
#define SASS_LINK_SWITCH_TO_MULTI_POINT    (2)
#define SASS_CONNECT_COUNT                 (3)

/**
 * @brief Device ID type for Dual-Mode Devices
 *
 */
typedef enum
{
    SASS_DEV_NEW_LINK = 0,
    SASS_DEV_DUAL_LINK,
    SASS_DEV_DUPLICATED,
} SASS_DEV_LINK_TYPE_E;

/**
 * @brief switch back event type
 *
 */
typedef enum
{
    SASS_EVT_SWITCH_BACK = 1,
    SASS_EVT_SWITCH_BACK_AND_RESUME,
    SASS_EVT_INVALID = 0xFFFF,
} SASS_BACK_EVT_E;

/**
 * @brief active device
 *
 */
typedef enum
{
 //this seeker is passive, and the active device is using the same account key
    SASS_DEV_IS_PASSIVE = 0x00,
 //this seeker is avtive device
    SASS_DEV_IS_ACTIVE = 0x01,
 //this seeker is passive, and the active device is non-SASS seeker.
	SASS_DEV_IS_PSSIVE_WITH_NONSASS = 0x02,
} SASS_ACTIVE_DEV_E;

/**
 * @brief switch reason
 *
 */
typedef enum
{
    SASS_REASON_UNSPECIFIED = 0,
    SASS_REASON_A2DP,
    SASS_REASON_HFP,
    SASS_REASON_LEA_MUSIC,
    SASS_REASON_LEA_GAME,
    SASS_REASON_LEA_CALL,
} SASS_REASON_E;

/**
 * @brief switch device
 *
 */
typedef enum
{
    SASS_DEV_THIS_DEVICE = 0x01,
    SASS_DEV_ANOTHER,
} SASS_SWITCH_DEV_E;

/**
 * @brief device type of the connected seeker
 *
 */
typedef enum
{
    SASS_DEV_TYPE_LAPTOP = 1 << 7,
    SASS_DEV_TYPE_PHONEA = 1 << 6,
    SASS_DEV_TYPE_PHONEB = 1 << 5,
    SASS_DEV_TYPE_TABLET = 1 << 4,
    SASS_DEV_TYPE_TV     = 1 << 3,
    SASS_DEV_TYPE_INVALID = 0xFFFF,
} SASS_DEV_TYPE_E;

/**
 * @brief The connection or audio state of the provider
 *
 */
typedef enum
{
    SASS_STATE_NO_CONNECTION = 0,
    SASS_STATE_PAGING,
    SASS_STATE_NO_DATA,
    SASS_STATE_NO_AUDIO,
    SASS_STATE_ONLY_A2DP,
    SASS_STATE_A2DP_WITH_AVRCP,
    SASS_STATE_HFP,
    SASS_STATE_LE_AUD_MEDIA_WITHOUT_CTRL,
    SASS_STATE_LE_AUD_MEDIA_CTRL,
    SASS_STATE_LE_AUD_CALL,
    SASS_STATE_LE_AUD_BIS,
    SASS_STATE_DISABLE_SWITCH = 0xF,
} SASS_CONN_STATE_E;

typedef enum
{
    SASS_HEAD_STATE_OFF = 0,
    SASS_HEAD_STATE_ON,
} SASS_HEAD_STATE_E;

typedef enum
{
    SASS_CONN_NONE_AVAILABLE = 0,
    SASS_CONN_AVAILABLE,
} SASS_CONN_AVAIL_E;

typedef enum
{
    SASS_CONN_NO_FOCUS = 0,
    SASS_CONN_FOCUS_MODE,

} SASS_FOCUS_MODE_E;

typedef enum
{
    SASS_MANTUAL_CONNECTED = 0,
    SASS_AUTO_RECONNECTED,

} SASS_AUTO_RECONN_E;

/**
 * @brief return status
 *
 */
typedef enum
{
    SASS_STATUS_OK = 0,
    SASS_STATUS_REDUNTANT,
    SASS_STATUS_FAIL,
} SASS_STATUS_E;

typedef enum
{
    SASS_PROFILE_A2DP = 1,
    SASS_PROFILE_AVRCP,
    SASS_PROFILE_HFP,
    SASS_PROFILE_LEA,
    SASS_PROFILE_INVALID = 0xFF,
} SASS_PROFILE_ID_E;

typedef enum
{
    SASS_EVENT_LEA_STREAM_STATE = 1,
    SASS_EVENT_LEA_PLAYBACK_STATE,
    SASS_EVENT_LEA_CALL_STATE,
    SASS_EVENT_LEA_METADATA_UPDATA_STATE,
    SASS_EVENT_LEA_INVALID = 0xFF,
} SASS_EVENT_LEA_T;

typedef struct {
    uint8_t  conLid;
    uint8_t  aseLid;
    uint8_t  state;
} SassLeaParam;

typedef enum
{
    SASS_PROCESS_SWITCH_ACTIVE_SRC = 1,
    SASS_PROCESS_INVALID = 0xFF,
} SASS_PENDING_PROCESS_E;

typedef enum
{
    SASS_SWITCH_STATUS_PRO_EXCHANGED = 1 << 0,
    SASS_SWITCH_STATUS_REC_SWITCH    = 1 << 1,
    SASS_SWITCH_STATUS_STREAMING     = 1 << 2,
} SASS_SWITCH_STATUS_E;

typedef struct {
    SASS_PENDING_PROCESS_E   proc;
    uint8_t                  status;
    uint8_t                  activeId;
} SassPendingProc;

typedef struct {
    uint8_t                  connId;
    uint8_t                  secondId; //for bt+lea connection
    SASS_CONN_STATE_E        state;
    SASS_DEV_TYPE_E          devType;
    bool                     initbySass; //connection is triggeed by sass or not
    uint16_t                 audState;//bit0~3: a2dp; bit4~7:avrcp; bit8~11:hfp;
    bt_bdaddr_t              btAddr;
    uint8_t                  accKey[SASS_ACCOUNT_KEY_SIZE];
    uint8_t                  session[SESSION_NOUNCE_NUM];
    bool                     updated;
    bool                     ntfSassMode;
    bool                     isSass;
    bool                     needNtfStatus;
    bool                     needNtfSwitch;
    bool                     waitPauseDone;//avoid send avrcp pause many times.
    bool                     isNeedResume;
    bool                     isActived;
    bool                     isCallsetup;
    bool                     isCallInd;
    bool                     proExchanged;
} SassBtInfo;

typedef struct
{
    bt_bdaddr_t                 reconnAddr;
    uint8_t                     evt;
} SassReconnectInfo;

typedef struct
{
    bool        resume;    //resume or not after hanging the call.
    bool        acceptNew; //accept new or keep old media/game for non-SASS cases.
    uint8_t     resumeId;  //device id needs to resume after call.
    bool        isLastMusic; //is last dev on music or game
} SassSwitchConfig;

typedef struct {
    SassBtInfo             connInfo[SASS_DEVICE_NUM];
    uint8_t                preference;
    uint8_t                connNum;
    uint8_t                activeId;
    uint8_t                lastActId;
    uint8_t                hunId;
    uint8_t                lastConnId;
    bool                   isMulti;
    bt_bdaddr_t            inUseAddr;
    bt_bdaddr_t            lastDev;
    bt_bdaddr_t            dropDevAddr;
    bt_bdaddr_t            disconectingAddr;
    SassSwitchConfig       config;
    SassPendingProc        pending;
    SassReconnectInfo      reconnInfo;
    SASS_HEAD_STATE_E      headState;
    SASS_CONN_AVAIL_E      connAvail;
    SASS_FOCUS_MODE_E      focusMode;
    SASS_AUTO_RECONN_E     autoReconn;
    SASS_CONN_STATE_E      connState;
    uint8_t                inuseKey[SASS_ACCOUNT_KEY_SIZE];
    uint8_t                lastInuseKey[SASS_ACCOUNT_KEY_SIZE];
    bool                   IfSendSwitchNTF;
} SassConnInfo;

typedef struct {
    SassBtInfo             connInfo[SASS_DEVICE_NUM];
    uint8_t                activeId;
} SassSyncInfo;

/**
 * @brief The event for update adv data
 *
 */
typedef enum
{
    SASS_EVT_ADD_DEV = 0,
    SASS_EVT_DEL_DEV,
    SASS_EVT_UPDATE_CONN_STATE,
    SASS_EVT_UPDATE_HEAD_STATE,
    SASS_EVT_UPDATE_FOCUS_STATE,
    SASS_EVT_UPDATE_RECONN_STATE,
    SASS_EVT_UPDATE_CUSTOM_DATA,
    SASS_EVT_UPDATE_MULTI_STATE,
    SASS_EVT_UPDATE_ACTIVE_DEV,
    SASS_EVT_UPDATE_INUSE_ACCKEY,
} SASS_UPDATE_EVT_E;

typedef struct {
    SASS_UPDATE_EVT_E event;
    uint8_t           devId;
    uint8_t           reason;
    bt_bdaddr_t       addr;
    union
    {
        SASS_CONN_AVAIL_E       connAvail;
        SASS_CONN_STATE_E       connState;
        SASS_HEAD_STATE_E       headState;
        SASS_FOCUS_MODE_E       focusMode;
        SASS_AUTO_RECONN_E      autoReconn;
        uint8_t                 cusData;
    } state;
} SassEvtParam;

typedef struct {
/*0bLLLLTTTT: L = length of connection status in bytes, type = 0b0101*/
    uint8_t lenType;
/* Connection state:0bHAFRSSSS  
*  H = on head detection; A = connection availability;
*  F = focus mode; R = auto-connected; S = connection state
*/
    uint8_t state;
/* custom data is sent from the Seeker of current active streaming to the provider via message stream.
*  0 if the current active streaming is not from Seeker.
*/
    uint8_t cusData;
/* 1: device is connected to the provider,
*  0: otherwise
*/
    uint8_t devBitMap;
} SassStateInfo;

#ifdef __cplusplus
extern "C" {
#endif

void gfps_sass_init(void);
uint8_t gfps_sass_get_active_dev(void);
void gfps_sass_get_capability(uint8_t devId);
void gfps_sass_send_session_nonce(uint8_t devId);
SassBtInfo *gfps_sass_get_connected_dev(uint8_t id);
SassBtInfo *gfps_sass_get_connected_dev_by_addr(const bt_bdaddr_t *addr);
bool gfps_sass_get_inuse_acckey(uint8_t *accKey);
void gfps_sass_set_inuse_acckey(uint8_t *accKey, bt_bdaddr_t *addr);
void gfps_sass_profile_event_handler(uint8_t pro, uint8_t devId, bt_bdaddr_t *btAddr,                                                   uint8_t event, uint8_t *param);
void gfps_sass_set_head_state(SASS_HEAD_STATE_E headstate);
SASS_HEAD_STATE_E gfps_sass_get_head_state(void);
bool gfps_sass_is_adv_in_use_key(void);
bool gfps_sass_is_there_in_use_dev(void);
void gfps_sass_update_state(SassEvtParam *evtParam);
bool gfps_sass_get_inuse_acckey_by_id(uint8_t device_id, uint8_t *accKey);
void gfps_sass_connect_handler(uint8_t device_id, const bt_bdaddr_t *addr);
void gfps_sass_disconnect_handler(uint8_t device_id, const bt_bdaddr_t *addr, uint8_t errCode);
void gfps_sass_get_sync_info(uint8_t *buf, uint16_t *len);
void gfps_sass_set_sync_info(uint8_t *buf, uint16_t len);
void gfps_sass_set_multi_status(const bt_bdaddr_t *addr, bool isMulti);
void gfps_sass_handler(uint8_t devId, uint8_t evt, void *param);
void gfps_sass_role_switch_prepare(void);
void gfps_sass_update_head_state(SASS_HEAD_STATE_E state);
bool gfps_sass_ntf_conn_status(uint8_t devId, bool isUseAdv, uint8_t *state);
void gfps_sass_exe_pending_switch_media(uint8_t devId);
#ifdef SASS_SECURE_ENHACEMENT
void gfps_sass_encrypt_connection_state(uint8_t *iv,  uint8_t *inUseKey, 
                                            uint8_t *outputData, uint8_t *dataLen, bool LT, 
                                            bool isUseAdv, uint8_t *inputData);
#else
void gfps_sass_encrypt_adv_data(uint8_t *FArray, uint8_t sizeOfFilter, uint8_t *inUseKey, 
                                               uint8_t *outputData, uint8_t *dataLen,
                                               bool isUseAdv, uint8_t *inputData);
#endif
void gfps_sass_check_if_need_reconnect(uint8_t devId);
void gfps_sass_add_dev_handler(uint8_t devId, bt_bdaddr_t *addr);
void gfps_sass_set_peer_streaming_state(bt_bdaddr_t *addr, uint8_t state);
void gfps_sass_set_profile_exchanged(uint8_t devId, bool isExchanged);
void gfps_sass_send_info_to_seeker(uint8_t devId);

#ifdef __cplusplus
}
#endif

#endif  // __GFPS_SASS_H__
