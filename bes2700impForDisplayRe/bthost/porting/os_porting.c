/*
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
#ifndef __OS_PORTING_H__
#define __OS_PORTING_H__

#include "cmsis.h"
#include "cmsis_os.h"
#include <stdio.h>
#include "os_porting.h"

int bt_int_lock()
{
    return int_lock();
}

void bt_int_unlock(int v)
{
    int_unlock(v);
}

#endif /* __OS_PORTING_H__ */

