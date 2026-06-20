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
#ifndef _A2DP_API_H
#define _A2DP_API_H
#include "bluetooth.h"
#include "avrcp_api.h"
#include "conmgr_api.h"
#include "a2dp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef BT_A2DP_SUPPORT

#define BTIF_AVTP_MSG_TYPE_COMMAND       0
#define BTIF_AVTP_MSG_TYPE_ACCEPT        2
#define BTIF_AVTP_MSG_TYPE_REJECT        3

#define A2DP_AAC_OCTET_NUMBER                     (6)
#define A2DP_AAC_OCTET0_MPEG2_AAC_LC              0x80
#define A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100  0x01
#define A2DP_AAC_OCTET2_CHANNELS_1                0x08
#define A2DP_AAC_OCTET2_CHANNELS_2                0x04
#define A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000  0x80
#define A2DP_AAC_OCTET3_VBR_SUPPORTED             0x80

#define BTIF_AVDTP_SIG_DISCOVER             0x01
#define BTIF_AVDTP_SIG_GET_CAPABILITIES     0x02
#define BTIF_AVDTP_SIG_SET_CONFIG           0x03
#define BTIF_AVDTP_SIG_GET_CONFIG           0x04
#define BTIF_AVDTP_SIG_RECONFIG             0x05
#define BTIF_AVDTP_SIG_OPEN                 0x06
#define BTIF_AVDTP_SIG_START                0x07
#define BTIF_AVDTP_SIG_CLOSE                0x08
#define BTIF_AVDTP_SIG_SUSPEND              0x09
#define BTIF_AVDTP_SIG_ABORT                0x0A
#define BTIF_AVDTP_SIG_SECURITY_CTRL        0x0B
#define BTIF_AVDTP_SIG_GET_ALL_CAPABILITIES 0x0C
#define BTIF_AVDTP_SIG_DELAYREPORT          0x0D

#ifndef  avdtp_codec_t
#define   avdtp_codec_t void
#endif
#ifndef avdtp_channel_t
#define   avdtp_channel_t void
#endif

#define BTIF_AVDTP_CP_TYPE_DTCP      0x0001

#define BTIF_AVDTP_CP_TYPE_SCMS_T    0x0002

#define BTIF_AVDTP_SRV_CAT_MEDIA_TRANSPORT      0x01
#define BTIF_AVDTP_SRV_CAT_REPORTING            0x02
#define BTIF_AVDTP_SRV_CAT_RECOVERY             0x03
#define BTIF_AVDTP_SRV_CAT_CONTENT_PROTECTION   0x04
#define BTIF_AVDTP_SRV_CAT_HEADER_COMPRESSION   0x05
#define BTIF_AVDTP_SRV_CAT_MULTIPLEXING         0x06
#define BTIF_AVDTP_SRV_CAT_MEDIA_CODEC          0x07
#define BTIF_AVDTP_SRV_CAT_DELAY_REPORTING      0x08

typedef U8 btif_avdtp_error_t;

#define BTIF_AVDTP_ERR_NO_ERROR                    0x00

#define BTIF_AVDTP_ERR_BAD_HEADER_FORMAT           0x01

#define BTIF_AVDTP_ERR_BAD_LENGTH                  0x11

#define BTIF_AVDTP_ERR_BAD_ACP_SEID                0x12

#define BTIF_AVDTP_ERR_IN_USE                      0x13

#define BTIF_AVDTP_ERR_NOT_IN_USE                  0x14

#define BTIF_AVDTP_ERR_BAD_SERV_CATEGORY           0x17

#define BTIF_AVDTP_ERR_BAD_PAYLOAD_FORMAT          0x18

#define BTIF_AVDTP_ERR_NOT_SUPPORTED_COMMAND       0x19

#define BTIF_AVDTP_ERR_INVALID_CAPABILITIES        0x1A

#define BTIF_AVDTP_ERR_BAD_RECOVERY_TYPE           0x22

#define BTIF_AVDTP_ERR_BAD_MEDIA_TRANSPORT_FORMAT  0x23

