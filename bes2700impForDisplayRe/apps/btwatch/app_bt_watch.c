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
#include "stdio.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "app_audio.h"
#include "app_utils.h"

#include "ecomm.h"

#include "eplatform_list.h"
#include "eplatform_log.h"
#include "comm_slave.h"

#include "evf.h"
#include "eplayer.h"
#include "file_like_stream.h"
#include "source_evf_file.h"

#include "eaudio.h"

#include "bt_service_server.h"
#include "bts_transport_ecomm.h"

#include "app_bt_watch.h"

//#define DUMP_STREAM

#define BT_PLAYER_CACHE_SIZE (8192)
#define SINK_TRACK_BUFF_SIZE (4096)

#if defined(DUMP_STREAM)
static unsigned int stream_last_value   = 0;
static unsigned int stream_curr_offset = 0;
static unsigned int stream_total_error = 0;
#endif

// ecomm
static ecomm_t ecomm;

// track
static eaudio_track_t *sink_track = NULL;
static eaudio_buffer_t sink_track_buffer;
static unsigned char sink_track_buff[SINK_TRACK_BUFF_SIZE];

// comm slave
static comm_slave_t comm_slave;
static comm_slave_t *g_comm_slave = &comm_slave;

// player
static eplayer_t player;
static evfile_t *player_file;
static ep_source_t player_source;
static ep_sink_t player_sink;
static ep_handle_t *player_handle;

// file stream
static file_like_stream_t player_file_stream;

static unsigned char stream_buff[BT_PLAYER_CACHE_SIZE];

// stream pack list
static eplatform_list_node_t _pack_list;

osMutexDef(_pack_list_lock_mutex);
osMutexId _pack_list_lock_mutex;

static void _pack_list_lock(void)
{
    osMutexWait(_pack_list_lock_mutex, osWaitForever);
}

static void _pack_list_unlock(void)
{
    osMutexRelease(_pack_list_lock_mutex);
}

static void _pack_list_lock_init(void)
{
    _pack_list_lock_mutex = osMutexCreate(osMutex(_pack_list_lock_mutex));
}

static void _pack_add_tail(ecomm_pack_t *pack)
{
    _pack_list_lock();
    eplatform_list_add_tail(&_pack_list, &pack->node);
    _pack_list_unlock();
}

static void _pack_list_remove_pack(ecomm_pack_t *pack)
{
    _pack_list_lock();
    eplatform_list_delete(&pack->node);
    _pack_list_unlock();
}

static ecomm_pack_t *_pack_get_next(void)
{
    ecomm_pack_t *pack = NULL;
    eplatform_list_node_t *node = NULL;
    _pack_list_lock();
    node = eplatform_list_get_head(&_pack_list);
    if (node != NULL)
        pack = eplatform_list_structure(node, ecomm_pack_t, node);
    _pack_list_unlock();

    return pack;
}

static int _stream_pack_list_process_next(comm_slave_t *comm_slave)
{
    unsigned int offset = 0;
    int do_free = 0, ret = 0;
    ecomm_pack_t *pack = NULL;
    file_like_stream_t *stream = &player_file_stream;

    _pack_list_lock();
    pack = _pack_get_next();
    if (pack != NULL) {
        offset = *((unsigned int *)pack->data);
        ret = file_like_stream_write(stream, pack->data+4, pack->data_len-4, offset);
        if (ret != -2) {
            _pack_list_remove_pack(pack);
            do_free = 1;
        }
    }
    _pack_list_unlock();

    if (pack != NULL && do_free) {
        comm_slave_free_stream_pack(comm_slave, pack);
    }
    return 0;
}

static int _ep_write_pcm(struct ep_sink *sink, unsigned char *pcm, unsigned int pcm_len_bytes)
{
    eplatform_logd("write_pcm:len=%d", pcm_len_bytes);

#if defined(DUMP_STREAM)
    {
        int i = 0, len = 0;
        unsigned int *buff = (unsigned int *)pcm;
        len = pcm_len_bytes/4;

        for (i = 0; i < len; ++i) {
            if (buff[i] != (stream_last_value+1)) {
                eplatform_logd("stream_dump_check:wanring,last_value=0x%x,now=0x%x,curr_offset=%d",
                    stream_last_value, buff[i], stream_curr_offset);
                stream_total_error++;
            }

            stream_curr_offset += sizeof(stream_last_value);
            stream_last_value = buff[i];
        }
    }
#endif
    if (sink_track) {
        eaudio_track_write(sink_track, pcm, pcm_len_bytes);
    }

    return 0;
}

static void eaudio_track_callback(eaudio_track_event_t event, eaudio_track_event_param_t *param)
{
    eplatform_logd("eaudio_track_callback:event=0x%x %s", event, eaudio_track_event_2_str(event));
}

