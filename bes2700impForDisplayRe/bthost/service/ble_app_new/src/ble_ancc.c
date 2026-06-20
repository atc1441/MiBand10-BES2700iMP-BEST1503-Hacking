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
#if defined(ANCC_ENABLED)
#include "gatt_service.h"

#define ANCC_MAX_LEN (128)

/**
 * Only one instance of the ANCS (Apple Notification Center Service) may be present on an NP.
 * Mobile Phone is NP, it has ANCS record, another device (NC) need register notifications to NP.
 * Then NP send notifications to NC. If NC need more detail about the notification, NC wirte
 * ANCS control pointer character to get more info.
 *
 */

#define ANC_SERVICE_UUID_128_LE 0xD0,0x00,0x2D,0x12,0x1E,0x4B,0x0F,0xA4,0x99,0x4E,0xCE,0xB5,0x31,0xF4,0x05,0x79
#define anc_ntf_source_uuid_128_le  0xBD,0x1D,0xA2,0x99,0xE6,0x25,0x58,0x8C,0xD9,0x42,0x01,0x63,0x0D,0x12,0xBF,0x9F
#define anc_data_source_uuid_128_le 0xFB,0x7B,0x7C,0xCE,0x6A,0xB3,0x44,0xBE,0xB5,0x4B,0xD6,0x24,0xE9,0xC6,0xEA,0x22
#define anc_ctl_point_uuid_128_le   0xD9,0xD9,0xAA,0xFD,0xBD,0x9B,0x21,0x98,0xA8,0x49,0xE1,0x45,0xF3,0xD8,0xD1,0x69

static uint8_t anc_prf_id;

typedef enum {
    ANC_CHAR_DATA_SOURCE,
    ANC_CHAR_NTFY_SOURCE,
    ANC_CHAR_CTRL_POINT,
    ANC_CHAR_MAX_NUM,
} anc_char_enum_t;

enum anc_event_id {
    EventIDNtfAdded = 0,
    EventIDNtfModified = 1,
    EventIDNtfRemoved = 2,
    EventIDNtfReserved = 3,
};

enum anc_category_id {
    IdOther                     = 0,
    IdIncomingcall              = 1,
    IdMissedCall                = 2,
    IdVoiceMail                 = 3,
    IdSocial                    = 4,
    IdSchedual                  = 5,
    IdEmail                     = 6,
    IdNews                      = 7,
    IdHealthandFitness          = 8,
    IdBusinessandFinance        = 9,
    IdLocation                  = 10,
    IdEntertainment             = 11,
    IdReserved                  = 12,
};

/// ANC Control Point: Command ID
enum anc_cmd_id {
    CmdIdGetNtfAttr     = 0,
    CmdIdGetAppAttr     = 1,
    CmdIdGPerfNtfAction = 2,
    CmdIdReserved       = 3,
};

/// ANC Control Point: Ntofication Attribute ID
enum anc_ntf_attr_id {
    AttrIdAppId     = 0,
    AttrIdTitle     = 1,  //need to be followed by a 2bytes max lengeth parameters
    AttrIdSubTitle  = 2,  //need to be followed by a 2bytes max lengeth parameters
    AttrIdMsg       = 3,  //need to be followed by a 2bytes max lengeth parameters
    AttrIdMsgSize   = 4,
    AttrIdDate      = 5,
    AttrIdPositiveActionLabel      = 6,
    AttrIdNegativeActionLabel      = 7,
    AttrIdReserved      = 8,
};

/// ANC: Action ID
enum anc_action_id {
    ActionIdPositive    = 0,
    ActionIdNegative    = 1,
    ActionIdReserved    = 2,
};

/// ANC: App Attribute ID
enum anc_app_attr_id {
    AppAttrIdDisplayName    = 0,
    AppAttrIdReserved       = 1,
};

typedef struct {
    uint8_t other;
    uint8_t incomingcall;
    uint8_t missedcall;
    uint8_t voicemail;
    uint8_t social;
    uint8_t schedual;
    uint8_t email;
    uint8_t news;
    uint8_t healthfitness;
    uint8_t businessfinance;
    uint8_t location;
    uint8_t entertrainment;
} anc_category_count;

typedef struct {
    bool appId;
    bool title;
    bool subTitle;
    bool msg;
    bool msgSize;
    bool date;
    uint16_t titleLen;
    uint16_t subtitleLen;
    uint16_t msgLen;
} app_anc_get_msg_detail;