#define BTIF_AVDTP_ERR_BAD_RECOVERY_FORMAT         0x25

#define BTIF_AVDTP_ERR_BAD_ROHC_FORMAT             0x26

#define BTIF_AVDTP_ERR_BAD_CP_FORMAT               0x27

#define BTIF_AVDTP_ERR_BAD_MULTIPLEXING_FORMAT     0x28

#define BTIF_AVDTP_ERR_UNSUPPORTED_CONFIGURATION   0x29

#define BTIF_AVDTP_ERR_BAD_STATE                   0x31

#define BTIF_AVDTP_ERR_NOT_SUPPORTED_CODEC_TYPE    0xC2

#define BTIF_AVDTP_ERR_UNKNOWN_ERROR               0xFF

typedef struct btif_avdtp_capability_t {
    btif_avdtp_capability_type_t type;
    union {
        btif_avdtp_codec_t codec;
        btif_avdtp_content_prot_t cp;
    } p;
} btif_avdtp_capability_t;

typedef uint8_t btif_avdtp_streamId_t;
typedef uint8_t btif_avdtp_media_type;
typedef uint8_t btif_avdtp_strm_endpoint_type_t;

typedef struct btif_avdtp_stream_info_t {
    btif_avdtp_streamId_t id;
    bool inUse;
    btif_avdtp_media_type mediaType;
    btif_avdtp_strm_endpoint_type_t streamType;
} btif_avdtp_stream_info_t;

typedef struct {
    U8 bitPool;
    uint8_t sampleFreq;
    uint8_t channelMode;
    uint8_t allocMethod;
    U8 numBlocks;
    U8 numSubBands;
    U8 numChannels;
    U8 mSbcFlag;
} btif_sbc_stream_info_short_t;

typedef enum
{
    CODEC_ACT_MODIFY_SBC_BITPOOL    = 0,
    CODEC_ACT_EN_OR_DISABLE_CODEC   = 1,
    CODEC_ACT_NUM,
} codec_action_e;

typedef struct _custom_act_param {
    uint8_t device_id;      // device to be operated (0xFF: all device will be operated)
    codec_action_e action;
    union
    {
        struct
        {
            // valid value: BT_A2DP_CODEC_TYPE_SBC, BT_A2DP_CODEC_TYPE_MPEG2_4_AAC, BT_A2DP_CODEC_TYPE_NON_A2DP
            uint8_t codec_type;
            // when codec_type is BT_A2DP_CODEC_TYPE_NON_A2DP, this filed is valid,
            // valid value: A2DP_NON_CODEC_TYPE_LHDC, A2DP_NON_CODEC_TYPE_LHDCV5
            uint8_t sub_codec_type;
            // true: discoverable, false: discoverable
            bool enable;
        }enable_codec;
        struct
        {
            uint8_t min_bitpool;    // valid value: 2-250;  0xFF: dont care;  default value:A2D_SBC_IE_MIN_BITPOOL  (2)
            uint8_t max_bitpool;    // valid value: 2-250;  0xFF: dont care;  default value:A2D_SBC_IE_MAX_BITPOOL  (250)
        } sbc_bp;
    } p;
}codec_act_param_t;

typedef void btif_avdtp_stream_t;

typedef uint8_t btif_a2dp_error_t;

#define BTIF_A2DP_ERR_NO_ERROR                         0x00

#define BTIF_A2DP_ERR_BAD_SERVICE                      0x80

#define BTIF_A2DP_ERR_INSUFFICIENT_RESOURCE            0x81

#define BTIF_A2DP_ERR_INVALID_CODEC_TYPE               0xC1

#define BTIF_A2DP_ERR_NOT_SUPPORTED_CODEC_TYPE   AVDTP_ERR_NOT_SUPPORTED_CODEC_TYPE

#define BTIF_A2DP_ERR_INVALID_SAMPLING_FREQUENCY       0xC3

#define BTIF_A2DP_ERR_NOT_SUPPORTED_SAMP_FREQ          0xC4

/** Channel mode not valid
 *
 *  SBC
 *  MPEG-1,2 Audio
 *  ATRAC family
 */
