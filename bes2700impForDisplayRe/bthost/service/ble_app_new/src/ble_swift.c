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
#ifdef SWIFT_ENABLED // microsoft bluetooth swift pair
#include "gatt_service.h"
#include "app_ble.h"
#include "app_bt.h"

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_ibrt_cmd_handler.h"
#endif

#define SWIFT_PAIR_MODE_LE_ONLY      (0)
#define SWIFT_PAIR_MODE_LE_AND_BT    (1)
#define SWIFT_PAIR_MODE_BT           (2)
#define SWIFT_PAIR_MODE             SWIFT_PAIR_MODE_BT

#define SWIFT_MS_HEADER_LEN         (6)
#define SWIFT_MS_VENDER_ID          (0xff0600)
#define SWIFT_MS_BEACON             (0x03)
#if (SWIFT_PAIR_MODE == SWIFT_PAIR_MODE_LE_ONLY)
#define SWIFT_MS_BEACON_SUB         (0)
#define SWIFT_ADV_DISPLAY_ICON      (0)
#define SWIFT_ADV_BT_ADDR           (0)
#elif (SWIFT_PAIR_MODE == SWIFT_PAIR_MODE_LE_AND_BT)
#define SWIFT_MS_BEACON_SUB         (0x02)
#define SWIFT_ADV_DISPLAY_ICON      (1)
#define SWIFT_ADV_BT_ADDR           (0)
#else
#define SWIFT_MS_BEACON_SUB         (0x01)
#define SWIFT_ADV_DISPLAY_ICON      (1)
#define SWIFT_ADV_BT_ADDR           (1)
#endif
#define SWIFT_RSSI                  (0x80)
#define SWIFT_DISPLAY_ICON_LEN      (3)
#define SWIFT_DISPLAY_ICON          (0x040424)

#define BLE_SWIFT_ADVERTISING_INTERVAL (100)

// adv mode should be ADV_MODE_EXTENDED or ADV_MODE_LEGACY
#define SWIFT_ADV_MODE_LEGACY        (0)
#define SWIFT_ADV_MODE_EXTEND        (1)
#define SWIFT_ADV_MODE               SWIFT_ADV_MODE_LEGACY

static bool enable_swift = false;

static bool app_swift_adv_activity_prepare(ble_adv_activity_t *adv)
{
    gap_adv_param_t *adv_param = &adv->adv_param;
    uint8_t service_data[64];
    uint8_t *data_ptr = service_data;
    uint8_t *length_byte = NULL;
    uint8_t max_len = 0;
    uint8_t curr_len = 0;
    uint8_t name_len = 0;
    int left_len = 0;
    const char *display_name = NULL;

    if (!ble_adv_is_allowed())
    {
        return false;
    }

#if defined(IBRT)
    if (!app_ble_check_ibrt_allow_adv(USER_SWIFT))
    {
        return false;
    }
#endif

    if (!enable_swift)
    {
        return false;
    }

    adv->user = USER_SWIFT;
    adv_param->connectable = true;
    adv_param->scannable = true;
#if (SWIFT_ADV_MODE == SWIFT_ADV_MODE_EXTEND)
    adv_param->use_legacy_pdu = false;
#else
    adv_param->use_legacy_pdu = true;
#endif
    adv_param->include_tx_power_data = true;
    adv_param->own_addr_use_rpa = true;
    adv_param->fast_advertising = false; // BLE_SWIFT_ADVERTISING_INTERVAL

    app_ble_set_adv_tx_power_level(adv, BLE_ADV_TX_POWER_LEVEL_1);

    app_ble_dt_set_flags(adv_param, true);

    length_byte = data_ptr;
    data_ptr += 1;

    data_ptr[0] = (SWIFT_MS_VENDER_ID >> 16) & 0xFF;
    data_ptr[1] = (SWIFT_MS_VENDER_ID >> 8) & 0xFF;
    data_ptr[2] = SWIFT_MS_VENDER_ID  & 0xFF;
    data_ptr[3] = SWIFT_MS_BEACON;
    data_ptr[4] = SWIFT_MS_BEACON_SUB;
    data_ptr[5] = SWIFT_RSSI;
    data_ptr += 6;

#if SWIFT_ADV_BT_ADDR
    memcpy(data_ptr, gap_hci_bt_address(), sizeof(bt_bdaddr_t));
    data_ptr += sizeof(bt_bdaddr_t);
#endif

#if SWIFT_ADV_DISPLAY_ICON
    data_ptr[0] = (SWIFT_DISPLAY_ICON >> 16) & 0xFF;
    data_ptr[1] = (SWIFT_DISPLAY_ICON >> 8) & 0xFF;
    data_ptr[2] = SWIFT_DISPLAY_ICON  & 0xFF;
    data_ptr += 3;
#endif

    max_len = adv_param->use_legacy_pdu ? GAP_MAX_LEGACY_ADV_DATA_LEN : sizeof(service_data);
    curr_len = data_ptr - service_data;
    left_len = max_len - curr_len - 1; // -1 for '\0'

    display_name = (char *)app_ble_get_dev_name();
    name_len = MIN(strlen(display_name), left_len);

    memcpy(data_ptr, (uint8_t *)display_name, name_len);
    data_ptr += name_len;

    data_ptr[0] = '\0';
    data_ptr += 1;

    curr_len = data_ptr - service_data;
    *length_byte = curr_len - 1;
    gap_dt_add_raw_data(&adv_param->adv_data, service_data, curr_len);

    return true;
}

void app_swift_init(void)
{
    TRACE(1, "[%s]", __func__);
    app_ble_register_advertising(BLE_SWIFT_ADV_HANDLE, app_swift_adv_activity_prepare);
}

void app_swift_enter_pairing_mode(void)
{
    enable_swift = true;
    bes_bt_me_write_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE, 0);
}

void app_swift_exit_pairing_mode(void)
{
    enable_swift = false;
}

#endif /* SWIFT_ENABLED */
