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
#include "app_bap.h"
#include "ble_app_dbg.h"

#include "app_gaf.h"
#include "app_gaf_custom_api.h"
#include "app_bap_capa_srv_msg.h"

#include "bap_unicast_server.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_BAP_CAPA_S_MAX_RECORD_NUM               (32)

#define APP_BAP_CAPA_S_MAX_RECORD_NUM               (32)

/*
 * INTERNAL VALUES
 ****************************************************************************************
 */
app_bap_capa_srv_env_t *p_app_capa_srv_env = NULL;

/*
 * INTERNAL FUNCTIONS
 ****************************************************************************************
 */
static void app_bap_pacs_cb_bond_data(uint8_t con_lid, uint8_t char_type, uint16_t cli_cfg_bf)
{
    LOG_D("app_bap capa srv bond_data_ind con_lid = %d, char_type = %d, cli_cfg_bf = %02x", con_lid,
          char_type, cli_cfg_bf);

    app_gaf_capa_srv_bond_data_ind_t bond_data =
    {
        .con_lid = con_lid,
        .cli_cfg_bf = cli_cfg_bf & 0b1111,// extract except pac
        .pac_cli_cfg_bf = (cli_cfg_bf & ~0b1111) >> 4,// extract pac
    };

    app_gaf_evt_report(APP_GAF_PACS_BOND_DATA_IND, (void *)&bond_data, sizeof(bond_data));
}

static void app_bap_pacs_cb_loc_change(uint8_t con_lid, uint8_t direction, uint32_t location_bf)
{
    LOG_I("app_bap capa srv con_lid = %d, direction = %d, location_bf = %d", con_lid, direction,
          location_bf);

    app_gaf_capa_srv_location_ind_t location =
    {
        .con_lid = con_lid,
        .direction = direction,
        .location_bf = location_bf,
    };

    app_gaf_evt_report(APP_GAF_PACS_LOCATION_SET_IND, (void *)&location, sizeof(location));
}

static void app_bap_pacs_cb_cccd_written(uint8_t con_lid, uint8_t char_type)
{
    LOG_D("app_bap capa srv cccd_written_ind con_lid= %d, char_type = %d", con_lid, char_type);

    app_gaf_capa_srv_cccd_written_ind_t cccd_first_written =
    {
        .con_lid = con_lid,
    };

    app_gaf_evt_report(APP_GAF_PACS_CCCD_WRITTEN_IND, (void *)&cccd_first_written,
                       sizeof(cccd_first_written));
}

static const bap_uc_srv_capa_evt_cb_t pacs_evt_cb =
{
    .cb_bond_data = app_bap_pacs_cb_bond_data,
    .cb_location = app_bap_pacs_cb_loc_change,
    .cb_cccd_written = app_bap_pacs_cb_cccd_written,
};

int app_bap_capa_srv_init(void)
{
    LOG_I("app_bap capa srv init");

    bap_uc_srv_capa_init_cfg_t pacs_init_cfg =
    {
        .pref_mtu = GAF_PREFERRED_MTU,
        .num_sink_pac_supp = p_app_capa_srv_env->nb_pacs / 2,
        .num_src_pac_supp = p_app_capa_srv_env->nb_pacs / 2,
        .location_bf_sink = p_app_capa_srv_env->dir_info[BAP_DIRECTION_SINK].location_bf,
        .location_bf_src = p_app_capa_srv_env->dir_info[BAP_DIRECTION_SRC].location_bf,
        .supp_context_bf_sink = p_app_capa_srv_env->dir_info[BAP_DIRECTION_SINK].context_bf_supp,
        .supp_context_bf_src = p_app_capa_srv_env->dir_info[BAP_DIRECTION_SRC].context_bf_supp,
#if (BLE_AHP_SERVER_SUPPORT)
        .num_sink_pac_v2_supp = 1,
        .num_src_pac_v2_supp = 1,
#endif /* BLE_AHP_CLIENT_SUPPORT */
    };

    return bap_uc_srv_pacs_init(&pacs_init_cfg, &pacs_evt_cb);
}

