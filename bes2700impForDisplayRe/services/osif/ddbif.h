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
#ifndef __DDBIF_H__
#define __DDBIF_H__
#include "nvrecord_bt.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define ddbif_list_saved_flash() nv_record_flash_flush()

#define ddbif_close() ddbif_list_saved_flash()

#define ddbif_open(bdAddr) nv_record_open(section_usrdata_ddbrecord)

#define ddbif_find_record(bdAddr, record) nv_record_ddbrec_find((bdAddr), (record))

#define ddbif_delete_record(bdAddr) nv_record_ddbrec_delete(bdAddr)

#define ddbif_enum_device_records(index, record) nv_record_enum_dev_records((unsigned short)(index), (record))

#define ddbif_add_record(record) nv_record_add(section_usrdata_ddbrecord, (btif_device_record_t *)(record))

bt_status_t ddbif_change_records_order(const btif_device_record_t *record, uint8_t index);

#if defined(__cplusplus)
}
#endif
#endif /*__DDBIF_H__*/

