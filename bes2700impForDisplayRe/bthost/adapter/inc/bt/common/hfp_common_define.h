/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __HFP_COMMON_DEFINE_H__
#define __HFP_COMMON_DEFINE_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HFP_VOL    (15)
#define MIN_HFP_VOL    (0)
#define HFP_DEFAULT_VOLUME (7)

typedef enum {
    BT_HFP_CHAN_STATE_CLOSED = 0,
    BT_HFP_CHAN_STATE_CONN_PENDING = 1,
    BT_HFP_CHAN_STATE_CONN_INCOMING = 2,
    BT_HFP_CHAN_STATE_NEGOTIATE = 3,
    BT_HFP_CHAN_STATE_OPEN = 4,
    BT_HFP_CHAN_STATE_DISC = 5,
} bt_hfp_chan_state_t;

typedef enum {
    BT_HFP_AUDIO_DISCON = 0,
    BT_HFP_AUDIO_CON = 1,
} bt_audio_state_t;

typedef uint8_t bt_hfp_call_active_t;
typedef uint8_t bt_hfp_call_setup_t;
typedef uint8_t bt_hfp_call_held_t;

#define BT_HFP_CALL_NONE    0
#define BT_HFP_CALL_ACTIVE  1

#define BT_HFP_CALL_SETUP_NONE  0
#define BT_HFP_CALL_SETUP_IN    1
#define BT_HFP_CALL_SETUP_OUT   2
#define BT_HFP_CALL_SETUP_ALERT 3

#define BT_HFP_CALL_HELD_NONE       0
#define BT_HFP_CALL_HELD_ACTIVE     1
#define BT_HFP_CALL_HELD_NO_ACTIVE  2


#define HFP_KEY_ANSWER_CALL             8
#define HFP_KEY_HANGUP_CALL             9
#define HFP_KEY_REDIAL_LAST_CALL        10
#define HFP_KEY_CHANGE_TO_PHONE         11
#define HFP_KEY_ADD_TO_EARPHONE         12
#define HFP_KEY_MUTE                    13
#define HFP_KEY_CLEAR_MUTE              14
//3way calls oper
#define HFP_KEY_THREEWAY_HOLD_AND_ANSWER              15
#define HFP_KEY_THREEWAY_HANGUP_AND_ANSWER            16
#define HFP_KEY_THREEWAY_HOLD_REL_INCOMING            17
#define HFP_KEY_THREEWAY_HOLD_ADD_HELD_CALL           18

#define HFP_KEY_DUAL_HF_HANGUP_ANOTHER                19
#define HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER    20
#define HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER      21
#define HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_ANOTHER  22
#define HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE 23
#define HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE 24
#define HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR 25

typedef enum {
    BT_HFP_SCO_CODEC_NONE = 0,
    BT_HFP_SCO_CODEC_CVSD = 1,
    BT_HFP_SCO_CODEC_MSBC = 2,
    BT_HFP_SCO_CODEC_LC3_SWB = 3,     // For LC3_SWB Codec
    BT_HFP_SCO_CODEC_XXXX = 16,    // Cunstomer Codec
} hfp_sco_codec_t;

typedef enum {
    BTIF_HF_HOLD_RELEASE_HELD_CALLS = 0,
    BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS = 1,
    BTIF_HF_HOLD_HOLD_ACTIVE_CALLS = 2,
    BTIF_HF_HOLD_ADD_HELD_CALL = 3,
    BTIF_HF_HOLD_CALL_TRANSFER = 4,
} btif_hf_hold_call_t;

typedef struct{
    uint8_t device_id;
    btif_hf_hold_call_t action;
} app_hfp_hold_call_t;

typedef enum app_hfp_hf_event {
    APP_HFP_HF_EVENT_SERVICE_CONNECTED = 0,
    APP_HFP_HF_EVENT_SERVICE_DISCONNECTED,
    APP_HFP_HF_EVENT_RING_IND,
    APP_HFP_HF_EVENT_CALLER_ID_IND,
    APP_HFP_HF_EVENT_AUDIO_CONNECTED,
    APP_HFP_HF_EVENT_AUDIO_DISCONNECTED,
} app_hfp_hf_event_t;

