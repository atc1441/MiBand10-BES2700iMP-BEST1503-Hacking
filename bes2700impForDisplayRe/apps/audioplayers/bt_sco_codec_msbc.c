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
#include <string.h>
#include "hal_trace.h"
#include "audio_dump.h"
#include "tgt_hardware.h"
#include "bt_sco_codec.h"
#include "plc_utils.h"
#include "bt_drv.h"
#include "speech_cfg.h"
#include "sbc_api.h"
#include "sbc_error_code.h"
#include "bt_drv_reg_op.h"

// ------------------------------ Macro ------------------------------

// #define SPEECH_RX_PLC_DUMP_DATA

// #define PLC_DEBUG_PRINT_DATA

#define ENABLE_LPC_PLC

#define ENABLE_PLC_ENCODER

// ------------------------------ Codec Common ------------------------------
#define CODEC_PACKET_SIZE           (57)
#define SCO_PACKET_SIZE             (60)
#define MSBC_PCM_FRAME_SAMPLES      (120)
#define MSBC_PCM_FRAME_SIZE         (MSBC_PCM_FRAME_SAMPLES * sizeof(int16_t))

#define SCO_PACKET_HEADER           (0x01)
#define SCO_PACKET_SEQ_NUM          (4)
#define SCO_PACKET_PADDING          (0x00)
static const uint8_t g_sco_packet_seq[SCO_PACKET_SEQ_NUM] = {0x08, 0x38, 0xc8, 0xf8};

typedef struct {
    uint8_t     header;
    uint8_t     seq;
    uint8_t     payload[CODEC_PACKET_SIZE];
    uint8_t     padding;
} sco_msbc_packet_t;

// ------------------------------ Encoder ------------------------------
static sbc_encoder_t *msbc_encoder = NULL;
static uint8_t g_encoder_seq_index = 0;

// ------------------------------ Decoder ------------------------------
static sbc_decoder_t *msbc_decoder = NULL;

static float msbc_eq_band_gain[8]= {0,0,0,0,0,0,0,0};

static uint8_t *msbc_buf_all = NULL;
static uint32_t msbc_find_first_sync = 0;
static uint32_t msbc_offset = 0;
static uint32_t next_frame_flag = 0;

// ------------------------------ PLC Encoder ------------------------------
#ifdef ENABLE_PLC_ENCODER
static sbc_encoder_t *msbc_plc_encoder = NULL;
static int16_t *msbc_plc_encoder_buffer = NULL;
#define MSBC_CODEC_DELAY    (73)
#endif

// ------------------------------ PLC ------------------------------
#if defined(ENABLE_LPC_PLC)
#include "lpc_plc_api.h"
static LpcPlcState *plc_st = NULL;
#else
#include "plc_16000.h"
static struct PLC_State plc_st;
#endif

static PacketLossState pld;

// ------------------------------ Dump ------------------------------
#ifdef SPEECH_RX_PLC_DUMP_DATA
static int16_t *audio_dump_temp_buf = NULL;
#endif

/**
 * Dynamic malloc RAM from speech_heap:
 *  1. PLC: 
 *  2. This module: 8k Bytes
 **/
