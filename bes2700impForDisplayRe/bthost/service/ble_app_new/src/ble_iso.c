/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/
#ifdef BLE_ISO_ENABLED
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "audio_dump.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "ble_app_dbg.h"
#include "bt_drv_reg_op.h"
#include "isoohci_int.h"
#include "app_bap_data_path_itf.h"

#include "gatt_service.h"
#include "app_ble.h"
#include "app_gaf.h"
#include "heap_api.h"

/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declearation***********************/

/**********************private function declearation************************/

/************************private variable defination************************/
#define APP_ISO_HEAP_DEBUG 0
#define APP_ISO_RX_MALLOC_DEBUG 0
struct data_path_itf *app_iso_rx_dp_itf = NULL;
struct data_path_itf *app_iso_tx_dp_itf = NULL;
static heap_handle_t app_iso_data_heap;

/****************************function defination****************************/

void *app_iso_rx_malloc(uint32_t size)
{
    size = ((size >> 2) + 1) << 2;
    if(size>0xffff)
    {
        ASSERT(0, "%s size:%d", __func__, size);
    }
    size&=0xffff;
    void *ptr = bes_bt_buf_malloc(size);
    ASSERT(ptr, "%s size:%d", __func__, size);
#if APP_ISO_RX_MALLOC_DEBUG
    TRACE(0,"[%s] ptr=%p size=%u user=%p",
        __func__, ptr, size, __builtin_return_address(0));
#endif
    return ptr;
}

void app_iso_rx_free(void *rmem)
{
#if APP_ISO_RX_MALLOC_DEBUG
    TRACE(0,"[%s] ptr=%p user=%p", __func__, rmem, __builtin_return_address(0));
#endif
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);
    bes_bt_buf_free(rmem);
}

void app_iso_heap_init(void *begin_addr, uint32_t size)
{
    app_iso_data_heap = heap_register(begin_addr,size);
}

void *app_iso_heap_malloc(uint32_t size)
{
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(app_iso_data_heap, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
#if APP_ISO_HEAP_DEBUG
    TRACE(0,"[%s] ptr=%p size=%u user=%p left/mini left=%u/%u",
        __func__, ptr, size, __builtin_return_address(0),
        heap_free_size(app_iso_data_heap),
        heap_minimum_free_size(app_iso_data_heap));
#endif
    return ptr;
}

void app_iso_heap_free(void *rmem)
{
#if APP_ISO_HEAP_DEBUG
    TRACE(0,"[%s] ptr=%p user=%p", __func__, rmem, __builtin_return_address(0));
#endif
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);

    heap_free(app_iso_data_heap,rmem);
}

void *app_iso_malloc_buff(uint16_t size)
{
    return cobuf_malloc(size);
}

void app_iso_free_buff(void *mem_ptr)
{
    cobuf_free(mem_ptr);
}

void app_iso_set_rx_dp_itf(struct data_path_itf *itf)
{
    LOG_D("%s old %p new %p", __func__, app_iso_rx_dp_itf, itf);
    if (app_iso_rx_dp_itf != itf)
    {
        app_iso_rx_dp_itf = itf;
    }
}

struct data_path_itf *app_iso_get_rx_dp_itf(void)
{
    //LOG_D("%s %p", __func__, app_iso_rx_dp_itf);
    return app_iso_rx_dp_itf;
}

void *app_iso_dp_itf_get_rx_data(uint16_t conhdl, dp_itf_iso_buffer_t *iso_buffer)
{
    gaf_media_data_t *p_sdu_buf = NULL;

    if (app_iso_rx_dp_itf != NULL)
    {
        if (app_iso_rx_dp_itf->cb_sdu_get != NULL)
        {
            p_sdu_buf = (gaf_media_data_t *)app_iso_rx_dp_itf->cb_sdu_get(conhdl, 0, NULL, NULL);
        }

        return p_sdu_buf;
    }
    else
    {
        LOG_E("[ISO DP ERR] dp_itf = NULL!!!");
    }

    return NULL;
}

void app_iso_dp_itf_rx_data_done(uint16_t conhdl, uint16_t sdu_len, uint32_t ref_time,
                                 uint8_t *p_buf)
{
    if (app_iso_rx_dp_itf->cb_sdu_done != NULL)
    {
        p_buf = (p_buf - OFFSETOF(isoohci_buffer_t, sdu));
        app_iso_rx_dp_itf->cb_sdu_done(conhdl, sdu_len, ref_time, p_buf, 0);
    }
}

void app_iso_set_tx_dp_itf(struct data_path_itf *itf)
{
    LOG_D("%s old %p new %p", __func__, app_iso_tx_dp_itf, itf);
    if (app_iso_tx_dp_itf != itf)
    {
        app_iso_tx_dp_itf = itf;
    }
}

struct data_path_itf *app_iso_get_tx_dp_itf(void)
{
    //LOG_D("%s %p", __func__, app_iso_tx_dp_itf);
    return app_iso_tx_dp_itf;
}

static void app_iso_dp_iso_packet_free(struct hci_tx_iso_desc_t *packet)
{
    app_iso_free_buff(packet);
}

