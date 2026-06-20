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
#include "plat_addr_map.h"

#include "hal_cmu.h"
#include "hal_sec.h"

#include CHIP_SPECIFIC_HDR(reg_sec)

//these sec regs can only be accessed in security state
static struct HAL_SEC_T *const sec = (struct HAL_SEC_T *)SEC_CTRL_BASE;

static struct HAL_SEC_T *const aon_sec = (struct HAL_SEC_T *)AON_SEC_CTRL_BASE;

void hal_sec_set_gdma_nonsec(bool nonsec)
{
    if (nonsec)
        sec->REG_00C |= (SEC_CFG_NONSEC_GDMA);
    else
        sec->REG_00C &= ~(SEC_CFG_NONSEC_GDMA);
}

void hal_sec_set_adma_nonsec(bool nonsec)
{
    if (nonsec)
        sec->REG_00C |= (SEC_CFG_NONSEC_ADMA);
    else
        sec->REG_00C &= ~(SEC_CFG_NONSEC_ADMA);
}

void hal_sec_set_bcm_nonsec(bool nonsec)
{
    if (nonsec)
        sec->REG_00C |= SEC_CFG_NONSEC_BCM;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_BCM;
}

void hal_sec_set_usb_nonsec(bool nonsec)
{
    if (nonsec)
        sec->REG_00C |= SEC_CFG_NONSEC_USB;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_USB;
}

void hal_sec_set_bt2mcu_nonsec(bool nonsec)
{
    if (nonsec)
        sec->REG_00C |= SEC_CFG_NONSEC_BT2MCU;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_BT2MCU;
}

void hal_sec_set_sens2mcu_nonsec(bool nonsec)
{
    if (nonsec)
        sec->REG_00C |= SEC_CFG_NONSEC_SENS2MCU;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_SENS2MCU;
}

void hal_sec_cfg_nonsec_bypass(bool bypass)
{
    if (bypass)
        sec->REG_004 |= SEC_CFG_NONSEC_BYPASS_MPC_SPINOR0;
    else
        sec->REG_004 &= ~SEC_CFG_NONSEC_BYPASS_MPC_SPINOR0;
}

void hal_sec_set_i2c_slv_nonsec(bool nonsec)
{
    if (nonsec)
        aon_sec->REG_000 |= AON_SEC_CFG_NOSEC_I2C_SLV;
    else
        aon_sec->REG_000 &= ~AON_SEC_CFG_NOSEC_I2C_SLV;
}

void hal_sec_set_sens2aon_nonsec(bool nonsec)
{
    if (nonsec)
        aon_sec->REG_000 |= AON_SEC_CFG_NOSEC_SENS;
    else
        aon_sec->REG_000 &= ~AON_SEC_CFG_NOSEC_SENS;
}

void hal_sec_set_sram0_nosec(bool nonsec)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_004 |= SEC_CFG_AP_PROT_SRAM0;
    if (nonsec)
        sec->REG_004 |= SEC_CFG_NONSEC_PROT_SRAM0;
    else
        sec->REG_004 &= ~SEC_CFG_NONSEC_PROT_SRAM0;
    sec->REG_004 |= SEC_CFG_SEC_RESP_PROT_SRAM0;
    sec->REG_004 &= ~SEC_CFG_NONSEC_BYPASS_PROT_SRAM0;
}

void hal_sec_set_sram1_nosec(bool nonsec)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_004 |= SEC_CFG_AP_PROT_SRAM1;
    if (nonsec)
        sec->REG_004 |= SEC_CFG_NONSEC_PROT_SRAM1;
    else
        sec->REG_004 &= ~SEC_CFG_NONSEC_PROT_SRAM1;
    sec->REG_004 |= SEC_CFG_SEC_RESP_PROT_SRAM1;
    sec->REG_004 &= ~SEC_CFG_NONSEC_BYPASS_PROT_SRAM1;
}

void hal_sec_set_sram2_7_nosec(int ram_id, bool nonsec)
{
    uint32_t val;
    ram_id = (ram_id-2)*4;

    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    val = sec->REG_000;
    val |= (SEC_CFG_AP_PROT_SRAM2>>ram_id);
    if (nonsec)
        val |= (SEC_CFG_NONSEC_PROT_SRAM2>>ram_id);
    else
        val &= ~(SEC_CFG_NONSEC_PROT_SRAM2>>ram_id);
    val |= (SEC_CFG_SEC_RESP_PROT_SRAM2>>ram_id);
    val &= ~((uint32_t)SEC_CFG_NONSEC_BYPASS_PROT_SRAM2>>ram_id);
    sec->REG_000 = val;
}

void hal_sec_set_sram2_nosec(bool nonsec)
{
    hal_sec_set_sram2_7_nosec(2, nonsec);
}

void hal_sec_set_sram3_nosec(bool nonsec)
{
    hal_sec_set_sram2_7_nosec(3, nonsec);
}

void hal_sec_set_sram4_nosec(bool nonsec)
{
    hal_sec_set_sram2_7_nosec(4, nonsec);
}

void hal_sec_set_sram5_nosec(bool nonsec)
{
    hal_sec_set_sram2_7_nosec(5, nonsec);
}

void hal_sec_set_sram6_nosec(bool nonsec)
{
    hal_sec_set_sram2_7_nosec(6, nonsec);
}

void hal_sec_set_sram7_nosec(bool nonsec)
{
    hal_sec_set_sram2_7_nosec(7, nonsec);
}

void hal_sec_set_sram8_nosec(bool nonsec)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_000 |= SEC_CFG_AP_PROT_SRAM8;
    if (nonsec)
        sec->REG_000 |= SEC_CFG_NONSEC_PROT_SRAM8;
    else
        sec->REG_000 &= ~SEC_CFG_NONSEC_PROT_SRAM8;
    sec->REG_000 |= SEC_CFG_SEC_RESP_PROT_SRAM8;
    sec->REG_000 &= ~SEC_CFG_NONSEC_BYPASS_PROT_SRAM8;
}

void hal_sec_set_mcu2sens_nosec(bool nonsec)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_000 |= SEC_CFG_AP_PROT_MCU2SENS;
    if (nonsec)
        sec->REG_000 |= SEC_CFG_NONSEC_PROT_MCU2SENS;
    else
        sec->REG_000 &= ~SEC_CFG_NONSEC_PROT_MCU2SENS;
    sec->REG_000 |= SEC_CFG_SEC_RESP_PROT_MCU2SENS;
    sec->REG_000 &= ~SEC_CFG_NONSEC_BYPASS_PROT_MCU2SENS;
}

void hal_sec_idau_enable(bool enable)
{
    if (enable) {
        sec->REG_01C |= SEC_IDAUEN;
    } else {
        sec->REG_01C &= ~SEC_IDAUEN;
    }
}

void hal_sec_init(void)
{
    hal_cmu_clock_enable(HAL_CMU_MOD_P_TZC);
    hal_cmu_clock_enable(HAL_CMU_AON_A_TZC);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_TZC);
    hal_cmu_reset_clear(HAL_CMU_AON_A_TZC);

    ///TODO: idau doesn't work  as expected now
    //hal_sec_idau_enable(true);

    hal_sec_set_gdma_nonsec(true);
    hal_sec_set_adma_nonsec(true);
    hal_sec_set_bcm_nonsec(true);
    hal_sec_set_usb_nonsec(true);
    hal_sec_set_bt2mcu_nonsec(true);
    hal_sec_set_i2c_slv_nonsec(true);
    hal_sec_set_sens2mcu_nonsec(true);
    hal_sec_set_sens2aon_nonsec(true);
}
