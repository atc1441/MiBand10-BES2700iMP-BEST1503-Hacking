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
#ifndef __APP_FPGA_TEST_PROXY_H__
#define __APP_FPGA_TEST_PROXY_H__


typedef enum
{
    TWS_SLAVE_PAIRING,
    TWS_MASTER_PAIRING,
    SET_ACCESS_MODE,
    TWS_CONNECT,
    TWS_DISCONNECT,
    W4_MOBILE_CONNECT,
    MOBILE_LINK_CONNECT,
    MOBILE_LINK_DISCONNECT,
    START_LINK_IBRT,
    STOP_LINK_IBRT,
    MOBILE_LINK_RW,
    DUMP_IBRT_CONN_INFO,
    DUMP_IBRT_MGR_INFO,
    IBRT_MGR_UNIT_TEST,
}fpga_test_cmd_type_t;

void app_fpga_test_proxy_entry(fpga_test_cmd_type_t cmd_type,uint32_t param = 0);



#endif

