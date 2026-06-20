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
#if defined(BLE_HID_ENABLE)
#include "gatt_service.h"
#include "me_api.h"

#define HID_PREFERRED_MTU 512

#define HOGP_BOOT_PROTOCOL_MODE     0x00
#define HOGP_REPORT_PROTOCOL_MODE   0x01

#define HOGP_INPUT_REPORT_TYPE      0x01 // send from hid device to hid host, hid host may GET_REPORT
#define HOGP_OUTPUT_REPORT_TYPE     0x02 // send from hid host to hid device, hid host SET_REPORT
#define HOGP_FEATURE_REPORT_TYPE    0x03 // no time-critical app-specific or init info, hid host may GET/SET_REPORT

/**
 * The HID Control Point characteristic is a control-point attribute that defines
 * the following HID Commands when written: Suspend, Exit Suspend. Only a single
 * instance of this characterisitc shall exist as part of the HID Service.
 *
 * Suspend (0x00) informs HID Device that HID Host is entering the Suspend State.
 * Exit Suspend (0x01) informs HID Device that HID Host is exiting the Suspend
 * State.
 *
 */

#define HOGP_CTRL_ENTER_SUSPEND 0x00
#define HOGP_CTRL_EXIT_SUSPEND  0x01

/**
 * RemoteWake flag (bit 0) indicating whether HID Device is capable of sending a
 * wake-signal to a HID Host.
 *
 * NormallyConnectable (bit 1) indicating whehter HID Device will be advertising
 * when bonded but not connected.
 */

#define HOGP_FLAG_REMOTE_WAKE           0x01
#define HOGP_FLAG_NORMAL_CONNECTABLE    0x02

#define HOGP_HID_VERSION 0x0111 // hid v1.11
#define HOGP_COUNTRY_CODE 0x00
#define HOGP_INFO_FLAGS (HOGP_FLAG_REMOTE_WAKE|HOGP_FLAG_NORMAL_CONNECTABLE)

#define HOGP_BOOT_MODE_MOUSE_INSTANCE 0 // boot mode mouse is in which hid service instance
#define HOGP_BOOT_MODE_KEYBOARD_INSTANCE 0 // boot mode keyboard is in which hid service instance

#define HOGPD_NB_HIDS_INST_MAX (2)
#define HOGPD_NB_REPORT_INST_MAX (5)

enum hogpd_report_cfg
{
    /// Input Report
    HOGPD_CFG_REPORT_IN     = 0x01,
    /// Output Report
    HOGPD_CFG_REPORT_OUT    = 0x02,
    // HOGPD_CFG_REPORT_FEAT can be used as a mask to check Report type
    /// Feature Report
    HOGPD_CFG_REPORT_FEAT   = 0x03,
    /// Input report with Write capabilities
    HOGPD_CFG_REPORT_WR     = 0x10,
};

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
static const uint8_t hid_consumer_report_map[] =
{
    HID_REPORT_MODE_SENSOR_DESCRIPTOR_DATA,
};
#else
#if defined(HID_MOUSE)
static const uint8_t hid_consumer_report_map[] =
{
    HID_REPORT_MODE_MOUSE_DESCRIPTOR_DATA,
};
#else
static const uint8_t hid_consumer_report_map[] =
{
    HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_DATA,
};
#endif
#endif

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
#define HOGP_SENSOR_BELONG_INSTANCE 0 // sensor is in which hid service instance
#define HOGP_REPORT_INDEX_SENSOR_DESCR_FEATURE  0
#define HOGP_REPORT_INDEX_SENSOR_STATE_FEATURE  1
#define HOGP_REPORT_INDEX_SENSOR_VALUE_INPUT    2

static const uint8_t hid_report_id[] = {
        HID_SENSOR_DESCR_FEATURE_REPORT_ID,
        HID_SENSOR_STATE_FEATURE_REPORT_ID,
        HID_SENSOR_VALUE_INPUT_REPORT_ID,
    };

static const uint8_t hid_report_type[] = {
        HOGPD_CFG_REPORT_FEAT,
        HOGPD_CFG_REPORT_FEAT,
        HOGPD_CFG_REPORT_IN,
    };
