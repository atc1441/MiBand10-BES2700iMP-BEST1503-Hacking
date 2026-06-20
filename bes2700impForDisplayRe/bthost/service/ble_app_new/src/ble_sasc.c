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
#ifdef BES_MOBILE_SAS
#include "gatt_service.h"

#define sas_service_uuid    0xFEF0
#define sas_character_uuid  0xFE66

static uint8_t sas_prf_id;

enum sasc_op_codes {
    /// Reserved operation code
    SASC_RESERVED_OP_CODE  = 0x00,
    /// Read attribute value Procedure
    SASC_READ_OP_CODE,
    /// Write attribute value Procedure
    SASC_WRITE_OP_CODE,
};

/// Internal codes for reading/writing a SAS characteristic with one single request
enum sasc_rd_wr_codes {
    /// Read SAS chararcteristic value——Switching Ambient audio Status
    SASC_RD_CHAR_STATUS              = 0x00,
    /// Write Switching Ambient Service characteristic value——open or close SAS audio
    SASC_WR_CHAR_STATUS              = 0x01,
};

typedef struct {
    gatt_prf_t head;
    gatt_peer_service_t *peer_service;
    gatt_peer_character_t *peer_character;
} sas_prf_t;

static void sasc_cb_discover_cmp(sas_prf_t *prf, uint8_t error_code)
{
    // SASC_ENABLE_RSP
}

static void sasc_cb_write_value_cmp(sas_prf_t *prf, uint16_t status, uint8_t error_code)
{
    // SASC_CMP_EVT SASC_WRITE_OP_CODE
}

static void sasc_cb_read_value_cmp(sas_prf_t *prf, uint16_t status, uint8_t error_code, uint16_t value)
{
    // SASC_CMP_EVT SASC_READ_OP_CODE
}

static void sasc_cb_audio_status_ind(sas_prf_t *prf, uint8_t value)
{
    // SASC_VALUE_IND SASC_RD_CHAR_STATUS
}

static int ble_sas_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    sas_prf_t *sas = (sas_prf_t *)prf;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            break;
        }
        case GATT_PROF_EVENT_CLOSED:
        {
            sas->peer_service = NULL;
            sas->peer_character = NULL;
            break;
        }
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = param.service;
            gatt_peer_service_t *s = p->service;
            uint16_t gap_chars[] = {sas_character_uuid};
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                break;
            }
            sas->peer_service = s;
            gattc_discover_multi_characters(prf, s, gap_chars, sizeof(gap_chars)/sizeof(uint16_t));
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_character_t *c = p->character;
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                break;
            }
            sas->peer_character = c;
            gattc_write_cccd_descriptor(prf, c, true, false);
            sasc_cb_discover_cmp(sas, ATT_ERROR_NO_ERROR);
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            uint16_t value = (p->error_code == ATT_ERROR_NO_ERROR) ? CO_COMBINE_UINT16_LE(p->value) : 0;
            sasc_cb_read_value_cmp(sas, SASC_RD_CHAR_STATUS, p->error_code, value);
            break;
        }
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        {
            gatt_profile_char_write_rsp_t *p = param.char_write_rsp;
            sasc_cb_write_value_cmp(sas, SASC_WR_CHAR_STATUS, p->error_code);
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *ntf = param.notify;
            if (ntf->service->service_uuid == sas_service_uuid)
            {
                sasc_cb_audio_status_ind(sas, ntf->value[0]);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

bt_status_t ble_sasc_start_discover(uint16_t connhdl)
{
    sas_prf_t *prf = NULL;

    prf = (sas_prf_t *)gattc_get_profile(sas_prf_id, connhdl);
    if (prf == NULL)
    {
        return BT_STS_FAILED;
    }

    gattc_discover_service(&prf->head, sas_service_uuid, NULL);
    return BT_STS_SUCCESS;
}

bt_status_t ble_sasc_read_value(uint16_t connhdl, uint16_t read_code)
{
    sas_prf_t *prf = NULL;

    prf = (sas_prf_t *)gattc_get_profile(sas_prf_id, connhdl);
    if (prf == NULL || prf->peer_character == NULL)
    {
        return BT_STS_FAILED;
    }

    if (read_code == SASC_RD_CHAR_STATUS)
    {
        gattc_read_character_value(&prf->head, prf->peer_character);
    }

    return BT_STS_SUCCESS;
}

bt_status_t ble_sasc_write_value(uint16_t connhdl, uint8_t write_code, uint16_t data)
{
    sas_prf_t *prf = NULL;

    prf = (sas_prf_t *)gattc_get_profile(sas_prf_id, connhdl);
    if (prf == NULL || prf->peer_character == NULL)
    {
        return BT_STS_FAILED;
    }

    if (write_code == SASC_WR_CHAR_STATUS)
    {
        uint8_t value[2] = {CO_SPLIT_UINT16_LE(data)};
        gattc_write_character_value(&prf->head, prf->peer_character, value, sizeof(uint16_t));
    }

    return BT_STS_SUCCESS;
}

void ble_sasc_init(void)
{
    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(sas_prf_t);
    prf_cfg.eatt_preferred = true;
    prf_cfg.enc_required = true;
    sas_prf_id = gattc_register_profile(ble_sas_client_callback, &prf_cfg);
}

#endif /* BES_MOBILE_SAS */
