/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "gsound_dbg.h"
#include "gsound_custom.h"
#include "gsound_custom_service.h"
#include "gsound_service.h"
#include "gsound_custom_audio.h"
#include "nvrecord_gsound.h"
#include "app_ai_manager_api.h"
#include "app_ai_tws.h"
#include "ai_thread.h"
#include "ai_manager.h"
#ifdef TWS_SYSTEM_ENABLED
#include "app_ibrt_internal.h"
#include "gsound_custom_tws.h"
#endif

#include "gsound_custom_bt.h"

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/
/**
 * @description: callback function used for gsoundLib when gsound service is
 *                enabled
 * @param       : void
 * @return      : void
 */
static void __on_gsound_enabled(void);

/**
 * @description : callback function used for gsoundLib when gsound service is
 *                disabled
 * @param       : void
 * @return      : void
 */
static void __on_gsound_disabled(void);

/************************private variable defination************************/
static GSoundServiceObserver gsound_service_observer = {
    __on_gsound_enabled,
    __on_gsound_disabled,
};

/****************************function defination****************************/
static inline int8_t to_ascii_hex(int8_t value)
{
    value &= 0x0F;
    return value + (value <= 9 ? '0' : ('A' - 10));
}

void gsound_service_init(bool enable)
{
    GLOG_I("gsound enable state:%d", enable);

    GSoundServiceConfig gsound_config = {
        .gsound_enabled = enable,
    };

    GSoundServiceInit(GSOUND_BUILD_ID,
                      &gsound_config,
                      &gsound_service_observer);
}

void gsound_service_enable_switch(bool onOff)
{
    GLOG_I("[%s] switch:%d", __func__, onOff);

    if (onOff && !gsound_is_service_enabled())
    {
#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
        app_ai_manager_set_spec_update_flag(AI_SPEC_GSOUND, false);
#endif    
        GSoundServiceEnable();
    }
    else if (!onOff && gsound_is_service_enabled())
    {
#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
        app_ai_manager_set_spec_update_flag(AI_SPEC_GSOUND, true);
#endif    
        GSoundServiceDisable();
    }
    else
    {
        GLOG_I("No need to switch gsound service.");
    }
}

static void __on_gsound_enabled(void)
{
    GLOG_I("[%s]+++", __func__);

    gsound_set_service_enable_state(true);
    if(gsound_bt_is_channel_connected(GSOUND_CHANNEL_CONTROL) || gsound_bt_is_channel_connected(GSOUND_CHANNEL_AUDIO))
    {
        uint8_t ai_connect_index = app_ai_get_connect_index_from_ai_spec(AI_SPEC_GSOUND);
        ai_manager_update_foreground_ai_conidx(ai_connect_index,true);
    }

    if (app_ai_manager_is_in_multi_ai_mode())
    {
        if (app_ai_manager_get_spec_connected_status(AI_SPEC_GSOUND) == 1)
        {
            app_ai_manager_voicekey_save_status(true);
            app_ai_manager_set_current_spec(AI_SPEC_GSOUND);
            app_ai_tws_sync_ai_manager_info();
        }
    }

    nv_record_gsound_rec_updata_enable_state(true);

    GLOG_I("[%s]---", __func__);
}

static void __on_gsound_disabled(void)
{
    GLOG_I("[%s]+++", __func__);

    gsound_set_service_enable_state(false);
    gsound_custom_audio_close_mic();

    if (app_ai_manager_is_in_multi_ai_mode())
    {
        if (app_ai_manager_get_spec_connected_status(AI_SPEC_GSOUND) == 1)
        {
            app_ai_manager_set_spec_connected_status(AI_SPEC_GSOUND, 0);
            uint8_t ai_connect_index = app_ai_get_connect_index_from_ai_spec(AI_SPEC_GSOUND);
            ai_manager_update_foreground_ai_conidx(ai_connect_index,false);
#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
            if (app_ai_manager_get_spec_update_flag())
            {
                app_ai_manager_spec_update_start_reboot();
            }
#endif
        }
    }

    nv_record_gsound_rec_updata_enable_state(false);

    GLOG_I("[%s]---", __func__);
}

void gsound_util_sn_conv(uint8_t const *const bes_sn,
                         uint32_t bes_len,
                         uint8_t *const gsound_sn,
                         uint32_t gsound_len)
{
    int32_t len;
    uint8_t *gsound_nxt = gsound_sn;

    ASSERT((NULL != gsound_sn) && (NULL != bes_sn),
           "invalid pointers");

    // Store length of smallest array from src (bes) and dest (gsound).
    // Note the "... - 1" and "... / 2" in line below. Since we are
    // converting from hex to ascii, we need 2 bytes of ascii for every
    // hex value, plus 1 byte for a null.
    len = min((( int32_t )bes_len), ((( int32_t )gsound_len - 1) / 2));

    // ensure string is NULL terminated
    // regardless of bes_sn parameter
    memset(gsound_sn, 0, gsound_len);
    int32_t i;

    for (i = 0; i < len; i++)
    {
        int8_t ch     = bes_sn[i];
        *gsound_nxt++ = to_ascii_hex(ch >> 4);
        *gsound_nxt++ = to_ascii_hex(ch);
    }
}
