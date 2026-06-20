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
#include "hal_trace.h"
#include "audio_dump.h"
#include "string.h"
#include "cmsis_os.h"

#ifdef AUDIO_DEBUG

// #define DATA_DUMP_TOTA
// #define DATA_DUMP_USB
// #define DUMP_PLC_ENABLE

#ifdef DATA_DUMP_TOTA
#include "app_tota_audio_dump.h"
#endif

#ifdef DATA_DUMP_USB
#include "data_dump_usb.h"
#endif

#if defined(DATA_DUMP_USB)
#define AUDIO_DUMP_HEAD_STR             ("[trace][audio dump]")
#else
#define AUDIO_DUMP_HEAD_STR             ("[audio dump]")
#endif
#define AUDIO_DUMP_HEAD_LEN             (sizeof(AUDIO_DUMP_HEAD_STR)-1)
#define AUDIO_DUMP_INDEX_LEN            (4)
#define AUDIO_DUMP_CRC_LEN              (4)
#define AUDIO_DUMP_DATA_LEN             (4)
#define AUDIO_DUMP_MAX_SAMPLE_BYTES     (sizeof(int))
#define AUDIO_DUMP_MAX_FRAME_LEN        (512)
#define AUDIO_DUMP_MAX_CHANNEL_NUM      (8)
#define AUDIO_DUMP_MAX_DATA_SIZE        (AUDIO_DUMP_MAX_FRAME_LEN * AUDIO_DUMP_MAX_CHANNEL_NUM * AUDIO_DUMP_MAX_SAMPLE_BYTES / sizeof(char))
#define AUDIO_DUMP_BUFFER_SIZE          (AUDIO_DUMP_HEAD_LEN + AUDIO_DUMP_INDEX_LEN + AUDIO_DUMP_CRC_LEN + AUDIO_DUMP_DATA_LEN + AUDIO_DUMP_MAX_DATA_SIZE)

static int audio_dump_index = 0;
static int audio_dump_crc = 0;
static int audio_dump_frame_len = 256;
static int audio_dump_sample_bytes = sizeof(short) / sizeof(char);
static int audio_dump_channel_num = 2;
static int audio_dump_data_size = 0;
static int audio_dump_buf_size = 0;
static char *audio_dump_data_ptr = NULL;
static char audio_dump_buf[AUDIO_DUMP_BUFFER_SIZE];

osMutexDef(audio_dump_buf_mutex);
static void *audio_dump_buf_mutex = NULL;

int audio_dump_buffer_mutex_init(void)
{
    if (audio_dump_buf_mutex == NULL){
        audio_dump_buf_mutex = osMutexCreate((osMutex(audio_dump_buf_mutex)));
    }
    return 0;
}

int audio_dump_buffer_mutex_lock(void)
{
    osMutexWait((osMutexId)audio_dump_buf_mutex, osWaitForever);
    return 0;
}

int audio_dump_buffer_mutex_unlock(void)
{
    osMutexRelease((osMutexId)audio_dump_buf_mutex);
    return 0;
}


void audio_dump_clear_up(void)
{
    memset(audio_dump_data_ptr, 0, audio_dump_data_size);
}

void audio_dump_init(int frame_len, int sample_bytes, int channel_num)
{
    TRACE(2, "%s frame_len:%d, sample_bytes:%d, channel_num:%d", __func__, frame_len, sample_bytes, channel_num);

     ASSERT(frame_len*channel_num*sample_bytes <= AUDIO_DUMP_MAX_DATA_SIZE,
           "[%s] dump data size is too large! Enlarge free RAM", __func__);

    char *buf_ptr = audio_dump_buf;
    audio_dump_index = 0;
    audio_dump_crc = 0;
    audio_dump_frame_len = frame_len;
    audio_dump_sample_bytes = sample_bytes / sizeof(char);
    audio_dump_channel_num = channel_num;

    /*
     * audio dump packet
     * [trace][audio_dump] + data_len(int32_t) + data
     * In uart mode, "[trace]" is added in hal_trace_output.
     * In usb mode, "[trace]" is added in audio_dump module.
     */
    memcpy(buf_ptr, AUDIO_DUMP_HEAD_STR, AUDIO_DUMP_HEAD_LEN);

#ifdef DUMP_PLC_ENABLE
    buf_ptr += AUDIO_DUMP_HEAD_LEN + AUDIO_DUMP_INDEX_LEN + AUDIO_DUMP_CRC_LEN;
#else
    buf_ptr += AUDIO_DUMP_HEAD_LEN;
#endif

    int *data_len = (int *)buf_ptr;
    *data_len = frame_len * channel_num * audio_dump_sample_bytes;
    audio_dump_data_size = *data_len;
#ifdef DUMP_PLC_ENABLE
    audio_dump_buf_size = AUDIO_DUMP_HEAD_LEN + AUDIO_DUMP_INDEX_LEN + AUDIO_DUMP_CRC_LEN + AUDIO_DUMP_DATA_LEN + audio_dump_data_size;
#else
    audio_dump_buf_size = AUDIO_DUMP_HEAD_LEN + AUDIO_DUMP_DATA_LEN + audio_dump_data_size;
#endif

    buf_ptr += AUDIO_DUMP_DATA_LEN;

    audio_dump_data_ptr = buf_ptr;

    audio_dump_clear_up();

    audio_dump_buffer_mutex_init();

#if defined(DATA_DUMP_USB)
    usb_serial_thread_init();
#endif

// #ifdef DATA_DUMP_TOTA
//     data_dump_tota_open();
//     data_dump_tota_start();
// #endif
}