int app_bap_capa_srv_deinit(void)
{
    LOG_I("app_bap capa srv deinit");

    return bap_uc_srv_pacs_deinit();
}

int app_bap_capa_srv_info_init(app_bap_capa_srv_dir_t *sink_capa_info,
                               app_bap_capa_srv_dir_t *src_capa_info)
{
    if (p_app_capa_srv_env != NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint16_t status = BT_STS_SUCCESS;
    // Total number of PAC groups
    uint8_t nb_pacs = 0;
    if (src_capa_info && sink_capa_info)
    {
        nb_pacs = sink_capa_info->nb_pacs + src_capa_info->nb_pacs;
    }

    do
    {
        // Environment size
        uint16_t size = sizeof(app_bap_capa_srv_env_t);
        // PAC local index
        uint8_t pac_lid;
        app_bap_capa_srv_dir_t *dir_info = NULL;
        app_bap_capa_srv_pac_t *p_pac = NULL;

        // Allocate environment
        p_app_capa_srv_env = (app_bap_capa_srv_env_t *)app_gaf_malloc_buff(size);

        if (NULL == p_app_capa_srv_env)
        {
            LOG_E("app_bap capa srv init malloc error");
            status = APP_GAF_ERR_MALLOC_ERROR;
            break;
        }

        // Initialize environment content
        memset(p_app_capa_srv_env, 0, size);

        p_app_capa_srv_env->preferred_mtu = APP_GAF_DFT_PREF_MTU;

        // Set information for each supported direction
        if (sink_capa_info)
        {
            dir_info = &p_app_capa_srv_env->dir_info[APP_GAF_DIRECTION_SINK];
            memcpy(dir_info, sink_capa_info, sizeof(app_bap_capa_srv_dir_t));
        }

        // Set information for Source direction
        if (src_capa_info)
        {
            dir_info = &p_app_capa_srv_env->dir_info[APP_GAF_DIRECTION_SRC];
            memcpy(dir_info, src_capa_info, sizeof(app_bap_capa_srv_dir_t));
        }

        if (nb_pacs == 0)
        {
            LOG_E("app_bap capa srv nb of pac error");
            status = APP_GAF_ERR_INVALID_PARAM;
            break;
        }

        // Keep provided information
        p_app_capa_srv_env->nb_pacs = nb_pacs;

        size = nb_pacs * sizeof(app_bap_capa_srv_pac_t);
        p_app_capa_srv_env->p_pac_info = (app_bap_capa_srv_pac_t *)app_gaf_malloc_buff(size);

        if (NULL == p_app_capa_srv_env->p_pac_info)
        {
            LOG_E("app_bap capa srv pac_info init malloc error");
            status = APP_GAF_ERR_MALLOC_ERROR;
            break;
        }

        for (pac_lid = 0; pac_lid < nb_pacs; pac_lid++)
        {
            p_pac = &p_app_capa_srv_env->p_pac_info[pac_lid];
            INIT_LIST_HEAD(&p_pac->list_record);
        }
    } while (0);

    if ((status != BT_STS_SUCCESS) && (p_app_capa_srv_env != NULL))
    {
        if (p_app_capa_srv_env->p_pac_info)
        {
            app_gaf_free_buff(p_app_capa_srv_env->p_pac_info);
        }

        app_gaf_free_buff(p_app_capa_srv_env);
    }

    return status;
}

int app_bap_capa_srv_info_deinit(void)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_SUCCESS;
    }

    uint8_t pac_lid = 0;
    uint8_t record_id = 0;

    while (pac_lid < p_app_capa_srv_env->nb_pacs)
    {
        record_id = p_app_capa_srv_env->nb_records;

        while (record_id--)
        {
            app_bap_capa_srv_delete_pac_record(pac_lid, record_id);
        }

        pac_lid++;
    }

    if (p_app_capa_srv_env->p_pac_info != NULL)
    {
        app_gaf_free_buff(p_app_capa_srv_env->p_pac_info);
    }

    record_id = 0;

    while (record_id < PACS_MAX_SUPP_PAC_SET_NUM_TOTAL)
    {
        app_bap_capa_srv_delete_pac_v2_record(record_id);
        record_id++;
    }

    record_id = 0;

    while (record_id < PACS_MAX_SUPP_CHAN_CAPA_NUM)
    {
        app_bap_capa_srv_delete_chan_capa_record(record_id);
        record_id++;
    }

    record_id = 0;

    while (record_id < PACS_MAX_SUPP_PREF_AUD_CFG_NUM)
    {
        app_bap_capa_srv_delete_pref_aud_cfg_record(record_id);
        record_id++;
    }

    app_gaf_free_buff(p_app_capa_srv_env);

    p_app_capa_srv_env = NULL;

    return BT_STS_SUCCESS;
}