#define BTIF_A2DP_ERR_INVALID_CHANNEL_MODE             0xC5

#define BTIF_A2DP_ERR_NOT_SUPPORTED_CHANNEL_MODE       0xC6

#define BTIF_A2DP_ERR_INVALID_SUBBANDS                 0xC7

#define BTIF_A2DP_ERR_NOT_SUPPORTED_SUBBANDS           0xC8

#define BTIF_A2DP_ERR_INVALID_ALLOCATION_METHOD        0xC9

#define BTIF_A2DP_ERR_NOT_SUPPORTED_ALLOC_METHOD       0xCA

#define BTIF_A2DP_ERR_INVALID_MIN_BITPOOL_VALUE        0xCB

#define BTIF_A2DP_ERR_NOT_SUPPORTED_MIN_BITPOOL_VALUE  0xCC

#define BTIF_A2DP_ERR_INVALID_MAX_BITPOOL_VALUE        0xCD

#define BTIF_A2DP_ERR_NOT_SUPPORTED_MAX_BITPOOL_VALUE  0xCE

#define BTIF_A2DP_ERR_INVALID_LAYER                    0xCF

#define BTIF_A2DP_ERR_NOT_SUPPORTED_LAYER              0xD0

#define BTIF_A2DP_ERR_NOT_SUPPORTED_CRC                0xD1

#define BTIF_A2DP_ERR_NOT_SUPPORTED_MPF                0xD2

#define BTIF_A2DP_ERR_NOT_SUPPORTED_VBR                0xD3

#define BTIF_A2DP_ERR_INVALID_BIT_RATE                 0xD4

#define BTIF_A2DP_ERR_NOT_SUPPORTED_BIT_RATE           0xD5

#define BTIF_A2DP_ERR_INVALID_OBJECT_TYPE              0xD6

#define BTIF_A2DP_ERR_NOT_SUPPORTED_OBJECT_TYPE        0xD7

#define BTIF_A2DP_ERR_INVALID_CHANNELS                 0xD8

#define BTIF_A2DP_ERR_NOT_SUPPORTED_CHANNELS           0xD9

#define A2DP_SCALABLE_OCTET_NUMBER (7)

#define BTIF_A2DP_ERR_INVALID_VERSION                  0xDA

#define BTIF_A2DP_ERR_NOT_SUPPORTED_VERSION            0xDB

#define BTIF_A2DP_ERR_NOT_SUPPORTED_MAXIMUM_SUL        0xDC

#define BTIF_A2DP_ERR_INVALID_BLOCK_LENGTH             0xDD

#define BTIF_A2DP_ERR_INVALID_CP_TYPE                  0xE0

#define BTIF_A2DP_ERR_INVALID_CP_FORMAT                0xE1

#define BTIF_A2DP_ERR_UNKNOWN_ERROR                    AVDTP_ERR_UNKNOWN_ERROR

typedef U16 btif_a22dp_version_t;

typedef U16 btif_a2dp_features_t;

/* Audio Player */
#define BTIF_A2DP_SRC_FEATURE_PLAYER    0x01

/* Microphone */
#define BTIF_A2DP_SRC_FEATURE_MIC       0x02

/* Tuner */
#define BTIF_A2DP_SRC_FEATURE_TUNER     0x04

/* Mixer */
#define BTIF_A2DP_SRC_FEATURE_MIXER     0x08

/* Headphones */
#define BTIF_A2DP_SNK_FEATURE_HEADPHONE 0x01

/* Loudspeaker */
#define BTIF_A2DP_SNK_FEATURE_SPEAKER   0x02

/* Audio Recorder */
#define BTIF_A2DP_SNK_FEATURE_RECORDER  0x04

/* Amplifier */
#define BTIF_A2DP_SNK_FEATURE_AMP       0x08

typedef U8 btif_a2dp_endpoint_type_t;

/* The stream is a source */
#define BTIF_A2DP_STREAM_TYPE_SOURCE  0

/* The stream is a sink */
#define BTIF_A2DP_STREAM_TYPE_SINK    1