typedef struct { /* empty */ } btif_hf_channel_t; /* used to hold the pointer to struct _hshf_channel */

typedef struct app_hfp_hf_event_param_t {
    union {
        struct {
            int device_id;
            bt_bdaddr_t *addr;
            btif_hf_channel_t *channel;
        } service_connected;
        struct {
            int device_id;
            btif_hf_channel_t *channel;
        } service_disconnected;
        struct {
            int device_id;
            const char *number;
            btif_hf_channel_t *channel;
        } ring_ind;
        struct {
            int device_id;
            int number_len;
            const char *number;
            btif_hf_channel_t *channel;
        } caller_id_ind;
        struct {
            int device_id;
            btif_hf_channel_t *channel;
        } audio_connected;
        struct {
            int device_id;
            btif_hf_channel_t *channel;
        } audio_disconnected;
    } p;
} app_hfp_hf_event_param_t;

typedef int (*app_hfp_hf_callback_t)(app_hfp_hf_event_t event, app_hfp_hf_event_param_t *param);

typedef enum {
    BTIF_HF_EVENT_SERVICE_CONNECT_REQ = 1,
    BTIF_HF_EVENT_SERVICE_CONNECTED = 2,
    BTIF_HF_EVENT_SERVICE_DISCONNECTED = 3,
    BTIF_HF_EVENT_AUDIO_CONNECTED = 4,
    BTIF_HF_EVENT_AUDIO_DISCONNECTED = 5,
    BTIF_HF_EVENT_AUDIO_DATA = 6,
    BTIF_HF_EVENT_AUDIO_DATA_SENT = 7,
    BTIF_HF_EVENT_GATEWAY_FEATURES = 8,
    BTIF_HF_EVENT_GW_HOLD_FEATURES = 9,
    BTIF_HF_EVENT_CALL_STATE = 10,
    BTIF_HF_EVENT_CALLER_ID = 11,
    BTIF_HF_EVENT_CALL_LISTING_ENABLED = 12,
    BTIF_HF_EVENT_RESPONSE_HOLD_APPL = 13,
    BTIF_HF_EVENT_CALL_IND = 14,
    BTIF_HF_EVENT_CALLSETUP_IND = 15,
    BTIF_HF_EVENT_CALLHELD_IND = 16,
    BTIF_HF_EVENT_RING_IND = 17,
    BTIF_HF_EVENT_WAIT_NOTIFY = 18,
    BTIF_HF_EVENT_CALLER_ID_NOTIFY = 19,
    BTIF_HF_EVENT_CURRENT_CALL_STATE = 20,
    BTIF_HF_EVENT_RESPONSE_HOLD = 21,
    BTIF_HF_EVENT_SERVICE_IND = 22,
    BTIF_HF_EVENT_BATTERY_IND = 23,
    BTIF_HF_EVENT_SIGNAL_IND = 24,
    BTIF_HF_EVENT_ROAM_IND = 25,
    BTIF_HF_EVENT_SMS_IND = 26,
    BTIF_HF_EVENT_VOICE_REC_STATE = 27,
    BTIF_HF_EVENT_VOICE_TAG_NUMBER = 28,
    BTIF_HF_EVENT_SPEAKER_VOLUME = 29,
    BTIF_HF_EVENT_MIC_VOLUME = 30,
    BTIF_HF_EVENT_IN_BAND_RING = 31,
    BTIF_HF_EVENT_NETWORK_OPERATOR = 32,
    BTIF_HF_EVENT_SUBSCRIBER_NUMBER = 33,
    BTIF_HF_EVENT_NO_CARRIER = 34,
    BTIF_HF_EVENT_BUSY = 35,
    BTIF_HF_EVENT_NO_ANSWER = 36,
    BTIF_HF_EVENT_DELAYED = 37,
    BTIF_HF_EVENT_BLACKLISTED = 38,
    BTIF_HF_EVENT_PHONEBOOK_STORAGE = 39,
    BTIF_HF_EVENT_PHONEBOOK_INFO = 40,
    BTIF_HF_EVENT_PHONEBOOK_SIZE = 41,
    BTIF_HF_EVENT_PHONEBOOK_ENTRY = 42,
    BTIF_HF_EVENT_AT_RESULT_DATA = 43,
    BTIF_HF_EVENT_COMMAND_COMPLETE = 44,
    BTIF_HF_EVENT_SIRI_STATUS = 45,
    BTIF_HF_EVENT_READ_AG_INDICATORS_STATUS = 46,
    BTIF_HF_EVENT_BES_TEST = 47,
    BTIF_HF_EVENT_SELECT_CHANNEL = 48,
    BTIF_HF_EVENT_RFCOMM_CONNECTED = 49,
    BTIF_HF_EVENT_REMOTE_NOT_SUPPORT = 50,
    BTIF_HF_EVENT_AUDIO_MOCK_CONNECTED = 51,
    BTIF_HF_EVENT_AUDIO_MOCK_DISCONNECTED = 52,
    BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED = 53,
    BTIF_HF_EVENT_AUDIO_CON_FAIL = 54,
} hf_event_t;