static int app_bap_capa_srv_get_valid_record_id()
{
    uint8_t record_id = 0;
    uint8_t flag[APP_BAP_CAPA_S_MAX_RECORD_NUM] = {0};

    for (uint8_t pac_lid = 0; pac_lid < p_app_capa_srv_env->nb_pacs; pac_lid++)
    {
        // Pointer to PAC information structure
        app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];

        struct list_node *p, *n;
        struct list_node *pac_rec_list = &p_pac_info->list_record;
        app_bap_capa_srv_record_t *p_pac_record = NULL;

        // Check all pac record in this list
        colist_iterate_safe(p, n, pac_rec_list)
        {
            p_pac_record = colist_structure(p, app_bap_capa_srv_record_t, hdr);

            flag[p_pac_record->record_id] = true;
        }
    }

    for (record_id = 0; record_id < APP_BAP_CAPA_S_MAX_RECORD_NUM; record_id++)
    {
        if (flag[record_id] == false)
        {
            break;
        }
    }

    return record_id;
}

int app_bap_capa_srv_get_pac_record_list(uint8_t pac_lid, uint8_t *record_list)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    if (NULL == record_list)
    {
        return BT_STS_INVALID_PARM;
    }

    uint8_t idx = 0;
    app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];

    struct list_node *p, *n;
    struct list_node *pac_rec_list = &p_pac_info->list_record;
    app_bap_capa_srv_record_t *p_pac_record = NULL;

    // Check all pac record in this list
    colist_iterate_safe(p, n, pac_rec_list)
    {
        p_pac_record = colist_structure(p, app_bap_capa_srv_record_t, hdr);

        record_list[idx] = p_pac_record->record_id;
    }

    return BT_STS_SUCCESS;
}

app_bap_capa_srv_record_t *app_bap_capa_srv_get_pac_record_info(uint8_t pac_lid, uint8_t record_id)
{
    if (p_app_capa_srv_env == NULL)
    {
        return NULL;
    }

    app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];

    struct list_node *p, *n;
    struct list_node *pac_rec_list = &p_pac_info->list_record;
    app_bap_capa_srv_record_t *p_pac_record = NULL;

    // Check all pac record in this list
    colist_iterate_safe(p, n, pac_rec_list)
    {
        p_pac_record = colist_structure(p, app_bap_capa_srv_record_t, hdr);

        if (record_id == p_pac_record->record_id)
        {
            return p_pac_record;
        }
    }

    return NULL;
}

static void app_bap_capa_srv_capa_set(app_gaf_codec_id_t *codec_id,
                                      app_gaf_bap_capa_t **p_capa_out, app_gaf_bap_capa_t *p_capa_in)
{
    if (!p_capa_in)
    {
        return;
    }

    uint16_t size = sizeof(app_gaf_bap_capa_t) + p_capa_in->add_capa.len;
    *p_capa_out = (app_gaf_bap_capa_t *)app_gaf_malloc_buff(size);
    if (NULL == (*p_capa_out))
    {
        LOG_E("app_bap capa srv codec set malloc error");
        return ;
    }

    memcpy(*p_capa_out, p_capa_in, size);
}