#else
#if defined(HID_MOUSE)
#define HOGP_REPORT_MODE_MOUSE_INSTANCE 0 // report mode mouse is in which hid service instance
#define HOGP_REPORT_INDEX_MOUSECLK_INPUT  0
#define HOGP_REPORT_INDEX_MOUSECTL_INPUT  1

static const uint8_t hid_report_id[] = {
        HID_MOUSECLK_INPUT_REPORT_ID,
        HID_MOUSECTL_INPUT_REPORT_ID,
    };

static const uint8_t hid_report_type[] = {
        HOGPD_CFG_REPORT_IN,
        HOGPD_CFG_REPORT_IN,
    };
#else
#define HOGP_REPORT_MODE_KEYBOARD_INSTANCE 0 // report mode keyboard is in which hid service instance
#define HOGP_REPORT_INDEX_MEDIAKEY_INPUT  0

static const uint8_t hid_report_id[] = {
        HID_MEDIAKEY_INPUT_REPORT_ID,
    };
static const uint8_t hid_report_type[] = {
        HOGPD_CFG_REPORT_IN,
    };
#endif
#endif

typedef struct {
    uint16_t bcd_hid; // version number of base usb hid spec implemented by hid device
    uint8_t country_code; // identify the country hid device hardware is localized for. most hardware is not localized (value 0x00)
    uint8_t flags;
} hogp_hid_info_t;

typedef struct {
    uint8_t proto_mode;
    uint8_t nb_report;
    uint16_t report_map_len;
    const uint8_t *report_map;
    const uint8_t *report_type;
    const uint8_t *report_id;
} ble_hid_report_cfg_t; // report cfg content is fixed no change

typedef struct {
    const ble_hid_report_cfg_t *report_cfg;
    uint8_t proto_mode;
    uint8_t exit_suspend;
    bool keyboard_boot_input_notify_enabled;
    bool mouse_boot_input_notify_enabled;
    uint8_t report_notify_enabled[HOGPD_NB_REPORT_INST_MAX];
    hid_mouse_boot_input_report_t mouse_boot_input;
    hid_keyboard_boot_input_report_t keyboard_boot_input;
    hid_keyboard_boot_output_report_t keyboard_boot_output;
#ifdef HOGP_HEAD_TRACKER_PROTOCOL
    uint8_t sensor_state_feature;
    struct bt_hid_sensor_report_t sensor_report;
#endif
} ble_hid_inst_t;

typedef struct {
    uint8_t nb_hid_inst;
    uint8_t curr_conidx;
    ble_hid_report_cfg_t cfg[HOGPD_NB_HIDS_INST_MAX];
} ble_hid_global_t;

typedef struct {
    gatt_svc_t svc;
    ble_hid_inst_t inst;
} ble_hid_svc_t; // each ble link has num of HOGPD_NB_HIDS_INST_MAX hid service (i.e. ble_hid_svc_t)

static ble_hid_global_t g_ble_hid_global;

ble_hid_global_t *ble_hid_global(void)
{
    return &g_ble_hid_global;
}

GATT_DECL_PRI_SERVICE(g_ble_hid_service,
    GATT_UUID_HID_SERVICE);

GATT_DECL_CHAR_WITH_CONST_VALUE(g_ble_hid_information,
    GATT_CHAR_UUID_HID_INFORMATION,
    GATT_RD_REQ,
    ATT_SEC_NONE,
    CO_SPLIT_UINT16_LE(HOGP_HID_VERSION),
    CO_UINT8_VALUE(HOGP_COUNTRY_CODE),
    CO_UINT8_VALUE(HOGP_INFO_FLAGS));

GATT_DECL_CHAR(g_ble_hid_contrl_point,
    GATT_CHAR_UUID_HID_CTRL_POINT,
    GATT_WR_CMD,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_hid_report_map,
    GATT_CHAR_UUID_HID_REPORT_MAP,
    GATT_RD_REQ,
    ATT_SEC_NONE);

