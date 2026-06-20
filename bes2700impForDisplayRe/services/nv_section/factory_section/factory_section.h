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
#ifndef __FACTORY_SECTIONS_H__
#define __FACTORY_SECTIONS_H__

#define ALIGN4 __attribute__((aligned(4)))
#define nvrec_mini_version      1
#define nvrec_dev_magic         0xba80
#define nvrec_current_version   2
#define FACTORY_SECTOR_SIZE     NORFLASH_API_SECTOR_MAX_SIZE

enum {
    NV_ROLE_IBRT_MASTER = 0x00,
    NV_ROLE_IBRT_SLAVE =  0x01,
    NV_ROLE_IBRT_UNKNOWN = 0xff,
};

typedef struct {
    unsigned short magic;
    unsigned short version;
    unsigned int crc ;
    unsigned int reserved0;
    unsigned int reserved1;
}section_head_t;

typedef struct {
    unsigned char device_name[248+1] ALIGN4;
    unsigned char bt_address[8] ALIGN4;
    unsigned char ble_address[8] ALIGN4;
    // connector addr of the dldtool is the default peer bt address
    unsigned char default_peer_bt_address[8] ALIGN4;
    unsigned int  xtal_fcap ALIGN4;
    unsigned int  rev1_data_len;
    unsigned int  rev2_data_len;
    unsigned int  rev2_crc;
    unsigned int  rev2_reserved0;
    unsigned int  rev2_reserved1;
    unsigned int  rev2_bt_name[63];
    unsigned int  rev2_bt_addr[2];
    unsigned int  rev2_ble_addr[2];
    // connector addr of the dldtool is the default peer bt address
    unsigned char  rev2_default_peer_bt_address[6];
    unsigned char  rev2_u8_reserved0;
    unsigned char  rev2_default_bt_nv_role;
    unsigned int  rev2_xtal_fcap;
    unsigned int  rev2_ble_name[8];
}factory_section_data_t;

typedef struct {
    section_head_t head;
    factory_section_data_t data;
}factory_section_t;

#ifdef __cplusplus
extern "C" {
#endif

void factory_section_init(void);
int factory_section_open(void);
void factory_section_original_btaddr_get(uint8_t *btAddr);
int factory_section_set_bt_name(const char *name,int len);
int factory_section_xtal_fcap_get(unsigned int *xtal_fcap);
int factory_section_xtal_fcap_set(unsigned int xtal_fcap);
uint8_t* factory_section_get_bt_address(void);
uint8_t* factory_section_get_ble_address(void);
int factory_section_set_bt_address(uint8_t* btAddr);
int factory_section_set_ble_address(uint8_t * bleAddr);
uint8_t* factory_section_get_bt_name(void);
uint8_t* factory_section_get_ble_name(void);
uint32_t factory_section_get_version(void);
uint8_t* factory_section_get_default_peer_bt_address(void);
uint8_t factory_section_get_default_bt_nv_role(void);

#ifdef __cplusplus
}
#endif
#endif