void audio_dump_deinit(void)
{
#if defined(DATA_DUMP_USB)
    usb_serial_thread_destroy();
#endif

// #ifdef DATA_DUMP_TOTA
//     data_dump_tota_stop();
//     data_dump_tota_close();
// #endif
}

void audio_dump_add_channel_data_from_multi_channels(int channel_id, void *pcm_buf, int pcm_len, int channel_num, int channel_index)
{
    ASSERT(audio_dump_frame_len >= pcm_len, "[%s] frame_len(%d) < pcm_len(%d)", __func__, audio_dump_frame_len, pcm_len);

    if (channel_id >= audio_dump_channel_num)
    {
        return;
    }

    audio_dump_buffer_mutex_lock();
    if (audio_dump_sample_bytes == sizeof(short))
    {
        short *_pcm_buf = (short *)pcm_buf;
        short *_dump_buf = (short *)audio_dump_data_ptr;

        for (int i = 0; i < pcm_len; i++)
        {
            _dump_buf[audio_dump_channel_num * i + channel_id] = _pcm_buf[i * channel_num + channel_index];
        }
    }
    else if (audio_dump_sample_bytes == sizeof(int))
    {
        int *_pcm_buf = (int *)pcm_buf;
        int *_dump_buf = (int *)audio_dump_data_ptr;

        for (int i = 0; i < pcm_len; i++)
        {
            _dump_buf[audio_dump_channel_num * i + channel_id] = _pcm_buf[i * channel_num + channel_index];
        }
    }
    else
    {
        ASSERT(0, "[%s] audio_dump_sample_bytes(%d) is invalid", __func__, audio_dump_sample_bytes);
    }
    audio_dump_buffer_mutex_unlock();
}

void audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(int channel_id, void *pcm_buf, int pcm_len, int channel_num, int channel_index, int right_shift)
{
    ASSERT(audio_dump_frame_len >= pcm_len, "[%s] frame_len(%d) < pcm_len(%d)", __func__, audio_dump_frame_len, pcm_len);

    if (channel_id >= audio_dump_channel_num)
    {
        return;
    }

    // return;

    audio_dump_buffer_mutex_lock();
    if (audio_dump_sample_bytes == sizeof(short))
    {
        int POSSIBLY_UNUSED *_pcm_buf = (int *)pcm_buf;
        short POSSIBLY_UNUSED *_dump_buf = (short *)audio_dump_data_ptr;

        for (int i = 0; i < pcm_len; i++)
        {
            _dump_buf[audio_dump_channel_num * i + channel_id] = _pcm_buf[i * channel_num + channel_index] >> right_shift;
        }
    }
    else
    {
        ASSERT(0, "[%s] audio_dump_sample_bytes(%d) is invalid", __func__, audio_dump_sample_bytes);
    }
    audio_dump_buffer_mutex_unlock();
}

void audio_dump_add_channel_data(int channel_id, void *pcm_buf, int pcm_len)
{
    audio_dump_add_channel_data_from_multi_channels(channel_id, pcm_buf, pcm_len, 1, 0);
}

#ifdef DUMP_PLC_ENABLE
static void audio_debug_add_index(void)
{
    int *index_ptr = (int *)(audio_dump_buf + AUDIO_DUMP_HEAD_LEN);

    *index_ptr = audio_dump_index;
}

// unsigned long crc32_c(unsigned long crc, const unsigned char *buf, unsigned int len)
// {
//     unsigned long c;

//     c = crc ^ 0xffffffffL;

//     while (len)
//     {
//         c = crc_table[(c ^ (*buf++)) & 0xff] ^ (c >> 8);
//         len--;
//     }

