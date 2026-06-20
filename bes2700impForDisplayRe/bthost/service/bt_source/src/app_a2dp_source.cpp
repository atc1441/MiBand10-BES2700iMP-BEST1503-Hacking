/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef BT_A2DP_SUPPORT
#if defined(BT_A2DP_SRC_ROLE)
#include "bt_source.h"
#include "app_a2dp_source.h"
#include "app_overlay.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "btapp.h"
#include "bt_drv_reg_op.h"
#include "app_bt.h"
#include "app_a2dp.h"
#include "app_utils.h"
#include "app_bt_func.h"
#include "app_audio.h"
#include "bt_common_define.h"
#ifndef FPGA
#ifdef A2DP_AAC_ON
#include "aac_api.h"
#endif
#endif
#include "hal_location.h"
#include "app_source_codec.h"
#include "bt_drv_interface.h"
#ifdef A2DP_SOURCE_LHDC_ON
#include "lhdc_enc_api.h"
#endif
#ifdef APP_USB_A2DP_SOURCE
#include "usb_audio_app.h"
#endif

#ifdef A2DP_SOURCE_LHDCV5_ON
#include "codec_lhdcv5.h"
#include "app_overlay.h"
#endif

#ifdef TEST_OVER_THE_AIR_ENANBLED
#include "app_spp_tota.h"
#endif

#ifdef __SOURCE_TRACE_RX__
#include "app_trace_rx.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#endif

#ifdef BT_SOURCE
#include "app_bt_stream.h"
#endif

#ifdef A2DP_ENCODER_CROSS_CORE
#include "a2dp_encoder_cc_bth.h"
#ifdef A2DP_ENCODER_CROSS_CORE_USE_M55
#include "mcu_dsp_m55_app.h"
#endif
#endif

#include "audio_codec_api.h"
#include "bt_if.h"

#ifdef APP_USB_A2DP_SOURCE
USB_STREAM_STATUS usb_stream_status = UNKNOWN;
static uint32_t usb_pre_vol = 0xFFFFFFFF;
#endif
#ifdef A2DP_SOURCE_LHDC_ON
extern bool one_frame_per_chennal;
#endif
#ifdef __SOURCE_TRACE_RX__
typedef struct {
    const char* name;
    APP_TRACE_RX_CALLBACK_T function;
} app_source_uart_handle_t;

typedef struct {
    uint32_t command;
    uint8_t *param;
    uint32_t len;
} SOURCE_RX_MSG;

static osThreadId app_source_trace_rx_tid = NULL;
static void app_source_trace_rx_thread(const void *arg);
osThreadDef(app_source_trace_rx_thread, osPriorityNormal, 1, 1024*2, "source_trace_rx");

#define SOURCE_TRACE_RX_MAILBOX_MAX (20)
static osMailQId source_rx_mailbox = NULL;
osMailQDef(source_rx_mailbox, SOURCE_TRACE_RX_MAILBOX_MAX, SOURCE_RX_MSG);

void app_source_trace_rx_handler_init(void)
{
    TRACE(1, "func:[%s]", __func__);
    app_source_trace_rx_tid = osThreadCreate(osThread(app_source_trace_rx_thread), NULL);
    source_rx_mailbox = osMailCreate(osMailQ(source_rx_mailbox), NULL);

    ASSERT(app_source_trace_rx_tid != NULL, "Failed to create source_trace_rx_thread");
    ASSERT(source_rx_mailbox, "Failed to create source rx mailbox");
}

static int source_rx_mailbox_put(SOURCE_RX_MSG *rx_msg)
{
    osStatus status;
    SOURCE_RX_MSG *msg_p = NULL;

    msg_p = (SOURCE_RX_MSG *)osMailAlloc(source_rx_mailbox, 0);
    if(!msg_p)
    {
        return -1;
    }

    msg_p->command = rx_msg->command;
    msg_p->param = rx_msg->param;
    msg_p->len = rx_msg->len;

    status = osMailPut(source_rx_mailbox, msg_p);

    return (int)status;
}

static int source_rx_mailbox_get(SOURCE_RX_MSG** rx_msg)
{
    osEvent evt;
    evt = osMailGet(source_rx_mailbox, osWaitForever);
    if (evt.status == osEventMail)
    {
        *rx_msg = (SOURCE_RX_MSG *)evt.value.p;
        return 0;
    }
    return -1;
}

static int rb_ctl_mailbox_free(SOURCE_RX_MSG* rx_msg)
{
    osStatus status;

    status = osMailFree(source_rx_mailbox, rx_msg);

    return (int)status;
}

static int source_rx_thread_post_msg(SOURCE_TRACE_RX_COMMAND command, uint8_t *param, uint32_t len)
{
    int ret;
    SOURCE_RX_MSG msg;

    if(!app_source_trace_rx_tid)
    {
        return -1;
    }

    msg.command = command;
    msg.param = param;
    msg.len = len;

    ret = source_rx_mailbox_put(&msg);

    return ret;
}

static unsigned int app_a2dp_source_search_device(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(SEARCH_DEVIECE, (uint8_t *)buf, len);

    return 0;
}

static unsigned int app_a2dp_source_connect_device(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(CONNECT_DEVICE, (uint8_t *)buf, len);

    return 0;
}

static unsigned int mobile_LAURENT_threshold(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(MOBILE_LAURENT_THRESHOLD, (uint8_t *)buf, len);

    return 0;
}

static unsigned int hdr_old_corr_thr(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(HDR_OLD_CORR_THR, (uint8_t *)buf, len);

    return 0;
}

static unsigned int mobile_rx_gain_sel(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(MOBILE_RX_GAIN_SEL, (uint8_t *)buf, len);

    return 0;
}

static unsigned int mobile_rx_gain_idx(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(MOBILE_RX_GAIN_IDX, (uint8_t *)buf, len);

    return 0;
}

static unsigned int fa_rx_gain_sel(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(FA_RX_GAIN_SEL, (uint8_t *)buf, len);

    return 0;
}

static unsigned int fa_rx_gain_idx(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(FA_RX_GAIN_IDX, (uint8_t *)buf, len);

    return 0;
}

static unsigned int fa_rx_lur_thr(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(FA_RX_LUR_THR, (uint8_t *)buf, len);

    return 0;
}

static unsigned int fa_old_corr_thr(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(FA_OLD_CORR_THR, (uint8_t *)buf, len);

    return 0;
}

static unsigned int ecc_block(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(ECC_BLOCK, (uint8_t *)buf, len);

    return 0;
}

static unsigned int ecc_enable(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(ECC_ENABLE, (uint8_t *)buf, len);

    return 0;
}

static unsigned int mobile_tx_power(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(MOBILE_TX_POWER, (uint8_t *)buf, len);

    return 0;
}

static unsigned int fa_tx_power(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(FA_TX_POWER, (uint8_t *)buf, len);

    return 0;
}

static unsigned int fast_lock_on_off(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(FAST_LOCK_ON_OFF, (uint8_t *)buf, len);

    return 0;
}

#ifdef A2DP_SOURCE_TEST
unsigned int app_a2dp_source_connect_device_test(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(CONNECT_DEVICE_TEST, (uint8_t *)buf, len);

    return 0;
}

extern uint8_t a2dp_source_pkt_sent_flag;
void a2dp_source_FPGA_send_sbc_packet(void);

#ifndef FPGA
void a2dp_source_send_sbc_timer_handler(void const *param);
osTimerDef(a2dp_source_send_sbc_timer, a2dp_source_send_sbc_timer_handler);
osTimerId a2dp_source_send_sbc_timer_id = NULL;
void a2dp_source_send_sbc_timer_handler(void const *param)
{
    a2dp_source_FPGA_send_sbc_packet();
}
#endif

static unsigned int app_a2dp_source_send_sbc_pkt_test(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(SEND_SBC_PKT_TEST, (uint8_t *)buf, len);

    return 0;
}

static unsigned int app_a2dp_source_toggle_stream_test(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(TOGGLE_STREAM_TEST, (uint8_t *)buf, len);

    return 0;
}

void fpga_a2dp_source_send_toggle_stream_timer_handler(void const *param);
osTimerDef(fpga_a2dp_source_send_toggle_stream_timer, fpga_a2dp_source_send_toggle_stream_timer_handler);
osTimerId fpga_a2dp_source_send_toggle_stream_timer_id = NULL;
void fpga_a2dp_source_send_toggle_stream_timer_handler(void const *param)
{
    app_a2dp_source_toggle_stream(app_bt_source_get_current_a2dp());
}

static unsigned int app_a2dp_source_sniff_test(unsigned char *buf, unsigned int len)
{
    source_rx_thread_post_msg(SOURCE_SNIFF_TEST, (uint8_t *)buf, len);

    return 0;
}
#endif

static const app_source_uart_handle_t app_source_uart_test_handle[]=
{
    {"search_device", app_a2dp_source_search_device},
    {"connect_device", app_a2dp_source_connect_device},
    {"mobile_LAURENT_threshold", mobile_LAURENT_threshold},
    {"hdr_old_corr_thr", hdr_old_corr_thr},
    {"mobile_rx_gain_sel", mobile_rx_gain_sel},
    {"mobile_rx_gain_idx", mobile_rx_gain_idx},
    {"fa_rx_gain_sel", fa_rx_gain_sel},
    {"fa_rx_gain_idx", fa_rx_gain_idx},
    {"fa_rx_lur_thr", fa_rx_lur_thr},
    {"fa_old_corr_thr", fa_old_corr_thr},
    {"ecc_block", ecc_block},
    {"ecc_enable", ecc_enable},
    {"mobile_tx_power", mobile_tx_power},
    {"fa_tx_power", fa_tx_power},
    {"fast_lock_on/off", fast_lock_on_off},
#ifdef A2DP_SOURCE_TEST
    {"connect_device_test", app_a2dp_source_connect_device_test},
    {"send_sbc_pkt_test", app_a2dp_source_send_sbc_pkt_test},
    {"toggle_stream_test", app_a2dp_source_toggle_stream_test},
    {"source_sniff_test", app_a2dp_source_sniff_test},
#endif
};
#endif  //__SOURCE_TRACE_RX__

static uint32_t a2dp_source_pcm_buffer_available_len();
void app_bt_source_set_a2dp_curr_stream(uint8_t device_id);
bool app_bt_source_check_sink_audio_activity(void);
void a2dp_source_reconfig_aac_device(void);
void a2dp_source_reconfig_lhdc_device(void);
#define A2DP_SOURCE_DATA_FRAME_NUM_WATER_LINE          (1)
static a2dp_source_packet_t *packet_sent = NULL;

static A2DP_SOURCE_STRUCT a2dp_source;
osMutexId a2dp_source_mutex_id = NULL;
osMutexDef(a2dp_source_mutex);

osMutexId a2dp_source_packet_list_mutex_id = NULL;
osMutexDef(a2dp_source_packet_list_mutex);
#ifdef A2DP_ENCODER_CROSS_CORE
static bool cc_encoder_init = false;
#endif
static void a2dp_source_packet_list_lock(void)
{
    osMutexWait(a2dp_source_packet_list_mutex_id, osWaitForever);
}

static void a2dp_source_packet_list_unlock(void)
{
    osMutexRelease(a2dp_source_packet_list_mutex_id);
}

static void a2dp_source_pcm_queue_lock(void)
{
    osMutexWait(a2dp_source_mutex_id, osWaitForever);
}

static void a2dp_source_pcm_queue_unlock(void)
{
    osMutexRelease(a2dp_source_mutex_id);
}

static void a2dp_source_sem_lock(a2dp_source_lock_t * lock)
{
    osSemaphoreWait(lock->_osSemaphoreId, osWaitForever);
}

static void a2dp_source_sem_unlock(a2dp_source_lock_t * lock)
{
    osSemaphoreRelease(lock->_osSemaphoreId);
}

static void a2dp_source_wait_pcm_data(void)
{
    a2dp_source_lock_t *lock = &(a2dp_source.data_lock);
    a2dp_source_sem_lock(lock);
}

static void a2dp_source_put_data(void)
{
    a2dp_source_lock_t *lock = &(a2dp_source.data_lock);
    a2dp_source_sem_unlock(lock);
}

int32_t a2dp_source_wait_sent(uint32_t timeout)
{
    int32_t ret = 0;
    a2dp_source_lock_t *lock = &(a2dp_source.sbc_send_lock);
    ret = osSemaphoreWait(lock->_osSemaphoreId, timeout);
    return ret;
}

void a2dp_source_notify_send(void)
{
    a2dp_source_lock_t *lock = &(a2dp_source.sbc_send_lock);
    a2dp_source_sem_unlock(lock);
}

int a2dp_source_pcm_buffer_read(uint8_t *buff, uint16_t len)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;
    int status = 0;
    static uint32_t read_count = 0;

    a2dp_source_pcm_queue_lock();
    status = PeekCQueue(&(a2dp_source.pcm_queue), len, &e1, &len1, &e2, &len2);
    if (len==(len1+len2))
    {
        memcpy(buff,e1,len1);
        memcpy(buff+len1,e2,len2);
        DeCQueue(&(a2dp_source.pcm_queue), 0, len);
        if (((++read_count) % 100) == 0)
        {
            TRACE(2, "pcm buff %d empty %d", a2dp_source.pcm_queue.len/len,
                    (a2dp_source.pcm_queue.size-a2dp_source.pcm_queue.len)/len);
        }
    }
    else
    {
        memset(buff, 0x00, len);
        status = -1;
    }
    a2dp_source_pcm_queue_unlock();
    return status;
}

