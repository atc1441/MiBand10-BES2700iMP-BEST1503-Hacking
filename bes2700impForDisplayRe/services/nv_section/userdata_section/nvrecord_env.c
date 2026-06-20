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
#if defined(NEW_NV_RECORD_ENABLED)
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#include "hal_trace.h"

static struct nvrecord_env_t localSystemInfo;

void nvrecord_rebuild_system_env(struct nvrecord_env_t* pSystemEnv, bool isRebuildAll)
{
    if (isRebuildAll)
    {
        memset((uint8_t *)pSystemEnv, 0, sizeof(struct nvrecord_env_t));

        pSystemEnv->ibrt_mode.mode = NVRAM_ENV_TWS_MODE_DEFAULT;
        pSystemEnv->ibrt_mode.conn_mode = NV_DEFAULT_EVN_TWS_CONN_MODE;
        pSystemEnv->ibrt_mode.tws_connect_success = 0;
    }

    pSystemEnv->nvrecord_env_len = sizeof(struct nvrecord_env_t)-sizeof(uint32_t);

    pSystemEnv->media_language.language = NVRAM_ENV_MEDIA_LANGUAGE_DEFAULT;

    pSystemEnv->factory_tester_status.status = NVRAM_ENV_FACTORY_TESTER_STATUS_DEFAULT;

    pSystemEnv->factory_test_env.test_type = NV_BT_ERROR_TEST_MODE;
    pSystemEnv->factory_test_env.test_end_timeout = 0;
    pSystemEnv->factory_test_env.tx_packet_nb = 0;

    pSystemEnv->le_rx_env.rx_channel = 0;
    pSystemEnv->le_rx_env.phy = 0;
    pSystemEnv->le_rx_env.mod_idx = 0;
    pSystemEnv->le_rx_env.test_done = false;
    pSystemEnv->le_rx_env.test_result = 0;

    pSystemEnv->le_tx_env.tx_channel = 0;
    pSystemEnv->le_tx_env.data_len = 0;
    pSystemEnv->le_tx_env.pkt_payload = 0;
    pSystemEnv->le_tx_env.phy = 0;

    pSystemEnv->bt_nonsignaling_env.hopping_mode = 0;
    pSystemEnv->bt_nonsignaling_env.whitening_mode = 0;
    pSystemEnv->bt_nonsignaling_env.tx_freq = 0;
    pSystemEnv->bt_nonsignaling_env.rx_freq = 0;
    pSystemEnv->bt_nonsignaling_env.power_level = 0;
    pSystemEnv->bt_nonsignaling_env.lt_addr = 0;
    pSystemEnv->bt_nonsignaling_env.edr_enabled = 0;
    pSystemEnv->bt_nonsignaling_env.packet_type = 0;
    pSystemEnv->bt_nonsignaling_env.payload_pattern = 0;
    pSystemEnv->bt_nonsignaling_env.payload_length = 0;
    pSystemEnv->bt_nonsignaling_env.tx_packet_num = 0;
    pSystemEnv->bt_nonsignaling_env.test_done = false;
    pSystemEnv->bt_nonsignaling_env.test_result = 0;
    pSystemEnv->bt_nonsignaling_env.hec_nb = 0;
    pSystemEnv->bt_nonsignaling_env.crc_nb = 0;

    pSystemEnv->aiManagerInfo.voice_key_enable = false;
    pSystemEnv->aiManagerInfo.setedCurrentAi = 0;
    pSystemEnv->aiManagerInfo.aiStatusDisableFlag = 0;
    pSystemEnv->aiManagerInfo.amaAssistantEnableStatus = 1;

    localSystemInfo = *pSystemEnv;
}

int nv_record_env_get(struct nvrecord_env_t **nvrecord_env)
{
    if (NULL == nvrecord_env)
    {
        return -1;
    }

    if (NULL == nvrecord_extension_p)
    {
        return -1;
    }

    localSystemInfo = nvrecord_extension_p->system_info;
    *nvrecord_env = &localSystemInfo;

    return 0;
}

int nv_record_env_set(struct nvrecord_env_t *nvrecord_env)
{
    if (NULL == nvrecord_extension_p)
    {
        return -1;
    }

    uint32_t lock = nv_record_pre_write_operation();
    nvrecord_extension_p->system_info = *nvrecord_env;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    return 0;
}

