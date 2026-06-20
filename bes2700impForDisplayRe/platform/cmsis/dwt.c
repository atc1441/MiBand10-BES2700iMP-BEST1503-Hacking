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
#include "plat_types.h"
#include "cmsis.h"
#include <stdint.h>
#include "dwt.h"

typedef struct {
  __IOM uint32_t COMP;
  __IOM uint32_t RESERVERED0;
  __IOM uint32_t FUNCTION;
  __IOM uint32_t RESERVERED1;
} dwt_watchpoint_config;

#ifdef DWT_ENABLED
int dwt_enable(void)
{
    int ret = !!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk);
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    return ret;
}

int dwt_disable(void)
{
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    return 0;
}

int watchpoint_enable(uint8_t wpt_index, uint32_t *word_address,
                                            enum watchpoint_rw access)
{
    const int num = (DWT->CTRL >> 28) & 0xf;
    dwt_watchpoint_config *config;
    int func = 0;

    if (wpt_index > num)
        return -1;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk /*enable tracing*/ |
            CoreDebug_DEMCR_MON_EN_Msk /*enable debug interrupt*/;

    uint32_t left = (uint32_t)word_address & 0x3;
    uint32_t align = 0b10;
    if (left == 0)
        align = 0b10;
    else if (left & 0x01)
        align = 0b00;
    else if (left & 0x02)
        align = 0b01;

    if (access == WPT_READ)
        func = 4;
    else if (access == WPT_WRITE)
        func = 5;
    else if (access == WPT_RW)
        func = 7;

    config = (dwt_watchpoint_config *)&DWT->COMP0 + wpt_index;
    config->COMP = (uint32_t)word_address;
    config->FUNCTION =  (align << 10)
                        | (0x01 << 4) /*Always generate a Debug exception */
                        | func;
    return 0;
}

int watchpoint_disable(uint8_t wpt_index)
{
    const int num = (DWT->CTRL >> 28) & 0xf;
    dwt_watchpoint_config *config;

    if (wpt_index > num)
        return -1;

    config = (dwt_watchpoint_config *)&DWT->COMP0 + wpt_index;
    config->FUNCTION = 0;

    return 0;
}

/*
 * return the number of supported watchpoints
 */
uint8_t watchpoint_get_numbers(void)
{
    const uint8_t num = (DWT->CTRL >> 28) & 0xf;

    return num;
}

#if 0
uint32_t  my_clobbered_variable;
void watchpoint_test()
{
    dwt_enable();
    watchpoint_disable(0);
    my_clobbered_variable = 0x5 * 5; /*here should be ok*/
    watchpoint_enable(0, &my_clobbered_variable, WPT_WRITE);
    my_clobbered_variable = 0x55;
}
#endif

#endif