void a2dp_clear_linein_pcm_buffer(void)
{
    a2dp_source_pcm_queue_lock();
    a2dp_source.pcm_queue.write = 0;
    a2dp_source.pcm_queue.len = 0;
    a2dp_source.pcm_queue.read = 0;
    a2dp_source_pcm_queue_unlock();
}

#ifdef A2DP_ENCODER_CROSS_CORE
static int a2dp_source_off_core_pcm_buffer_write(uint8_t *pcm_buf, uint16_t len)
{
    a2dp_encoder_bth_feed_pcm_data_into_off_core(pcm_buf, len);
    return 0;
}
#endif

POSSIBLY_UNUSED static void a2dp_source_pcm_buffer_mul8(uint8_t * pcm_buf, uint16_t len)
{
    int32_t* buff = (int32_t*)pcm_buf;
    for (int i = 0; i < len/4; i++) {
        buff[i] = buff[i] << 8;
    }
}

static int a2dp_source_pcm_buffer_write(uint8_t * pcm_buf, uint16_t len)
{
    CQueue *Q = &(a2dp_source.pcm_queue);
    if (AvailableOfCQueue(Q) < (int)len)
    {
        a2dp_clear_linein_pcm_buffer();
        TRACE(2, "buff data write avail_len=%d input_len=%d", AvailableOfCQueue(Q), len);
    }
    EnCQueue(Q, pcm_buf, len);
#ifdef A2DP_ENCODER_CROSS_CORE
    struct BT_DEVICE_T *curr_connected_dev = NULL;
    uint8_t curr_codec_type;
    uint8_t curr_a2dp_non_type;
    uint16_t pcm_size = 0;
    uint8_t* pcm_buff = 0;
    uint16_t check_len = 0;
    curr_connected_dev = app_bt_get_connected_sink_device();
    if (curr_connected_dev == NULL || curr_connected_dev->a2dp_conn_flag == false || !curr_connected_dev->a2dp_streamming)
    {
        TRACE(0, "source device NULL or not conn return");
        return -1;
    }
    curr_codec_type = curr_connected_dev->codec_type;
    curr_a2dp_non_type = curr_connected_dev->a2dp_non_type;
    //TRACE(0, "%s, %d", __func__, curr_a2dp_non_type);
    switch (curr_codec_type) {
        case BT_A2DP_CODEC_TYPE_NON_A2DP:
            if (0) {
            }
#ifdef  A2DP_SOURCE_LHDCV5_ON
            else if (curr_a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5) {
                pcm_buff = a2dp_source_lhdcv5_frame_buffer();
                pcm_size = A2DP_LHDCV5_TRANS_SIZE;
                check_len = A2DP_LHDCV5_TRANS_SIZE*2;
            }
#endif
#ifdef  A2DP_SOURCE_LDAC_ON
            else if (curr_a2dp_non_type == A2DP_SOURCE_NON_TYPE_LDAC) {
                pcm_buff = a2dp_source_ldac_frame_buffer();
                pcm_size = A2DP_LDAC_TRANS_SIZE*3;
                check_len = A2DP_LDAC_TRANS_SIZE*3;
            }
#endif
        break;
        case BT_A2DP_CODEC_TYPE_MPEG2_4_AAC:
            pcm_buff = a2dp_source_aac_frame_buffer();
            pcm_size = A2DP_AAC_TRANS_SIZE;
            check_len = A2DP_AAC_TRANS_SIZE;
        break;
        default:
        break;
    }
    if ((a2dp_source_pcm_buffer_available_len() >= check_len) && check_len && pcm_size && pcm_buff) {
        if (a2dp_source_pcm_buffer_read(pcm_buff, pcm_size) == 0)
        {
            //a2dp_source_pcm_buffer_mul8(pcm_buff, pcm_size); //todo
            a2dp_source_off_core_pcm_buffer_write(pcm_buff, pcm_size);
        }
    }
#endif
    return 0;
}

static uint32_t a2dp_source_pcm_buffer_available_len()
{
    uint32_t read_len = 0;
    a2dp_source_pcm_queue_lock();
    read_len=a2dp_source.pcm_queue.len;
    a2dp_source_pcm_queue_unlock();
    //TRACE(2,"%s %d",__func__,read_len);
    return read_len;
}

uint32_t a2dp_source_write_pcm_data(uint8_t * pcm_buf, uint32_t len)
{
    //TRACE(0, "a2dp_source_write_pcm_data len %d", len);
    a2dp_source_pcm_queue_lock();
    a2dp_source.is_encoded_packet = false;
    a2dp_source_pcm_buffer_write(pcm_buf, len);
    a2dp_source_pcm_queue_unlock();

    a2dp_source_put_data();
    return len;
}

void a2dp_source_usb_stream_start(void)
{
#ifdef APP_USB_A2DP_SOURCE
    usb_stream_status = STREAM_START;
#endif
    uint8_t curr_a2dp = app_bt_source_get_current_a2dp();
    TRACE(0, "%s curr_a2dp %d", __func__, curr_a2dp);
    app_a2dp_source_start_stream(curr_a2dp);
}

void a2dp_source_usb_stream_stop(void)
{
#ifdef APP_USB_A2DP_SOURCE
    usb_stream_status = STREAM_STOP;
#endif
    uint8_t curr_a2dp = app_bt_source_get_current_a2dp();
    TRACE(0, "%s curr_a2dp %d", __func__, curr_a2dp);
    app_a2dp_source_toggle_stream(curr_a2dp);
}

void a2dp_source_usb_stream_vol_change(uint32_t vol)
{
    uint8_t curr_a2dp = app_bt_source_get_current_a2dp();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(curr_a2dp);

    uint8_t bt_vol = a2dp_convert_local_vol_to_bt_vol(vol);

    if (curr_device == NULL)
    {
#ifdef APP_USB_A2DP_SOURCE
        usb_pre_vol = 0xFFFFFFFF;
#endif
        TRACE(0, "curr_device NULL");
        return;
    }

    if (curr_device->avrcp_conn_flag && curr_device->avrcp_channel)
    {
#ifdef APP_USB_A2DP_SOURCE
        if (usb_pre_vol == vol)
        {
            TRACE(0, "usb_pre_vol %d vol %d", usb_pre_vol, vol);
            return;
        }
        usb_pre_vol = vol;
#endif
        app_bt_start_custom_function_in_bt_thread((uint32_t)curr_device->avrcp_channel, (uint32_t)bt_vol, (uint32_t)(uintptr_t)btif_avrcp_ct_set_absolute_volume);
    }
    else
    {
        TRACE(0, "%s curr_a2dp %d avrcp_conn %d avrcp_channel %p bt_vol %d", __func__, curr_a2dp, curr_device->avrcp_conn_flag, curr_device->avrcp_channel, bt_vol);
    }
}

struct a2dp_encoded_data_t {
    uint8_t encoded_type;
    uint8_t *buffer;
    uint32_t buffer_length;
    uint16_t buffer_frame_len;
};

struct a2dp_encoded_packet_t {
    struct list_node node;
    struct a2dp_encoded_data_t parameters;
};

uint32_t a2dp_source_write_encoded_data(uint8_t encoded_type, uint8_t * pcm_buf, uint32_t len, uint16_t frame_len)
{
    struct a2dp_encoded_packet_t *a2dp_encoded_packet = NULL;
    a2dp_encoded_packet = (struct a2dp_encoded_packet_t *)bes_bt_buf_malloc(sizeof(struct a2dp_encoded_packet_t));
    if (!a2dp_encoded_packet)
    {
        TRACE(1, "%s alloc a2dp_encoded_packet failed", __func__);
        return 0;
    }
    a2dp_source.is_encoded_packet = true;
    a2dp_source_pcm_queue_lock();

    INIT_LIST_HEAD(&a2dp_encoded_packet->node);
    a2dp_encoded_packet->parameters.encoded_type     = encoded_type;
    a2dp_encoded_packet->parameters.buffer           = pcm_buf;
    a2dp_encoded_packet->parameters.buffer_length    = len;
    a2dp_encoded_packet->parameters.buffer_frame_len = frame_len;

    btif_colist_addto_tail(&a2dp_encoded_packet->node, &bt_source_manager.encoded_packet_list);
    a2dp_source_pcm_buffer_write(pcm_buf, len);
    a2dp_source_pcm_queue_unlock();
    a2dp_source_put_data();
    return len;
}

void app_bt_source_notify_a2dp_link_disconnected(bt_bdaddr_t *remote)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;
    device = app_bt_source_find_device(remote);

    if (device)
    {
        device->base_device->a2dp_streamming = false;
        device->base_device->a2dp_conn_flag = false;

        a2dp_source_stop_pcm_capture(device->base_device->device_id);
    }
}

#define A2DP_LINEIN_SIZE_OFFSET 1024*2

void app_a2dp_source_audio_init(void)
{
#ifdef A2DP_ENCODER_CROSS_CORE
#ifdef A2DP_ENCODER_CROSS_CORE_USE_M55
    a2dp_encoder_cc_sema_init();
    app_dsp_m55_init(APP_DSP_M55_USER_A2DP);
#endif
#endif

#ifdef APP_USB_A2DP_SOURCE
    usb_pre_vol = 0xFFFFFFFF;
#endif
    if (!bt_source_manager.config.av_enable)
    {
        return;
    }

    uint8_t device_id = app_bt_source_get_current_a2dp();
    struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);
    uint8_t *a2dp_linein_buff = NULL;
    app_audio_mempool_init();

    //get heap from app_audio_buffer
    app_audio_mempool_get_buff(&a2dp_linein_buff, A2DP_LINEIN_SIZE_OFFSET+A2DP_LINEIN_SIZE);
    InitCQueue(&a2dp_source.pcm_queue, A2DP_LINEIN_SIZE, ( CQItemType *)a2dp_linein_buff+A2DP_LINEIN_SIZE_OFFSET);
    TRACE(0,"app_a2dp_source_audio_init codec type=%d",curr_device->base_device->codec_type);

    if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
    {
#if defined(A2DP_SOURCE_LHDC_ON) ||  defined(A2DP_SOURCE_LHDCV5_ON)
        if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
        {
            TRACE(0, "aud_sample_rate %d sample_bit %d lossless %d", curr_device->aud_sample_rate, curr_device->base_device->sample_bit, curr_device->is_lossless_on);
            app_overlay_select(APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
            uint32_t bit_rate = 2000;
            if (curr_device->is_lossless_on)
            {
                bit_rate = 1400;
            }
            a2dp_source_lhdcv5_enc_set_config(curr_device->aud_sample_rate, curr_device->base_device->sample_bit, bit_rate, curr_device->is_lossless_on);
            a2dp_source_lhdcv5_encoder_init();
        }
        else if(curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDC)
        {
            a2dp_source_lhdc_encoder_init();
        }
        else
#endif
        {
            TRACE(0,"a2dp_non_type is wrong, plz check");
        }

    }
    else if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_SBC)
    {
        a2dp_source_sbc_encoder_init();
    }
}

void app_a2dp_source_audio_deinit(void)
{
#if defined(A2DP_DECODER_CROSS_CORE) || defined(SMF_A2DP_SOURCE_AAC_ON)
#ifdef A2DP_DECODER_CROSS_CORE
    a2dp_enoder_bth_stop_stream();
#endif
#ifdef A2DP_DECODER_CROSS_CORE_USE_M55
    app_dsp_m55_deinit(APP_DSP_M55_USER_A2DP);
    a2dp_encoder_cc_sema_deinit();
    cc_encoder_init = false;
#endif
#else //A2DP_DECODER_CROSS_CORE
    if (!bt_source_manager.config.av_enable)
    {
        return;
    }
#ifdef APP_USB_A2DP_SOURCE
    usb_pre_vol = 0xFFFFFFFF;
#endif
    if (!bt_source_manager.config.av_enable)
    {
        return;
    }

    uint8_t device_id = app_bt_source_get_current_a2dp();
    struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);

    if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
    {
        if (0)
        {
        }
#if defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
        else if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
        {
            a2dp_source_lhdcv5_encoder_deinit();
        }
        else if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDC)
        {
            lhdc_enc_deinit();
        }
#endif
#if defined(A2DP_SOURCE_LDAC_ON)
        else if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LDAC)
        {
            a2dp_source_ldac_encoder_deinit();
        }
#endif
        else
        {
            TRACE(0, "Err:a2dp_non_type is wrong, plz check");
        }
    }
    else if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_SBC)
    {
        a2dp_source_sbc_encoder_deinit();
    }
#if defined(A2DP_SOURCE_AAC_ON)
    else if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
    {
        aacenc_deinit();
    }
#endif
#endif //A2DP_DECODER_CROSS_CORE
    //clear buffer data
    a2dp_source.pcm_queue.write=0;
    a2dp_source.pcm_queue.len=0;
    a2dp_source.pcm_queue.read=0;
}