void app_iso_dp_itf_send_data_directly(uint16_t conhdl, uint16_t seq_num, uint8_t *payload,
                                       uint16_t payload_len, uint32_t ref_time)
{
    struct hci_tx_iso_desc_t *tx_iso_desc = NULL;
    struct hci_iso_timestamp_header_t *iso_packet = NULL;
    uint16_t iso_packet_len = sizeof(struct hci_iso_timestamp_header_t) + payload_len;
    uint16_t iso_conn_handle = (conhdl & HCI_CONN_HANDLE_MASK) | HCI_ISO_PB_COMPLETE_SDU |
                               HCI_ISO_TS_FLAG_SET;
    uint16_t iso_data_load_length = HCI_ISO_SDU_MAX_HEADER_LEN + payload_len;  // length 0~13 rfu 14~15
    uint32_t iso_sdu_timestamp = ref_time;
    uint16_t iso_sdu_sequence_num = seq_num;
    uint16_t iso_sdu_length = (payload_len & HCI_ISO_SDU_LEN_MASK) | HCI_ISO_PS_GOOD_DATA;

    if (conhdl == HCI_INVALID_CONN_HANDLE)
    {
        LOG_D("%s invalid conhdl %04x", __func__, conhdl);
        return;
    }

    if (payload == NULL || payload_len == 0 || payload_len > HCI_ISO_SDU_LEN_MASK ||
            iso_data_load_length > HCI_ISO_PYL_LEN_MASK)
    {
        LOG_D("%s invalid payload: %p len %d", __func__, payload, payload_len);
        return;
    }

    tx_iso_desc = (struct hci_tx_iso_desc_t *)app_iso_malloc_buff(sizeof(struct hci_tx_iso_desc_t) +
                                                           iso_packet_len);
    if (tx_iso_desc == NULL)
    {
        LOG_D("%s malloc fail: conhdl %04x len %d seqn %d", __func__, conhdl, payload_len, seq_num);
        return;
    }

    iso_packet = (struct hci_iso_timestamp_header_t *)(tx_iso_desc + 1);
    iso_packet->iso_hci_type = HCI_DATA_TYPE_ISO;
    iso_packet->iso_conn_handle = co_host_to_uint16_le(iso_conn_handle);
    iso_packet->iso_data_load_length = co_host_to_uint16_le(iso_data_load_length);
    iso_packet->iso_sdu_timestamp = co_host_to_uint32_le(iso_sdu_timestamp);
    iso_packet->iso_sdu_sequence_num = co_host_to_uint16_le(iso_sdu_sequence_num);
    iso_packet->iso_sdu_length = co_host_to_uint16_le(iso_sdu_length);
    memcpy(iso_packet + 1, payload, payload_len);

    tx_iso_desc->connhdl = (conhdl & HCI_CONN_HANDLE_MASK);
    tx_iso_desc->iso_packet_len = iso_packet_len;
    tx_iso_desc->iso_sdu_seqn = iso_sdu_sequence_num;
    tx_iso_desc->tx_packet_free = app_iso_dp_iso_packet_free;

    hci_send_iso_packet(tx_iso_desc);
}

int app_iso_get_free_packet_num(void)
{
    return hci_get_iso_free_packet_num();
}

void app_iso_dp_itf_send_data(uint16_t conhdl, uint16_t seq_num, uint8_t *payload,
                              uint16_t payload_len, uint32_t ref_time)
{
    app_iso_dp_itf_send_data_directly(conhdl, seq_num, payload, payload_len, ref_time);
}

void app_iso_dp_set_rx_dp_itf(void)
{
    const struct data_path_itf *_rx_dp_itf = NULL;

    _rx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_RX);

    if (_rx_dp_itf == NULL)
    {
        TRACE(0,"get rx data path interface failed");
        _rx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_RX);
    }

    app_iso_set_rx_dp_itf((struct data_path_itf *)_rx_dp_itf);
}


void app_iso_dp_itf_data_come_callback_register(void *callback)
{
    LOG_I("%s %p", __func__, callback);
    isoohci_data_come_callback_register(callback);
}

void app_iso_dp_itf_data_come_callback_deregister(void)
{
    LOG_I("%s", __func__);
    isoohci_data_comed_callback_deregister();
}

void app_iso_dp_tx_iso_stop(uint16_t conhdl)
{
    struct data_path_itf *_tx_dp_itf = app_iso_tx_dp_itf;

    LOG_I("app_iso_dp_tx_iso_stop, conhdl = %d _tx_dp_itf %p", conhdl, _tx_dp_itf);
    if (_tx_dp_itf && (NULL != _tx_dp_itf->cb_stop))
    {
        _tx_dp_itf->cb_stop(conhdl, 0);
    }
}

void app_iso_dp_rx_iso_stop(uint16_t conhdl)
{
    struct data_path_itf *_rx_dp_itf = app_iso_rx_dp_itf;
    LOG_I("app_iso_dp_rx_iso_stop, conhdl = %d _rx_dp_itf %p", conhdl, _rx_dp_itf);

    if (_rx_dp_itf && (NULL != _rx_dp_itf->cb_stop))
    {
        _rx_dp_itf->cb_stop(conhdl, 0);
    }
}


#endif

/// @} APP