typedef struct {
    uint32_t ntfId;
    uint8_t *att;
    uint32_t attLen;
} app_anc_get_ntf_attr_param;

typedef struct {
    uint32_t appIdLen;
    char *appId;
    uint8_t *att;
    uint32_t attLen;
} app_anc_get_app_attr_param;

typedef struct {
    uint32_t ntfUid;
    enum anc_action_id actionId;
} app_anc_perform_ntf_act_param;

typedef struct {
    uint8_t evtID;
    uint8_t evtFlag;
    uint8_t cateID;
    uint8_t cateCount;
    uint32_t ntfUID;
} anc_notification_info;

typedef struct {
    enum anc_cmd_id cmdId;
    uint8_t param[ANCC_MAX_LEN];
} app_anc_info_param;

typedef struct {
    gatt_prf_t head;
    bool peer_ds_write_notified;
    bool peer_ns_write_notified;
    anc_category_count count;
    gatt_peer_service_t *peer_service;
    gatt_peer_character_t *peer_char[ANC_CHAR_MAX_NUM];
} anc_prf_t;

void ancc_write_req(uint16_t connhdl, anc_char_enum_t char_enum, const uint8_t *value, uint16_t len)
{
    anc_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;
    bool notify_enabled = false;

    if (char_enum >= ANC_CHAR_MAX_NUM || value == NULL || len == 0)
    {
        return;
    }

    prf = (anc_prf_t *)gattc_get_profile(anc_prf_id, connhdl);
    if (prf == NULL)
    {
        return;
    }

    c = prf->peer_char[char_enum];
    if (c == NULL)
    {
        return;
    }

    if (char_enum == ANC_CHAR_CTRL_POINT)
    {
        gattc_write_character_value(&prf->head, c, value, len);
    }
    else if (char_enum == ANC_CHAR_DATA_SOURCE)
    {
        notify_enabled = value[0] ? true : false;
        gattc_write_cccd_descriptor(&prf->head, c, notify_enabled, false);
        prf->peer_ds_write_notified = notify_enabled;
    }
    else if (char_enum == ANC_CHAR_NTFY_SOURCE)
    {
        notify_enabled = value[0] ? true : false;
        gattc_write_cccd_descriptor(&prf->head, c, notify_enabled, false);
        prf->peer_ns_write_notified = notify_enabled;
    }
}

void ancc_read_req(uint16_t connhdl, anc_char_enum_t char_enum)
{
    anc_prf_t *prf = NULL;
    gatt_peer_character_t *c = NULL;

    if (char_enum >= ANC_CHAR_MAX_NUM)
    {
        return;
    }

    prf = (anc_prf_t *)gattc_get_profile(anc_prf_id, connhdl);
    if (prf == NULL)
    {
        return;
    }

    c = prf->peer_char[char_enum];
    if (c == NULL)
    {
        return;
    }

    if (char_enum == ANC_CHAR_DATA_SOURCE || char_enum == ANC_CHAR_NTFY_SOURCE)
    {
        gattc_read_descriptor_value(&prf->head, c, GATT_DESC_UUID_CHAR_CLIENT_CONFIG);
    }
}

void app_ancc_get_notification_info(uint16_t connhdl, app_anc_get_ntf_attr_param *param)
{
    app_anc_info_param infoParam;
    uint16_t len = param->attLen + sizeof(uint32_t) + sizeof(enum anc_cmd_id);

    infoParam.cmdId = CmdIdGetNtfAttr;
    memcpy(infoParam.param, (uint8_t *)&(param->ntfId), sizeof(uint32_t));
    memcpy(infoParam.param + sizeof(uint32_t), param->att, param->attLen);

    ancc_write_req(connhdl, ANC_CHAR_CTRL_POINT, (uint8_t *)&infoParam, len);
}

void app_ancc_get_app_info(uint16_t connhdl, app_anc_get_app_attr_param *param)
{
    app_anc_info_param infoParam;
    uint16_t len = param->appIdLen + param->attLen + sizeof(enum anc_cmd_id);

    infoParam.cmdId = CmdIdGetAppAttr;
    memcpy(infoParam.param, (uint8_t *)&(param->appId), param->appIdLen);
    memcpy(infoParam.param + param->appIdLen, param->att, param->attLen);

    ancc_write_req(connhdl, ANC_CHAR_CTRL_POINT, (uint8_t *)&infoParam, len);
}

