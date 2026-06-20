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
#include "app_gaf_dbg.h"
#include "app_gaf.h"
#include "app_iap_tm_msg.h"
/*
 * FUNCTION DEFINATIONS
 ****************************************************************************************
 */

int app_iap_tm_start_cmd(uint8_t stream_lid, uint8_t transmit, uint8_t payload_type)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_iap_read_iso_tx_sync_cmd(uint8_t stream_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_gaf_iap_msg_tm_cnt_get_cmd(uint8_t stream_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_gaf_iap_msg_test_mode_stop_cmd(uint8_t stream_lid)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

int app_iap_tm_init(void)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

