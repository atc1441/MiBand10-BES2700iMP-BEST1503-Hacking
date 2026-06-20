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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "app_gaf.h"
#include "app_ble.h"
#include "ble_app_dbg.h"
#include "app_bap.h"
#include "app_arc.h"
#include "app_acc.h"
#include "app_atc.h"
#include "app_cap.h"
#include "app_hap.h"
#include "app_tmap.h"
#include "app_gmap.h"
#include "app_gaf_custom_api.h"
#include "app_iap_tm_msg.h"
#include "app_acc_dts_msg.h"

#include "beslib_info.h"

/*EXTERN ITF*/
/*BAP*/
#if APP_GAF_BAP_ENABLE
extern int app_bap_server_init(app_bap_capa_srv_dir_t *sink_capa_info, app_bap_capa_srv_dir_t *src_capa_info, uint32_t curr_role_bf);
extern int app_bap_server_deinit(void);
#ifdef AOB_MOBILE_ENABLED
extern int app_bap_client_init(void);
extern int app_bap_client_deinit(void);
#endif /// AOB_MOBILE_ENABLED
#endif /// APP_GAF_BAP_ENABLE
/*ARC*/
#if APP_GAF_ARC_ENABLE
extern int app_arc_server_init(void);
extern int app_arc_server_deinit(void);
#ifdef AOB_MOBILE_ENABLED
/* ble audio gaf arc(Audio Rendering Control) init */
extern int app_arc_client_init(void);
extern int app_arc_client_deinit(void);
#endif /// AOB_MOBILE_ENABLED
#endif /// APP_GAF_ARC_ENABLE
/*ACC*/
#if APP_GAF_ACC_ENABLE
/* ble audio gaf acc(Audio Content Control) init */
extern int app_acc_client_init(void);
extern int app_acc_client_deinit(void);
#ifdef AOB_MOBILE_ENABLED
extern int app_acc_server_init(void);
extern int app_acc_server_deinit(void);
#endif
#endif /// APP_GAF_ACC_ENABLE
/*ATC*/
#if APP_GAF_ATC_ENABLE
#ifdef AOB_MOBILE_ENABLED
/* ble audio gaf atc(Audio Topology Control) init */
extern int app_atc_client_init(void);
extern int app_atc_client_deinit(void);
#endif
extern int app_atc_server_init(void);
extern int app_atc_server_deinit(void);
#endif /// APP_GAF_ATC_ENABLE
/*GMAP*/
#if APP_GAF_GMAP_ENABLE
/* ble audio gaf gmap (Gaming Audio profile) init */
extern int app_gmap_client_init(void);
extern int app_gmap_client_deinit(void);
extern int app_gmap_server_init(uint8_t role_bf);
extern int app_gmap_server_deinit(void);
#endif /// APP_GAF_GMAP_ENABLE
/*TMAP*/
#if APP_GAF_TMAP_ENABLE
/* ble audio gaf tmap (Telephony and Media AUdio profile) init */
extern int app_tmap_client_init(void);
extern int app_tmap_client_deinit(void);
extern int app_tmap_server_init(uint16_t);
extern int app_tmap_server_deinit(void);
#endif /// APP_GAF_TMAP_ENABLE
/*IAP TM*/
#if APP_GAF_IAP_TM_ENABLE
extern int app_iap_tm_init(void);
#endif /// APP_GAF_IAP_TM_ENABLE
/*HAP*/
#if APP_GAF_HAP_ENABLE
#ifdef AOB_MOBILE_ENABLED
extern int app_hap_client_init(void);
extern int app_hap_client_deinit(void);
#endif
extern int app_hap_server_init(void);
extern int app_hap_server_deinit(void);
#endif /// APP_GAF_HAP_ENABLE
/*CAP*/
#if APP_GAF_CAP_ENABLE
#ifdef AOB_MOBILE_ENABLED
/* ble audio gaf cap(Common Audio Control) init */
extern int app_cap_client_init(void);
extern int app_cap_client_deinit(void);
#endif /// AOB_MOBILE_ENABLED
extern int app_cap_server_init(void);
extern int app_cap_server_deinit(void);
#endif /// APP_GAF_CAP_ENABLE


/*INTERNAL VARIABLE*/
static GAF_EVT_REPORT_BUNDLE_T _report_bundle =
{
    .earbud_report = NULL,
#ifdef AOB_MOBILE_ENABLED
    .mobile_report = NULL,
#endif
};

