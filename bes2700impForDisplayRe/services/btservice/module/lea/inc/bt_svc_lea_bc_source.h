/**
 * @brief Bluetooth Service Low Energy Audio Unicast Client
 *
 * @copyright Copyright (c) 2015-20223 BES Technic.
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
 */
#ifndef __BT_SVC_LEA_BC_SOURCE_H__
#define __BT_SVC_LEA_BC_SOURCE_H__

/*****************************header include********************************/

/******************************macro defination*****************************/

/******************************type defination******************************/
enum VOLUME_SOURCE
{
    APP_BIS_LOCAL_SOURCE,
    APP_BIS_REMOTE_SOURCE,
};

typedef enum
{
    BT_SVC_LEA_BC_SRC_CODEC_OK,
    BT_SVC_LEA_BC_SRC_CODEC_ERROR,
} BT_SVC_LEA_BC_SRC_CODEC_RETURN_E;

typedef enum
{
    BT_SVC_LEA_BC_SRC_ENCODER,
    BT_SVC_LEA_BC_SRC_DECODER,
} BT_SVC_LEA_BC_SRC_CODEC_TYPE_E;

typedef enum
{
    BT_SVC_LEA_BC_SRC_STREAM_0 = 0,
    BT_SVC_LEA_BC_SRC_STREAM_1,
    BT_SVC_LEA_BC_SRC_STREAM_2,
    BT_SVC_LEA_BC_SRC_STREAM_3,
    BT_SVC_LEA_BC_SRC_STREAM_4,
    BT_SVC_LEA_BC_SRC_STREAM_MAX,
} BT_SVC_LEA_BC_SRC_AUDIO_STREAM_IDX_E;

typedef struct
{
    uint8_t  codec_type:1;
    uint8_t  pcm_interlaced:1;
    uint8_t  bits_depth;
    uint8_t  channel;
    uint32_t sample_rate;
    uint32_t frame_duration_ms;
    uint32_t frame_size;
} bt_svc_lea_bc_src_codec_param_t;

typedef struct
{
    uint8_t   pack_num;
    uint16_t  pack_size;
    void      *list_info;
}  bt_svc_lea_bc_src_buff_list_t;

typedef struct
{
    // pcm sampling param
    bool     external_trig;
    uint8_t  bits_depth;
    uint8_t  channel_num;
    uint16_t volume_level;
    uint32_t sample_rate;
    uint32_t duration_ms;
    void     (*triggered_cb)(void);
    uint32_t (*play_read_data_cb)(uint8_t *data, uint32_t data_len);
    uint32_t (*play_prompt_data_cb)(uint8_t *data, uint32_t data_len);
} bt_svc_lea_bc_src_media_param_t;
/****************************function declearation**************************/

//***************************
// input source
//***************************
void bt_svc_lea_bc_src_bt_src_init(uint8_t mode);

void bt_svc_lea_bc_src_bt_src_deinit(void);

void bt_svc_lea_bc_src_usb_src_init(uint8_t mode);

void bt_svc_lea_bc_src_usb_src_deinit();

void bt_svc_lea_bc_src_wifi_src_init(uint8_t mode);

void bt_svc_lea_bc_src_wifi_src_deinit();

void bt_svc_lea_bc_src_linein_src_init(uint8_t mode);

void bt_svc_lea_bc_src_linein_src_deinit();

uint8_t bt_svc_lea_bc_src_linein_stream_start();

uint8_t bt_svc_lea_bc_src_linein_stream_stop();

uint8_t bt_svc_lea_bc_src_smf_src_stream_start();

uint8_t bt_svc_lea_bc_src_smf_src_stream_stop();

void bt_svc_lea_bc_src_smf_src_stream_init(uint8_t mode);

void bt_svc_lea_bc_src_smf_src_stream_deinit();

void bt_svc_lea_bc_src_bt_src_output_stop(void);

void bt_svc_lea_bc_src_start_bt_src(void);

void bt_svc_lea_bc_src_stop_bt_src(void);

//***************************
// codec
//***************************
void *bt_svc_lea_bc_src_codec_creat(bt_svc_lea_bc_src_codec_param_t *param);

bool bt_svc_lea_bc_src_codec_delete(void *codec);

int bt_svc_lea_bc_src_codec_decode(void *codec, uint8_t *input, uint32_t input_len,
                                                    uint8_t *output, uint32_t *output_len);

int bt_svc_lea_bc_src_codec_encode(void *codec, uint8_t *input, uint32_t input_len,
                                                    uint8_t *output, uint32_t *output_len);

//***************************
// buffer
//***************************
void bt_svc_lea_bc_src_buffer_init();

void bt_svc_lea_bc_src_buffer_deinit();

void bt_svc_lea_bc_src_buffer_free(void *ptr);

uint8_t *bt_svc_lea_bc_src_buffer_malloc(uint32_t size);

void bt_svc_lea_bc_src_buf_list_creat(bt_svc_lea_bc_src_buff_list_t *list);

void bt_svc_lea_bc_src_buf_list_destroy(    bt_svc_lea_bc_src_buff_list_t      *list);

uint8_t *bt_svc_lea_bc_src_buf_list_get_data_packet(bt_svc_lea_bc_src_buff_list_t *list, uint16_t *data_len);

uint8_t *bt_svc_lea_bc_src_buf_list_get_free_packet(bt_svc_lea_bc_src_buff_list_t *list);

void bt_svc_lea_bc_src_buf_list_push_data_packet(bt_svc_lea_bc_src_buff_list_t *list, uint8_t *data, uint16_t data_len);

void bt_svc_lea_bc_src_buf_list_push_free_packet(bt_svc_lea_bc_src_buff_list_t *list, uint8_t* data);

uint8_t bt_svc_lea_bc_src_buf_list_get_data_pkt_num(bt_svc_lea_bc_src_buff_list_t *list);

//***************************
// media
//***************************
void bt_svc_lea_bc_src_media_start(bt_svc_lea_bc_src_media_param_t *audio_info);

void bt_svc_lea_bc_src_media_stop();

void bt_svc_lea_bc_src_media_audio_trigger(uint32_t expected_play_time);

void bt_svc_lea_bc_src_media_audio_set_volume(bool source, uint8_t volume);

void bt_svc_lea_bc_src_media_audio_volume_clean(void);

uint8_t bt_svc_lea_bc_src_media_audio_get_volume(void);

uint32_t bt_svc_lea_bc_src_media_audio_get_start_play_time();

uint8_t bt_svc_lea_bc_src_media_player_play(void (start_cb)(uint8_t device_id, uint32_t context_type),
        void (stop_cb)(uint8_t device_id, uint32_t context_type));

uint8_t bt_svc_lea_bc_src_media_player_stop(void);

//***************************
// prompt
//***************************
uint8_t bt_svc_lea_bc_src_prompt_play(uint8_t id);

uint32_t bt_svc_lea_bc_src_prompt_data_get(uint8_t *buf, uint32_t buf_len);


#endif
