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
#ifndef __EXPORT_NV_FN_ROM_H__
#define __EXPORT_NV_FN_ROM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"

typedef bt_status_t (*__NV_RECORD_OPEN)(SECTIONS_ADP_ENUM section_id);
typedef bt_status_t (*__NV_RECORD_ADD)(SECTIONS_ADP_ENUM type, void *record);
typedef int (*__NV_RECORD_BTDEV_REC_FIND)(const bt_bdaddr_t *bd_ddr, nvrec_btdevicerecord **record);
typedef void (*__NV_RECORD_FLASH)(void);
typedef void (*__NV_RECORD_BTDEV_SET_PNP_INFO)(nvrec_btdevicerecord* pRecord, bt_dip_pnp_info_t* pPnpInfo);
typedef bt_status_t (*__NV_RECORD_DDBREC_FIND)(const bt_bdaddr_t* bd_ddr, btif_device_record_t *record);
typedef bool (*__NV_RECORD_GET_PNP_INFO)(bt_bdaddr_t *bdAddr, bt_dip_pnp_info_t *pPnpInfo);

struct EXPORT_NV_FN_ROM_T {
    void *reserved;

    __NV_RECORD_OPEN nv_record_open;
    __NV_RECORD_ADD nv_record_add;
    __NV_RECORD_BTDEV_REC_FIND nv_record_btdevicerecord_find;
    __NV_RECORD_FLASH nv_record_flash_flush;
    __NV_RECORD_BTDEV_SET_PNP_INFO nv_record_btdev_set_pnp_info;
    __NV_RECORD_DDBREC_FIND        nv_record_ddbrec_find;
    __NV_RECORD_GET_PNP_INFO       nv_record_get_pnp_info;
};

void export_register_nv_fn(const struct EXPORT_NV_FN_ROM_T* fn);

#ifdef __cplusplus
}
#endif

#endif /* __EXPORT_NV_FN_ROM_H__ */