void app_ancc_get_message_detail(uint16_t connhdl, uint32_t uid, app_anc_get_msg_detail detail)
{
    app_anc_get_ntf_attr_param ntfInfo;
    uint8_t att[32] = {0};
    uint8_t len = 0;

    if (detail.appId) {
        att[len++] = AttrIdAppId;
    }

    if (detail.title) {
        att[len++] = AttrIdTitle;
        att[len] = detail.titleLen;
        len += 2;
    }

    if(detail.subTitle) {
        att[len++] = AttrIdSubTitle;
        att[len] = detail.subtitleLen;
        len += 2;
    }

    if (detail.msg) {
        att[len++] = AttrIdMsg;
        att[len] = detail.msgLen;
        len += 2;
    }

    if (detail.msgSize) {
        att[len++] = AttrIdMsgSize;
    }

    if (detail.date) {
        att[len++] = AttrIdDate;
    }

    ntfInfo.ntfId = uid;
    ntfInfo.att = att;
    ntfInfo.attLen = len;
    app_ancc_get_notification_info(connhdl, &ntfInfo);
}

void app_anc_parse_message_detail(const uint8_t *data, uint32_t dataLen)
{
    uint32_t uid = (uint32_t)(data[1]);
    uint32_t len = sizeof(uint32_t) + sizeof(enum anc_cmd_id);
    uint8_t attrId;
    uint16_t attrLen = 0;
    TRACE(1, "Received ANC rsp ntf attr uid:%d", uid);

    while (dataLen > len)
    {
        attrId = data[len];
        attrLen = (uint16_t)(data[len+1]);
        len += (3+attrLen);
        TRACE(1, "Received ANC attr:%d data", attrId);
        DUMP8("%2x ", data+len-attrLen, attrLen);

    }
}

void app_anc_parse_app_detail(const uint8_t *data, uint32_t dataLen)
{
    uint32_t uid = (uint32_t)(data[1]);
    uint32_t len = sizeof(enum anc_cmd_id);
    uint8_t attrId;
    uint16_t attrLen = 0;
    TRACE(1, "Received ANC rsp ntf attr uid:%d", uid);

    while (data[len] != '\0')
    {
        len++;
    }

    while (dataLen > len)
    {
        attrId = data[len];
        attrLen = (uint16_t)(data[len+1]);
        len += (3+attrLen);
        TRACE(1, "Received ANC attr:%d data", attrId);
        DUMP8("%2x ", data+len-attrLen, attrLen);

    }
}

#ifdef ANCS_ENABLED
void ancs_proxy_send_notification(uint16_t connhdl, anc_char_enum_t char_enum, const uint8_t *value, uint16_t len);
void ancs_proxy_set_ready_flag(uint16_t connhdl, bool peer_ds_discovered, bool peer_ns_discovered, bool peer_cp_discovered);
#endif

static void app_ancc_parse_notification_info(anc_prf_t *prf, anc_char_enum_t char_enum, const uint8_t *value, uint16_t len)
{
    if (value == NULL || len == 0)
    {
        return;
    }

    if (char_enum == ANC_CHAR_DATA_SOURCE)
    {
         if (value[0] == CmdIdGetNtfAttr)
         {
             app_anc_parse_message_detail(value, len);
#ifdef ANCS_ENABLED
             ancs_proxy_send_notification(prf->head.connhdl, ANC_CHAR_DATA_SOURCE, value, len);
#endif
         }
         else if (value[0] == CmdIdGetAppAttr)
         {
             app_anc_parse_app_detail(value, len);
#ifdef ANCS_ENABLED
            ancs_proxy_send_notification(prf->head.connhdl, ANC_CHAR_DATA_SOURCE, value, len);
#endif
         }
    }
    else if (char_enum == ANC_CHAR_NTFY_SOURCE)
    {
        anc_notification_info *info = (anc_notification_info *)value;
        uint8_t flag = info->evtFlag;
        uint8_t categoryId = info->cateID;
        uint8_t count = info->cateCount;

        if (categoryId < sizeof(anc_category_count))
        {
            uint8_t *p_count = (uint8_t *)&prf->count;
            p_count[categoryId] = count;
        }

        TRACE(3, "Received ANC notification count:%d, id:%d, flag:%d", count, categoryId, flag);

        switch (info->evtID)
        {
            case EventIDNtfAdded:
            {
                TRACE(0, "Received ANC notification added.");
                app_anc_get_msg_detail detail = 
                    {
                        .appId = true,
                        .title = true,
                        .subTitle = true,
                        .msg = true,
                        .msgSize = true,
                        .date = true,
                        .titleLen = 16,
                        .subtitleLen = 16,
                        .msgLen = 128,
                    };
                app_ancc_get_message_detail(prf->head.connhdl, info->ntfUID, detail);
                break;
            }
            case EventIDNtfModified:
            {
                TRACE(0, "Received ANC notification modified.");
#ifdef ANCS_ENABLED
                ancs_proxy_send_notification(prf->head.connhdl, ANC_CHAR_NTFY_SOURCE, value, len);
#endif
                break;
            }
            case EventIDNtfRemoved:
            {
                TRACE(0, "Received ANC notification removed.");
#ifdef ANCS_ENABLED
                ancs_proxy_send_notification(prf->head.connhdl, ANC_CHAR_NTFY_SOURCE, value, len);
#endif
                break;
            }
            default:
                break;
        }
    }
}

