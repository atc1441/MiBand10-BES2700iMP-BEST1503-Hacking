/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#include "tota_stream_data_transfer.h"
#include "app_tota_cmd_code.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_audio.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_audio_EQ.h"
#include "cmsis_os.h"

#if defined(TOTA_EQ_TUNING)
#include "hal_cmd.h"
#include "audio_cfg.h"
#include "crc32_c.h"
#endif
extern uint8_t app_audio_get_eq();
extern int app_audio_set_eq(uint8_t index);
extern bool app_meridian_eq(bool onoff);
extern bool app_is_meridian_on();

/**/
static void _audio_EQ_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);

/*-----------------------------------------------------------------------------*/
static void _audio_EQ_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"%s Func code 0x%x, param len %d", __func__, funcCode, paramLen);
    TOTA_LOG_DBG(0,"Param content:");
    DUMP8("%02x ", ptrParam, paramLen);
    APP_TOTA_CMD_RET_STATUS_E rsp_status = TOTA_NO_ERROR;
    uint8_t resData[48]={0};
    uint32_t resLen=1;
    switch (funcCode)
    {
        case OP_TOTA_MERIDIAN_EFFECT_CMD:
            resData[0] = app_meridian_eq(ptrParam[0]);
            break;
        case OP_TOTA_EQ_SELECT_CMD:
            break;
        case OP_TOTA_EQ_SET_CMD:
        {
#if defined(TOTA_EQ_TUNING)
            resData[0] = app_tota_audio_EQ(ptrParam,paramLen);
#else
            rsp_status = TOTA_EQ_GET_FAILED;
#endif
            break;
        }
        case OP_TOTA_GET_SECINFO_EQ:
        {
#if defined(TOTA_EQ_TUNING) && defined(AUDIO_SECTION_ENABLE)
            APP_TOTA_AUDIO_EQ_SECTION_INFO_T  *secInfo = (APP_TOTA_AUDIO_EQ_SECTION_INFO_T *)resData;
            audio_cfg_get_eq_section_info(&secInfo->startAddr, &secInfo->totalSize, &secInfo->version);
            app_tota_send_data(funcCode, resData, sizeof(APP_TOTA_AUDIO_EQ_SECTION_INFO_T));
            return;
#else
            rsp_status = TOTA_EQ_GET_FAILED;
#endif
            break;
        }
        case OP_TOTA_EQ_GET_CMD:
        {
#if defined(TOTA_EQ_TUNING) && defined(AUDIO_SECTION_ENABLE)
            uint8_t * pdata = (uint8_t *)audio_cfg_get_default_audio_section();
            if(NULL != pdata)
            {
                app_tota_send_data(funcCode,pdata,sizeof_audio_section());
                return;
            }
            else
            {
                rsp_status = TOTA_EQ_GET_FAILED;
            }
#endif
            break;
        }
        case OP_TOTA_EQ_SET_DATA:
        default:
            break;
    }
    app_tota_send_rsp(funcCode,rsp_status,resData,resLen);

}

TOTA_COMMAND_TO_ADD(OP_TOTA_MERIDIAN_EFFECT_CMD, _audio_EQ_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_SELECT_CMD, _audio_EQ_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_SET_CMD, _audio_EQ_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_GET_CMD, _audio_EQ_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_EQ_SET_DATA, _audio_EQ_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_SECINFO_EQ, _audio_EQ_cmd_handle, false, 0, NULL );