static void app_bap_capa_srv_capa_metadata_set(app_gaf_codec_id_t *codec_id,
                                               app_gaf_bap_capa_metadata_t **p_metadata_out,
                                               app_gaf_bap_capa_metadata_t *p_metadata_in)
{
    if (NULL == p_metadata_in)
    {
        return ;
    }
    uint16_t size = sizeof(app_gaf_bap_capa_metadata_t) + p_metadata_in->add_metadata.len;
    *p_metadata_out = (app_gaf_bap_capa_metadata_t *)app_gaf_malloc_buff(size);
    if (NULL == (*p_metadata_out))
    {
        LOG_E("app_bap capa srv metadata set malloc error");
        return ;
    }
    memcpy(*p_metadata_out, p_metadata_in, size);
}

int app_bap_capa_srv_update_pac_record_info(uint8_t pac_lid, uint8_t record_id,
                                            app_gaf_codec_id_t *codec_id,
                                            app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    if (pac_lid >= p_app_capa_srv_env->nb_pacs)
    {
        LOG_W("app_bap capa srv pac_lid error");
        return BT_STS_INVALID_PARM;
    }

    app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];

    struct list_node *p, *n;
    struct list_node *pac_rec_list = &p_pac_info->list_record;
    app_bap_capa_srv_record_t *p_pac_record = NULL;

    // Check all pac record in this list
    colist_iterate_safe(p, n, pac_rec_list)
    {
        p_pac_record = colist_structure(p, app_bap_capa_srv_record_t, hdr);

        if (record_id == p_pac_record->record_id)
        {
            app_gaf_free_buff((void *)(p_pac_record->p_capa));

            if (p_pac_record->p_metadata)
            {
                app_gaf_free_buff((void *)(p_pac_record->p_metadata));
            }

            memcpy(&p_pac_record->codec_id, codec_id, sizeof(app_gaf_codec_id_t));

            app_bap_capa_srv_capa_set(codec_id, &p_pac_record->p_capa, p_capa);
            app_bap_capa_srv_capa_metadata_set(codec_id, &p_pac_record->p_metadata, p_metadata);

            gen_aud_metadata_t *p_aud_metadata = (gen_aud_metadata_t *)app_gaf_malloc_buff(
                                                     sizeof(gen_aud_metadata_t) + p_pac_record->p_metadata->add_metadata.len);

            if (p_aud_metadata == NULL)
            {
                return BT_STS_NO_RESOURCES;
            }

            gen_aud_init_metadata(p_aud_metadata);

            p_aud_metadata->basic_metadata.preferred_audio_context = p_pac_record->p_metadata->param.context_bf;
            p_aud_metadata->add_metadata.len = p_pac_record->p_metadata->add_metadata.len;

            if (p_aud_metadata->add_metadata.len != 0)
            {
                memcpy(p_aud_metadata->add_metadata.data,
                       p_pac_record->p_metadata->add_metadata.data, p_aud_metadata->add_metadata.len);
            }

            uint16_t status = bap_uc_srv_add_pac_record(pac_lid, record_id, p_pac_record->codec_id.codec_id,
                                                        (gen_aud_capa_t *)p_pac_record->p_capa,
                                                        p_aud_metadata);
            app_gaf_free_buff(p_aud_metadata);

            return status;
        }
    }

    return BT_STS_FAILED;
}