typedef void (*btif_a2dp_callback)(uint8_t device_id, a2dp_stream_t * Stream, const a2dp_callback_parms_t * Info);


typedef void btif_av_device_t;

struct btif_get_codec_cap_t
{
    uint8_t ** cap;
    uint16_t * cap_len;
    bool     done;
};

typedef struct {
    list_entry_t node;          /* Used internally by A2DP. */
    btif_avdtp_stream_info_t info;  /* Stream information */
} btif_a2dp_streamInfo_t;

void btif_a2dp_init(btif_a2dp_callback cb, btif_a2dp_callback source_cb);
btif_a2dp_stream_t *btif_get_a2dp_stream(a2dp_stream_t *stream);
btif_a2dp_stream_t *btif_a2dp_get_stream(int *count);
struct avdtp_local_sep *btif_a2dp_get_local_seps(int *count);

bt_status_t btif_a2dp_close_stream_for_PTS(a2dp_stream_t *Stream);

btif_a2dp_stream_t *btif_a2dp_alloc_sink_stream(void);

btif_a2dp_stream_t *btif_a2dp_alloc_source_stream(void);

uint16_t btif_avdtp_parse_mediaHeader(btif_media_header_t * header,
                                      btif_a2dp_callback_parms_t * Info, uint8_t avdtp_cp);

uint16_t btif_a2dp_stream_get_media_mtu(a2dp_stream_t *stream);

void a2dp_set_config_codec(btif_avdtp_codec_t * config_codec,
                           const btif_a2dp_callback_parms_t * Info);

void btif_a2dp_stream_init(btif_a2dp_stream_t *Stream, btif_a2dp_endpoint_type_t stream_type);

void btif_a2dp_disable_aac_codec(uint32_t disable);

void btif_a2dp_disable_sbc_codec(uint32_t disable);

void btif_a2dp_set_codec_parameters(codec_act_param_t *codec_act);

void btif_a2dp_disable_vendor_codec(uint32_t disable);

bt_status_t btif_a2dp_register(btif_a2dp_stream_t *Stream,
                               btif_a2dp_endpoint_type_t sep_type,
                               btif_avdtp_codec_t *sep_codec,
                               btif_avdtp_content_prot_t *sep_cp,
                               uint8_t sep_priority,
                               btif_a2dp_callback Callback);

bt_status_t btif_a2dp_deregister(btif_a2dp_stream_t *Stream, uint8_t codec_type);

void btif_a2dp_set_copy_protection_enable(a2dp_stream_t *stream, bool enable);

btif_remote_device_t *btif_a2dp_get_remote_device(a2dp_stream_t * stream);

uint8_t *btif_a2dp_get_stream_devic_cmgrHandler_remdev_bdAddr(a2dp_stream_t * Stream);

void *btif_a2dp_get_stream_device(a2dp_stream_t * Stream);

void *btif_a2dp_get_stream_devic_cmgrHandler_bt_handler(a2dp_stream_t * Stream);

void *btif_a2dp_get_stream_devic_cmgrHandler_remdev(a2dp_stream_t * Stream);

uint8_t btif_a2dp_get_stream_devic_cmgrHandler_remdev_role(a2dp_stream_t * Stream);

btif_cmgr_handler_t *btif_a2dp_get_stream_devic_cmgrHandler(a2dp_stream_t * Stream);
bt_bdaddr_t *btif_a2dp_stream_conn_remDev_bdAddr(a2dp_stream_t * Stream);

uint8_t *btif_a2dp_get_remote_device_version(btif_remote_device_t * remDev);

btif_a2dp_event_t btif_a2dp_get_cb_event(a2dp_callback_parms_t * info);

bt_status_t btif_a2dp_set_sink_delay(int device_id, U16 delayMs);

bt_status_t btif_a2dp_set_stream_config(a2dp_stream_t * Stream,
                                        btif_avdtp_codec_t * Codec,
                                        btif_avdtp_content_prot_t * Cp);

bt_status_t btif_a2dp_open_stream(btif_avdtp_codec_t *prev_conn_codec, bt_bdaddr_t * Addr);

bt_status_t btif_a2dp_start_stream(a2dp_stream_t * Stream);