struct hfp_context {
    uint16_t remote_dev_hcihandle;
    /*TODO:should remove the status */
    bt_status_t status;         /* Status of the callback event             */
    bt_hfp_chan_state_t state;
    bt_bdaddr_t remote_dev_bdaddr;
    const char *call_number;
    const char *sim_card_number;
    bt_hfp_call_active_t call;
    bt_hfp_call_setup_t call_setup;
    bt_hfp_call_held_t call_held;
    hf_event_t event;
    uint8_t *audio_data;
    uint16_t audio_data_len;
    uint8_t error_code;
    uint8_t disc_reason;
    uint8_t disc_reason_saved;
    uint8_t speaker_volume;
    uint8_t mic_volume;
    uint8_t bsir_enable;
    uint8_t voice_rec_state;
    uint8_t ciev_status;
    uint32_t **chan_sel_channel;
    void *chan_sel_remDev;
    char *ptr;
    uint8_t sco_codec;
    uint8_t sco_con_fail_reason;
};

typedef struct bt_hfp_state_t {
    bt_bdaddr_t remote;
    uint8_t hfp_is_connected;
    uint8_t sco_is_connected;
    uint8_t hfp_nego_codec;
    uint8_t call_is_active;
    bt_hfp_chan_state_t hfp_chan_state;
    bt_hfp_call_setup_t callsetup_state;
    bt_hfp_call_held_t callheld_state;
    uint8_t service_status;
    uint8_t signal_status;
    uint8_t roam_status;
    uint8_t battery_status;
    uint8_t spk_volume;
    uint8_t mic_volume;
    uint8_t in_band_ring_enable;
    uint8_t voice_rec_state;
} bt_hfp_state_t;

typedef enum {
    BT_HFP_ANSWER_CALL = 1,
    BT_HFP_HANGUP_CALL,
    BT_HFP_REDIAL_CALL,
} BT_HFP_CALL_ACTION_T;

typedef void (*hfp_vol_sync_done_cb)(void);

#ifdef IBRT
typedef struct ibrt_hfp_status_t
{
    uint8_t audio_state;
    uint8_t localVolume;
    uint8_t sync_ctx;
    bt_bdaddr_t mobile_addr;
    uint8_t callsetup_state;
    uint8_t call_state;
    uint8_t callhold_state;
} __attribute__((packed)) ibrt_hfp_status_t;
#endif

// Maintain consistency with MAX_NUMBER_LEN
#define MAX_CARD_NUMBER_LEN               (33)
struct hfp_sim_card_number {
    char number[MAX_CARD_NUMBER_LEN];
};

#ifdef __cplusplus
}
#endif
#endif /* __HFP_COMMON_DEFINE_H__ */