// Protocol Mode: mandatory for HID devices suppoting Boot Protocol Mode, otherwise optional
GATT_DECL_CHAR(g_ble_hid_protocol_mode,
    GATT_CHAR_UUID_HID_PROTOCOL_MODE,
    GATT_RD_REQ|GATT_WR_REQ|GATT_WR_CMD,
    ATT_SEC_NONE);

// Boot Keyboard Input Report: mandatory for HID Devices operting as keyboards, else excluded
GATT_DECL_CHAR(g_ble_hid_boot_keyboard_input_report,
    GATT_CHAR_UUID_BOOT_KEYBOARD_INPUT_REPORT,
    GATT_RD_REQ|GATT_NTF_PROP, // GATT_WR_REQ is optional property
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_hid_boot_keyboard_input_report_cccd,
    ATT_SEC_NONE);

// Boot Keyboard Output Report: mandatory for HID Devices operting as keyboards, else excluded
GATT_DECL_CHAR(g_ble_hid_boot_keyboard_output_report,
    GATT_CHAR_UUID_BOOT_KEYBOARD_OUTPUT_REPORT,
    GATT_RD_REQ|GATT_WR_REQ|GATT_WR_CMD,
    ATT_SEC_NONE);

// Boot Mouse Input Report: mandatory for HID Devices operting as mice, else excluded
GATT_DECL_CHAR(g_ble_hid_boot_mouse_input_report,
    GATT_CHAR_UUID_BOOT_MOUSE_INPUT_REPORT,
    GATT_RD_REQ|GATT_NTF_PROP, // GATT_WR_REQ is optional property
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_hid_boot_mouse_input_report_cccd,
    ATT_SEC_NONE);

GATT_DECL_CHAR(g_ble_hid_input_report_character,
    GATT_CHAR_UUID_HID_REPORT,
    GATT_RD_REQ|GATT_NTF_PROP,
    ATT_SEC_NONE);

POSSIBLY_UNUSED GATT_DECL_CHAR(g_ble_hid_output_report_character,
    GATT_CHAR_UUID_HID_REPORT,
    GATT_RD_REQ|GATT_WR_REQ|GATT_WR_CMD,
    ATT_SEC_NONE);

POSSIBLY_UNUSED GATT_DECL_CHAR(g_ble_hid_feature_report_character,
    GATT_CHAR_UUID_HID_REPORT,
    GATT_RD_REQ|GATT_WR_REQ,
    ATT_SEC_NONE);

GATT_DECL_RRCD_DESCRIPTOR(g_ble_hid_report_rrcd);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_hid_report_cccd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_hid_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_hid_service),
    /* Characteristics */
    gatt_attribute(g_ble_hid_information),
    /* Characteristics */
    gatt_attribute(g_ble_hid_contrl_point),
    /* Characteristics */
    gatt_attribute(g_ble_hid_report_map),
    /* Characteristics */
    gatt_attribute(g_ble_hid_protocol_mode),
    /* Characteristics */
    gatt_attribute(g_ble_hid_boot_keyboard_input_report),
        gatt_attribute(g_ble_hid_boot_keyboard_input_report_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_hid_boot_keyboard_output_report),
    /* Characteristics */
    gatt_attribute(g_ble_hid_boot_mouse_input_report),
        gatt_attribute(g_ble_hid_boot_mouse_input_report_cccd),
    /* Multiple HID Report Characters */
#ifdef HOGP_HEAD_TRACKER_PROTOCOL
    gatt_char_attribute(g_ble_hid_feature_report_character, 0),
        gatt_attribute(g_ble_hid_report_rrcd),
    gatt_char_attribute(g_ble_hid_feature_report_character, 1),
        gatt_attribute(g_ble_hid_report_rrcd),
    gatt_char_attribute(g_ble_hid_input_report_character, 2),
        gatt_attribute(g_ble_hid_report_rrcd),
        gatt_attribute(g_ble_hid_report_cccd),
#else
#if defined(HID_MOUSE)
    gatt_char_attribute(g_ble_hid_input_report_character, 0),
        gatt_attribute(g_ble_hid_report_rrcd),
        gatt_attribute(g_ble_hid_report_cccd),
    gatt_char_attribute(g_ble_hid_input_report_character, 1),
        gatt_attribute(g_ble_hid_report_rrcd),
        gatt_attribute(g_ble_hid_report_cccd),