bt_status_t ble_ancc_start_discover(uint16_t connhdl)
{
    anc_prf_t *prf = NULL;
    uint8_t anc_service_uuid_le[16] = {ANC_SERVICE_UUID_128_LE};

    prf = (anc_prf_t *)gattc_get_profile(anc_prf_id, connhdl);
    if (prf == NULL)
    {
        return BT_STS_FAILED;
    }

    gattc_discover_service(&prf->head, 0, anc_service_uuid_le);
    return BT_STS_SUCCESS;
}

static int ble_anc_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    anc_prf_t *anc = (anc_prf_t *)prf;

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            break;
        }
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = param.service;
            gatt_peer_service_t *s = p->service;
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                break;
            }
            anc->peer_service = s;
            gatt_char_uuid_t gap_chars[] = {
                    {{anc_data_source_uuid_128_le}, true},
                    {{anc_ntf_source_uuid_128_le}, true},
                    {{anc_ctl_point_uuid_128_le}, true},
                };
            gattc_discover_multi_128_characters(prf, s, gap_chars, ARRAY_SIZE(gap_chars));
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_character_t *c = p->character;
            if (p->discover_idx < ANC_CHAR_MAX_NUM)
            {
                if (p->error_code == ATT_ERROR_NO_ERROR)
                {
                    anc->peer_char[p->discover_idx] = c;
                }
                else
                {
                    anc->peer_char[p->discover_idx] = NULL;
                }
            }
            if (p->discover_cmpl)
            {
                if (anc->peer_char[ANC_CHAR_DATA_SOURCE])
                {
                    gattc_write_cccd_descriptor(prf, anc->peer_char[ANC_CHAR_DATA_SOURCE], true, false);
                    anc->peer_ds_write_notified = true;
                }

                if (anc->peer_char[ANC_CHAR_NTFY_SOURCE])
                {
                    gattc_write_cccd_descriptor(prf, anc->peer_char[ANC_CHAR_NTFY_SOURCE], true, false);
                    anc->peer_ns_write_notified = true;
                }

#ifdef ANCS_ENABLED
                ancs_proxy_set_ready_flag(prf->connhdl,
                    anc->peer_char[ANC_CHAR_DATA_SOURCE] ? true : false,
                    anc->peer_char[ANC_CHAR_NTFY_SOURCE] ? true : false,
                    anc->peer_char[ANC_CHAR_CTRL_POINT] ? true : false);
#endif
            }
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *p = param.notify;
            gatt_peer_service_uuid_t service_uuid = gattc_get_service_uuid(p->service);
            uint8_t anc_service_uuid_le[16] = {ANC_SERVICE_UUID_128_LE};
            if (memcmp(service_uuid.uuid_le, anc_service_uuid_le, 16) != 0)
            {
                break;
            }
            if (p->character == anc->peer_char[ANC_CHAR_DATA_SOURCE])
            {
                app_ancc_parse_notification_info(anc, ANC_CHAR_DATA_SOURCE, p->value, p->value_len);
            }
            else if (p->character == anc->peer_char[ANC_CHAR_NTFY_SOURCE])
            {
                app_ancc_parse_notification_info(anc, ANC_CHAR_NTFY_SOURCE, p->value, p->value_len);
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        case GATT_PROF_EVENT_DESC_READ_RSP:
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        default:
        {
            break;
        }
    }

    return 0;
}

void ble_ancc_init(void)
{
    gattc_cfg_t prf_cfg = {0};
    prf_cfg.prf_size = sizeof(anc_prf_t);
    anc_prf_id = gattc_register_profile(ble_anc_client_callback, &prf_cfg);
}

#endif /* ANCC_ENABLED */