#if defined(APP_LINEIN_A2DP_SOURCE) || defined(APP_I2S_A2DP_SOURCE)
static uint32_t a2dp_source_linein_more_pcm_data(uint8_t * pcm_buf, uint32_t len)
{
    return a2dp_source_write_pcm_data(pcm_buf, len);
}
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)
//////////start the audio linein stream for capure the pcm data
int app_a2dp_source_linein_on(bool on)
{
    uint8_t *buff_play = NULL;
    struct AF_STREAM_CONFIG_T stream_cfg;
    static bool isRun =  false;
    uint8_t device_id = app_bt_source_get_current_a2dp();
    struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);

    TRACE(2,"app_a2dp_source_linein_on work:%d op:%d", isRun, on);

    if (isRun == on)
    {
        return 0;
    }

    if (on)
    {
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
#if defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
            uint8_t curr_device_a2dp_non_type = curr_device->base_device->a2dp_non_type;

            if (curr_device_a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
            {
                app_overlay_select(APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
            }
            else if (curr_device_a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDC)
            {
                app_overlay_select(APP_OVERLAY_A2DP_LHDC_ENCODER);
            }
            else
#endif
            {
                TRACE(0, "a2dp_non_type is wrong, plz check");
            }
        }
       app_a2dp_source_audio_init();

#if defined(BT_MULTI_SOURCE)
        if (app_bt_source_count_streaming_aac())
        {
            app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);
        }
        else
#endif
        {
            app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);
        }

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_32;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)curr_device->base_device->a2dp_channel_num;
        stream_cfg.sample_rate = curr_device->aud_sample_rate;

#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif

        stream_cfg.vol = 10;
        //stream_cfg.io_path = AUD_INPUT_PATH_HP_MIC;
        //stream_cfg.io_path =AUD_INPUT_PATH_MAINMIC;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1);
        stream_cfg.io_path = AUD_INPUT_PATH_LINEIN;
        stream_cfg.handler = a2dp_source_linein_more_pcm_data;

#ifdef A2DP_SOURCE_AAC_ON
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
        {
            app_overlay_select(APP_OVERLAY_A2DP_AAC);
            stream_cfg.data_size = BT_A2DP_SOURCE_AAC_LINEIN_BUFF_SIZE;
            app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_AAC_LINEIN_BUFF_SIZE);
            stream_cfg.data_ptr = buff_play;
        }
        else
#endif
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
            {
                stream_cfg.data_size = BT_A2DP_SOURCE_LHDCV5_LINEIN_BUFF_SIZE;
                app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_LHDCV5_LINEIN_BUFF_SIZE);
                stream_cfg.data_ptr = buff_play;
            }
            else
            {
                stream_cfg.data_size = BT_A2DP_SOURCE_LHDC_LINEIN_BUFF_SIZE;
                app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_LHDC_LINEIN_BUFF_SIZE);
                stream_cfg.data_ptr = buff_play;
            }
        }
        else
#endif
#ifdef A2DP_SOURCE_LDAC_ON
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_LDAC && curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LDAC)
        {
            //app_overlay_select(APP_OVERLAY_A2DP_AAC);
            stream_cfg.data_size = BT_A2DP_SOURCE_LDAC_LINEIN_BUFF_SIZE * 3;
            app_audio_mempool_get_buff(&buff_play, stream_cfg.data_size);
            stream_cfg.data_ptr = buff_play;
        }
        else
#endif
        {
            stream_cfg.data_size = BT_A2DP_SOURCE_SBC_LINEIN_BUFF_SIZE;
            app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_SBC_LINEIN_BUFF_SIZE);
            stream_cfg.data_ptr = buff_play;
        }

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

        TRACE(5,"app_source_linein_on on: channel %d sample_rate %d sample_bits %d bitrate %d vbr %d",
                curr_device->base_device->a2dp_channel_num, curr_device->aud_sample_rate,
                curr_device->base_device->sample_bit, curr_device->aud_bit_rate,
                curr_device->base_device->vbr_support);
        TRACE(5,"codec:%d/%d", curr_device->base_device->codec_type,curr_device->base_device->a2dp_non_type);
    }
    else
    {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        TRACE(0,"app_source_linein_on off");
        app_a2dp_source_audio_deinit();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
    }

    isRun=on;
    return 0;
}
#endif

#if defined(APP_I2S_A2DP_SOURCE)
int app_a2dp_source_I2S_onoff(bool onoff)
{
    static bool isRun =  false;
    uint8_t *buff_play = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;
    uint8_t device_id = app_bt_source_get_current_a2dp();
    struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);

    TRACE(2,"app_a2dp_source_I2S_onoff work:%d op:%d", isRun, onoff);

    if (isRun == onoff)
        return 0;

    if (onoff)
    {
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
            {
                TRACE(2,"%s:select overlay %d", __func__, APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
                app_overlay_select(APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
            }
            else
            {
                TRACE(2,"%s:select overlay %d", __func__, APP_OVERLAY_A2DP_LHDC_ENCODER);
                app_overlay_select(APP_OVERLAY_A2DP_LHDC_ENCODER);
            }
        }
#endif
        app_a2dp_source_audio_init();

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)curr_device->base_device->a2dp_channel_num;
        stream_cfg.sample_rate = curr_device->aud_sample_rate;

        stream_cfg.device = AUD_STREAM_USE_I2S0_SLAVE;
        stream_cfg.handler = a2dp_source_linein_more_pcm_data;
        //stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_cap_buff);

#ifdef A2DP_SOURCE_AAC_ON
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
        {
            app_overlay_select(APP_OVERLAY_A2DP_AAC);
            stream_cfg.data_size = BT_A2DP_SOURCE_AAC_LINEIN_BUFF_SIZE;
            app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_AAC_LINEIN_BUFF_SIZE);
            stream_cfg.data_ptr = buff_play;
        }
        else
#endif
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
        if (curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            if (curr_device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
            {
                TRACE(2,"%s:select overlay %d", __func__, APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
                app_overlay_select(APP_OVERLAY_A2DP_LHDC_V5_ENCODER);
                stream_cfg.data_size = BT_A2DP_SOURCE_LHDCV5_LINEIN_BUFF_SIZE;
                app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_LHDCV5_LINEIN_BUFF_SIZE);
                stream_cfg.data_ptr = buff_play;
            }
            else
            {
                TRACE(2,"%s:select overlay %d", __func__, APP_OVERLAY_A2DP_LHDC_ENCODER);
                app_overlay_select(APP_OVERLAY_A2DP_LHDC_ENCODER);
                stream_cfg.data_size = BT_A2DP_SOURCE_LHDC_LINEIN_BUFF_SIZE;
                app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_LHDC_LINEIN_BUFF_SIZE);
                stream_cfg.data_ptr = buff_play;
            }
        }
        else
#endif
        {
            stream_cfg.data_size = BT_A2DP_SOURCE_SBC_LINEIN_BUFF_SIZE;
            app_audio_mempool_get_buff(&buff_play, BT_A2DP_SOURCE_SBC_LINEIN_BUFF_SIZE);
            stream_cfg.data_ptr = buff_play;
        }

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    }
    else
    {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        app_a2dp_source_audio_deinit();
        app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
    }

    isRun = onoff;
    TRACE(1,"%s end!\n", __func__);
    return 0;
}
#endif

#define BT_SOURCE_MAX_CODEC_PACKET_BUFFER_SIZE (8)

static a2dp_source_packet_t g_source_codec_packets[BT_SOURCE_MAX_CODEC_PACKET_BUFFER_SIZE];

static void a2dp_source_clear_packet_list(void)
{
    a2dp_source_packet_t *packet_node = NULL;

    a2dp_source_packet_list_lock();

    while ((packet_node = (a2dp_source_packet_t *)btif_colist_get_head(&bt_source_manager.codec_packet_list)))
    {
        btif_colist_delete(&packet_node->node);
        memset(packet_node, 0, sizeof(a2dp_source_packet_t));
    }

    INIT_LIST_HEAD(&bt_source_manager.codec_packet_list);

    a2dp_source_packet_list_unlock();
}

void a2dp_source_clear_encoded_packet_list(void)
{
    struct a2dp_encoded_packet_t *a2dp_encoded_packet = NULL;

    a2dp_source_packet_list_lock();

    while ((a2dp_encoded_packet = (struct a2dp_encoded_packet_t *)btif_colist_get_head(&bt_source_manager.encoded_packet_list)))
    {
        btif_colist_delete(&(a2dp_encoded_packet->node));
        bes_bt_buf_free(a2dp_encoded_packet);
    }
    a2dp_source_packet_list_unlock();
}

static a2dp_source_packet_t *a2dp_source_get_free_packet(void)
{
    int i = 0;
    a2dp_source_packet_t *packet_node = NULL;
    a2dp_source_packet_t *found = NULL;

    a2dp_source_packet_list_lock();

    for (i = 0; i < BT_SOURCE_MAX_CODEC_PACKET_BUFFER_SIZE; i += 1)
    {
        packet_node = g_source_codec_packets + i;
        if (!packet_node->inuse)
        {
            found = packet_node;
            break;
        }
    }

    if (!found)
    {
        TRACE(1, "[%s] get free node error!!!", __func__);
        TRACE(0, "audio data tx overflow! discard packet!");
        packet_node = (a2dp_source_packet_t *)btif_colist_get_head(&bt_source_manager.codec_packet_list);
        btif_colist_delete(&packet_node->node);
        found = packet_node;
    }

    memset(found, 0, sizeof(a2dp_source_packet_t));
    found->packet.reserved_data_size = A2DP_CODEC_DATA_SIZE;
    found->packet.reserved_header_size = A2DP_CODEC_HEADER_SIZE;
    found->packet.data = found->buffer + found->packet.reserved_header_size;

    a2dp_source_packet_list_unlock();

    return found;
}

bool a2dp_source_packet_list_is_empty(void)
{
    bool is_empty = false;

    a2dp_source_packet_list_lock();

    is_empty = colist_is_list_empty(&bt_source_manager.codec_packet_list);

    a2dp_source_packet_list_unlock();

    return is_empty;
}

void a2dp_source_insert_packet_to_list(a2dp_source_packet_t *packet)
{
    a2dp_source_packet_list_lock();

    packet->inuse = true;

    btif_colist_addto_tail(&packet->node, &bt_source_manager.codec_packet_list);

    a2dp_source_packet_list_unlock();
}

static uint32_t a2dp_source_packet_list_length(void)
{
    a2dp_source_packet_list_lock();

    uint32_t len = 0;
    struct list_node* list_n= &bt_source_manager.codec_packet_list;
    len = btif_colist_item_count(list_n);

    a2dp_source_packet_list_unlock();
    return len;
}

static bool a2dp_source_stream_all_pending(void)
{
    int i = 0;
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    uint8_t stremaing_count = 0;
    uint8_t pending_count = 0;

    a2dp_source_packet_list_lock();

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_source_get_device(i);

        if (curr_device->base_device->a2dp_streamming)
        {
            stremaing_count += 1;
            if (curr_device->prev_packet_is_pending)
            {
                pending_count += 1;
            }
        }
    }

    a2dp_source_packet_list_unlock();

    return (pending_count == stremaing_count);
}

#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
static uint8_t source_lhdc_encoder_type = 0;
#endif
bool a2dp_source_encode_packet(uint8_t codec_type)
{
    a2dp_source_packet_t *packet_node = NULL;
    bool encoded_succ = false;

    if (!btif_check_l2cap_mtu_buffer_available())
    {
        TRACE(1, "[%s] l2cap mtu buffer not enough!", __func__);
        return false;
    }

    a2dp_source_packet_list_lock();

    if (a2dp_source_stream_all_pending())
    {
        TRACE(1, "[%s] prev packet pending", __func__);
        //goto false_return_label;
    }
#ifdef A2DP_ENCODER_CROSS_CORE
    if (!cc_encoder_init) {
        A2DP_AUDIO_CC_OUTPUT_CONFIG_T cc_config;
        uint8_t device_id = app_bt_source_get_current_a2dp();
        struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);
        uint8_t codec_type = curr_device->base_device->codec_type;
        uint16_t maxosize_pframe = AAC_OUT_SIZE;
        if (codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP) {
    #ifdef A2DP_SOURCE_LHDC_ON
            codec_type = A2DP_ENCODER_CODEC_TYPE_LHDC;
            maxosize_pframe = LHDCV5_OUT_SIZE;
    #else
            codec_type = A2DP_ENCODER_CODEC_TYPE_LDAC;
            maxosize_pframe = LDAC_OUT_SIZE;
    #endif
        }
        cc_config.sample_rate = curr_device->aud_sample_rate;
        cc_config.num_channels = 2;
        cc_config.bits_depth = curr_device->base_device->sample_bit;
        cc_config.frame_samples = 0;   //TODO
        TRACE(1, "current sample rate:%d/%d/%d", curr_device->aud_sample_rate, curr_device->base_device->sample_bit, 2);
        a2dp_encoder_bth_start_stream((A2DP_ENCODER_CODEC_TYPE_E)codec_type, &cc_config, 0xffff, maxosize_pframe);
        cc_encoder_init = true;
    }
