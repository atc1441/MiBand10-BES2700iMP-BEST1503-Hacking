/***************************************************************************
*
*Copyright 2015-2020 BES.
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
#include "gsound_target_hotword_internal.h"
#include "nvrecord_gsound.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/

/****************************function defination****************************/
const uint8_t *GSoundTargetHotwordGetModelFile(const char *model_id)
{
    const uint8_t *ptr = NULL;

    if (model_id)
    {
        ptr = (const uint8_t *)nv_record_gsound_rec_get_hotword_model_addr(model_id, false, 0);
        GLOG_D("pointer of the model_id [%s] is:%p", model_id, ptr);
        ASSERT(ptr, "INVALID model address");
    }
    else
    {
        ASSERT(0, "Received null pointer from libgsound.");
    }

    return ptr;
}

GSoundStatus GSoundTargetHotwordGetSupportedModels(uint8_t buffer_len,
                                                   char *const models_out)
{
    GSoundStatus ret = GSOUND_STATUS_OK;
    
    if(nv_record_gsound_rec_get_supported_model_id(models_out, &buffer_len))
    {
        ret = GSOUND_STATUS_ERROR;
    }
    GLOG_D("[%s] models_out:%s, length_out:%d", __func__, models_out, buffer_len);

    return ret;
}