int app_bap_capa_srv_add_pac_record(uint8_t pac_lid, app_gaf_codec_id_t *codec_id,
                                    app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    if (pac_lid >= p_app_capa_srv_env->nb_pacs)
    {
        LOG_W("app_bap capa srv pac_lid error");
        return BT_STS_INVALID_PARM;
    }

    app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];

    app_bap_capa_srv_record_t *p_record_info = (app_bap_capa_srv_record_t *)app_gaf_malloc_buff(sizeof(
                                                                                                    app_bap_capa_srv_record_t));

    if (p_record_info == NULL)
    {
        LOG_E("app_bap capa srv record_info init malloc error");
        return BT_STS_NO_RESOURCES;
    }

    memset(p_record_info, 0, sizeof(app_bap_capa_srv_record_t));

    INIT_LIST_HEAD(&p_record_info->hdr);

    colist_addto_tail(&p_record_info->hdr, &p_pac_info->list_record);

    p_record_info->record_id = app_bap_capa_srv_get_valid_record_id();

    memcpy(&p_record_info->codec_id.codec_id[0], &codec_id->codec_id[0], APP_GAF_CODEC_ID_LEN);
    app_bap_capa_srv_capa_set(codec_id, &p_record_info->p_capa, p_capa);
    app_bap_capa_srv_capa_metadata_set(codec_id, &p_record_info->p_metadata, p_metadata);

    p_pac_info->nb_records++;
    p_app_capa_srv_env->nb_records++;

    gen_aud_capa_t *p_capa_i = (gen_aud_capa_t *)app_gaf_malloc_buff(sizeof(gen_aud_capa_t) +
                                                                     p_record_info->p_capa->add_capa.len);
    gen_aud_metadata_t *p_metadata_i = (gen_aud_metadata_t *)app_gaf_malloc_buff(sizeof(
                                                                                     gen_aud_metadata_t) + p_record_info->p_metadata->add_metadata.len);

    if (p_capa_i == NULL || p_metadata_i == NULL)
    {
        if (p_capa_i != NULL)
        {
            app_gaf_free_buff(p_capa_i);
        }

        if (p_metadata_i != NULL)
        {
            app_gaf_free_buff(p_metadata_i);
        }

        return BT_STS_NO_RESOURCES;
    }

    memset(p_capa_i, 0, sizeof(*p_capa_i));

    gen_aud_init_metadata(p_metadata_i);

    p_capa_i->basic_capa_param = *(gen_aud_codec_capa_param_t *)&p_record_info->p_capa->param;
    p_metadata_i->basic_metadata.preferred_audio_context = p_record_info->p_metadata->param.context_bf;
    memcpy(&p_capa_i->add_capa_param, &p_record_info->p_capa->add_capa,
           sizeof(p_record_info->p_capa->add_capa) + p_record_info->p_capa->add_capa.len);
    memcpy(&p_metadata_i->add_metadata, &p_record_info->p_metadata->add_metadata,
           sizeof(p_record_info->p_metadata->add_metadata) + p_record_info->p_metadata->add_metadata.len);

    uint16_t status = bap_uc_srv_add_pac_record(pac_lid, p_record_info->record_id,
                                                p_record_info->codec_id.codec_id,
                                                p_capa_i,
                                                p_metadata_i);
    app_gaf_free_buff(p_capa_i);
    app_gaf_free_buff(p_metadata_i);

    return status;
}

int app_bap_capa_srv_delete_pac_record(uint8_t pac_lid, uint8_t record_id)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    uint16_t status = BT_STS_SUCCESS;

    app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];

    struct list_node *p, *n;
    struct list_node *pac_rec_list = &p_pac_info->list_record;
    app_bap_capa_srv_record_t *p_pac_record = NULL;

    // Check all pac record in this list
    colist_iterate_safe(p, n, pac_rec_list)
    {
        p_pac_record = colist_structure(p, app_bap_capa_srv_record_t, hdr);

        if (record_id == p_pac_record->record_id)
        {
            status = bap_uc_srv_del_pac_record(p_pac_record->record_id);

            app_gaf_free_buff((void *)(p_pac_record->p_capa));

            if (p_pac_record->p_metadata)
            {
                app_gaf_free_buff((void *)(p_pac_record->p_metadata));
            }

            colist_delete(p);
            app_gaf_free_buff(p_pac_record);

            p_pac_info->nb_records--;
            break;
        }
    }

    return status;
}