void nv_record_update_ibrt_info(uint32_t newMode, bt_bdaddr_t *ibrtPeerAddr)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    TRACE(2,"##%s,%d",__func__,newMode);

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.ibrt_mode.mode = newMode;
    memcpy(nvrecord_extension_p->system_info.ibrt_mode.record.bdAddr.address,
        ibrtPeerAddr->address, BTIF_BD_ADDR_SIZE);

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_update_tws_side(uint8_t side)
{
    if (NULL == nvrecord_extension_p) {
        return;
    }

    TRACE(2, "##%s,%d", __func__, side);

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.ibrt_mode.tws_side = side;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

int nv_record_get_tws_side()
{
    return nvrecord_extension_p->system_info.ibrt_mode.tws_side;
}

void nv_record_update_conn_mode(uint8_t mode)
{
    if (NULL == nvrecord_extension_p) {
        return;
    }

    TRACE(2, "##%s,%d", __func__, mode);

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.ibrt_mode.conn_mode = mode;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

int nv_record_get_conn_mode()
{
    return nvrecord_extension_p->system_info.ibrt_mode.conn_mode;
}

int nv_record_set_tws_name(uint8_t* name, uint8_t length)
{
    if (NULL == nvrecord_extension_p)
    {
        TRACE(0,"nv_record: nvrecord_extension is null!");
        return -1;
    }
    if (TWS_NAME_MAX_LENGTH <= length)
    {
        TRACE(2,"nv_record: tws name invalid length, current len:%d,max:%d", length, TWS_NAME_MAX_LENGTH);
        return -1;
    }

    if(nvrecord_extension_p->system_info.ibrt_mode.tws_enable)
    {
        uint32_t lock = nv_record_pre_write_operation();
        nvrecord_extension_p->system_info.ibrt_mode.tws_name_len = length;
        memcpy(nvrecord_extension_p->system_info.ibrt_mode.tws_name, name, length);
        nv_record_update_runtime_userdata();
        nv_record_post_write_operation(lock);
        TRACE(0,"nv_record: set tws name success");
    }
    return 0;
}

int nv_record_get_tws_name(uint8_t* name, uint8_t* length)
{
    if (NULL == nvrecord_extension_p) {
        TRACE(0, "nv_record: nvrecord_extension is null!");
        return -1;
    }
    uint8_t len = nvrecord_extension_p->system_info.ibrt_mode.tws_name_len;
    if (len == 0) {
        TRACE(0, "nv_record: tws name is invalid!");
        return -1;
    }

    memcpy(name, nvrecord_extension_p->system_info.ibrt_mode.tws_name, len);
    name[len] = '\0';
    *length = len;

    return 0;
}

void nv_record_clean_tws_name(void)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }
    uint32_t lock = nv_record_pre_write_operation();
    nvrecord_extension_p->system_info.ibrt_mode.tws_name_len = 0;
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    TRACE(0,"nv_record: clesn tws name success");
}

bool nv_record_tws_name_is_valid(void)
{
    bool status = false;

    if (NULL == nvrecord_extension_p)
    {
        TRACE(0,"nv_record: nvrecord_extension is null!");
        return status;
    }
    if (nvrecord_extension_p->system_info.ibrt_mode.tws_name_len > 0)
    {
        status = true;
    }
    TRACE(1,"nv_record: tws name valid:%d", status);
    return status;

}

void nv_record_set_tws_enable(void)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }
    uint32_t lock = nv_record_pre_write_operation();
    nvrecord_extension_p->system_info.ibrt_mode.tws_enable = true;
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    TRACE(0,"nv_record: set tws enble");
}

void nv_record_clean_tws_enable(void)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }
    uint32_t lock = nv_record_pre_write_operation();
    nvrecord_extension_p->system_info.ibrt_mode.tws_enable = false;
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    TRACE(0,"nv_record: clean tws enble");
}

bool nv_record_tws_is_enabled(void)
{
    bool status = false;

    if (NULL == nvrecord_extension_p)
    {
        TRACE(0,"nv_record: nvrecord_extension is null!");
        return status;
    }
    status  = nvrecord_extension_p->system_info.ibrt_mode.tws_enable;
    TRACE(1,"nv_record: get tws enble:%d", status);
    return status;
}

void nv_record_update_factory_tester_status(uint32_t status)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.factory_tester_status.status = status;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_update_factory_tx_single_tone_env(uint8_t channel)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    TRACE(1,"factory tx single tone %d \n", channel);

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.factory_test_env.test_type = NV_BT_SINGLETONE_TX_MODE;

    nvrecord_extension_p->system_info.le_tx_env.tx_channel = channel;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}