#endif
    packet_node = a2dp_source_get_free_packet();
    switch (codec_type)
    {
#ifdef A2DP_SOURCE_AAC_ON
        case BT_A2DP_CODEC_TYPE_MPEG2_4_AAC:
#ifndef A2DP_ENCODER_CROSS_CORE
            if (a2dp_source_pcm_buffer_available_len() >= A2DP_AAC_TRANS_SIZE)
            {
                if (a2dp_source_pcm_buffer_read(a2dp_source_aac_frame_buffer(), A2DP_AAC_TRANS_SIZE) == 0)
                {
                    encoded_succ = a2dp_source_encode_aac_packet(packet_node);
                }
            }
#else
            encoded_succ = a2dp_source_encode_aac_packet(packet_node);
            packet_node->packet.frameNum = 1;
            packet_node->packet.frameSize = packet_node->packet.dataLen;
            packet_node->codec_type = BT_A2DP_CODEC_TYPE_MPEG2_4_AAC;
#endif
            break;
#endif
        case BT_A2DP_CODEC_TYPE_NON_A2DP:
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
#ifndef A2DP_ENCODER_CROSS_CORE
            if(source_lhdc_encoder_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
            {
                if (a2dp_source_pcm_buffer_available_len() >= (A2DP_LHDCV5_TRANS_SIZE*2))
                {
                    if (a2dp_source_pcm_buffer_read(a2dp_source_lhdcv5_frame_buffer(), A2DP_LHDCV5_TRANS_SIZE) == 0)
                    {
                        encoded_succ = a2dp_source_encode_lhdcv5_packet(packet_node);
                    }
                }
            }
            else
            {
                if (a2dp_source_pcm_buffer_available_len() >= (A2DP_LHDC_TRANS_SIZE*2)) //Todo: confirm the value of "A2DP_LHDC_TRANS_SIZE"
                {
                    encoded_succ = a2dp_source_encode_lhdc_packet(packet_node);
                }
            }
#else
            encoded_succ = a2dp_source_encode_lhdcv5_packet(packet_node);
            if (packet_node->packet.dataLen) {
                packet_node->packet.frameNum = packet_node->packet.dataLen/876;    //todo
                packet_node->packet.frameSize = packet_node->packet.dataLen/packet_node->packet.frameNum;
                packet_node->codec_type = BT_A2DP_CODEC_TYPE_LHDC;
            }
#endif
#endif
#if  defined(A2DP_SOURCE_LDAC_ON)
#ifndef A2DP_ENCODER_CROSS_CORE
            if (a2dp_source_pcm_buffer_available_len() >= (A2DP_LDAC_TRANS_SIZE))
            {
                if (a2dp_source_pcm_buffer_read(a2dp_source_ldac_frame_buffer(), A2DP_LDAC_TRANS_SIZE) == 0)
                {
                    encoded_succ = a2dp_source_encode_ldac_packet(packet_node);
                }
            }
#else
            encoded_succ = a2dp_source_encode_ldac_packet(packet_node);
            packet_node->packet.frameNum = 2;
            packet_node->packet.frameSize = packet_node->packet.dataLen/packet_node->packet.frameNum;
            packet_node->codec_type = BT_A2DP_CODEC_TYPE_LDAC;
#endif
#endif
            break;
        case BT_A2DP_CODEC_TYPE_SBC:
            if (a2dp_source_pcm_buffer_available_len() >= A2DP_SBC_TRANS_SIZE)
            {
                if (a2dp_source_pcm_buffer_read(a2dp_source_sbc_frame_buffer(), A2DP_SBC_TRANS_SIZE) == 0)
                {
                    encoded_succ = a2dp_source_encode_sbc_packet(packet_node);
                }
                else
                {
                    TRACE(2,"pcm_buffer_read != 0 !!!");
                }
            }

            break;
        default:
            TRACE(2,"%s invalid codec_type %d", __func__, codec_type);
            break;
    }

    if (!encoded_succ)
    {
        //TRACE(2, "[%s] encode failure codec_type=%d", __func__, codec_type);
        goto false_return_label;
    }

    a2dp_source_insert_packet_to_list(packet_node);

    a2dp_source_packet_list_unlock();
    return true;

false_return_label:
    //TRACE(1, "[%s] failure codec_type %d", __func__, codec_type);
    a2dp_source_packet_list_unlock();
    return false;
}

void a2dp_source_send_packet(uint32_t device_id)
{
    a2dp_source_packet_t *packet_node = NULL;
    a2dp_source_packet_t *list_head = NULL;
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    bt_status_t status = BT_STS_FAILED;
    uint32_t device_index = device_id - BT_SOURCE_DEVICE_ID_BASE;

    curr_device = app_bt_source_get_device(device_id);
    list_head = (a2dp_source_packet_t *)&bt_source_manager.codec_packet_list;

    if (!btif_check_l2cap_mtu_buffer_available())
    {
        TRACE(1, "[%s] l2cap buffer not enough", __func__);
        return;
    }

    if (!curr_device->base_device->a2dp_streamming)
    {
        TRACE(1, "[%s] a2dp streaming not start", __func__);
        return;
    }

    a2dp_source_packet_list_lock();

    if (curr_device->prev_packet_is_pending)
    {
        TRACE(1, "[%s] prev packet pending", __func__);
        goto return_label;
    }

    packet_node = (a2dp_source_packet_t *)(list_head->node.next);
    while (packet_node != list_head)
    {
        if (!packet_node->already_sent[device_index] && packet_node->codec_type == curr_device->base_device->codec_type)
        {
            packet_sent = packet_node;
            break;
        }
        packet_node = (a2dp_source_packet_t *)packet_node->node.next;
    }

    if (packet_sent == NULL)
    {
        TRACE(1, "[%s] list node empty", __func__);
        goto return_label;
    }

    if (packet_sent->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
    {
        status = btif_a2dp_stream_send_aac_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
    }
/*  reserved modify change 2024-11-28, need fixed send codec bug
    else if(packet_sent->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
    {
        if (0)
		{
		}
#if defined(A2DP_SOURCE_LHDCV5_ON)
        else if (packet_sent->codec_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
        {
            status = btif_a2dp_stream_send_lhdcv5_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
        }
#endif
#if  defined(A2DP_SOURCE_LHDC_ON)
        else if (packet_sent->codec_non_type == A2DP_SOURCE_NON_TYPE_LHDCV)
        {
            status = btif_a2dp_stream_send_lhdc_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
        }
#endif
#if  defined(A2DP_SOURCE_LDAC_ON)
	    else if(packet_sent->codec_non_type == BT_A2DP_CODEC_TYPE_LDAC)
	    {

	        status = btif_a2dp_stream_send_ldac_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
	    }
#endif
    }
    */
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
    else if(packet_sent->codec_type == BT_A2DP_CODEC_TYPE_LHDC)
    {
        if (source_lhdc_encoder_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
        {
            status = btif_a2dp_stream_send_lhdcv5_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
        }
        else
        {
            status = btif_a2dp_stream_send_lhdc_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
        }
    }
#endif
#if  defined(A2DP_SOURCE_LDAC_ON)
    else if(packet_sent->codec_type == BT_A2DP_CODEC_TYPE_LDAC)
    {

        status = btif_a2dp_stream_send_ldac_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
    }
#endif
    else
    {
        status = btif_a2dp_stream_send_sbc_packet(curr_device->base_device->a2dp_connected_stream, &packet_sent->packet, NULL);
    }

    if (status == BT_STS_PENDING)
    {
        packet_sent->already_sent[device_index] = true;
        curr_device->prev_packet_is_pending = true;
    }

return_label:
    a2dp_source_packet_list_unlock();
}

void a2dp_source_send_specified_packet(uint32_t device_id, a2dp_source_packet_t *packet_node)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    bt_status_t status = BT_STS_FAILED;

    curr_device = app_bt_source_get_device(device_id);

    if (!btif_check_l2cap_mtu_buffer_available())
    {
        return;
    }

    a2dp_source_packet_list_lock();

    if (curr_device->prev_packet_is_pending)
    {
        goto return_label;
    }

    if (packet_node->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
    {
        status = btif_a2dp_stream_send_aac_packet(curr_device->base_device->a2dp_connected_stream, &packet_node->packet, NULL);
    }
    else
    {
        status = btif_a2dp_stream_send_sbc_packet(curr_device->base_device->a2dp_connected_stream, &packet_node->packet, NULL);
    }

    if (status == BT_STS_PENDING)
    {
        curr_device->prev_packet_is_pending = true;
    }

return_label:
    a2dp_source_packet_list_unlock();

}

void a2dp_source_send_sbc_packet(void)
{
    a2dp_source_packet_t *packet_node = NULL;
    uint8_t device_id = app_bt_source_get_streaming_a2dp();

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        return;
    }

    packet_node = a2dp_source_get_free_packet();

    if (a2dp_source_encode_sbc_packet(packet_node))
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)device_id, (uint32_t)packet_node, (uint32_t)a2dp_source_send_specified_packet);
    }
}

#ifdef A2DP_SOURCE_AAC_ON
void a2dp_source_send_aac_packet(void)
{
    a2dp_source_packet_t *packet_node = NULL;
    uint8_t device_id = app_bt_source_get_streaming_a2dp();

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        return;
    }

    packet_node = a2dp_source_get_free_packet();

    if (a2dp_source_encode_aac_packet(packet_node))
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)device_id, (uint32_t)packet_node, (uint32_t)a2dp_source_send_specified_packet);
    }
}
#endif

#define LHDCV_BR_FIX1400  1     // fix the exception, temporary plan
static void a2dp_source_set_packet_tx_done(uint8_t device_id)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    a2dp_source_packet_list_lock();
    uint32_t device_index = device_id - BT_SOURCE_DEVICE_ID_BASE;

    curr_device = app_bt_source_get_device(device_id);

    if (packet_sent)
    {
        packet_sent->already_sent[device_index] = false;
#if (LHDCV_BR_FIX1400)
        if (packet_sent->inuse == false)
        {
            TRACE(1, "maybe delete in a2dp_source_get_free_packet");
        }
        else
        {
            packet_sent->inuse = false;
            btif_colist_delete(&packet_sent->node);
        }
#else
        packet_sent->inuse = false;
        btif_colist_delete(&packet_sent->node);
#endif
        packet_sent = NULL;
    }

    if (curr_device)
    {
        curr_device->prev_packet_is_pending = false;
    }
    a2dp_source_packet_list_unlock();

#if 1
    if (a2dp_source_packet_list_length() >= A2DP_SOURCE_DATA_FRAME_NUM_WATER_LINE) {
        TRACE(1, "[%s] continue send packet", __func__);
        a2dp_source_send_packet(device_id);
    }
#endif
}

void app_a2dp_source_clear_encoded_packet_list_and_pcm_buffer(void)
{
    a2dp_source_clear_encoded_packet_list();
    a2dp_clear_linein_pcm_buffer();
}

////////////////////////////creat the thread for send sbc data to a2dp sink device ///////////////////
static void a2dp_send_thread(const void *arg);

#if defined (A2DP_SOURCE_AAC_ON)
osThreadDef(a2dp_send_thread, osPriorityHigh, 1, 1024*10, "a2dp_send_thread");
#elif defined (A2DP_SOURCE_LHDC_ON) || defined (A2DP_SOURCE_LHDCV5_ON)
osThreadDef(a2dp_send_thread, osPriorityHigh, 1, 1024*6, "a2dp_send_thread");
#else
osThreadDef(a2dp_send_thread, osPriorityHigh, 1, 1024*2, "a2dp_send_thread");
#endif

