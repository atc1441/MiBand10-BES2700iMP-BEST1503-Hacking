/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

#include "bt_common_define.h"
#include "prf_types.h"

#ifdef BLE_HOST_SUPPORT

uint8_t ble_prf_pack_date_time(uint8_t *p_buf, const prf_date_time_t *p_date_time)
{
    *(uint16_t *)(p_buf) = co_host_to_uint16_le(p_date_time->year);
    p_buf += 2;
    p_buf[0] = p_date_time->month;
    p_buf += 1;
    p_buf[0] = p_date_time->day;
    p_buf += 1;
    p_buf[0] = p_date_time->hour;
    p_buf += 1;
    p_buf[0] = p_date_time->min;
    p_buf += 1;
    p_buf[0] = p_date_time->sec;

    return 7;
}

uint8_t ble_prf_pack_date(uint8_t *p_buf, const prf_date_t *p_date)
{
    *(uint16_t *)(p_buf) = co_host_to_uint16_le(p_date->year);
    p_buf += 2;
    p_buf[0] = p_date->month;
    p_buf += 1;
    p_buf[0] = p_date->day;

    return 4;
}

uint8_t ble_prf_unpack_date_time(const uint8_t *p_buf, prf_date_time_t *p_date_time)
{
    p_date_time->year   = CO_COMBINE_UINT16_LE(p_buf);
    p_buf += 2;
    p_date_time->month  = p_buf[0];
    p_buf += 1;
    p_date_time->day    = p_buf[0];
    p_buf += 1;
    p_date_time->hour   = p_buf[0];
    p_buf += 1;
    p_date_time->min    = p_buf[0];
    p_buf += 1;
    p_date_time->sec    = p_buf[0];

    return 7;
}

uint8_t ble_prf_unpack_date(const uint8_t *p_buf, prf_date_t *p_date)
{
    p_date->year   = CO_COMBINE_UINT16_LE(p_buf);
    p_buf += 2;
    p_date->month  = p_buf[0];
    p_buf += 1;
    p_date->day    = p_buf[0];

    return 4;
}
#endif