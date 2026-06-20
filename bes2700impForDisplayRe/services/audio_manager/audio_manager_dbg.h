/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#ifndef __AUDIO_MANAGER_DBG_H__
#define __AUDIO_MANAGER_DBG_H__


typedef enum
{
    RET_OK,
    RET_OOM,
    RET_STOP,
    RET_INVALID_PARAMS,
    RET_FAIL
}AM_RET;


#define return_if_fail(p) if(!(p)) \
    {TRACE(2, "%s:%d Warning: "#p" failed.\n", __func__, __LINE__); return;}

#define return_val_if_fail(p, ret) if(!(p)) \
    {TRACE(2, "%s:%d Warning: "#p" failed.\n", __func__, __LINE__); return (ret);}

#endif /* __AUDIO_MANAGER_DBG_H__ */