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
#ifndef NVRECORD_BT_H
#define NVRECORD_BT_H
#include "cmsis.h"
#include "nvrecord_extension.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NEW_DEVICE_CB_MOBILE         = 0,
    NEW_DEVICE_CB_TWS            = 1,
    NEW_DEVICE_CB_ANY_DEVICE     = 2,
    NEW_DEVICE_CB_USER1          = 3,

    NEW_DEVICE_CB_NUM,
} NEW_DEVICE_CB_E;

typedef void (*nv_record_btdevice_new_device_paired_func_t)(const uint8_t* btAddr);
typedef void (*NEWDEVICE_PAIRED_CB_T)(const uint8_t* btAddr);

void nv_record_btdevicerecord_set_a2dp_vol(nvrec_btdevicerecord* pRecord, int8_t vol);
void nv_record_btdevicerecord_set_a2dp_abs_vol(nvrec_btdevicerecord* pRecord, int8_t vol);
void nv_record_btdevicerecord_set_hfp_vol(nvrec_btdevicerecord* pRecord, int8_t vol);
void nv_record_btdevicevolume_set_a2dp_vol(btdevice_volume* device_vol, int8_t vol);
void nv_record_btdevicevolume_set_hfp_vol(btdevice_volume* device_vol, int8_t vol);
void nv_record_btdevicerecord_set_pnp_info(nvrec_btdevicerecord* pRecord, bt_dip_pnp_info_t* pPnpInfo);
void nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_profile* device_plf, bool isActive);
void nv_record_btdevicerecord_set_hfp_profile_active_state(btdevice_profile* device_plf, bool isActive);
int nv_record_enum_latest_two_paired_dev(btif_device_record_t* record1,btif_device_record_t* record2);
int nv_record_get_paired_dev_list(nvrec_btdevicerecord** record);
void nv_record_all_ddbrec_print(void);
void nv_record_update_runtime_userdata(void);
void nvrecord_rebuild_paired_bt_dev_info(NV_RECORD_PAIRED_BT_DEV_INFO_T* pPairedBtInfo);
int nv_record_btdevicerecord_find(const bt_bdaddr_t *bd_ddr, nvrec_btdevicerecord **record);
void nv_record_btdevicerecord_set_a2dp_profile_codec(btdevice_profile* device_plf, uint8_t a2dpCodec);
bt_status_t nv_record_ddbrec_clear(void);
bt_status_t nv_record_ddbrec_delete(const bt_bdaddr_t *bdaddr);
bt_status_t nv_record_enum_dev_records(unsigned short index,btif_device_record_t* record);
bt_status_t nv_record_ddbrec_find(const bt_bdaddr_t *bd_ddr, btif_device_record_t*record);
bool nv_record_get_pnp_info(bt_bdaddr_t *bdAddr, bt_dip_pnp_info_t *pPnpInfo);
bt_status_t nv_record_add(SECTIONS_ADP_ENUM type,void *record);
bt_status_t nv_record_change_order(const btif_device_record_t *record, uint8_t target);
int nv_record_get_paired_dev_count(void);
void ram_record_ddbrec_init(void);
#ifdef FPGA
bt_status_t ram_record_ddbrec_find(const bt_bdaddr_t* bd_ddr, nvrec_btdevicerecord **record);
bt_status_t ram_record_ddbrec_add(const nvrec_btdevicerecord* param_rec);
bt_status_t ram_record_ddbrec_delete(const bt_bdaddr_t *bdaddr);
#endif

#ifdef TOTA_CRASH_DUMP_TOOL_ENABLE
void nv_record_btdevicerecord_crash_dump(void);
#endif

bt_status_t nv_record_ddbrec_clear_all_bt_paired_list();
uint8_t* nv_record_btdevice_get_latest_paired_device_bt_addr(void);
void nv_record_bt_device_register_newly_paired_device_callback(NEW_DEVICE_CB_E index, nv_record_btdevice_new_device_paired_func_t func);

#define NVREC_DEV_NEWEST_REV        2
#define NVREC_DEV_VERSION_1         1

#define BLE_NAME_LEN_IN_NV  32

/*
    this is the nvrec dev zone struct :
    version|magic   16bit|16bit
    crc         32bit
    reserve[0]      32bit
    reserv[1]       32bit
    dev local name  max 249*8bit
    dev bt addr         64bit
    dev ble addr        64bit
    calib data      32bit
*/
#define nvrec_dev_version   1
#define nvrec_dev_magic      0xba80
typedef enum
{
    dev_version_and_magic,      //0
    dev_crc,                    //1
    dev_reserv1,                //2
    dev_reserv2,                //3
    dev_name,                   //[4~66]
    dev_bt_addr = 67,           //[67~68]
    dev_ble_addr = 69,          //[69~70]
    dev_dongle_addr = 71,
    dev_xtal_fcap = 73,         //73
    dev_data_len,
}nvrec_dev_enum;

// following the former nv rec dev info
typedef enum
{
    rev2_dev_data_len = 75,             //75, length of the valid content, excluding crc
    rev2_dev_crc,                       //76, crc value of the following data
    rev2_dev_section_start_reserved,    //77
    rev2_dev_reserv2,                   //78
    rev2_dev_name,                      //[79~141]
    rev2_dev_bt_addr = 142,             //[142~143]
    rev2_dev_ble_addr = 144,            //[144~145]
    rev2_dev_dongle_addr = 146,         //[146~147]
    rev2_dev_xtal_fcap = 148,           //148
    rev2_dev_ble_name = 149,            //[149~156]
#ifdef NVREC_BAIDU_DATA_SECTION
    rev2_dev_prod_sn = 157,             //[157~160]
#endif
    // TODO: add the new section in the future if needed

    rev2_dev_section_end = 157,

}nvrec_dev_rev_2_enum;

int nvrec_dev_get_dongleaddr(bt_bdaddr_t *dongleaddr);
int nvrec_dev_get_btaddr(char *btaddr);
char* nvrec_dev_get_bt_name(void);
const char* nvrec_dev_get_ble_name(void);
void nvrecord_bt_init(void);

int nv_record_get_sink_paired_dev(btif_device_record_t *record, uint8_t max_num);
#ifdef APP_SOUND_ENABLE
int nv_record_stash_original_bt_record(void);
int nv_record_pop_original_bt_record(void);
#endif

#ifdef __cplusplus
}
#endif
#endif