#if 0
static uint32_t app_gaf_cmp_evt_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    if (NULL == param)
    {
        return (KE_MSG_CONSUMED);
    }

    gaf_cmp_evt_t *p_cmp_evt = (gaf_cmp_evt_t *)param;
    uint8_t layer = GAF_LAYER(p_cmp_evt->cmd_code);

    switch (layer)
    {
        case (GAF_LAYER_BAP):
        {
            app_bap_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_ARC):
        {
            app_arc_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_ACC):
        {
            app_acc_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_ATC):
        {
            app_atc_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_CAP):
        {
            app_cap_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_IAP):
        {
            app_iap_tm_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_TMAP):
        {
            app_tmap_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_HAP):
        {
            app_hap_cmp_evt_handler(param);
        }
        break;

        case (GAF_LAYER_GMAP):
        {
            app_gmap_cmp_evt_handler(param);
        }
        break;

        default:
        {
            LOG_W("gaf unsupported cmp evt msg, layer = %d", layer);
        }
        break;
    }

    return (KE_MSG_CONSUMED);
}

static uint32_t app_gaf_rsp_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (NULL == param)
    {
        return (KE_MSG_CONSUMED);
    }

    gaf_rsp_t *p_rsp = (gaf_rsp_t *)param;
    uint8_t layer = GAF_LAYER(p_rsp->req_code);

    switch (layer)
    {
        case (GAF_LAYER_BAP):
        {
            app_bap_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_ARC):
        {
            app_arc_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_ACC):
        {
            app_acc_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_ATC):
        {
            app_atc_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_CAP):
        {
            app_cap_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_HAP):
        {
            app_hap_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_TMAP):
        {
            app_tmap_rsp_handler(param);
        }
        break;

        case (GAF_LAYER_GMAP):
        {
            app_gmap_rsp_handler(param);
        }
        break;

        default:
        {
            LOG_W("gaf unsupported rsp msg, layer = %d", layer);
        }
        break;
    }

    return (KE_MSG_CONSUMED);
}

static uint32_t app_gaf_ind_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (NULL == param)
    {
        return (KE_MSG_CONSUMED);
    }

    gaf_ind_t *p_ind = (gaf_ind_t *)param;
    uint8_t layer = GAF_LAYER(p_ind->ind_code);

    switch (layer)
    {
        case (GAF_LAYER_BAP):
        {
            app_bap_ind_handler(param);
        }
        break;

        case (GAF_LAYER_ARC):
        {
            app_arc_ind_handler(param);
        }
        break;

        case (GAF_LAYER_ACC):
        {
            app_acc_ind_handler(param);
        }
        break;

        case (GAF_LAYER_ATC):
        {
            app_atc_ind_handler(param);
        }
        break;

        case (GAF_LAYER_CAP):
        {
            app_cap_ind_handler(param);
        }
        break;

        case (GAF_LAYER_TMAP):
        {
            app_tmap_ind_handler(param);
        }
        break;

        case (GAF_LAYER_IAP):
        {
            app_iap_tm_ind_handler(param);
        }
        break;

        case (GAF_LAYER_HAP):
        {
            app_hap_ind_handler(param);
        }
        break;

        case (GAF_LAYER_GMAP):
        {
            app_gmap_ind_handler(param);
        }
        break;

        default:
        {
            LOG_W("gaf unsupported ind msg, layer = %d", layer);
        }
        break;
    }

    return (KE_MSG_CONSUMED);
}

static uint32_t app_gaf_req_ind_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    if (NULL == param)
    {
        return (KE_MSG_CONSUMED);
    }

    gaf_req_ind_t *p_req_ind = (gaf_req_ind_t *)param;
    uint8_t layer = GAF_LAYER(p_req_ind->req_ind_code);

    switch (layer)
    {
        case (GAF_LAYER_BAP):
        {
            app_bap_req_ind_handler(param);
        }
        break;

        case (GAF_LAYER_ARC):
        {
            app_arc_req_ind_handler(param);
        }
        break;

        case (GAF_LAYER_ACC):
        {
            app_acc_req_ind_handler(param);
        }
        break;

        case (GAF_LAYER_ATC):
        {
            app_atc_req_ind_handler(param);
        }
        break;

        case (GAF_LAYER_CAP):
        {
            app_cap_req_ind_handler(param);
        }
        break;

        case (GAF_LAYER_TMAP):
        {
            app_tmap_req_ind_handler(param);;
        }
        break;

        case (GAF_LAYER_GMAP):
        {
            app_gmap_req_ind_handler(param);
        }
        break;

        default:
        {
            LOG_W("gaf unsupported req ind msg, layer = %d", layer);
        }
        break;
    }

    return (KE_MSG_CONSUMED);
}