int app_bap_capa_srv_clear_pac_record(uint8_t pac_lid)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    app_bap_capa_srv_pac_t *p_pac_info = &p_app_capa_srv_env->p_pac_info[pac_lid];
    struct list_node *p, *n;
    struct list_node *pac_rec_list = &p_pac_info->list_record;
    app_bap_capa_srv_record_t *p_pac_record = NULL;

    // Check all pac record in this list
    colist_iterate_safe(p, n, pac_rec_list)
    {
        p_pac_record = colist_structure(p, app_bap_capa_srv_record_t, hdr);

        bap_uc_srv_del_pac_record(p_pac_record->record_id);

        app_gaf_free_buff((void *)(p_pac_record->p_capa));

        if (p_pac_record->p_metadata)
        {
            app_gaf_free_buff((void *)(p_pac_record->p_metadata));
        }

        colist_delete(p);
        app_gaf_free_buff(p_pac_record);
    }

    p_pac_info->nb_records = 0;

    return BT_STS_SUCCESS;
}

int app_bap_capa_srv_add_pac_v2_record(uint8_t direction, app_gaf_codec_id_t *codec_id,
                                       app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata)
{
    uint16_t status = BT_STS_SUCCESS;
    gen_aud_var_info_t *p_capa_ltv = NULL;
    gen_aud_capa_t *p_capa_i = (gen_aud_capa_t *)p_capa;
    gen_aud_metadata_t *p_metadata_i = NULL;
    uint16_t capa_size = 0;

    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    if (direction >= BAP_DIRECTION_MAX || p_capa == NULL)
    {
        LOG_W("app_bap capa srv pac_lid error");
        return BT_STS_INVALID_PARM;
    }

    do
    {
        // Capabilities
        status = gen_aud_codec_capa_check(codec_id->codec_id, p_capa_i, NULL, &capa_size);
        if (status != BT_STS_SUCCESS)
        {
            break;
        }
        capa_size += sizeof(gen_aud_var_info_t);
        p_capa_ltv = app_gaf_malloc_buff(capa_size);
        if (p_capa_ltv == NULL)
        {
            status = BT_STS_NO_RESOURCES;
            break;
        }
        p_capa_ltv->len = gen_aud_codec_capa_pack((uint8_t *)p_capa_ltv->data, p_capa_i);
        // Metadata
        p_metadata_i = (gen_aud_metadata_t *)app_gaf_malloc_buff(sizeof(gen_aud_metadata_t) +
                                                                 (p_metadata ? p_metadata->add_metadata.len : 0));
        if (p_metadata_i == NULL)
        {
            status = BT_STS_NO_RESOURCES;
            break;
        }
        gen_aud_init_metadata(p_metadata_i);
        if (p_metadata != NULL)
        {
            p_metadata_i->basic_metadata.preferred_audio_context = p_metadata->param.context_bf;
            memcpy(&p_metadata_i->add_metadata, &p_metadata->add_metadata,
                   sizeof(p_metadata->add_metadata) + p_metadata->add_metadata.len);
        }

        status = pacs_add_pac_v2_record(direction, p_app_capa_srv_env->nb_v2_records,
                                        codec_id->codec_id,
                                        p_capa_ltv,
                                        p_metadata_i);

        if (status == BT_STS_SUCCESS)
        {
            p_app_capa_srv_env->nb_v2_records++;
            LOG_I("%s nb records %d", __func__, p_app_capa_srv_env->nb_v2_records);
        }
    } while (0);

    if (p_metadata_i != NULL)
    {
        app_gaf_free_buff(p_metadata_i);
    }

    if (p_capa_ltv != NULL)
    {
        app_gaf_free_buff(p_capa_ltv);
    }

    return status;
}