bt_status_t btif_a2dp_idle_stream(a2dp_stream_t * Stream);

bt_status_t btif_a2dp_suspend_stream(a2dp_stream_t * Stream);

bt_status_t btif_a2dp_start_stream_rsp(a2dp_stream_t * Stream, btif_a2dp_error_t error);

bt_status_t btif_a2dp_close_stream(a2dp_stream_t * Stream);

bt_status_t btif_a2dp_reconfig_stream_rsp(a2dp_stream_t * Stream,
                                          btif_a2dp_error_t Error,
                                          btif_avdtp_capability_type_t Type);

bt_status_t btif_a2dp_reconfig_stream(a2dp_stream_t * Stream,
                                      btif_avdtp_codec_t * codec_cfg,
                                      btif_avdtp_content_prot_t * cp);

void btif_a2dp_reconfig_codec_to_vendor_codec(a2dp_stream_t *Stream, uint8_t codec_id, uint8_t a2dp_non_type);

void btif_a2dp_reconfig_codec_to_aac(a2dp_stream_t *Stream);

void btif_a2dp_reconfig_codec_to_sbc(a2dp_stream_t *Stream);

void btif_a2dp_reconfig_codec(a2dp_stream_t *Stream, uint8_t code_type);

uint8_t btif_a2dp_security_control_req(a2dp_stream_t *stream, uint8_t *data, uint16_t len);
uint8_t btif_a2dp_security_control_rsp(a2dp_stream_t *stream,uint8_t* data,uint16_t len, uint8_t error);

bt_status_t btif_a2dp_open_stream_rsp(a2dp_stream_t * Stream,
                                      btif_a2dp_error_t Error,
                                      btif_avdtp_capability_type_t CapType);

bool btif_a2dp_stream_has_remote_device(btif_a2dp_stream_t * stream);

bt_bdaddr_t *btif_a2dp_stream_get_remote_bd_addr(btif_a2dp_stream_t * stream);

btif_a2dp_stream_t *btif_get_a2dp_stream(a2dp_stream_t * stream);

btif_a2dp_endpoint_type_t btif_a2dp_get_stream_type(a2dp_stream_t * Stream);

bt_a2dp_stream_state_t btif_a2dp_get_stream_state(a2dp_stream_t * Stream);

uint16_t btif_a2dp_get_stream_chnl_sigchnl_l2ChannelId(a2dp_stream_t * Stream);

void btif_a2dp_set_stream_state(a2dp_stream_t * Stream, bt_a2dp_stream_state_t state);

void btif_a2dp_set_stream_conn_l2ChannelId(a2dp_stream_t * Stream, uint16_t id);

void btif_a2dp_set_stream_chnl_conn_l2ChannelId(a2dp_stream_t * Stream, uint16_t id);

void btif_a2dp_set_stream_chnl_sigChnl_l2ChannelId(a2dp_stream_t * Stream, uint16_t id);

void btif_a2dp_set_stream_loc_streamId(a2dp_stream_t * Stream, uint8_t id);

void btif_a2dp_set_stream_remote_streamId(a2dp_stream_t * Stream, uint8_t id);

bool btif_a2dp_is_stream_device_has_delay_reporting(a2dp_stream_t * Stream);

btif_avdtp_codec_t *btif_a2dp_get_stream_codec(a2dp_stream_t * Stream);
btif_avdtp_codec_t *btif_a2dp_get_stream_codec_from_id(uint8_t device_id);

uint16_t btif_a2dp_get_stream_conn_remDev_hciHandle(a2dp_stream_t * Stream);

uint16_t btif_a2dp_get_stream_device_cmgrhandler_remDev_hciHandle(a2dp_stream_t * Stream);

uint8_t *btif_a2dp_get_stream_cp_info(a2dp_stream_t *Stream);

bt_status_t btif_a2dp_get_stream_capabilities(a2dp_stream_t * Stream);

bt_status_t btif_a2dp_stream_send_sbc_packet(a2dp_stream_t * stream,
                                             btif_a2dp_sbc_packet_t * Packet,
                                             btif_sbc_stream_info_short_t * StreamInfo);