static void a2dp_send_thread(const void *arg)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    int max_encoded_packets = 3;
    int i = 0;
    uint8_t encode_type = 0;
    uint8_t aac_count = 0;
    uint8_t sbc_count = 0;
    uint8_t lhdc_count = 0;
    uint8_t lhdcv5_count = 0;
    uint8_t ldac_count = 0;
    bool allow_send = false;

    while (1)
    {
        a2dp_source_wait_pcm_data();

        if (app_bt_source_count_streaming_a2dp() == 0)
        {
            a2dp_source_wait_sent(osWaitForever);
            continue;
        }

        allow_send = false; // firstly set false.
        app_sysfreq_req(APP_SYSFREQ_USER_A2DP_ENCODE, APP_SYSFREQ_208M);

        /**
         * PCM DMA data comes every 15ms, commonly it send 1 sbc packet per 15ms,
         * if there has air interfence or long range test and go back, it will
         * send 2 sbc packets per 15ms to consume the queued packets automatically.
         */

        if (a2dp_source.is_encoded_packet)
        {
    #ifndef SMF_A2DP_SOURCE_AAC_ON
            a2dp_source_reconfig_aac_device(); // should reconfig aac codec device to sbc
    #endif
            struct a2dp_encoded_packet_t *a2dp_encoded_packet = NULL;
            a2dp_source_packet_t *packet_node = NULL;
            packet_node = a2dp_source_get_free_packet();

            a2dp_encoded_packet = (struct a2dp_encoded_packet_t *)btif_colist_get_head(&bt_source_manager.encoded_packet_list);
            if (a2dp_encoded_packet && packet_node)
            {
                allow_send = true;
            }
            else
            {
                allow_send = false;
            }
            if (a2dp_source_pcm_buffer_available_len() >= a2dp_encoded_packet->parameters.buffer_length)
            {
                a2dp_source_pcm_buffer_read(packet_node->packet.data, a2dp_encoded_packet->parameters.buffer_length);
                packet_node->codec_type       = a2dp_encoded_packet->parameters.encoded_type;
                packet_node->packet.dataLen   = a2dp_encoded_packet->parameters.buffer_length;
                packet_node->packet.frameSize = a2dp_encoded_packet->parameters.buffer_frame_len;

                a2dp_source_packet_list_lock();
                a2dp_source_insert_packet_to_list(packet_node);
                btif_colist_delete(&(a2dp_encoded_packet->node));
                bes_bt_buf_free(a2dp_encoded_packet);
                a2dp_source_packet_list_unlock();
            }
        }
        else
        {
            sbc_count = app_bt_source_count_streaming_sbc();

            aac_count = app_bt_source_count_streaming_aac();

            lhdc_count = app_bt_source_count_streaming_lhdc();

            lhdcv5_count = app_bt_source_count_streaming_lhdcv5();

            ldac_count = app_bt_source_count_streaming_ldac();

            if (aac_count && sbc_count)
            {
                // break;
                a2dp_source_reconfig_aac_device(); // should reconfig aac codec device to sbc
                encode_type = BT_A2DP_CODEC_TYPE_SBC;

            }
            else if(lhdc_count&&sbc_count)
            {
                a2dp_source_reconfig_lhdc_device(); // should reconfig lhdc codec device to sbc
                encode_type = BT_A2DP_CODEC_TYPE_SBC;
            }
            else if(lhdcv5_count&&sbc_count)
            {
                a2dp_source_reconfig_lhdc_device(); // should reconfig lhdc codec device to sbc
                encode_type = BT_A2DP_CODEC_TYPE_SBC;
            }
            else if (aac_count)
            {
                encode_type = BT_A2DP_CODEC_TYPE_MPEG2_4_AAC;
            }
            else if (lhdc_count)
            {
                encode_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
                source_lhdc_encoder_type = A2DP_SOURCE_NON_TYPE_LHDC;
#endif
            }
            else if (lhdcv5_count)
            {
                encode_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
                source_lhdc_encoder_type = A2DP_SOURCE_NON_TYPE_LHDCV5;
#endif
            }
            else if (ldac_count)
            {
                encode_type = BT_A2DP_CODEC_TYPE_LDAC;
            }
            else
            {
                encode_type = BT_A2DP_CODEC_TYPE_SBC;
            }

            for (i = 0; i < max_encoded_packets; i += 1)
            {
                if (!a2dp_source_encode_packet(encode_type))
                {
                    break;
                }
                else
                {
                    allow_send = true;
                }
            }
        }

        // TRACE(1, "packet list len=%d,allow_send=%d", a2dp_source_packet_list_length(), allow_send);
        if ((a2dp_source_packet_list_length() >= A2DP_SOURCE_DATA_FRAME_NUM_WATER_LINE) && allow_send)
        {
            for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
            {
                curr_device = app_bt_source_get_device(i);

                if (curr_device->base_device->a2dp_streamming)
                {
                    app_bt_start_custom_function_in_bt_thread((uint32_t)i, (uint32_t)NULL, (uint32_t)a2dp_source_send_packet);
                }
            }
        }
    }
}


#ifdef A2DP_SOURCE_TEST
extern void a2dp_source_FPGA_send_sbc_packet(void);
uint8_t a2dp_source_pkt_sent_flag = 0;
#endif

#define BT_TPOLL_DEFAULT_INTERVAL 40

static void app_bt_set_tpoll_interval(uint16_t device_id, uint16_t interval)
{
    a2dp_stream_t *stream = NULL;
    btif_remote_device_t *btm_conn = NULL;
    uint16_t conn_handle = 0;
    stream = app_bt_source_get_device(device_id)->base_device->a2dp_connected_stream;
    if (stream)
    {
        btm_conn = btif_a2dp_get_remote_device(stream);
        conn_handle = btif_me_get_remote_device_hci_handle(btm_conn);
        //TRACE(3, "%s handle %04x interval %d", __func__, conn_handle, interval);
        bt_drv_reg_op_set_tpoll(conn_handle-0x80, interval);
    }
}

static void app_bt_get_tpoll_interval(uint16_t device_id)
{
    a2dp_stream_t *stream = NULL;
    btif_remote_device_t *btm_conn = NULL;
    uint16_t conn_handle = 0;
    uint16_t interval = 0;
    stream = app_bt_source_get_device(device_id)->base_device->a2dp_connected_stream;
    if (stream)
    {
        btm_conn = btif_a2dp_get_remote_device(stream);
        conn_handle = btif_me_get_remote_device_hci_handle(btm_conn);
        interval = bt_drv_reg_op_get_tpoll(conn_handle-0x80);
        TRACE(4, "(d%x) %s handle %04x interval %d", device_id, __func__, conn_handle, interval);
    }
}

void a2dp_source_start_pcm_capture(uint8_t device_id)
{
    bt_source_manager.curr_source_playing_a2dp = device_id;

    app_bt_source_set_a2dp_curr_stream(device_id);

    TRACE(3, "(d%x) %s input_on %d", device_id, __func__, bt_source_manager.a2dp_source_input_on);

    if (!bt_source_manager.a2dp_source_input_on)
    {
#if defined(APP_LINEIN_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
#elif defined(APP_I2S_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_I2S_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
#elif defined(APP_USB_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_USB_AUDIO, (uint8_t)APP_BT_SETTING_OPEN, 0);
#endif
        bt_source_manager.a2dp_source_input_on = true;

        a2dp_source_clear_packet_list();

        a2dp_source_notify_send();
    }
}

void a2dp_source_stop_pcm_capture(uint8_t device_id)
{
    uint8_t streaming_a2dp_count = app_bt_source_count_streaming_a2dp();

    if (bt_source_manager.curr_source_playing_a2dp == device_id)
    {
        bt_source_manager.curr_source_playing_a2dp = BT_SOURCE_DEVICE_INVALID_ID;
    }

    TRACE(4, "(d%x) %s input_on %d strming_a2dp_count %d", device_id, __func__, bt_source_manager.a2dp_source_input_on, streaming_a2dp_count);

    if (bt_source_manager.a2dp_source_input_on && streaming_a2dp_count == 0)
    {
#if defined(APP_LINEIN_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
#elif defined(APP_I2S_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_I2S_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
#elif defined(APP_USB_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_USB_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
#endif
        bt_source_manager.a2dp_source_input_on = false;

        a2dp_source_notify_send();
    }

    app_bt_source_set_a2dp_curr_stream(BT_DEVICE_AUTO_CHOICE_ID);
}


struct source_pts_set_configuration_info
{
    uint8_t channelMode;
    uint8_t bitPool;
    uint8_t allocMethod;
    uint8_t numBlocks;
    uint8_t numSubBands;
};
struct source_pts_set_configuration_info source_pts_config_info;

void a2dp_source_pts_set_configuration_info(uint8_t * codec_elements)
{
    if((codec_elements[1] & A2D_SBC_IE_BLOCKS_MSK) == A2D_SBC_IE_BLOCKS_4)
    {
        source_pts_config_info.numBlocks = 4;
    }
    else if((codec_elements[1] & A2D_SBC_IE_BLOCKS_MSK) == A2D_SBC_IE_BLOCKS_8)
    {
        source_pts_config_info.numBlocks = 8;
    }
    else if((codec_elements[1] & A2D_SBC_IE_BLOCKS_MSK) == A2D_SBC_IE_BLOCKS_12)
    {
        source_pts_config_info.numBlocks = 12;
    }
    else if((codec_elements[1] & A2D_SBC_IE_BLOCKS_MSK) == A2D_SBC_IE_BLOCKS_16)
    {
        source_pts_config_info.numBlocks = 16;
    }

    if((codec_elements[1] & A2D_SBC_IE_SUBBAND_MSK) == A2D_SBC_IE_SUBBAND_4)
    {
        source_pts_config_info.numSubBands = 4;
    }
    else if((codec_elements[1] & A2D_SBC_IE_SUBBAND_MSK) == A2D_SBC_IE_SUBBAND_8)
    {
        source_pts_config_info.numSubBands = 8;
    }

    if((codec_elements[1] & A2D_SBC_IE_ALLOC_MD_MSK) == A2D_SBC_IE_ALLOC_MD_S)
    {
        source_pts_config_info.allocMethod = 1;
    }
    else if((codec_elements[1] & A2D_SBC_IE_ALLOC_MD_MSK) == A2D_SBC_IE_ALLOC_MD_L)
    {
        source_pts_config_info.allocMethod = 0;
    }

    if((codec_elements[0] & A2D_SBC_IE_CH_MD_MSK) == A2D_SBC_IE_CH_MD_MONO)
    {
        source_pts_config_info.channelMode = 0;
    }
    else if((codec_elements[0] & A2D_SBC_IE_CH_MD_MSK) == A2D_SBC_IE_CH_MD_DUAL)
    {
        source_pts_config_info.channelMode = 1;
    }
    else if((codec_elements[0] & A2D_SBC_IE_CH_MD_MSK) == A2D_SBC_IE_CH_MD_STEREO)
    {
        source_pts_config_info.channelMode = 2;
    }
    else if((codec_elements[0] & A2D_SBC_IE_CH_MD_MSK) == A2D_SBC_IE_CH_MD_JOINT)
    {
        source_pts_config_info.channelMode = 3;
    }

    source_pts_config_info.bitPool = codec_elements[2];//min
    TRACE(0,"source_pts_config_info numBlocks=%d,allocMethod=%d,channelMode=%d,bitPool=%d,numSubBands=%d\n",
        source_pts_config_info.numBlocks,source_pts_config_info.allocMethod,source_pts_config_info.channelMode,
        source_pts_config_info.bitPool,source_pts_config_info.numSubBands);
}

uint8_t source_pts_sbc_buf[] =
{
    0x9c,0x31,0x10,0x34,0x31,0x11,0x00,0x00,0x7d,0xdb,0x7d,0xdb,0x75,0x9b,0x6e,0x5b,
};

extern void a2dp_source_pts_sbc_init(uint8_t *codec_info);
void a2dp_source_pts_send_sbc_packet(void)
{
    TRACE(0,"%s",__func__);
    uint8_t codec_info[5];
    memcpy(&codec_info[0],&source_pts_config_info,sizeof(codec_info));
    TRACE(0,"codec_info %d %d %d %d %d",codec_info[0],codec_info[1],codec_info[2],codec_info[3],codec_info[4]);
    a2dp_source_pts_sbc_init(codec_info);
    memcpy(a2dp_source_sbc_frame_buffer(), &source_pts_sbc_buf[0], sizeof(source_pts_sbc_buf));
    a2dp_source_send_sbc_packet();
}

#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
static bool app_a2dp_source_is_lhdcv5_encodec(uint8_t *codec_info)
{
    return (codec_info[0] == 0x3a && codec_info[1] == 0x05 &&
           codec_info[2] == 0x00 && codec_info[3] == 0x00 &&
           codec_info[4] == 0x35 && codec_info[5] == 0x4c);
}
static bool app_a2dp_source_is_lhdc_encodec(uint8_t *codec_info)
{
    return (codec_info[0] == 0x3a && codec_info[1] == 0x05 &&
           codec_info[2] == 0x00 && codec_info[3] == 0x00 &&
           codec_info[4] == 0x33 && codec_info[5] == 0x4c);
}
#endif

#if  defined(A2DP_SOURCE_LDAC_ON)
static bool app_a2dp_source_is_ldac_encodec(uint8_t *codec_info)
{
    return (codec_info[0] == 0x2d && codec_info[1] == 0x01 &&
           codec_info[2] == 0x00 && codec_info[3] == 0x00 &&
           codec_info[4] == 0xaa && codec_info[5] == 0x00);
}
#endif

void a2dp_source_callback(uint8_t device_id, a2dp_stream_t *Stream, const a2dp_callback_parms_t *info)
{
    app_bt_source_audio_event_param_t param;
    btif_a2dp_callback_parms_t * Info = (btif_a2dp_callback_parms_t *)info;
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON) || defined(A2DP_SOURCE_LDAC_ON)
    uint8_t elem_data;