int app_bap_capa_srv_delete_pac_v2_record(uint8_t record_lid)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    uint16_t status = BT_STS_SUCCESS;

    if (record_lid >= p_app_capa_srv_env->nb_v2_records)
    {
        return BT_STS_INVALID_PARM;
    }

    status = pacs_del_pac_v2_record(record_lid);

    if (status == BT_STS_SUCCESS)
    {
        p_app_capa_srv_env->nb_v2_records--;
    }

    return status;
}

int app_bap_capa_srv_add_chan_capa_record(uint32_t channel_type, const uint8_t *p_desc_val, uint8_t val_len)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    pacs_chan_capa_desc_u *p_desc = NULL;

    if ((channel_type >> 16) == PACS_CHAN_TYPE_ID_GENERIC)
    {
        p_desc = (pacs_chan_capa_desc_u *)app_gaf_malloc_buff(sizeof(pacs_gen_chan_desc_t));
    }
    else
    {
        p_desc = (pacs_chan_capa_desc_u *)app_gaf_malloc_buff(sizeof(pacs_raw_chan_desc_t) + val_len);
    }

    if (p_desc == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    if ((channel_type >> 16) == PACS_CHAN_TYPE_ID_GENERIC)
    {
        p_desc->gen_chan_desc.gen_aud_chan_mask = CO_COMBINE_UINT32_LE(p_desc_val);
    }
    else
    {
        p_desc->raw_chan_desc.aud_chan_sets_num = val_len;
        memcpy(p_desc->raw_chan_desc.audio_chan_set, p_desc_val, val_len);
    }

    uint16_t status = pacs_add_chan_capa_record(p_app_capa_srv_env->nb_chan_capa_records, channel_type, p_desc);

    if (status == BT_STS_SUCCESS)
    {
        p_app_capa_srv_env->nb_chan_capa_records++;
        LOG_I("%s nb records %d", __func__, p_app_capa_srv_env->nb_chan_capa_records);
    }

    app_gaf_free_buff(p_desc);

    return status;
}

int app_bap_capa_srv_delete_chan_capa_record(uint8_t record_lid)
{
    uint16_t status = BT_STS_SUCCESS;

    if (record_lid >= p_app_capa_srv_env->nb_chan_capa_records)
    {
        return BT_STS_INVALID_PARM;
    }

    status = pacs_del_chan_capa_record(record_lid);

    if (status == BT_STS_SUCCESS)
    {
        p_app_capa_srv_env->nb_chan_capa_records--;
    }

    return status;
}

