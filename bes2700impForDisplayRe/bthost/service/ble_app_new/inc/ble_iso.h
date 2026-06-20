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
 * @{
 ****************************************************************************************
 */

#ifdef BLE_ISO_ENABLED

/*****************************header include********************************/
#include "isoohci_int.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
/// SDU Buffer structure
typedef struct
{
    /// Time_Stamp
    uint32_t        time_stamp;
    /// Packet Sequence Number
    uint16_t        pkt_seq_nb;
    /// length of the ISO SDU (in bytes)
    uint16_t        sdu_length;
    /// Reception status (@see enum hci_iso_pkt_stat_flag)
    uint8_t         status;
    /// SDU
    uint8_t         *sdu;
} dp_itf_iso_buffer_t;

/****************************function declearation**************************/
#ifdef __cplusplus
extern "C" {
#endif
void *app_iso_rx_malloc(uint32_t size);
void app_iso_rx_free(void *rmem);
void app_iso_heap_init(void *begin_addr, uint32_t size);
void *app_iso_heap_malloc(uint32_t size);
void app_iso_heap_free(void *rmem);
void *app_iso_malloc_buff(uint16_t size);
void app_iso_free_buff(void *mem_ptr);
void app_iso_set_rx_dp_itf(struct data_path_itf *itf);
struct data_path_itf *app_iso_get_rx_dp_itf(void);
void *app_iso_dp_itf_get_rx_data(uint16_t conhdl, dp_itf_iso_buffer_t *iso_buffer);
void app_iso_dp_itf_rx_data_done(uint16_t conhdl, uint16_t sdu_len, uint32_t ref_time, uint8_t *p_buf);
void app_iso_set_tx_dp_itf(struct data_path_itf *itf);
struct data_path_itf *app_iso_get_tx_dp_itf(void);
int app_iso_get_free_packet_num(void);
void app_iso_dp_itf_send_data(uint16_t conhdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time);
void app_iso_dp_itf_send_data_directly(uint16_t conhdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time);
void app_iso_dp_set_rx_dp_itf(void);
void app_iso_dp_itf_data_come_callback_register(void *callback);
void app_iso_dp_itf_data_come_callback_deregister(void);

void app_iso_dp_tx_iso_stop(uint16_t conhdl);
void app_iso_dp_rx_iso_stop(uint16_t conhdl);

#ifdef __cplusplus
}
#endif

#endif