void nv_record_update_factory_le_rx_test_env(uint32_t to, uint16_t remote_tx_nb,uint8_t test_type,
                                uint8_t rx_channel, uint8_t phy, uint8_t mod_idx)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.factory_test_env.test_type = test_type;
    nvrecord_extension_p->system_info.factory_test_env.test_end_timeout = to;
    nvrecord_extension_p->system_info.factory_test_env.tx_packet_nb = remote_tx_nb;

    nvrecord_extension_p->system_info.le_rx_env.rx_channel = rx_channel;
    nvrecord_extension_p->system_info.le_rx_env.phy = phy;
    nvrecord_extension_p->system_info.le_rx_env.mod_idx = mod_idx;
    nvrecord_extension_p->system_info.le_rx_env.test_done = false;
    nvrecord_extension_p->system_info.le_rx_env.test_result = 0;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_update_factory_bt_nonsignaling_env(uint32_t to, uint8_t test_type,
                                uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length,
                                uint32_t tx_packet_num)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.factory_test_env.test_type = test_type;
    nvrecord_extension_p->system_info.factory_test_env.test_end_timeout = to;

    nvrecord_extension_p->system_info.bt_nonsignaling_env.hopping_mode = hopping_mode;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.whitening_mode = whitening_mode;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.tx_freq = tx_freq;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.rx_freq = rx_freq;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.power_level = power_level;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.lt_addr = lt_addr;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.edr_enabled = edr_enabled;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.packet_type = packet_type;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.payload_pattern = payload_pattern;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.payload_length = payload_length;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.tx_packet_num = tx_packet_num;

    nvrecord_extension_p->system_info.bt_nonsignaling_env.test_done = false;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.test_result = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.hec_nb = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.crc_nb = 0;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_update_factory_le_tx_test_env(uint32_t to, uint8_t test_type, uint8_t tx_channel,
                                uint8_t data_len, uint8_t pkt_payload, uint8_t phy)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.factory_test_env.test_type = test_type;
    nvrecord_extension_p->system_info.factory_test_env.test_end_timeout = to;
    nvrecord_extension_p->system_info.factory_test_env.tx_packet_nb = 0;

    nvrecord_extension_p->system_info.le_tx_env.tx_channel = tx_channel;
    nvrecord_extension_p->system_info.le_tx_env.data_len = data_len;
    nvrecord_extension_p->system_info.le_tx_env.pkt_payload = pkt_payload;
    nvrecord_extension_p->system_info.le_tx_env.phy = phy;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_reset_factory_test_env(void)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.factory_test_env.test_type = NV_BT_ERROR_TEST_MODE;
    nvrecord_extension_p->system_info.factory_test_env.test_end_timeout = 0;

    nvrecord_extension_p->system_info.le_rx_env.rx_channel = 0;
    nvrecord_extension_p->system_info.le_rx_env.phy = 0;
    nvrecord_extension_p->system_info.le_rx_env.mod_idx = 0;

    nvrecord_extension_p->system_info.le_tx_env.tx_channel = 0;
    nvrecord_extension_p->system_info.le_tx_env.data_len = 0;
    nvrecord_extension_p->system_info.le_tx_env.pkt_payload = 0;
    nvrecord_extension_p->system_info.le_tx_env.phy = 0;

    nvrecord_extension_p->system_info.bt_nonsignaling_env.hopping_mode = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.whitening_mode = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.tx_freq = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.rx_freq = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.power_level = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.lt_addr = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.edr_enabled = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.packet_type = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.payload_pattern = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.payload_length = 0;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.tx_packet_num = 0;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_update_factory_le_rx_test_result(bool test_done, uint16_t result)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.le_rx_env.test_done = test_done;
    nvrecord_extension_p->system_info.le_rx_env.test_result = result;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_update_factory_bt_nonsignaling_test_result(bool test_done, uint16_t result, uint16_t hec_nb,uint16_t crc_nb)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    uint32_t lock = nv_record_pre_write_operation();

    nvrecord_extension_p->system_info.bt_nonsignaling_env.test_done = test_done;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.test_result = result;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.hec_nb = hec_nb;
    nvrecord_extension_p->system_info.bt_nonsignaling_env.crc_nb = crc_nb;

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
}

void nv_record_factory_get_le_rx_test_result(bool* test_done, uint16_t* result)
{
    do{
        if (NULL == nvrecord_extension_p)
        {
            break;
        }

        if(test_done == NULL || result == NULL)
        {
            break;
        }

        uint32_t lock = nv_record_pre_write_operation();

        *test_done = nvrecord_extension_p->system_info.le_rx_env.test_done;
        *result = nvrecord_extension_p->system_info.le_rx_env.test_result;

        nv_record_post_write_operation(lock);
    }while(0);
}

void nv_record_factory_get_bt_nonsignalint_rx_test_result(bool* test_done, uint16_t* result, uint16_t* hec_nb, uint16_t* crc_nb)
{
    do{
        if (NULL == nvrecord_extension_p)
        {
            break;
        }

        if(test_done == NULL || result == NULL)
        {
            break;
        }

        uint32_t lock = nv_record_pre_write_operation();

        *test_done = nvrecord_extension_p->system_info.bt_nonsignaling_env.test_done;
        *result = nvrecord_extension_p->system_info.bt_nonsignaling_env.test_result;
        *hec_nb = nvrecord_extension_p->system_info.bt_nonsignaling_env.hec_nb;
        *crc_nb = nvrecord_extension_p->system_info.bt_nonsignaling_env.crc_nb;

        nv_record_post_write_operation(lock);
    }while(0);
}

int nv_record_env_init(void)
{
    nv_record_open(section_usrdata_ddbrecord);
    return 0;
}
#endif // #if defined(NEW_NV_RECORD_ENABLED)