//     return c ^ 0xffffffffL;
// }

static void audio_debug_add_crc(void)
{
    int *crc_ptr = (int *)(audio_dump_buf + AUDIO_DUMP_HEAD_LEN + AUDIO_DUMP_CRC_LEN);

    *crc_ptr = 0;
}
#endif

void audio_dump_run(void)
{
#ifdef DATA_DUMP_TOTA
    int offset = AUDIO_DUMP_HEAD_LEN + AUDIO_DUMP_DATA_LEN;

#ifdef DUMP_PLC_ENABLE
    offset += AUDIO_DUMP_INDEX_LEN + AUDIO_DUMP_CRC_LEN;
#endif

    app_tota_audio_dump_send((uint8_t *)&audio_dump_buf[offset], audio_dump_buf_size - offset);
    // data_dump_tota_run((uint8_t *)&audio_dump_buf[offset], audio_dump_buf_size - offset);
#elif defined(DATA_DUMP_USB)
    usb_serial_send_data();
#else
#ifdef DUMP_PLC_ENABLE
    audio_debug_add_index();
    audio_debug_add_crc();
    audio_dump_index++;

    // if (audio_dump_index % 20 == 0)
    // {
    //     audio_dump_index++;
    // }
#endif
    AUDIO_DEBUG_DUMP((const unsigned char *)audio_dump_buf, audio_dump_buf_size);
#endif
}

void audio_dump_get_buff_info(uint8_t **buf, uint32_t *len)
{
    if (buf) {
        *buf = (uint8_t *)audio_dump_buf;
    }
    if (len) {
        *len = audio_dump_buf_size;
    }
}

// Make sure DATA_DUMP_BUF_SIZE < TRACE_BUF_SIZE
#define DATA_DUMP_USER_NUM      (12)
#define DATA_DUMP_BUF_SIZE      (1024 * 6)
static uint8_t data_dump_buf[DATA_DUMP_BUF_SIZE];
static int data_dump_index[DATA_DUMP_USER_NUM];

void data_dump_init(void)
{
    for (uint32_t i=0; i<DATA_DUMP_USER_NUM; i++) {
        data_dump_index[i] = 0;
    }
}

void data_dump_deinit(void)
{
    ;
}

void data_dump_run(uint32_t ch, const char *str, void *data_buf, uint32_t data_len)
{
    uint8_t *buf_ptr = data_dump_buf;
    uint32_t len = 0;

    // head
    memcpy(buf_ptr + len, str, strlen(str));
    len += strlen(str);

#ifdef DUMP_PLC_ENABLE
    int32_t *int_ptr = NULL;

    ASSERT(ch < DATA_DUMP_USER_NUM, "[%s] ch(%d) is invalid!", __func__, ch);

    // index
    int_ptr = (int32_t *)(buf_ptr + len);
    *int_ptr = data_dump_index[ch];
    len += sizeof(int32_t);

    // crc
    int_ptr = (int32_t *)(buf_ptr + len);
    *int_ptr = 0;
    len += sizeof(int32_t);
	
    data_dump_index[ch]++;

    // if (data_dump_index[ch] % 20 == 0)
    // {
    //     data_dump_index[ch]++;
    // }
#endif

    // length
    memcpy(buf_ptr + len, &data_len, sizeof(uint32_t));
    len += sizeof(uint32_t);

    // data
    memcpy(buf_ptr + len, (uint8_t *)data_buf, data_len);
    len += data_len;

    ASSERT(len < DATA_DUMP_BUF_SIZE, "[%s] len(%d) > DATA_DUMP_BUF_SIZE", __func__, len);

    // TRACE(5,"[%s] %d, %d, %d, %d", __func__, strlen(str), sizeof(uint32_t), data_len, len);
    AUDIO_DEBUG_DUMP(buf_ptr, len);
}
#else // AUDIO_DEBUG
void audio_dump_clear_up(void) {;}
void audio_dump_init(int frame_len, int sample_bytes, int channel_num) {;}
void audio_dump_deinit(void) {;}
void audio_dump_add_channel_data_from_multi_channels(int channel_id, void *pcm_buf, int pcm_len, int channel_num, int channel_index) {;}
void audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(int channel_id, void *pcm_buf, int pcm_len, int channel_num, int channel_index, int right_shift) {;}
void audio_dump_add_channel_data(int channel_id, void *pcm_buf, int pcm_len) {;}
void audio_dump_run(void) {;}
void data_dump_init(void) {;}
void data_dump_deinit(void) {;}
void data_dump_run(uint32_t ch, const char *str, void *data_buf, uint32_t data_len) {;}
#endif
