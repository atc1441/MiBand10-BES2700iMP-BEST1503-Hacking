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
#ifndef __APP_IBRT_IF_NVRECORD__
#define __APP_IBRT_IF_NVRECORD__
#include "cmsis_os.h"

#include "app_tws_ibrt.h"

#ifdef __cplusplus
extern "C" {
#endif

int app_ibrt_nvrecord_find(const bt_bdaddr_t *bd_ddr, nvrec_btdevicerecord **record);

void app_ibrt_nvrecord_config_load(void *config);

void app_ibrt_nvrecord_update_ibrt_mode_tws(bool status);

bt_status_t app_ibrt_nvrecord_get_mobile_addr(bt_bdaddr_t mobile_addr_list[],uint8_t *count);

int app_ibrt_nvrecord_get_latest_mobiles_addr(bt_bdaddr_t *mobile_addr1, bt_bdaddr_t* mobile_addr2);

void app_ibrt_nvrecord_delete_all_mobile_record(void);

bt_status_t app_ibrt_nvrecord_get_mobile_paired_dev_list(nvrec_btdevicerecord *nv_record,uint8_t *count);

bt_status_t app_ibrt_nvrecord_find_mobile_nv_record(const bt_bdaddr_t *mobile_addr,nvrec_btdevicerecord *nv_record);

int app_ibrt_nvrecord_choice_mobile_addr(bt_bdaddr_t *mobile_addr, uint8_t index);

#if defined(FPGA)
bt_status_t app_ibrt_nvrecord_get_mobile_addr_on_fpga(bt_bdaddr_t mobile_addr_list[],uint8_t *count,bool master);
bt_status_t app_ibrt_nvrecord_get_paired_dev_list_on_fpga(nvrec_btdevicerecord* record,uint8_t *count,bool master);

#endif

#ifdef __cplusplus
}
#endif

#endif/*__APP_IBRT_IF_NVRECORD__*/
