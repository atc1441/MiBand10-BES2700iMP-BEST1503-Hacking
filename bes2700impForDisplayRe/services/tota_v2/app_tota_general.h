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

#ifndef __APP_TOTA_GENERAL_H__
#define __APP_TOTA_GENERAL_H__

#include "app_battery.h"
#include "factory_section.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_ibrt_internal.h"
#include "app_bt.h"
#include "app_key.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#endif

#define BT_BLE_LOCAL_NAME_LEN       32

#ifdef CUSTOMER_APP_BOAT
#define APP_TOTA_BUTTON_SETTINGS_CONTROL_CMD              0x01
#define APP_TOTA_FACTORY_RESET_CMD                        0x02
#define APP_TOTA_MUSIC_PLAY_SETTINGS_CMD                  0x03
#define APP_TOTA_BATTERY_LEVEL_CMD                        0x04
#define APP_TOTA_EARBUD_FIT_TEST_CMD                      0x05
#define APP_TOAT_EQ_CMD                                   0x06

#define BUTTON_SETTING_LEFT_EARPHONE_CMD       0x01
#define BUTTON_SETTING_RIGHT_EARPHONE_CMD      0x02
#define BUTTON_SETTING_CLICK_CMD               0x01
#define BUTTON_SETTING_DOUBLE_CLICK_CMD        0x02
#define BUTTON_SETTING_PRE_SONG_CMD            0x01
#define BUTTON_SETING_NEXT_SONG_CMD            0x02
#define BUTTON_SETTING_ANC_CMD                 0x03
#define BUTTON_SETTING_CALL_CMD                0x04
#define BUTTON_SETTING_VOLUME_UP_CMD           0x05
#define BUTTON_SETTING_VOLUME_DOWN_CMD         0x06
#define BUTTON_SETTING_PLAY_MUSIC_CMD          0x07
#define BUTTON_SETTING_PAUSE_MUSIC_CMD         0x08
#define BUTTON_SETTING_VOICE_ASSITANT_CMD      0x09


#define MUSIC_PLAY_SETTINGS_PLAY_CMD                      0x01
#define MUSIC_PLAY_SETTINGS_PAUSE_CMD                     0x02
#define MUSIC_PLAY_SETTINGS_NEXT_CMD                      0x03
#define MUSIC_PLAY_SETTINGS_PRE_CMD                       0x04

#define EARBUD_DETECK_ON_CMD                     0x01
#define EARBUD_DETECK_OFF_CMD                    0x00

#endif

typedef struct{
    /* bt-ble info */
    char btName[BT_BLE_LOCAL_NAME_LEN];         // 32 bytes
    char bleName[BT_BLE_LOCAL_NAME_LEN];        // 32 bytes
    bt_bdaddr_t btLocalAddr;                    // 6 bytes
    bt_bdaddr_t btPeerAddr;                     // 6 bytes
    bt_bdaddr_t bleLocalAddr;                   // 6 bytes
    bt_bdaddr_t blePeerAddr;                    // 6 bytes

    /* ibrt info */
    uint8_t ibrtRole;                       // 1 byte

    /* crystal info */
    uint32_t crystal_freq;                      // 4 bytes
    uint32_t xtal_fcap;                         // 4 bytes
    
    /* battery info */
    APP_BATTERY_MV_T        battery_volt;       // 2 bytes
    uint8_t                 battery_level;      // 1 byte
    APP_BATTERY_STATUS_T    battery_status;     // 4 bytes

    /* firmware info */
    uint8_t fw_version[4];                      // 4 bytes
    
    /* ear location info */
    uint8_t          ear_location;       // APP_TWS_SIDE_T

    /* rssi info */
    uint8_t                 rssi[48];           // 48 bytes
    uint32_t                rssi_len;           // 4 bytes
} general_info_t;


/*--functions from other file--*/
#ifdef FIRMWARE_REV
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);
#endif

extern int app_bt_stream_local_volume_get(void);
extern uint8_t app_audio_get_eq();
extern int app_audio_set_eq(uint8_t index);
extern bool app_meridian_eq(bool onoff);
extern bool app_is_meridian_on();

#ifdef BLE
extern unsigned char *bt_get_ble_local_address(void);
#endif
/*--functions from other file--*/


/*--interface--*/
void app_tota_general_init();
void app_tota_sync_leak_deteck_handle(uint8_t *p_buff, uint8_t length);
void app_tota_sync_audio_eq_handle(uint8_t *p_buff, uint8_t length);

void app_ibrt_send_leak_deteck_status_handle();
void app_tota_start_leak_detect(void);
bool app_tota_get_detect_flag();
void app_tota_set_leak_detect_value();
void app_tota_general_button_event_handler(APP_KEY_STATUS *status, bool is_right);
void app_tota_general_button_event_init(void);
void app_tota_general_button_event_info_set(uint8_t* ptrParam, uint16_t paramLen);

#endif