static int _ep_first_frame(struct ep_sink *sink, ep_sink_first_frame_data_t *first_frame)
{
    eaudio_track_param_t track_param;
    eplatform_logd("first_frame:bits=%d,sample_rate=%d,channel_num=%d",
        first_frame->bits, first_frame->sample_rate, first_frame->channel_num);

    sink_track_buffer.buff     = sink_track_buff;
    sink_track_buffer.buff_len = sizeof(sink_track_buff);

    track_param.name               = "btwatchslave_player";
    track_param.buf                = &sink_track_buffer;
    track_param.format.bits        = first_frame->bits;
    track_param.format.channel_num = first_frame->channel_num;
    track_param.format.sample_rate = first_frame->sample_rate;
    track_param.device_type        = EAUDIO_DEVICE_OUT_PRIMARY;

    if (1) {
        sink_track = eaudio_track_alloc(&track_param, eaudio_track_callback, NULL);

        if (sink_track != NULL) {
            eaudio_track_start(sink_track);
        }
    }

    return 0;
}

// player callback
static int _eplayer_callback(eplayer_event_t event, eplayer_event_param_t *param)
{
    eplatform_logd("player_evt:event=%d%s,param=%p", event, eplayer_event_2_str(event), param);

    switch (event) {
        case EPLAYER_EVT_STOPPED:
        if (sink_track != NULL) {
            eaudio_track_stop(sink_track);
        }
        break;
        case EPLAYER_EVT_STARTED:
        break;
        default:
        break;
    }

    return 0;
}

// stream callback
static int _file_like_stream_event_callback(file_like_stream_event_t event, file_like_stream_event_param_t *param)
{
    comm_slave_t *comm_slave = NULL;
    file_like_stream_t *stream = param->stream;

    comm_slave = stream->priv;
    eplatform_logd("stream_evt:event=%d%s,param=%p", event, file_like_stream_event_2_str(event), param);

    switch (event) {
        case FILE_LIKE_STREAM_EVT_SEEK:
            comm_slave_seek_stream(comm_slave, 0xffffffff, param->p.seek.offset);
        break;
        case FILE_LIKE_STREAM_EVT_SPACE_CHANGE:
            comm_slave_ping_stream(comm_slave);
            _stream_pack_list_process_next(comm_slave);
        break;
        case FILE_LIKE_STREAM_EVT_PULL:
        break;
        default:
        break;
    }

    return 0;
}

static int _comm_slave_event_callback(comm_slave_event_t event, comm_slave_event_param_t *param)
{
    comm_slave_t *comm_slave = param->comm_slave;

    eplatform_logv("comm_slave_callback:event=%d %s, param=%p", event, comm_slave_event_2_str(event), param);
    switch (event) {
        case COMM_SLAVE_EVENT_START_PLAY:
        {
            char resource_name[64];

            memset(resource_name, '\0', sizeof(resource_name));
            sprintf(resource_name, "evfls:0x%08x:media.%s", param->p.start_play.handle, param->p.start_play.extname);

            file_like_stream_set_length(&player_file_stream, param->p.start_play.filesize);

            evf_prepare(resource_name, &player_file_stream);
            player_file = evf_fopen(resource_name, "rb");

            eplatform_logd("play_start_cmd:player_file %p", player_file);
            eplatform_logd("play_start_cmd:extname %s,handle=0x%x,filesize=%ld", param->p.start_play.extname, param->p.start_play.handle, param->p.start_play.filesize);

            if (player_file != NULL) {
                ep_source_copy(&player_source, &ep_source_evf_file);
                player_source.priv      = player_file;
                player_sink.priv        = player_file;
                player_sink.write_pcm   = _ep_write_pcm;
                player_sink.first_frame = _ep_first_frame;
                player_handle = eplayer_setup_handle(&player, &player_source, &player_sink, resource_name);

                // start player
                eplayer_start(&player, player_handle);

                // start stream
                comm_slave_start_stream(comm_slave, param->p.start_play.handle);
            }
        }
        break;
        case COMM_SLAVE_EVENT_STREAM_DATA:
        {
            ecomm_pack_t *pack = param->p.stream_data.pack;
            _pack_add_tail(pack);
            _stream_pack_list_process_next(comm_slave);
        }
        default:
        break;
    }

    return 0;
}

int bt_player_slave_open(void)
{
    app_sysfreq_req(APP_SYSFREQ_USER_APP_12, APP_SYSFREQ_208M);

    // eaudio init
    eaudio_init();
    eaudio_open();

    // player init
    eplayer_config_t config;
    config.decode_task = 1;
    eplayer_open(&player, _eplayer_callback, &config);

	ecomm_open(&ecomm, NULL);
    ecomm_register_instance("bes-btwatch-slave-ecomm-1", &ecomm);

    // pack list init
    eplatform_list_init(&_pack_list);
    _pack_list_lock_init();

    // stream init
    player_file_stream.priv = g_comm_slave;
    file_like_stream_open(&player_file_stream, _file_like_stream_event_callback, stream_buff, sizeof(stream_buff));

    // comm slave open
    comm_slave_open(g_comm_slave, _comm_slave_event_callback);

    // bt service server
    bt_service_server_init(&bts_transport_ecomm);
    return 0;
}

int app_bt_watch_open(void)
{
    return 0;
    return bt_player_slave_open();
}