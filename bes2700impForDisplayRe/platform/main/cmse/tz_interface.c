/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * interface.c      Secure/non-secure callable application code
 *
 * Version 1.0
 *    Initial Release
 *---------------------------------------------------------------------------*/
#include "string.h"
#include "plat_types.h"
#include <arm_cmse.h>     // CMSE definitions
#include "tz_interface.h"    // Header file with secure interface API
#include "hal_sleep.h"
#include "hal_trace.h"
#include "hal_timer.h"

/* typedef for non-secure callback functions */
typedef funcptr funcptr_NS __attribute__((cmse_nonsecure_call));

/* Non-secure callable (entry) function */
__attribute__((cmse_nonsecure_entry))
int func1(int x)
{
    TRACE(0, "FUNC:%s", __func__);
#ifndef SIMU
    hal_sys_timer_delay(MS_TO_TICKS(100));
#endif
    return x+3;
}

/* Non-secure callable (entry) function, calling a non-secure callback function */
__attribute__((cmse_nonsecure_entry))
int func2(funcptr callback, int x)
{
    funcptr_NS callback_NS;               // non-secure callback function pointer
    int y;

    TRACE(0, "FUNC:%s", __func__);
#ifndef SIMU
    hal_sys_timer_delay(MS_TO_TICKS(200));
#endif

    /* return function pointer with cleared LSB */
    callback_NS = (funcptr_NS)cmse_nsfptr_create(callback);

    y = callback_NS (x+1);

    return (y+2);
}

char *sec_demo_string = "CMSE SECURE STRING\r\n";
__attribute__((cmse_nonsecure_entry))
int get_demo_string (char *dst, uint32_t len)
{
    char *dst_p_ok;

    dst_p_ok = cmse_check_address_range (dst, len, CMSE_NONSECURE);
    if (dst_p_ok != NULL) {
        /* requested copy range is completely in non-secure memory */
        if (len > strlen(sec_demo_string)) {
            strcpy(dst_p_ok, sec_demo_string);
            return 0;
        } else
            return -1;
    }
    else
    {
        //do something else
        return -2;
    }
}