#endif

    if (device_id == BT_DEVICE_INVALID_ID && Info->event == BTIF_A2DP_EVENT_STREAM_CLOSED)
    {
        // a2dp profile is closed due to acl created fail
        TRACE(2,"a2dp_source_callabck::A2DP_EVENT_STREAM_CLOSED acl created error=%x", Info->discReason);

        param.p.a2dp_source_connect_fail.addr = &Info->remote;
        param.p.a2dp_source_connect_fail.reason = Info->discReason;
        app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_SRC_CONNECT_FAIL, &param);
        return;
    }

    curr_device = app_bt_source_get_device(device_id);

    ASSERT(device_id >= BT_SOURCE_DEVICE_ID_BASE &&
           device_id < (BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM) &&
           curr_device->base_device->btif_a2dp_stream->a2dp_stream == Stream,
           "a2dp source device channel must match");

    if (BTIF_A2DP_EVENT_STREAM_PACKET_SENT != Info->event)
    {
        TRACE(3,"(d%x) %s event %d", device_id, __func__, Info->event);
    }

    switch(Info->event) {
        case BTIF_A2DP_REMOTE_NOT_SUPPORT:
            TRACE(1,"%s ::A2DP_REMOTE_NOT_SUPPORT", __func__);
            break;
        case BTIF_A2DP_EVENT_AVDTP_CONNECT: /* avdtp singal channel created */
            TRACE(2,"%s ::A2DP_EVENT_AVDTP_CONNECT %d", __func__, Info->event);
#if defined(TEST_OVER_THE_AIR_ENANBLED) && !defined(TOTA_v2)
            app_spp_tota_client_open(&curr_device->base_device->remote);
#endif
            break;
        case BTIF_A2DP_EVENT_CODEC_INFO: /* avdtp codec set configured */
            TRACE(3,"%s ::A2DP_EVENT_CODEC_INFO %d codec %d", __func__, Info->event, Info->p.codec->codecType);
            DUMP8("%02x ", Info->p.codec->elements, Info->p.codec->elemLen);
            break;
        case BTIF_A2DP_EVENT_STREAM_OPEN: /* avdtp stream channel created */
            {
                uint8_t *codec_elements = Info->p.codec->elements;

                TRACE(6,"%s ::A2DP_EVENT_STREAM_OPEN stream_id %x codec %d %02x BITPOOL:%d/%d",
                    __func__, device_id, Info->p.configReq->codec.codecType,
                    codec_elements[0], codec_elements[2], codec_elements[3]);

                a2dp_source_pts_set_configuration_info(codec_elements);

                if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
                {
                    curr_device->base_device->codec_type = BT_A2DP_CODEC_TYPE_MPEG2_4_AAC;
                    curr_device->base_device->sample_bit = 16;
                    // convert aac sample_rate to sbc sample_rate format
                    if (codec_elements[1] & A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100)
                    {
                        TRACE(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate 44100", device_id);
                        curr_device->base_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
                        curr_device->aud_sample_rate = AUD_SAMPRATE_44100;
                    }
                    else if (codec_elements[2] & A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000)
                    {
                        TRACE(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate 48000", device_id);
                        curr_device->base_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_48;
                        curr_device->aud_sample_rate = AUD_SAMPRATE_48000;
                    }
                    else
                    {
                        TRACE(1,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac sample_rate not 48000 or 44100, set to 44100", device_id);
                        curr_device->base_device->sample_rate = A2D_SBC_IE_SAMP_FREQ_44;
                        curr_device->aud_sample_rate = AUD_SAMPRATE_44100;
                    }

                    if (codec_elements[2] & A2DP_AAC_OCTET2_CHANNELS_1)
                    {
                        curr_device->base_device->a2dp_channel_num = 1;
                    }
                    else
                    {
                        curr_device->base_device->a2dp_channel_num = 2;
                    }

                    curr_device->aud_bit_rate = ((codec_elements[3] & 0x7f) << 16) | (codec_elements[4] << 8) | codec_elements[5];

                    curr_device->base_device->vbr_support = (codec_elements[3] & 0x80) ? true : false;

                    TRACE(3,"(d%x) ::A2DP_EVENT_STREAM_OPEN aac bit rate %d vbr %d", device_id,
                            curr_device->aud_bit_rate, curr_device->base_device->vbr_support);
                }
                else if(Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_NON_A2DP)
                {
                    TRACE(5,"%s ::A2DP_EVENT_STREAM_OPEN stream_id %x codec type LHDC, 0:%02X, 1:%02X, 2:%02X",
                    __func__, device_id, codec_elements[0], codec_elements[1], codec_elements[2]);
                    curr_device->base_device->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;

                    TRACE(6, "3:%02X, 4:%02X, 5:%02X, 6:%02X, 7:%02X, 8:%02X",
                    codec_elements[3], codec_elements[4], codec_elements[5],
                    codec_elements[6], codec_elements[7], codec_elements[8]);
                    if (0) {
                    }
#if  defined(A2DP_SOURCE_LHDC_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
                    else if (app_a2dp_source_is_lhdcv5_encodec(codec_elements))
                    {
                        curr_device->base_device->a2dp_non_type = A2DP_SOURCE_NON_TYPE_LHDCV5;
                        elem_data = A2DP_LHDCV5_SR_DATA(codec_elements[6]);
                        switch (elem_data)
                        {
                            case A2DP_LHDCV5_SR_192000: 
                                curr_device->aud_sample_rate = AUD_SAMPRATE_192000;
                                break;
                            case A2DP_LHDCV5_SR_96000 :
                                curr_device->aud_sample_rate = AUD_SAMPRATE_96000 ;
                                break;
                            case A2DP_LHDCV5_SR_48000 :
                                curr_device->aud_sample_rate = AUD_SAMPRATE_48000 ;
                                break;
                            case A2DP_LHDCV5_SR_44100 :
                                curr_device->aud_sample_rate = AUD_SAMPRATE_44100 ;
                                break;
                            default:
                                ASSERT(0, "sample_rate ERROR elem_data %d", elem_data);
                                break;
                        }

                        elem_data = A2DP_LHDCV5_FMT_DATA(codec_elements[7]);
                        switch (elem_data)
                        {
                            case A2DP_LHDCV5_FMT_32:
                                curr_device->base_device->sample_bit = 32 ;
                                break;
                            case A2DP_LHDCV5_FMT_24:
                                curr_device->base_device->sample_bit = 24 ;
                                break;
                            case A2DP_LHDCV5_FMT_16:
                                curr_device->base_device->sample_bit = 16 ;
                                break;
                            default:
                                ASSERT(0, "sample_bit ERROR elem_data %d", elem_data);
                                break;
                        }

                        elem_data = A2DP_LHDCV5_LOSSLESS_DATA(codec_elements[9]);
                        switch (elem_data)
                        {
                        case A2DP_LHDCV5_LOSSLESS_ON:
                            curr_device->is_lossless_on = A2DP_LHDCV5_LOSSLESS_ON;
                            break;
                        case A2DP_LHDCV5_LOSSLESS_OFF:
                            curr_device->is_lossless_on = A2DP_LHDCV5_LOSSLESS_OFF;
                            break;
                        default:
                            ASSERT(0, "lossless mode ERROR elem_data %d", elem_data);
                            break;
                        }
                        curr_device->base_device->a2dp_channel_num =2;
                    }
                    else if(app_a2dp_source_is_lhdc_encodec(codec_elements))
                    {
                        curr_device->base_device->a2dp_non_type = A2DP_SOURCE_NON_TYPE_LHDC;
                        curr_device->base_device->sample_bit = 16;
                        elem_data = A2DP_LHDC_FMT_DATA(codec_elements[6]);
                        if (elem_data == A2DP_LHDC_FMT_24)
                        {
                            curr_device->base_device->sample_bit = 24;
                        }

                        curr_device->base_device->a2dp_channel_num =2;

                        curr_device->aud_sample_rate = AUD_SAMPRATE_48000;
                        elem_data = A2DP_LHDC_SR_DATA(codec_elements[6]);
                        if (elem_data == A2DP_LHDC_SR_44100)
                        {
                            curr_device->aud_sample_rate = AUD_SAMPRATE_44100;
                        }

                        one_frame_per_chennal = false;

                        elem_data = A2DP_LHDC_COF_DATA(codec_elements[8]);

                        if (elem_data != A2DP_LHDC_COF_CSC_DISABLE)
                        {
                            one_frame_per_chennal = true;
                        }
                    }
#endif
#ifdef A2DP_SOURCE_LDAC_ON
                    else if(app_a2dp_source_is_ldac_encodec(codec_elements))
                    {
                        curr_device->base_device->a2dp_non_type = A2DP_SOURCE_NON_TYPE_LDAC;
                        elem_data = A2DP_LDAC_SR_DATA(codec_elements[6]);
                        switch (elem_data)
                        {
                        case A2DP_LDAC_SR_96000:
                            curr_device->aud_sample_rate = AUD_SAMPRATE_96000;
                            break;
                        case A2DP_LDAC_SR_88200:
                            curr_device->aud_sample_rate = AUD_SAMPRATE_88200;
                            break;
                        case A2DP_LDAC_SR_48000:
                            curr_device->aud_sample_rate = AUD_SAMPRATE_48000;
                            break;
                        case A2DP_LDAC_SR_44100:
                            curr_device->aud_sample_rate = AUD_SAMPRATE_44100;
                            break;
                        default:
                            ASSERT(0, "sample rate ERROR elem_data %d", elem_data);
                            break;
                        }
                        elem_data = A2DP_LDAC_CM_DATA(codec_elements[7]);
                        switch (elem_data)
                        {
                        case A2DP_LDAC_CM_MONO:
                            curr_device->base_device->a2dp_channel_num = 1;
                            break;
                        case A2DP_LDAC_CM_DUAL:
                            curr_device->base_device->a2dp_channel_num = 2;
                            break;
                        case A2DP_LDAC_CM_STEREO:
                            curr_device->base_device->a2dp_channel_num = 2;
                            break;
                        default:
                            ASSERT(0, "channel ERROR elem_data %d", elem_data);
                            break;
                        }
                        curr_device->base_device->sample_bit = 24;
                    }
#endif
                }
                else
                {
                    curr_device->base_device->codec_type = BT_A2DP_CODEC_TYPE_SBC;
                    curr_device->base_device->sample_bit = 16;
                    curr_device->base_device->sample_rate = (Info->p.configReq->codec.elements[0] & A2D_SBC_IE_SAMP_FREQ_MSK);
                    TRACE(0, "curr_device->base_device->sample_rate 0x%x", curr_device->base_device->sample_rate);
                    if(Info->p.configReq->codec.elements[0] & A2D_SBC_IE_CH_MD_MONO)
                    {
                        curr_device->base_device->a2dp_channel_num  = 1;
                    }
                    else
                    {
                        curr_device->base_device->a2dp_channel_num = 2;
                    }

                    if((Info->p.configReq->codec.elements[1] & A2D_SBC_IE_SUBBAND_MSK) == A2D_SBC_IE_SUBBAND_4)
                    {
                        TRACE(0,"numSubBands is only support 8!");
                    }

                    if (curr_device->base_device->sample_rate & A2D_SBC_IE_SAMP_FREQ_48)
                    {
                        curr_device->aud_sample_rate = AUD_SAMPRATE_48000;
                    }
                    else
                    {
                        curr_device->aud_sample_rate = AUD_SAMPRATE_44100;
                    }
                    TRACE(0, "aud_sample_rate 0x%x", curr_device->aud_sample_rate);
                }

                curr_device->base_device->a2dp_conn_flag = true;

                app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_OPEN, NULL);
#ifndef APP_USB_A2DP_SOURCE
                if (Info->start_stream_already_sent)
                {
                    TRACE(0, "already streaming, break direactly");
                    break;
                }
#endif
#if defined(BT_WATCH_APP)
                if (app_bt_source_check_sink_audio_activity())
                {
                    curr_device->a2dp_paused_by_sink = true;
                }
                else if (app_bt_source_count_streaming_sco())
                {
                    curr_device->a2dp_paused_by_ag_sco = true;
                }
#else
#ifndef BQB_PROFILE_TEST
#ifdef APP_USB_A2DP_SOURCE
                if (usb_stream_status == STREAM_START)
                {
                    //BQB TEST, No need send start cmd when A2DP OPEN
                    app_a2dp_source_start_stream(device_id);
                }
                usb_pre_vol = 0xFFFFFFFF;  // set usb_pre_vol to an invalid value
// #elif defined(APP_LINEIN_A2DP_SOURCE)
//                 app_a2dp_source_linein_on(true);
//                 app_a2dp_source_start_stream(device_id);
#endif
#ifdef A2DP_SOURCE_AUTO_START
                //BQB TEST, No need send start cmd when A2DP OPEN
                app_a2dp_source_start_stream(device_id);
#endif
#endif
#endif
            }
            break;
        case BTIF_A2DP_EVENT_STREAM_RECONFIG_IND: /* avdtp codec is reconfigured */
            TRACE(3,"%s ::A2DP_EVENT_STREAM_RECONFIG_IND %d codec %d", __func__, Info->event, Info->p.configReq->codec.codecType);
            if (Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
            {
                TRACE(1,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND ##AAC", device_id);
            #if defined(A2DP_AAC_ON)
                if (((Info->p.codec->elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED) &&
                    (((a2dp_codec_aac_elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED) == 0))
            #else
                if ((Info->p.codec->elements[3]) & A2DP_AAC_OCTET3_VBR_SUPPORTED)
            #endif
                {
                    Info->error = BTIF_A2DP_ERR_NOT_SUPPORTED_VBR;
                    TRACE(1,"(d%x) stream reconfig: VBR  UNSUPPORTED!!!!!!", device_id);
                }
            }
            else if(Info->p.configReq->codec.codecType == BT_A2DP_CODEC_TYPE_SBC)
            {
                TRACE(5,"(d%x) ::A2DP_EVENT_STREAM_RECONFIG_IND ##SBC", device_id);
            }
            break;
        case BTIF_A2DP_EVENT_STREAM_STARTED:
            app_bt_conn_stop_sniff(curr_device->base_device->acl_conn_hdl);
            app_bt_stay_active_rem_dev(curr_device->base_device->acl_conn_hdl);

            curr_device->base_device->a2dp_streamming = true;
            curr_device->base_device->a2dp_play_pause_flag = 1;
            curr_device->base_device->avrcp_playback_status = 1;

            avrcp_target_send_play_status_change_notify(device_id);

            TRACE(3,"%s ::A2DP_EVENT_STREAM_STARTED %d source device %x", __func__,
                    curr_device->base_device->codec_type, device_id);

            app_bt_get_tpoll_interval(device_id);

            app_bt_set_tpoll_interval(device_id, BT_TPOLL_DEFAULT_INTERVAL);

            app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_START, NULL);
            break;
        case BTIF_A2DP_EVENT_STREAM_IDLE:
            TRACE(1,"%s ::A2DP_EVENT_STREAM_IDLE", __func__);
            // fallthrough
        case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
            app_a2dp_source_clear_encoded_packet_list_and_pcm_buffer();
            TRACE(3,"%s ::A2DP_EVENT_STREAM_SUSPENDED %d device %x", __func__,
                                        curr_device->base_device->codec_type, device_id);

            curr_device->base_device->a2dp_streamming = false;

            app_bt_allow_sniff_rem_dev(curr_device->base_device->acl_conn_hdl);

            curr_device->base_device->a2dp_play_pause_flag = 0;
            curr_device->base_device->avrcp_playback_status = 2;

            avrcp_target_send_play_status_change_notify(device_id);

            app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_SUSPEND, NULL);
            break;
        case BTIF_A2DP_EVENT_STREAM_CLOSED:
            app_a2dp_source_clear_encoded_packet_list_and_pcm_buffer();
            TRACE(2,"%s ::A2DP_EVENT_STREAM_CLOSED device %d", __func__, device_id);
            curr_device->base_device->a2dp_streamming = false;
            curr_device->base_device->a2dp_play_pause_flag = 0;
            curr_device->is_lossless_on = 0;
            if (btif_a2dp_is_disconnected(Stream))
            {
                curr_device->base_device->a2dp_conn_flag = false;
                curr_device->base_device->avrcp_playback_status = 0;

                app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_CLOSE, NULL);

                if (curr_device->base_device->avrcp_conn_flag)
                {
                    app_bt_disconnect_avrcp_profile(curr_device->base_device->avrcp_channel);
                }
            }
            a2dp_source_set_packet_tx_done(device_id);
            break;
        case BTIF_A2DP_EVENT_STREAM_DATA_IND:
            /* there is no stream data ind for a2dp source */
            break;
        case BTIF_A2DP_EVENT_STREAM_PACKET_SENT:
            //TRACE(1,"%s ::A2DP_EVENT_STREAM_PACKET_SENT\n", __func__);
            a2dp_source_set_packet_tx_done(device_id);
            if (curr_device->base_device->a2dp_streamming) {
                a2dp_source_notify_send();
            }
#if defined(A2DP_SOURCE_TEST)
           if(a2dp_source_pkt_sent_flag)
            {
                osDelay(10);
#ifndef BT_SOURCE
                a2dp_source_FPGA_send_sbc_packet();
#endif
            }
#endif
            break;
#ifdef __A2DP_AVDTP_CP__
        case BTIF_A2DP_EVENT_CP_INFO:
            TRACE(4, "%s ::A2DP_EVENT_CP_INFO %d cpType %02x len %02x", __func__, Info->event,
                Info->p.cp ? Info->p.cp->cpType : 0xff, Info->p.cp ? Info->p.cp->dataLen : 0xff);
            if(Info->p.cp && Info->p.cp->cpType == BTIF_AVDTP_CP_TYPE_SCMS_T)
            {
                curr_device->base_device->avdtp_cp = true;
            }
            else
            {
                curr_device->base_device->avdtp_cp = false;
            }
            btif_a2dp_set_copy_protection_enable(Stream, curr_device->base_device->avdtp_cp);
            break;
        case BTIF_A2DP_EVENT_STREAM_SECURITY_IND:
            TRACE(2,"%s ::A2DP_EVENT_STREAM_SECURITY_IND %d", __func__, Info->event);
            DUMP8("%02x ",Info->p.data,Info->len);
            btif_a2dp_security_control_rsp(Stream,&Info->p.data[1],Info->len-1,Info->error);
            break;
        case BTIF_A2DP_EVENT_STREAM_SECURITY_CNF:
            TRACE(2,"%s ::A2DP_EVENT_STREAM_SECURITY_CNF %d", __func__, Info->event);
            break;
#endif
        default:
            break;
    }

    app_a2dp_bt_driver_callback(device_id, Info->event);
}

uint8_t a2dp_source_get_play_status(bt_bdaddr_t* remote_addr)
{
    int i = 0;
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_source_get_device(i);
        if (memcmp(remote_addr, &curr_device->base_device->remote, sizeof(bt_bdaddr_t)) == 0)
        {
            if (curr_device->base_device->a2dp_conn_flag)
            {
                return curr_device->base_device->a2dp_streamming ? 1 : 2;
            }
        }
    }

    return 0;
}

extern void a2dp_init(void);

#ifdef APP_USB_A2DP_SOURCE
static const USB_AUDIO_SOURCE_EVENT_CALLBACK_T a2dp_usb_audio_stream_func_cb_list =
{
    .init_cb = app_a2dp_source_audio_init,
    .deinit_cb = app_a2dp_source_audio_deinit,
    .data_playback_cb = a2dp_source_write_pcm_data,
    .data_capture_cb = NULL,
    .data_prep_cb   = NULL,
    .playback_start_cb = a2dp_source_usb_stream_start,
    .playback_stop_cb = a2dp_source_usb_stream_stop,
    .capture_start_cb = NULL,
    .capture_stop_cb = NULL,
    .playback_vol_change_cb = a2dp_source_usb_stream_vol_change,
};

void app_a2dp_set_a2dp_source()
{
    usb_audio_source_config_init(&a2dp_usb_audio_stream_func_cb_list);
    TRACE(0,"register a2dp as the usb audio success!");
}
#endif

#if defined(mHDT_SUPPORT)
#define MHDT_RATE_EDR4    0x08
#define MHDT_RATE_EDR6    0x10
#define MHDT_RATE_EDR8    0x20

void app_a2dp_source_enter_mhdt_mode(uint8 tx_rates,uint8 rx_rates)
{
    struct BT_DEVICE_T *curr_connected_dev = NULL;
    uint8 tx_rates_parm = 0;
    uint8 rx_rates_parm = 0;
    // Parameter validity check
    if (((tx_rates != 4) && (tx_rates != 6) && (tx_rates != 8)) ||
        ((rx_rates != 4) && (rx_rates != 6) && (rx_rates != 8)))
    {
        TRACE(0, "rate_error tx_rate: %d rx_rate: %d", tx_rates, rx_rates);
        return;
    }
    curr_connected_dev = app_bt_get_connected_sink_device();
    if (curr_connected_dev == NULL)
    {
        TRACE(0, "no connected sink device");
        return;
    }

    switch (tx_rates)
    {
    case 4:
        tx_rates_parm = MHDT_RATE_EDR4;
        break;
    case 6:
        tx_rates_parm = MHDT_RATE_EDR6;
        break;
    case 8:
        tx_rates_parm = MHDT_RATE_EDR8;
        break;
    default:
        TRACE(0, "tx_rate error");
        break;
    }

    switch (rx_rates)
    {
    case 4:
        rx_rates_parm = MHDT_RATE_EDR4;
        break;
    case 6:
        rx_rates_parm = MHDT_RATE_EDR6;
        break;
    case 8:
        rx_rates_parm = MHDT_RATE_EDR8;
        break;
    default:
        TRACE(0, "rx_rate error");
        break;
    }

    TRACE(0, "d(%x) enter mhdt_mode", curr_connected_dev->device_id);
    btif_me_mhdt_enter_mhdt_mode(curr_connected_dev->btm_conn, tx_rates_parm, rx_rates_parm);
}

void app_a2dp_source_exit_mhdt_mode(void)
{
    struct BT_DEVICE_T *curr_connected_dev = NULL;
    curr_connected_dev = app_bt_get_connected_sink_device();
    if (curr_connected_dev == NULL)
    {
        TRACE(0, "no connected sink device");
        return;
    }
    TRACE(0, "d(%x) exit mhdt_mode", curr_connected_dev->device_id);
    btif_me_mhdt_exit_mhdt_mode(curr_connected_dev->btm_conn);
}
#endif

#if defined(mHDT_SUPPORT) && defined(A2DP_SOURCE_LHDCV5_ON)
void app_a2dp_source_mhdt_mode_change_callback(struct bdaddr_t remote, bool isIn_mhdt_mode)
{
    TRACE(0, "app_a2dp_source_mhdt_mode_change_callback");
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    uint8_t curr_codec_type;
    uint8_t curr_a2dp_non_type;
    curr_device = app_bt_source_find_device(&remote);
    if (curr_device == NULL || curr_device->base_device->a2dp_conn_flag == false)
    {
        TRACE(0, "source device NULL or not conn return");
        return;
    }

    curr_codec_type = curr_device->base_device->codec_type;
    curr_a2dp_non_type = curr_device->base_device->a2dp_non_type;
    if (isIn_mhdt_mode) // need reconfig to LHDCV5
    {
        if (curr_codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP && curr_a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5)
        {
            TRACE(0, "curr codec LHDCV5, no need reconfig");
            return;
        }
        else
        {
            app_bt_a2dp_reconfig_to_vendor_codec(curr_device->base_device->a2dp_connected_stream, BT_A2DP_CODEC_TYPE_NON_A2DP, A2DP_SOURCE_NON_TYPE_LHDCV5);
        }
    }
    else    // need to reconfig to aac
    {
        if (curr_codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
        {
             TRACE(0, "curr codec AAC, no need reconfig");
        }
        else
        {
            app_bt_a2dp_reconfig_to_aac(curr_device->base_device->a2dp_connected_stream);
        }
    }
}
#endif

void app_a2dp_source_init(void)
{
    TRACE(1, "%s", __func__);
    int i = 0;
    struct BT_SOURCE_DEVICE_T* device = NULL;
    btif_avdtp_content_prot_t *cp = NULL;
    a2dp_source_lock_t *lock = NULL;

    if (bt_source_manager.config.av_enable)
    {
#if defined(mHDT_SUPPORT) && defined(A2DP_SOURCE_LHDCV5_ON)
        btif_register_mhdt_mode_change_callback(app_a2dp_source_mhdt_mode_change_callback);
#endif
        a2dp_init();

#ifdef __SOURCE_TRACE_RX__
        app_source_trace_rx_handler_init();
        app_trace_rx_open();
        for(uint8_t index = 0; index < ARRAY_SIZE(app_source_uart_test_handle); index++)
        {
            app_trace_rx_register(
                app_source_uart_test_handle[index].name,
                app_source_uart_test_handle[index].function);
        }
#endif

        for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            device = app_bt_source_get_device(i);

            device->base_device->btif_a2dp_stream = btif_a2dp_alloc_source_stream();
            device->base_device->profile_mgr.stream = device->base_device->btif_a2dp_stream->a2dp_stream;
            device->base_device->a2dp_connected_stream = device->base_device->btif_a2dp_stream->a2dp_stream;
            device->base_device->channel_mode = 0;
            device->base_device->a2dp_channel_num = 1;
            device->base_device->a2dp_conn_flag = 0;
            device->base_device->a2dp_streamming = 0;
            device->base_device->a2dp_play_pause_flag = 0;
            device->base_device->play_status_notify_registered = false;
            device->aud_sample_rate = AUD_SAMPRATE_44100;
            device->aud_bit_rate = 96000;

#ifdef __A2DP_AVDTP_CP__
            device->base_device->avdtp_cp = 0;
            memset(device->base_device->a2dp_avdtp_cp_security_data, 0, sizeof(device->base_device->a2dp_avdtp_cp_security_data));
            device->base_device->a2dp_avdtp_cp.cpType = BTIF_AVDTP_CP_TYPE_SCMS_T;
            device->base_device->a2dp_avdtp_cp.data = device->base_device->a2dp_avdtp_cp_security_data;
            device->base_device->a2dp_avdtp_cp.dataLen = 0;
            cp = &device->base_device->a2dp_avdtp_cp;
#else
            cp = NULL;
#endif

            btif_a2dp_stream_init(device->base_device->btif_a2dp_stream, BTIF_A2DP_STREAM_TYPE_SOURCE);

            a2dp_source_register_sbc_codec(device->base_device->btif_a2dp_stream, cp, 0, a2dp_source_callback);

#ifdef A2DP_SOURCE_AAC_ON
            a2dp_source_register_aac_codec(device->base_device->btif_a2dp_stream, cp, 1, a2dp_source_callback);
#endif
#ifdef A2DP_SOURCE_LHDC_ON//if aac open,seq priority should set 2
            a2dp_source_register_lhdc_codec(device->base_device->btif_a2dp_stream, cp, 1, a2dp_source_callback);
#endif
#ifdef A2DP_SOURCE_LHDCV5_ON//if aac open,seq priority should set 2
            a2dp_source_register_lhdcv5_codec(device->base_device->btif_a2dp_stream, cp, 2, a2dp_source_callback);
#endif
#ifdef A2DP_SOURCE_LDAC_ON
            a2dp_source_register_ldac_codec(device->base_device->btif_a2dp_stream, cp, 3, a2dp_source_callback);
#endif
        }
    }

    if(a2dp_source_mutex_id == NULL)
    {
        a2dp_source_mutex_id = osMutexCreate((osMutex(a2dp_source_mutex)));
    }

    if(a2dp_source_packet_list_mutex_id == NULL)
    {
        a2dp_source_packet_list_mutex_id = osMutexCreate((osMutex(a2dp_source_packet_list_mutex)));
    }

    lock = &(a2dp_source.data_lock);
    memset(lock, 0, sizeof(a2dp_source_lock_t));
#ifdef CMSIS_OS_RTX
    lock->_osSemaphoreDef.semaphore = lock->_semaphore_data;
#endif
    lock->_osSemaphoreId = osSemaphoreCreate(&(lock->_osSemaphoreDef), 0);
    lock = &(a2dp_source.sbc_send_lock);
    memset(lock, 0, sizeof(a2dp_source_lock_t));
#ifdef CMSIS_OS_RTX
    lock->_osSemaphoreDef.semaphore = lock->_semaphore_data;
#endif
    lock->_osSemaphoreId = osSemaphoreCreate(&(lock->_osSemaphoreDef), 0);
    a2dp_source.sbc_send_id = osThreadCreate(osThread(a2dp_send_thread), NULL);
#if defined(APP_USB_A2DP_SOURCE)
    usb_audio_source_config_init(&a2dp_usb_audio_stream_func_cb_list);
#endif
}

void a2dp_source_reconfig_aac_device(void)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    int i = 0;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_source_get_device(i);
        if (curr_device->base_device->a2dp_streamming && curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
        {
            app_bt_a2dp_reconfig_to_sbc(curr_device->base_device->a2dp_connected_stream);
        }
    }
}