#else
    gatt_char_attribute(g_ble_hid_input_report_character, 0),
        gatt_attribute(g_ble_hid_report_rrcd),
        gatt_attribute(g_ble_hid_report_cccd),
#endif
#endif
};

static bool hogpd_report_read_ind_handler(ble_hid_svc_t *hid, const uint8_t *character, uint8_t report_idx, void *read_ctx)
{
    if (character == g_ble_hid_boot_keyboard_input_report)
    {
        gatts_write_read_rsp_data(read_ctx, (uint8_t *)&hid->inst.keyboard_boot_input, sizeof(hid_keyboard_boot_input_report_t));
        return true;
    }
    else if (character == g_ble_hid_boot_keyboard_output_report)
    {
        gatts_write_read_rsp_data(read_ctx, (uint8_t *)&hid->inst.keyboard_boot_output, sizeof(hid_keyboard_boot_output_report_t));
        return true;
    }
    else if (character == g_ble_hid_boot_mouse_input_report)
    {
        gatts_write_read_rsp_data(read_ctx, (uint8_t *)&hid->inst.mouse_boot_input, sizeof(hid_mouse_boot_input_report_t));
        return true;
    }

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
    if (gatts_get_char_byte_16_bit_uuid(character) == GATT_CHAR_UUID_HID_REPORT &&
        hid->svc.service_inst_id == HOGP_SENSOR_BELONG_INSTANCE)
    {
        if (report_idx == HOGP_REPORT_INDEX_SENSOR_DESCR_FEATURE)
        {
            uint8_t desc_feature[23 + 10 + 6];
            struct bdaddr_t remote = {{0}};
            uint8_t *dest_remote = desc_feature+23+10;
            app_bt_get_local_device_address(&remote);
            memcpy(desc_feature, "#AndroidHeadTracker#1.0", 23);
            memcpy(desc_feature+23, "\0\0\0\0\0\0\0\0BT", 10);
            dest_remote[0] = remote.address[5]; // copy bd_addr to report from MSB to LSB
            dest_remote[1] = remote.address[4];
            dest_remote[2] = remote.address[3];
            dest_remote[3] = remote.address[2];
            dest_remote[4] = remote.address[1];
            dest_remote[5] = remote.address[0];
            gatts_write_read_rsp_data(read_ctx, desc_feature, sizeof(desc_feature));
            return true;
        }
        else if (report_idx == HOGP_REPORT_INDEX_SENSOR_STATE_FEATURE)
        {
            gatts_write_read_rsp_data(read_ctx, &hid->inst.sensor_state_feature, 1);
            return true;
        }
        else if (report_idx == HOGP_REPORT_INDEX_SENSOR_VALUE_INPUT)
        {
            gatts_write_read_rsp_data(read_ctx, (uint8_t *)&hid->inst.sensor_report, sizeof(struct bt_hid_sensor_report_t));
            return true;
        }
    }
#endif

    return false;
}

static bool hogpd_report_write_ind_handler(ble_hid_svc_t *hid, gatt_server_char_write_t *p, uint8_t report_idx)
{
    if (p->character == g_ble_hid_boot_keyboard_output_report)
    {
        hid_keyboard_boot_output_report_t *report = (hid_keyboard_boot_output_report_t *)p->value;
        hid->inst.keyboard_boot_output = *report;
        return true;
    }

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
    if (gatts_get_char_byte_16_bit_uuid(p->character) == GATT_CHAR_UUID_HID_REPORT &&
        hid->svc.service_inst_id == HOGP_SENSOR_BELONG_INSTANCE)
    {
        if (report_idx == HOGP_REPORT_INDEX_SENSOR_STATE_FEATURE)
        {
            uint8_t sensor_state = p->value[0];
            uint8_t sensor_power_state = (sensor_state & 0x02) ? true : false;
            uint8_t sensor_reporting_state = (sensor_state & 0x01) ? true : false;
            uint8_t sensor_interval = 10 + ((sensor_state & 0xfc) >> 2);
            hid->inst.sensor_state_feature = sensor_state;
            TRACE(0, "hogp head tracker state: sesnor power %d reporting %d interval %d ms",
                sensor_power_state, sensor_reporting_state, sensor_interval);
            return true;
        }
    }
#endif

    return false;
};