int app_bap_capa_srv_add_pref_aud_cfg_record(uint16_t use_case_id, uint16_t data_present_bits,
                                             const app_gaf_pref_aud_cfg_data_t *p_pref_aud_cfg_data)
{
    if (p_app_capa_srv_env == NULL)
    {
        return BT_STS_NOT_READY;
    }

    if (p_pref_aud_cfg_data == NULL || data_present_bits == 0)
    {
        return BT_STS_INVALID_PARM;
    }

    pacs_pref_aud_cfg_t *pref_aud_cfg = app_gaf_malloc_buff(sizeof(pacs_pref_aud_cfg_t));

    if (pref_aud_cfg == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    pref_aud_cfg->data_present_bits = data_present_bits;
    pref_aud_cfg->use_case_id = use_case_id;
    pref_aud_cfg->data_list = *(pacs_pref_aud_cfg_data_t *)p_pref_aud_cfg_data;

    uint16_t status = pacs_add_preferred_audio_cfg_record(p_app_capa_srv_env->nb_pref_aud_cfg_records, pref_aud_cfg);

    if (status == BT_STS_SUCCESS)
    {
        p_app_capa_srv_env->nb_pref_aud_cfg_records++;
        LOG_I("%s use case id 0x%2x num %d records", __func__, use_case_id, p_app_capa_srv_env->nb_pref_aud_cfg_records);
    }

    app_gaf_free_buff(pref_aud_cfg);

    return status;
}

int app_bap_capa_srv_delete_pref_aud_cfg_record(uint8_t record_lid)
{
    uint16_t status = BT_STS_SUCCESS;

    if (record_lid >= p_app_capa_srv_env->nb_pref_aud_cfg_records)
    {
        return BT_STS_INVALID_PARM;
    }

    status = pacs_del_pref_aud_cfg_record(record_lid);

    if (status == BT_STS_SUCCESS)
    {
        p_app_capa_srv_env->nb_pref_aud_cfg_records--;
    }

    return status;
}

int app_bap_capa_srv_delete_pref_aud_cfg_record_by_use_case(uint16_t use_case_id)
{
    uint8_t deleted_cnt = pacs_del_pref_aud_cfg_record_by_use_case(use_case_id);
    p_app_capa_srv_env->nb_pref_aud_cfg_records -= deleted_cnt;

    return deleted_cnt ? BT_STS_SUCCESS : BT_STS_FAILED;
}

int app_bap_capa_srv_set_supp_location_bf(enum app_gaf_direction direction, uint32_t location_bf)
{
    p_app_capa_srv_env->dir_info[direction].location_bf = location_bf;
    return bap_uc_srv_set_audio_location(direction, location_bf);
}

int app_bap_capa_srv_get_location_bf(enum app_gaf_direction direction)
{
    return p_app_capa_srv_env->dir_info[direction].location_bf;
}

int app_bap_capa_srv_set_supp_context_bf(uint8_t con_lid, enum app_gaf_direction direction,
                                         uint16_t context_bf_supp)
{
    p_app_capa_srv_env->dir_info[direction].context_bf_supp = context_bf_supp;
    return bap_uc_srv_set_supp_audio_context(direction, context_bf_supp);
}

int app_bap_capa_srv_set_ava_context_bf(uint8_t con_lid, uint16_t context_bf_ava_sink,
                                        uint16_t context_bf_ava_src)
{
    if (con_lid >= BLE_CONNECTION_MAX)
    {
        LOG_W("app_bap capa srv input con_lid error");
        return BT_STS_INVALID_PARM;
    }

    uint16_t context_bf_ava[APP_GAF_DIRECTION_MAX] = {context_bf_ava_sink, context_bf_ava_src};

    app_gaf_direction_t dir;

    for (dir = APP_GAF_DIRECTION_SINK; dir < APP_GAF_DIRECTION_MAX; dir++)
    {
        p_app_capa_srv_env->dir_info[dir].context_bf_ava[con_lid] = context_bf_ava[dir];
    }

    uint16_t status = bap_uc_srv_set_ava_audio_context(con_lid, APP_GAF_DIRECTION_SINK, context_bf_ava_sink);

    if (status == BT_STS_SUCCESS)
    {
        status = bap_uc_srv_set_ava_audio_context(con_lid, APP_GAF_DIRECTION_SRC, context_bf_ava_src);
    }

    return status;
}

int app_bap_capa_srv_get_ava_context_bf(uint8_t con_lid, uint16_t *context_bf_ava_sink,
                                        uint16_t *context_bf_ava_src)
{
    if (con_lid >= BLE_CONNECTION_MAX)
    {
        LOG_W("app_bap capa srv input con_lid error");
        return BT_STS_INVALID_PARM;
    }

    *context_bf_ava_sink =
        p_app_capa_srv_env->dir_info[APP_GAF_DIRECTION_SINK].context_bf_ava[con_lid];
    *context_bf_ava_src = p_app_capa_srv_env->dir_info[APP_GAF_DIRECTION_SRC].context_bf_ava[con_lid];

    return BT_STS_SUCCESS;
}

int app_bap_capa_srv_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t pac_cli_cfg_bf)
{
    return bap_uc_srv_restore_pacs_cli_cfg_cache(con_lid, ((uint32_t)(cli_cfg_bf & 0b1111) | (pac_cli_cfg_bf << 4)));
}

int app_bap_capa_srv_rsp_handler(void const *param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_bap_capa_srv_ind_handler(void const *param)
{
    LOG_I("%s does not support", __func__);
    return 0;
}
#endif

/// @} APP