int32_t bt_sco_codec_msbc_init(uint32_t sample_rate)
{
    TRACE(0, "[%s] SCO: MSBC...", __func__);

    ASSERT(SCO_PACKET_SIZE == sizeof(sco_msbc_packet_t),
        "[%s] SCO_PACKET_SIZE != sizeof(sco_msbc_packet_t)", __func__);

    g_encoder_seq_index = 0;

    // Add 1 to ensure it never be out of bounds when msbc_offset is 1
    msbc_buf_all = (uint8_t *)speech_calloc(SCO_PACKET_SIZE * 3 + 1, sizeof(uint8_t));    // 181 Bytes
    memset(msbc_buf_all, 0x0, (SCO_PACKET_SIZE * 3 + 1) * sizeof(uint8_t));

    msbc_encoder = (sbc_encoder_t *)speech_calloc(1, sizeof(sbc_encoder_t));  // 2176 Bytes
    msbc_decoder = (sbc_decoder_t *)speech_calloc(1, sizeof(sbc_decoder_t));  // 5376 Bytes

    // init msbc encoder
    sbc_stream_info_t info;
    info.flags = SBC_FLAGS_MSBC;
    info.num_channels = 1;
    info.channel_mode = SBC_CHANNEL_MODE_MONO;
    info.bit_pool     = 26;
    info.sample_rate  = SBC_SAMPLERATE_16K;
    info.alloc_method = SBC_ALLOC_METHOD_LOUDNESS;
    info.num_blocks   = MSBC_BLOCKS;
    info.num_subbands = 8;
    sbc_encoder_open(msbc_encoder, info);

    for (uint8_t i=0; i<ARRAY_SIZE(msbc_eq_band_gain); i++) {
        msbc_eq_band_gain[i] = 1.0;
    }

    sbc_decoder_open(msbc_decoder);
    sbc_decoder_set(msbc_decoder, SBC_FLAGS, &info.flags);

    next_frame_flag = 0;
    msbc_find_first_sync = 0;

#if defined(ENABLE_PLC_ENCODER)
    msbc_plc_encoder = (sbc_encoder_t *)speech_calloc(1, sizeof(sbc_encoder_t));  // 2176 Bytes

    sbc_encoder_open(msbc_plc_encoder, info);

    msbc_plc_encoder_buffer = (int16_t *)speech_calloc(MSBC_PCM_FRAME_SAMPLES + MSBC_CODEC_DELAY, sizeof(int16_t));      // 480 Bytes
#endif

#if defined(ENABLE_LPC_PLC)
    plc_st = lpc_plc_create(sample_rate);
#else
    InitPLC(&plc_st);
#endif

    packet_loss_detection_init(&pld);

#ifdef SPEECH_RX_PLC_DUMP_DATA
    audio_dump_temp_buf = (int16_t *)speech_calloc(MSBC_PCM_FRAME_SAMPLES, sizeof(int16_t));     // 240 Bytes
    audio_dump_init(MSBC_PCM_FRAME_SAMPLES, sizeof(short), 3);
#endif

    return 0;
}

int32_t bt_sco_codec_msbc_deinit(void)
{
    TRACE(0, "[%s] ...", __func__);

    packet_loss_detection_report(&pld);

#if defined(ENABLE_LPC_PLC)
    lpc_plc_destroy(plc_st);
#endif

    speech_free(msbc_buf_all);
    speech_free(msbc_encoder);
    speech_free(msbc_decoder);

#if defined(ENABLE_PLC_ENCODER)
    speech_free(msbc_plc_encoder);
    speech_free(msbc_plc_encoder_buffer);
#endif

#ifdef SPEECH_RX_PLC_DUMP_DATA
    speech_free(audio_dump_temp_buf);
#endif

    return 0;
}

// msbc_size = 120, pcm_size = 480
int32_t bt_sco_codec_msbc_encoder(uint8_t *msbc_buf, uint32_t msbc_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    // TRACE(0, "[%s] msbc_size: %d, pcm_size: %d", __func__, msbc_size, pcm_size);

    ASSERT(msbc_size / SCO_PACKET_SIZE == pcm_size / MSBC_PCM_FRAME_SIZE,
        "[%s] msbc_size = %d, pcm_size = %d", __func__, msbc_size, pcm_size);

    uint32_t loop_cnt = msbc_size / SCO_PACKET_SIZE;
    sco_msbc_packet_t *sco_packet_ptr = (sco_msbc_packet_t *)msbc_buf;

    // TRACE(0, "[%s] msbc_size = %d, pcm_size = %d", __func__, msbc_size, pcm_size);

    // Parser packet
    for (uint32_t i=0; i<loop_cnt; i++) {
        sco_packet_ptr->header  = SCO_PACKET_HEADER;
        sco_packet_ptr->padding = SCO_PACKET_PADDING;
        sco_packet_ptr->seq     = g_sco_packet_seq[g_encoder_seq_index];

        pcm_frame_t pcm_data;
        sbc_frame_t sbc_data;
        sbc_data.sbc_data = sco_packet_ptr->payload;
        sbc_data.valid_size = 0;
        sbc_data.buffer_size = SCO_PACKET_SIZE;
        pcm_data.pcm_data = (int16_t *)pcm_buf;
        pcm_data.buffer_size = MSBC_PCM_FRAME_SIZE;
        pcm_data.valid_size = MSBC_PCM_FRAME_SIZE;
        sbc_encoder_process_frame(msbc_encoder, &pcm_data, &sbc_data);

        sco_packet_ptr++;
        pcm_buf += MSBC_PCM_FRAME_SIZE;
        g_encoder_seq_index = (g_encoder_seq_index + 1) % SCO_PACKET_SEQ_NUM;
    }

    return 0;
}