static int ble_hid_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    ble_hid_global_t *g = ble_hid_global();
    ble_hid_svc_t *hid = (ble_hid_svc_t *)svc;

    switch (event)
    {
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            if (svc->service_inst_id < g->nb_hid_inst)
            {
                hid->inst.report_cfg = g->cfg + svc->service_inst_id;
                hid->inst.proto_mode = hid->inst.report_cfg->proto_mode;
                memset(&hid->inst.keyboard_boot_input, 0, sizeof(hid->inst.keyboard_boot_input));
                memset(&hid->inst.keyboard_boot_output, 0, sizeof(hid->inst.keyboard_boot_output));
                memset(&hid->inst.mouse_boot_input, 0, sizeof(hid->inst.mouse_boot_input));
#ifdef HOGP_HEAD_TRACKER_PROTOCOL
                if (svc->service_inst_id == HOGP_SENSOR_BELONG_INSTANCE)
                {
                    hid->inst.sensor_state_feature = 0;
                    memset(&hid->inst.sensor_report, 0, sizeof(hid->inst.sensor_report));
                }
#endif
            }
            else
            {
                CO_LOG_ERR_2(BT_STS_INVALID_HID_INST_ID, svc->service_inst_id, g->nb_hid_inst);
            }
            if (g->curr_conidx == GAP_INVALID_CONIDX)
            {
                g->curr_conidx = svc->con_idx;
            }
            break;
        }
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            if (g->curr_conidx == svc->con_idx)
            {
                g->curr_conidx = GAP_INVALID_CONIDX;
            }
            break;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            uint8_t report_index = p->char_attr->inst_id;
            if (p->character == g_ble_hid_report_map)
            {
                if (report_index < hid->inst.report_cfg->nb_report)
                {
                    gatts_write_read_rsp_data(p->ctx, hid->inst.report_cfg->report_map, hid->inst.report_cfg->report_map_len);
                    return true;
                }
                else
                {
                    CO_LOG_ERR_2(BT_STS_INVALID_REPORT_INDEX, report_index, hid->inst.report_cfg->nb_report);
                }
            }
            else if (p->character == g_ble_hid_protocol_mode)
            {
                gatts_write_read_rsp_data(p->ctx, &hid->inst.proto_mode, 1);
                return true;
            }
            else
            {
                if (hogpd_report_read_ind_handler(hid, p->character, report_index, p->ctx))
                {
                    return true;
                }
            }
            break;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            uint8_t report_index = p->char_attr->inst_id;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            if (p->character == g_ble_hid_contrl_point)
            {
                hid->inst.exit_suspend = p->value[0] ? true : false;
                return true;
            }
            else if (p->character == g_ble_hid_protocol_mode)
            {
                hid->inst.proto_mode = p->value[0] ? HOGP_REPORT_PROTOCOL_MODE : HOGP_BOOT_PROTOCOL_MODE;
                return true;
            }
            else
            {
                if (hogpd_report_write_ind_handler(hid, p, report_index))
                {
                    return true;
                }
            }
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            uint8_t report_index = p->char_attr->inst_id;
            uint16_t cccd_config = 0;
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_report_rrcd)
            {
                if (report_index < hid->inst.report_cfg->nb_report)
                {
                    gatt_rrcd_value_t value;
                    value.report_id = hid->inst.report_cfg->report_id[report_index];
                    value.report_type = hid->inst.report_cfg->report_type[report_index];
                    gatts_write_read_rsp_data(p->ctx, (uint8_t *)&value, sizeof(gatt_rrcd_value_t));
                    return true;
                }
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_boot_keyboard_input_report_cccd)
            {
                cccd_config = hid->inst.keyboard_boot_input_notify_enabled ? 0x0001 : 0;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_boot_mouse_input_report_cccd)
            {
                cccd_config = hid->inst.mouse_boot_input_notify_enabled ? 0x0001 : 0;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_report_cccd)
            {
                if (report_index < hid->inst.report_cfg->nb_report)
                {
                    cccd_config = hid->inst.report_notify_enabled[report_index] ? 0x0001 : 0;
                }
                else
                {
                    CO_LOG_ERR_2(BT_STS_INVALID_REPORT_INDEX, report_index, hid->inst.report_cfg->nb_report);
                    return false;
                }
            }

            cccd_config = co_host_to_uint16_le(cccd_config);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            return true;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint8_t report_index = p->char_attr->inst_id;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            bool notify_enabled = false;
            if (config & GATT_CCCD_SET_NOTIFICATION)
            {
                notify_enabled = true;
            }
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_boot_keyboard_input_report_cccd)
            {
                hid->inst.keyboard_boot_input_notify_enabled = notify_enabled;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_boot_mouse_input_report_cccd)
            {
                hid->inst.mouse_boot_input_notify_enabled = notify_enabled;
            }
            else if ((uint8_t *)p->desc_attr->attr_data == g_ble_hid_report_cccd)
            {
                if (report_index < hid->inst.report_cfg->nb_report)
                {
                    hid->inst.report_notify_enabled[report_index] = notify_enabled;
                }
                else
                {
                    CO_LOG_ERR_2(BT_STS_INVALID_REPORT_INDEX, report_index, hid->inst.report_cfg->nb_report);
                }
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

void ble_hid_device_init(void)
{
    ble_hid_global_t *g = ble_hid_global();
    ble_hid_report_cfg_t *curr_cfg = NULL;
    uint8_t nb_hid_inst = 0;
    uint8_t nb_report = 0;
    int i = 0;

    nb_hid_inst = 1;
    if (nb_hid_inst > HOGPD_NB_HIDS_INST_MAX)
    {
        nb_hid_inst = HOGPD_NB_HIDS_INST_MAX;
    }

    g->curr_conidx = GAP_INVALID_CONIDX;
    g->nb_hid_inst = nb_hid_inst;

    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(ble_hid_svc_t);
    cfg.preferred_mtu = HID_PREFERRED_MTU;
    cfg.eatt_preferred = false;

    for (i = 0; i < nb_hid_inst; i += 1)
    {
        curr_cfg = g->cfg + i;

        nb_report = sizeof(hid_report_id)/sizeof(uint8_t);
        if (nb_report > HOGPD_NB_REPORT_INST_MAX)
        {
            nb_report = HOGPD_NB_REPORT_INST_MAX;
        }

        curr_cfg->proto_mode = HOGP_REPORT_PROTOCOL_MODE;
        curr_cfg->nb_report = nb_report;
        curr_cfg->report_map_len = sizeof(hid_consumer_report_map);
        curr_cfg->report_map = hid_consumer_report_map;
        curr_cfg->report_type = hid_report_id;
        curr_cfg->report_id = hid_report_type;

        cfg.service_inst_id = i;
        gatts_register_service(g_ble_hid_attr_list, ARRAY_SIZE(g_ble_hid_attr_list), ble_hid_server_callback, &cfg);
    }
}

static void ble_hid_device_send_boot_report(uint8_t hid_inst_id, const uint8_t *character, const uint8_t *report, uint16_t len)
{
    ble_hid_global_t *g = ble_hid_global();
    ble_hid_svc_t *hid = NULL;
    uint8_t conidx = g->curr_conidx;

    if (conidx == GAP_INVALID_CONIDX)
    {
        return;
    }

    if (bt_defer_curr_func_4(ble_hid_device_send_boot_report,
            bt_fixed_param(hid_inst_id),
            bt_fixed_param(character),
            bt_alloc_param_size(report, len),
            bt_fixed_param(len)))
    {
        return;
    }

    hid = (ble_hid_svc_t *)gatts_get_service(gap_conn_idx_as_hdl(conidx), g_ble_hid_service, hid_inst_id);
    if (hid == NULL)
    {
        return;
    }

    gatts_send_notification(gap_conn_bf(conidx), character, report, len);
}

bt_status_t ble_hid_send_boot_mouse_report(const hid_mouse_boot_input_report_t *report)
{
    ble_hid_device_send_boot_report(HOGP_BOOT_MODE_MOUSE_INSTANCE, g_ble_hid_boot_mouse_input_report,
        (uint8_t *)&report, sizeof(report));
    return BT_STS_SUCCESS;
}

bt_status_t ble_hid_send_boot_keyboard_report(const hid_keyboard_boot_input_report_t *report)
{
    ble_hid_device_send_boot_report(HOGP_BOOT_MODE_KEYBOARD_INSTANCE, g_ble_hid_boot_keyboard_input_report,
        (uint8_t *)&report, sizeof(report));
    return BT_STS_SUCCESS;
}

static void ble_hid_device_send_report(uint8_t hid_inst_id, uint8_t report_idx, const uint8_t *report, uint16_t len)
{
    ble_hid_global_t *g = ble_hid_global();
    ble_hid_svc_t *hid = NULL;
    uint8_t conidx = g->curr_conidx;
    gatt_char_notify_t notify = {0};

    if (conidx == GAP_INVALID_CONIDX)
    {
        return;
    }

    if (bt_defer_curr_func_4(ble_hid_device_send_report,
            bt_fixed_param(hid_inst_id),
            bt_fixed_param(report_idx),
            bt_alloc_param_size(report, len),
            bt_fixed_param(len)))
    {
        return;
    }

    hid = (ble_hid_svc_t *)gatts_get_service(gap_conn_idx_as_hdl(conidx), g_ble_hid_service, hid_inst_id);
    if (hid == NULL || report_idx > hid->inst.report_cfg->nb_report)
    {
        return;
    }

#if defined(HID_MOUSE)
    if (hid->inst.report_cfg->report_id[report_idx] == HID_MOUSECLK_INPUT_REPORT_ID &&
        hid->inst.proto_mode == HOGP_BOOT_PROTOCOL_MODE)
    {
        ble_hid_send_boot_mouse_report((hid_mouse_boot_input_report_t *)report);
        return;
    }
#endif

    notify.character = g_ble_hid_input_report_character;
    notify.char_instance_id = report_idx;

    gatts_send_value_notification(gap_conn_bf(conidx), &notify, report, len);

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
    if (hid->inst.report_cfg->report_id[report_idx] == HID_SENSOR_VALUE_INPUT_REPORT_ID)
    {
        hid->inst.sensor_report = *((struct bt_hid_sensor_report_t *)report);
    }
#endif
}

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
bt_status_t ble_hid_send_sensor_report(const struct bt_hid_sensor_report_t *report)
{
    ble_hid_device_send_report(HOGP_SENSOR_BELONG_INSTANCE, HOGP_REPORT_INDEX_SENSOR_VALUE_INPUT,
        (uint8_t *)report, sizeof(struct bt_hid_sensor_report_t));
    return BT_STS_SUCCESS;
}
#else
#if defined(HID_MOUSE)
bt_status_t ble_hid_send_mouse_input_report(const hid_mouse_boot_input_report_t *report)
{
    ble_hid_device_send_report(HOGP_REPORT_MODE_MOUSE_INSTANCE, HOGP_REPORT_INDEX_MOUSECLK_INPUT,
        (uint8_t *)report, sizeof(hid_mouse_boot_input_report_t));
    return BT_STS_SUCCESS;
}

bt_status_t ble_hid_send_mouse_control_report(const hid_mousectl_report_t *report)
{
    ble_hid_device_send_report(HOGP_REPORT_MODE_MOUSE_INSTANCE, HOGP_REPORT_INDEX_MOUSECTL_INPUT,
        (uint8_t *)report, sizeof(hid_mousectl_report_t));
    return BT_STS_SUCCESS;
}
#else
bt_status_t ble_hid_send_mediakey_report(const struct keyboard_control_key_t *report)
{
    ble_hid_device_send_report(HOGP_REPORT_MODE_KEYBOARD_INSTANCE, HOGP_REPORT_INDEX_MEDIAKEY_INPUT,
        (uint8_t *)report, sizeof(struct keyboard_control_key_t));
    return BT_STS_SUCCESS;
}
#endif
#endif

#endif /* BLE_HID_ENABLE */