void a2dp_source_reconfig_lhdc_device(void)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    int i = 0;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        curr_device = app_bt_source_get_device(i);
        if (curr_device->base_device->a2dp_streamming && curr_device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            app_bt_a2dp_reconfig_to_sbc(curr_device->base_device->a2dp_connected_stream);
        }
    }
}

void app_a2dp_source_start_stream(uint8_t device_id)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    curr_device = app_bt_source_get_device(device_id);
    TRACE(0, "(d%x) %s a2dp_conn_flag %d a2dp_streaming %d", device_id, __func__, curr_device->base_device->a2dp_conn_flag, curr_device->base_device->a2dp_streamming);
    if (curr_device->base_device->a2dp_conn_flag && !curr_device->base_device->a2dp_streamming)
    {
        btif_a2dp_start_stream(curr_device->base_device->a2dp_connected_stream);
    }
}

void app_a2dp_source_suspend_stream(uint8_t device_id)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    TRACE(2,"(d%x) %s", device_id, __func__);
    curr_device = app_bt_source_get_device(device_id);
    if (curr_device->base_device->a2dp_conn_flag && curr_device->base_device->a2dp_streamming)
    {
        if (BT_STS_PENDING == btif_a2dp_suspend_stream(curr_device->base_device->a2dp_connected_stream))
        {
            curr_device->base_device->a2dp_streamming = false;
            curr_device->base_device->a2dp_play_pause_flag = 0;
            curr_device->base_device->avrcp_playback_status = 2;
        }
    }
}