extern uint32_t btdrv_syn_get_curr_ticks(void);
int32_t bt_sco_codec_msbc_decoder(uint8_t *msbc_buf, uint32_t msbc_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    // TRACE(0, "[%s] msbc_size: %d, pcm_size: %d", __func__, msbc_size, pcm_size);

    ASSERT(msbc_size / SCO_PACKET_SIZE == pcm_size / MSBC_PCM_FRAME_SIZE,
        "[%s] msbc_size = %d, pcm_size = %d", __func__, msbc_size, pcm_size);

    pcm_frame_t pcm_data;
    uint32_t msbc_offset_lowdelay = 0;
    uint32_t i,j;
    uint8_t *msbc_buffer=(uint8_t *)msbc_buf;
    int frame_flag[6];  // 1: good frame; 0:bad frame;
    int ret;
    uint32_t frame_counter=0;
    uint32_t msbc_offset_total = 0;
    int msbc_offset_drift[6] = {0, };

    short *dec_pcm_buf=( short *)pcm_buf;
    uint8_t dec_msbc_buf[SCO_PACKET_SIZE] = {0, };

    // uint32_t timer_begin=hal_sys_timer_get();

    // TRACE(2,"process_downlink_msbc_frames:pcm_size:%d,msbc_size:%d",pcm_size,msbc_size);

    // TRACE(1,"decode_msbc_frame,msbc_size:%d",msbc_size);
    for(i =0; i<msbc_size; i++) {
        msbc_buf_all[i + SCO_PACKET_SIZE] = msbc_buffer[i];
    }

    /*
    for(i =0; i<msbc_size/2; i=i+10)
    {
       TRACE(10,"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,",
       DecMsbcBufAll[i],DecMsbcBufAll[i+1],DecMsbcBufAll[i+2],DecMsbcBufAll[i+3],DecMsbcBufAll[i+4],
       DecMsbcBufAll[i+5],DecMsbcBufAll[i+6],DecMsbcBufAll[i+7],DecMsbcBufAll[i+8],DecMsbcBufAll[i+9]);
    }
    */

    for(j =0; j<msbc_size/SCO_PACKET_SIZE; j++) {
        frame_flag[j]=0;
    }

    if (msbc_find_first_sync == 0) {
        for (i =0; i<msbc_size; i++) {
            if((msbc_buf_all[i]==0x01)&&((msbc_buf_all[i+1]&0x0f)==0x08)&&(msbc_buf_all[i+2]==0xad))break;
        }

        TRACE(1,"sync......:%d",i);

        if (i < msbc_size) {
            msbc_find_first_sync = 1;
            msbc_offset = i % SCO_PACKET_SIZE;
            msbc_offset_total = i;
        } else {
            for (j = 0; j < msbc_size / SCO_PACKET_SIZE; j++) {
                frame_flag[j] = 1;
            }
        }
    }

    if (msbc_find_first_sync == 1) {
        int value = 0;
        uint8_t headerm1 = 0;
        uint8_t header0 = 0;
        uint8_t header1 = 0;
        uint8_t header2 = 0;
        uint8_t header3 = 0;
        // uint8_t tail0 = 0;

        if(msbc_offset==0||msbc_offset==1) {
            msbc_offset_lowdelay=msbc_offset+60;
            // msbc_offset_lowdelay=msbc_offset;
        } else {
            msbc_offset_lowdelay=msbc_offset;
        }

        // check again
        for (j =0; j<(msbc_size/SCO_PACKET_SIZE); j++) {
            if (next_frame_flag == 1) {
                next_frame_flag = 0;
                frame_flag[j] = 1;
                continue;
            }

            if (msbc_offset_lowdelay == 0 && j == 0) {
                headerm1=0x01;
            } else {
                headerm1 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE-1];
            }

            header0 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE];
            header1 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE + 1];
            header2 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE + 2];
            header3 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE + 3];

            /*if ((headerm1 == 0x01) && ((header0 & 0x0f) == 0x08) && (header1 == 0xad) ||
                (header0 == 0x01) && ((header1 & 0x0f) == 0x08) && (header2 == 0xad) ||
                (header1 == 0x01) && ((header2 & 0x0f) == 0x08) && (header3 == 0xad))
            {
                frame_flag[j] = 0;
            }*/
            if ((headerm1 == 0x01) && ((header0 & 0x0f) == 0x08) && (header1 == 0xad)) {
                frame_flag[j] = 0;
                // It seems that offset is reduced by 1
                msbc_offset_drift[j] = -1;
                TRACE(1,"[%s] msbc_offset is reduced by 1", __FUNCTION__);
                /*
                tail0 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE + 59 - 1];
                if (tail0 == 0x00 || tail0 == 0x01|| tail0==0xff)
                {
                    frame_flag[j] = 0;
                }
                else
                {
                    frame_flag[j] = 1;
                    next_frame_flag = 1;
                }
                */
            } else if ((header0 == 0x01) && ((header1 & 0x0f) == 0x08) && (header2 == 0xad)) {
                frame_flag[j] = 0;
                /*
                tail0 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE + 59];
                if (tail0 == 0x00 || tail0 == 0x01|| tail0==0xff)
                {
                    frame_flag[j] = 0;
                }
                else
                {
                    frame_flag[j] = 1;
                    next_frame_flag = 1;
                }
                */
            } else if ((header1 == 0x01) && ((header2 & 0x0f) == 0x08) && (header3 == 0xad)) {
                frame_flag[j] = 0;
                msbc_offset_drift[j] = 1;
                TRACE(1,"[%s] msbc_offset is increased by 1", __FUNCTION__);
                /*
                tail0 = msbc_buf_all[msbc_offset_lowdelay + j*SCO_PACKET_SIZE + 59 + 1];
                if (tail0 == 0x00 || tail0==0x01|| tail0==0xff)
                {
                    frame_flag[j] = 0;
                }
                else
                {
                    frame_flag[j] = 1;
                    next_frame_flag = 1;
                }
                */
            } else {
                if ((header0 == MSBC_MUTE_PATTERN)&& ((header1 & 0x0f) == (MSBC_MUTE_PATTERN & 0x0f)) && (header2 == MSBC_MUTE_PATTERN)) {
                    frame_flag[j]=1;
                } else {
                    if((msbc_offset_lowdelay+j*SCO_PACKET_SIZE)>=msbc_offset_total) {
                        frame_flag[j]=3;
                    } else {
                        frame_flag[j]=1;
                    }
                }
            }
        }

        for(j =0; j<msbc_size/SCO_PACKET_SIZE; j++) {
            value=value|frame_flag[j];
        }

        // abnormal msbc packet.
        if (value > 1) {
            msbc_find_first_sync = 0;
        }
    }

    while ( (frame_counter < msbc_size / SCO_PACKET_SIZE) &&
            (frame_counter < pcm_size / MSBC_PCM_FRAME_SIZE)) {
        //TRACE(3,"[%s] decoding, offset %d, offset drift %d", __FUNCTION__, msbc_offset, msbc_offset_drift[frame_counter]);
        // skip first byte when msbc_offset == 0 and msbc_offset_drift == -1
        uint32_t start_idx = 0;
        if (msbc_offset_lowdelay == 0 && msbc_offset_drift[frame_counter] == -1) {
            start_idx = 1;
            dec_msbc_buf[0] = 0x01;
        }
        for(i = start_idx; i<SCO_PACKET_SIZE; i++)
        {
            //DecMsbcBuf[i]=DecMsbcBufAll[i+msbc_offset_lowdelay+frame_counter*SCO_PACKET_SIZE+2];
            dec_msbc_buf[i]=msbc_buf_all[i+msbc_offset_lowdelay+msbc_offset_drift[frame_counter]+frame_counter*SCO_PACKET_SIZE];
        }

        //TRACE(1,"msbc header:0x%x",DecMsbcBuf[0]);

#ifdef SPEECH_RX_PLC_DUMP_DATA
        audio_dump_add_channel_data(2, (short *)dec_msbc_buf, SCO_PACKET_SIZE/2);
#endif

        uint32_t transmit_msbc_offset = msbc_find_first_sync ?
                                        ((msbc_offset - MSBC_OFFSET_BYTES + SCO_PACKET_SIZE) % SCO_PACKET_SIZE) :
                                        0;

        plc_type_t plc_type = packet_loss_detection_process(&pld, dec_msbc_buf, transmit_msbc_offset);

        if(plc_type != PLC_TYPE_PASS) {
            memset(dec_pcm_buf, 0, MSBC_PCM_FRAME_SAMPLES * sizeof(int16_t));
            goto do_plc;
        }

        pcm_data.pcm_data = dec_pcm_buf;
        pcm_data.buffer_size = MSBC_PCM_FRAME_SAMPLES*2;
        pcm_data.valid_size = 0;
        sbc_frame_t sbc_data;
        sbc_data.sbc_data = dec_msbc_buf;
        sbc_data.valid_size = SCO_PACKET_SIZE;
        ret = sbc_decoder_process_frame(msbc_decoder, &sbc_data, &pcm_data);
        //ASSERT(ret == BT_STS_SUCCESS, "[%s] msbc decoder should never fail", __FUNCTION__);
        if (ret != SBC_OK) {
            plc_type = PLC_TYPE_DECODER_ERROR;
            packet_loss_detection_update_histogram(&pld, plc_type);
        }

do_plc:
        if (plc_type == PLC_TYPE_PASS) {
#if defined(ENABLE_LPC_PLC)
            lpc_plc_save(plc_st, dec_pcm_buf);
#else
            PLC_good_frame(&plc_st, dec_pcm_buf, dec_pcm_buf);
#endif
#ifdef SPEECH_RX_PLC_DUMP_DATA
            audio_dump_add_channel_data(0, (short *)dec_pcm_buf, MSBC_PCM_FRAME_SIZE/2);
#endif
        } else {
            TRACE(1,"PLC bad frame, plc type: %d, curr clk=%x", plc_type, btdrv_syn_get_curr_ticks());
            if(plc_type != 1)
            {
#if defined(PLC_DEBUG_PRINT_DATA)
                TRACE(3,"0x4000E000:%x,0x4000E010:%x,0xD022069C:%x", 
					BTDIGITAL_REG(0x4000E000),BTDIGITAL_REG(0x4000E010),BTDIGITAL_REG(0xD022069C));
                btdrv_regop_dump_btpcm_reg();
                bt_drv_reg_sco_fifo_dump();
                DUMP16("0x%x ", dec_msbc_buf, MSBC_PCM_FRAME_SIZE/2);
#endif
            }
#ifdef SPEECH_RX_PLC_DUMP_DATA
            for(uint32_t i=0; i< MSBC_PCM_FRAME_SIZE/2; i++) {
                  audio_dump_temp_buf[i] = (plc_type - 1) * 5000;
            }
            audio_dump_add_channel_data(0, audio_dump_temp_buf, MSBC_PCM_FRAME_SIZE/2);
#endif
#if defined(ENABLE_LPC_PLC)
            lpc_plc_generate(plc_st, dec_pcm_buf,
#if defined(ENABLE_PLC_ENCODER)
                msbc_plc_encoder_buffer
#else
                NULL
#endif
            );

#if defined(ENABLE_PLC_ENCODER)
            pcm_data.valid_size = MSBC_PCM_FRAME_SIZE;
            pcm_data.buffer_size = MSBC_PCM_FRAME_SIZE;
            pcm_data.pcm_data = (int16_t *)(msbc_plc_encoder_buffer + MSBC_CODEC_DELAY);
            sbc_decoder_update_state(msbc_decoder, msbc_plc_encoder, &pcm_data);
#endif

#else
            pcm_data.sampleFreq = BTIF_SBC_CHNL_SAMPLE_FREQ_16;
            pcm_data.numChannels = 1;
            pcm_data.dataLen = 0;
            pcm_data.data = (uint8_t *)dec_pcm_buf;

            ret = btif_sbc_decode_frames(msbc_decoder,
                    (uint8_t *)indices0,
                    SCO_PACKET_SIZE, &byte_decode,
                    &pcm_data, MSBC_PCM_FRAME_SAMPLES*2,
                    msbc_eq_band_gain);

            PLC_bad_frame(&plc_st, dec_pcm_buf, dec_pcm_buf);

            ASSERT(ret == BT_STS_SUCCESS, "[%s] msbc decoder should never fail", __FUNCTION__);
#endif
        }

#ifdef SPEECH_RX_PLC_DUMP_DATA
        audio_dump_add_channel_data(1, (short *)dec_pcm_buf, MSBC_PCM_FRAME_SIZE/2);
        audio_dump_run();
#endif

       dec_pcm_buf=dec_pcm_buf+MSBC_PCM_FRAME_SAMPLES;
       frame_counter++;
    }

    for(i =0; i<SCO_PACKET_SIZE; i++) {
        msbc_buf_all[i]=msbc_buf_all[i+msbc_size];
    }

    //TRACE(1,"msbc + plc:%d", (hal_sys_timer_get()-timer_begin));
    return 0;
}

bt_sco_codec_t bt_sco_codec_msbc = {
    .init       = bt_sco_codec_msbc_init,
    .deinit     = bt_sco_codec_msbc_deinit,
    .encoder    = bt_sco_codec_msbc_encoder,
    .decoder    = bt_sco_codec_msbc_decoder
};