static uint32_t app_gaf_msg_dflt_handler(ke_msg_id_t const msgid,
                                         void *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    LOG_I("gaf unknow msg = 0x%0x", msgid);
    // Drop the message
    return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
const struct ke_msg_handler app_gaf_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER, (ke_msg_func_t)app_gaf_msg_dflt_handler},

    {GAF_CMP_EVT, (ke_msg_func_t)app_gaf_cmp_evt_handler},
    {GAF_RSP, (ke_msg_func_t)app_gaf_rsp_handler},
    {GAF_IND, (ke_msg_func_t)app_gaf_ind_handler},
    {GAF_REQ_IND, (ke_msg_func_t)app_gaf_req_ind_handler},
};

const struct app_subtask_handlers app_gaf_handlers = {&app_gaf_msg_handler_list[0], ARRAY_LEN(app_gaf_msg_handler_list)};
#endif

void app_gaf_earbuds_init(app_bap_capa_srv_dir_t *sink_capa_info, app_bap_capa_srv_dir_t *src_capa_info, uint32_t role_bf)
{
    LOG_I("gaf core %s", BESLIB_INFO_STR);
#if APP_GAF_BAP_ENABLE
    //Notice: BAP services init on earbuds
    app_bap_server_init(sink_capa_info, src_capa_info, role_bf);
#endif

#if APP_GAF_ARC_ENABLE
    //Notice: ARC services init on earbuds
    app_arc_server_init();
#endif

#if APP_GAF_ACC_ENABLE
    //Notice: ACC Clients init on earbuds
    app_acc_client_init();
#endif

#if APP_GAF_ATC_ENABLE
    //Notice: ATC services init on earbuds
    app_atc_server_init();
#endif

#if APP_GAF_TMAP_ENABLE
    role_bf = GAF_TMAP_ROLE_ALLSUPP_MASK & (~GAF_TMAP_ROLE_CG_BIT);
    role_bf &= ~GAF_TMAP_ROLE_UMS_BIT;
    // TMAP services init on earbuds
    app_tmap_server_init(role_bf);
    app_tmap_client_init();
#endif

#if APP_GAF_GMAP_ENABLE
    role_bf = GAF_GMAP_ROLE_UGT_BIT |
#if (APP_BLE_BIS_SRC_ENABLE)
              GAF_GMAP_ROLE_BGS_BIT |
#endif
              GAF_GMAP_ROLE_BGR_BIT;
    app_gmap_server_init(role_bf);
    app_gmap_client_init();
#endif

#if APP_GAF_HAP_ENABLE
    app_hap_server_init();
#endif

#if APP_GAF_CAP_ENABLE
    app_cap_server_init();
#endif
}

void app_gaf_earbuds_deinit_impl(void)
{
#if APP_GAF_BAP_ENABLE
    app_bap_server_deinit();
#endif

#if APP_GAF_ARC_ENABLE
    app_arc_server_deinit();
#endif

#if APP_GAF_ACC_ENABLE
    app_acc_client_deinit();
#endif

#if APP_GAF_ATC_ENABLE
    app_atc_server_deinit();
#endif

#if APP_GAF_TMAP_ENABLE
    app_tmap_server_deinit();
    app_tmap_client_deinit();
#endif

#if APP_GAF_GMAP_ENABLE
    app_gmap_server_deinit();
    app_gmap_client_deinit();
#endif

#if APP_GAF_IAP_TM_ENABLE

#endif

#if APP_GAF_HAP_ENABLE
    app_hap_server_deinit();
#endif

#if APP_GAF_CAP_ENABLE
    app_cap_server_deinit();
#endif
}

void app_gaf_earbuds_deinit(void)
{
    bt_defer_call_func_0(app_gaf_earbuds_deinit_impl);
}