void app_a2dp_source_toggle_stream(uint8_t device_id)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    curr_device = app_bt_source_get_device(device_id);

    if (curr_device->base_device->a2dp_conn_flag)
    {
        if (curr_device->base_device->a2dp_streamming)
        {
            app_a2dp_source_suspend_stream(device_id);
        }
        else
        {
            app_a2dp_source_start_stream(device_id);
        }
    }
}

#ifdef __SOURCE_TRACE_RX__
extern void bt_drv_reg_op_swagc_mode_set(uint8_t mode);
extern "C" void btdrv_fa_config_rx_gain(bool ecc_rx_gain_en, uint8_t ecc_rx_gain_idx);
extern "C" void btdrv_fa_new_mode_corr_thr(uint8_t corr_thr);
extern "C" void btdrv_ecc_enable(bool enable);
extern "C" void btdrv_fa_old_mode_corr_thr(uint8_t corr_thr);
extern "C" void btdrv_fast_lock_config(bool fastlock_on);
extern void bt_source_device_search_callback(bt_bdaddr_t *remote_addr);
extern void set_bt_search_address(bt_bdaddr_t address);
extern void set_bt_search_device_searched(bool device_searched);
extern device_info_t get_device_info_by_index(uint32_t index);

static void trace_rx_thread_handler(SOURCE_RX_MSG* rx_msg)
{
    SOURCE_TRACE_RX_COMMAND commad = (SOURCE_TRACE_RX_COMMAND)rx_msg->command;
    TRACE(2, "func:[%s],command:%d", __func__, commad);

    const char *command_param = (char *)rx_msg->param;

    switch(commad)
    {
        case SEARCH_DEVIECE:
        {
            APP_KEY_STATUS key_status;
            key_status.code = 1;
            key_status.event = 8;
            bt_key_send(&key_status);
            app_bt_source_search_device();

            break;
        }
        case CONNECT_DEVICE:
        {
            uint32_t device_index = 0;
            device_info_t selected_device;

            sscanf(command_param, "%d", &device_index);

            selected_device = get_device_info_by_index(device_index);

            set_bt_search_address(selected_device.addr);
            set_bt_search_device_searched(true);

            bt_source_reconnect_a2dp_profile(&selected_device.addr);
            break;
        }
        case MOBILE_LAURENT_THRESHOLD:
        {
            break;
        }
        case HDR_OLD_CORR_THR:
        {
            break;
        }
        case MOBILE_RX_GAIN_SEL:
        {
            uint32_t mode;
            sscanf(command_param, "%d", &mode);
            bt_drv_reg_op_swagc_mode_set(mode);
            break;
        }
        case MOBILE_RX_GAIN_IDX:
        {
            break;
        }
        case FA_RX_GAIN_SEL:
        case FA_RX_GAIN_IDX:
        {
            uint32_t ecc_rx_gain_en;
            uint32_t ecc_rx_gain_idx;

            sscanf(command_param, "%d,%d", &ecc_rx_gain_en, &ecc_rx_gain_idx);
            btdrv_fa_config_rx_gain((bool)ecc_rx_gain_en, ecc_rx_gain_idx);
            break;
        }
        case FA_RX_LUR_THR:
        {
            uint32_t corr_thr;

            sscanf(command_param, "%d", &corr_thr);
            btdrv_fa_new_mode_corr_thr(corr_thr);
            break;
        }
        case FA_OLD_CORR_THR:
        {
            uint32_t corr_thr;

            sscanf(command_param, "%d", &corr_thr);
            btdrv_fa_old_mode_corr_thr(corr_thr);
            break;
        }
        case ECC_BLOCK:
        case ECC_ENABLE:
        {
#ifdef __FASTACK_ECC_ENABLE__
            btdrv_ecc_enable(true);
#endif
            break;
        }
        case MOBILE_TX_POWER:
        {
            btdrv_write_rf_reg(0x189, 0x0076);
            btdrv_write_rf_reg(0x18a, 0x0076);
            btdrv_write_rf_reg(0x18b, 0x0076);
            break;
        }
        case FA_TX_POWER:
        {
            BTDIGITAL_REG(0xd0220474)|=(1<<20);//enable fa txpwr cntl
            BTDIGITAL_REG(0xd0220474)|=(0x7<<21);//set fa txgain idx=7
            break;
        }
        case FAST_LOCK_ON_OFF:
        {
            uint32_t fastlock_on;

            sscanf(command_param, "%d", &fastlock_on);
            btdrv_fast_lock_config(fastlock_on);
            break;
        }
    #ifdef A2DP_SOURCE_TEST
        case CONNECT_DEVICE_TEST:
        {
            //uint8_t sink_addr[6] = {0x77,0x77,0x33,0x22,0x11,0x11};//{0x63,0x55,0x44,0x33,0x22,0x11};//;//;
            uint32_t device_index = 0;
            device_info_t selected_device;

            //sscanf(command_param, "%d", &device_index);

            selected_device = get_device_info_by_index(device_index);
            app_bt_source_set_source_addr(selected_device.addr.address);
            //memcpy(&selected_device.addr.address[0],&source_test_bt_addr[0],6);
            set_bt_search_address(selected_device.addr);
            set_bt_search_device_searched(true);

            bt_source_reconnect_a2dp_profile(&selected_device.addr);
            break;
        }
        case SEND_SBC_PKT_TEST:
        {
            #if 0//ndef FPGA
            if (!a2dp_source_send_sbc_timer_id){
                a2dp_source_send_sbc_timer_id = osTimerCreate(osTimer(a2dp_source_send_sbc_timer), osTimerPeriodic, NULL);
            }
            if (a2dp_source_send_sbc_timer_id != NULL) {
                osTimerStart(a2dp_source_send_sbc_timer_id, 20);
            }
            #else
            a2dp_source_pkt_sent_flag = 1;
            a2dp_source_FPGA_send_sbc_packet();
            #endif

            break;
        }
        case TOGGLE_STREAM_TEST:
        {
            #if 0//ndef FPGA
            if(a2dp_source_send_sbc_timer_id)
            {
                osTimerStop(a2dp_source_send_sbc_timer_id);
            }
            #else
            if(a2dp_source_pkt_sent_flag)
            {
                a2dp_source_pkt_sent_flag = 0;
            }
            #endif
            app_a2dp_source_toggle_stream(app_bt_source_get_current_a2dp());
            break;
        }
        case SOURCE_SNIFF_TEST:
        {
            if (!fpga_a2dp_source_send_toggle_stream_timer_id){
                fpga_a2dp_source_send_toggle_stream_timer_id = osTimerCreate(osTimer(fpga_a2dp_source_send_toggle_stream_timer), osTimerPeriodic, NULL);
            }
            if (fpga_a2dp_source_send_toggle_stream_timer_id != NULL) {
                osTimerStart(fpga_a2dp_source_send_toggle_stream_timer_id, 25000);//25s
            }
            break;
        }
    #endif
        default:
        {
            break;
        }
    }
}

static void app_source_trace_rx_thread(void const *argument)
{
    TRACE(1, "thread:[%s]", __func__);

    while(1)
    {
        SOURCE_RX_MSG* rx_msg = NULL;

        if(!source_rx_mailbox_get(&rx_msg))
        {
            trace_rx_thread_handler(rx_msg);

            rb_ctl_mailbox_free(rx_msg);
        }
    }
}

#ifdef A2DP_SOURCE_TEST
void app_a2dp_source_fpga_connect_device(void)
{
    //uint8_t sink_addr[6] = {0x77,0x77,0x33,0x22,0x11,0x11};//{0x63,0x55,0x44,0x33,0x22,0x11};
    uint32_t device_index = 0;
    device_info_t selected_device;

    //sscanf(command_param, "%d", &device_index);

    selected_device = get_device_info_by_index(device_index);

    app_bt_source_set_source_addr(selected_device.addr.address);
    //memcpy(&selected_device.addr.address[0],&source_test_bt_addr[0],6);
    set_bt_search_address(selected_device.addr);
    set_bt_search_device_searched(true);

    bt_source_reconnect_a2dp_profile(&selected_device.addr);
}

uint8_t fpga_sbc_buf[] =
{
    0x9c,0x31,0x10,0x34,0x31,0x11,0x00,0x00,0x7d,0xdb,0x7d,0xdb,0x75,0x9b,0x6e,0x5b,
    0x20,0x9c,0x12,0x6d,0x19,0xd2,0x0e,0x5b,0x29,0x13,0x44,0x22,0x3e,0x1a,0x21,0xa4,
    0x29,0xaa,0x7a,0x05,0xb4,0xdb,0x56,0x51,0x9c,0x31,0x10,0xa6,0x32,0x21,0x00,0x00,
    0x13,0x13,0x72,0xb4,0xa5,0xcc,0x6b,0x49,0x63,0xca,0xa2,0x1d,0x7a,0xba,0x5d,0x31,
    0x4b,0xbb,0x75,0x2c,0x8b,0xd5,0xc4,0x23,0xe4,0x32,0xc4,0x32,0x92,0xb1,0xaa,0xbe,
};

void a2dp_source_FPGA_send_sbc_packet(void)
{
    memcpy(a2dp_source_sbc_frame_buffer(), &fpga_sbc_buf[0], sizeof(fpga_sbc_buf));
    a2dp_source_send_sbc_packet();
}
#endif // A2DP_SOURCE_TEST
#endif // __SOURCE_TRACE_RX__

#endif /* BT_A2DP_SRC_ROLE */
#endif /* BT_A2DP_SUPPORT */