bt_status_t btif_a2dp_stream_send_ldac_packet(a2dp_stream_t * stream,
                                             btif_a2dp_sbc_packet_t * Packet,
                                             btif_sbc_stream_info_short_t * StreamInfo);


bt_status_t btif_a2dp_stream_send_aac_packet(a2dp_stream_t *stream,
                                                        btif_a2dp_sbc_packet_t *Packet,
                                                        btif_sbc_stream_info_short_t *StreamInfo);
bt_status_t btif_a2dp_stream_send_lhdc_packet(a2dp_stream_t *stream,
        btif_a2dp_sbc_packet_t *Packet,
        btif_sbc_stream_info_short_t *StreamInfo);

bt_status_t btif_a2dp_stream_send_lhdcv5_packet(a2dp_stream_t *stream,
        btif_a2dp_sbc_packet_t *Packet,
        btif_sbc_stream_info_short_t *StreamInfo);

void btif_a2dp_sync_avdtp_streaming_state(bt_bdaddr_t *addr);

void btif_app_a2dp_source_init(void);

uint8_t btif_a2dp_get_cb_error(const btif_a2dp_callback_parms_t * Info);

uint8_t btif_a2dp_set_dst_stream(a2dp_callback_parms_t *Info, a2dp_stream_t *stream);

uint16_t btif_a2dp_get_stream_conn_l2ChannelId(a2dp_stream_t * Stream);

btif_media_header_t *btif_a2dp_get_stream_media_header(a2dp_stream_t * stream);

int tws_if_get_a2dpbuff_available(void);

bool btif_a2dp_is_disconnected(a2dp_stream_t *Stream);

uint8_t btif_a2dp_confirm_stream_state(a2dp_stream_t *Stream, uint8_t old_state, uint8_t new_state);

void btif_a2dp_accept_stream_request_command(bt_bdaddr_t* remote, uint8_t transaction, uint8_t signal_id);

btif_remote_device_t *btif_a2dp_get_remote_device_from_cbparms(a2dp_stream_t *Stream, const a2dp_callback_parms_t *info);

btif_avdtp_codec_type_t btif_a2dp_get_codec_type(const a2dp_callback_parms_t *info);

void btif_a2dp_register_multi_link_connect_not_allowed_callback(bool (*cb)(uint8_t device_id));

void btif_a2dp_set_codec_info_func(void (*func)(uint8_t dev_num, const uint8_t *codec));
void btif_a2dp_get_codec_info_func(void (*func)(uint8_t dev_num, uint8_t *codec));
void btif_a2dp_get_codec_non_type_func(uint8_t (*func)(uint8_t *elements));
bool btif_a2dp_is_profile_initiator(const bt_bdaddr_t* remote);
void btif_a2dp_set_codec_info(uint8_t dev_num, const uint8_t *codec);
void btif_a2dp_get_codec_info(uint8_t dev_num, uint8_t *codec);
uint8_t btif_a2dp_get_codec_non_type(uint8_t *elements);
bt_status_t btif_a2dp_send_signal_message(const bt_bdaddr_t *remote, bt_a2dp_signal_msg_header_t *header, const uint8_t *data, uint16_t len);

#if defined(IBRT)
uint32_t btif_a2dp_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_a2dp_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
uint8_t btif_a2dp_is_critical_avdtp_cmd_handling(void);
void btif_a2dp_critical_avdtp_cmd_timeout(void);
void btif_a2dp_force_disconnect_a2dp_profile(uint8_t device_id,uint8_t reason);
#endif /* IBRT */

/* Callout functions, do not call directly */
uint8_t a2dp_stream_confirm_stream_state(uint8_t index, uint8_t old_state, uint8_t new_state);
uint8_t a2dp_stream_locate_the_connected_dev_id(a2dp_stream_t *Stream);
void btif_a2dp_register_multi_link_close_req_allowed_callback(uint8 (*cb)(uint8_t device_id));

#endif /* BT_A2DP_SUPPORT */
#ifdef __cplusplus
}
#endif
#endif