#ifdef AOB_MOBILE_ENABLED
void app_gaf_mobile_init(void)
{
#if APP_GAF_BAP_ENABLE
    //Notice: BAP Clients init on mobile
    app_bap_client_init();
#endif

#if APP_GAF_ARC_ENABLE
    //Notice: ARC Clients init on mobile
    app_arc_client_init();
#endif

#if APP_GAF_ACC_ENABLE
    //Notice: ACC services init on mobile
    app_acc_server_init();
#endif

#if APP_GAF_ATC_ENABLE
    //Notice: ATC&CAP Clients init on mobile
    app_atc_client_init();
#endif

#if APP_GAF_TMAP_ENABLE
    uint16_t role_bf = GAF_TMAP_ROLE_ALLSUPP_MASK & (~GAF_TMAP_ROLE_CT_BIT);
    role_bf &= ~GAF_TMAP_ROLE_UMR_BIT;
    // TMAP services init on mobile
    app_tmap_server_init(role_bf);
    app_tmap_client_init();
#endif

#if APP_GAF_GMAP_ENABLE
    role_bf = GAF_GMAP_ROLE_UGG_BIT |
#if (APP_BLE_BIS_SRC_ENABLE)
              GAF_GMAP_ROLE_BGS_BIT |
#endif
              GAF_GMAP_ROLE_BGR_BIT;
    app_gmap_server_init(role_bf);
    app_gmap_client_init();
#endif

#if APP_GAF_IAP_TM_ENABLE
    app_iap_tm_init();
#endif

#if APP_GAF_HAP_ENABLE
    app_hap_client_init();
#endif

#if APP_GAF_CAP_ENABLE
    app_cap_client_init();
#endif
}

void app_gaf_mobile_deinit_impl(void)
{
#if APP_GAF_BAP_ENABLE
    app_bap_client_deinit();
#endif

#if APP_GAF_ARC_ENABLE
    app_arc_client_deinit();
#endif

#if APP_GAF_ACC_ENABLE
    app_acc_server_deinit();
#endif

#if APP_GAF_ATC_ENABLE
    app_atc_client_deinit();
#endif

#if APP_GAF_TMAP_ENABLE
    app_tmap_server_deinit();
    app_tmap_client_deinit();
#endif

#if APP_GAF_GMAP_ENABLE
    app_gmap_server_deinit();
    app_gmap_client_deinit();
#endif

#if APP_GAF_IAP_TM_ENABLE

#endif

#if APP_GAF_HAP_ENABLE
    app_hap_client_deinit();
#endif

#if APP_GAF_CAP_ENABLE
    app_cap_client_deinit();
#endif
}

void app_gaf_mobile_deinit(void)
{
    bt_defer_call_func_0(app_gaf_mobile_deinit_impl);
}
#endif

void app_gaf_mobile_start_discovery(uint8_t con_lid)
{
#ifdef AOB_MOBILE_ENABLED
#if APP_GAF_BAP_ENABLE
    app_bap_start(con_lid);
#endif

#if APP_GAF_ARC_ENABLE
    app_arc_start(con_lid);
#endif

#if APP_GAF_ACC_ENABLE
    app_acc_start(con_lid, true);
#endif

#if APP_GAF_ATC_ENABLE
    app_cap_start(con_lid);
#endif

#if APP_GAF_HAP_ENABLE
    app_hap_start(con_lid);
#endif

/// TODO:These two (tmap/gmap) can also init in both side, central or peripheral
#if APP_GAF_TMAP_ENABLE
    app_tmap_start(con_lid);
#endif

#if APP_GAF_GMAP_ENABLE
    app_gmap_start(con_lid);
#endif
#endif
}

void app_gaf_evt_report_register(void *handler)
{
    /// input parameter check
    ASSERT(handler, "NULL pointer received in %s", __func__);
    /// update the report handler
    memcpy(&_report_bundle, handler, sizeof(GAF_EVT_REPORT_BUNDLE_T));
}

void app_gaf_evt_report(uint16_t evt, uint8_t *param, uint32_t length)
{
    if (_report_bundle.earbud_report)
    {
        _report_bundle.earbud_report(evt, param, length);
    }
    else
    {
        LOG_W("Earbud report is null!");
    }
}

#ifdef AOB_MOBILE_ENABLED
void app_gaf_mobile_evt_report(uint16_t evt, uint8_t *param, uint32_t length)
{
    if (_report_bundle.mobile_report)
    {
        _report_bundle.mobile_report(evt, param, length);
    }
    else
    {
        LOG_W("Mobile report is null!");
    }
}
#endif

/**
 * @brief Blueelf adapter view as stack part using cobuf mem
 */

void *app_gaf_malloc_buff(uint16_t size)
{
    return cobuf_malloc(size);
}

void app_gaf_free_buff(void *mem_ptr)
{
    cobuf_free(mem_ptr);
}
#endif

/// @} APP
