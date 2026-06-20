#ifndef __REG_SEC_BEST1306_H__
#define __REG_SEC_BEST1306_H__

#include "plat_types.h"

struct HAL_SEC_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
};

// reg_00
#define SEC_CFG_AP_PROT_SRAM8                               (1 << 0)
#define SEC_CFG_NONSEC_PROT_SRAM8                           (1 << 1)
#define SEC_CFG_SEC_RESP_PROT_SRAM8                         (1 << 2)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM8                    (1 << 3)
#define SEC_CFG_AP_PROT_MCU2SENS                            (1 << 4)
#define SEC_CFG_NONSEC_PROT_MCU2SENS                        (1 << 5)
#define SEC_CFG_SEC_RESP_PROT_MCU2SENS                      (1 << 6)
#define SEC_CFG_NONSEC_BYPASS_PROT_MCU2SENS                 (1 << 7)
#define SEC_CFG_AP_PROT_SRAM7                               (1 << 8)
#define SEC_CFG_NONSEC_PROT_SRAM7                           (1 << 9)
#define SEC_CFG_SEC_RESP_PROT_SRAM7                         (1 << 10)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM7                    (1 << 11)
#define SEC_CFG_AP_PROT_SRAM6                               (1 << 12)
#define SEC_CFG_NONSEC_PROT_SRAM6                           (1 << 13)
#define SEC_CFG_SEC_RESP_PROT_SRAM6                         (1 << 14)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM6                    (1 << 15)
#define SEC_CFG_AP_PROT_SRAM5                               (1 << 16)
#define SEC_CFG_NONSEC_PROT_SRAM5                           (1 << 17)
#define SEC_CFG_SEC_RESP_PROT_SRAM5                         (1 << 18)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM5                    (1 << 19)
#define SEC_CFG_AP_PROT_SRAM4                               (1 << 20)
#define SEC_CFG_NONSEC_PROT_SRAM4                           (1 << 21)
#define SEC_CFG_SEC_RESP_PROT_SRAM4                         (1 << 22)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM4                    (1 << 23)
#define SEC_CFG_AP_PROT_SRAM3                               (1 << 24)
#define SEC_CFG_NONSEC_PROT_SRAM3                           (1 << 25)
#define SEC_CFG_SEC_RESP_PROT_SRAM3                         (1 << 26)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM3                    (1 << 27)
#define SEC_CFG_AP_PROT_SRAM2                               (1 << 28)
#define SEC_CFG_NONSEC_PROT_SRAM2                           (1 << 29)
#define SEC_CFG_SEC_RESP_PROT_SRAM2                         (1 << 30)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM2                    (1 << 31)

// reg_04
#define SEC_CFG_AP_PROT_SRAM1                               (1 << 0)
#define SEC_CFG_NONSEC_PROT_SRAM1                           (1 << 1)
#define SEC_CFG_SEC_RESP_PROT_SRAM1                         (1 << 2)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM1                    (1 << 3)
#define SEC_CFG_AP_PROT_SRAM0                               (1 << 4)
#define SEC_CFG_NONSEC_PROT_SRAM0                           (1 << 5)
#define SEC_CFG_SEC_RESP_PROT_SRAM0                         (1 << 6)
#define SEC_CFG_NONSEC_BYPASS_PROT_SRAM0                    (1 << 7)
#define SEC_CFG_AP_PROT_ROM0                                (1 << 8)
#define SEC_CFG_NONSEC_PROT_ROM0                            (1 << 9)
#define SEC_CFG_SEC_RESP_PROT_ROM0                          (1 << 10)
#define SEC_CFG_NONSEC_BYPASS_PROT_ROM0                     (1 << 11)
#define SEC_CFG_INIT_VALUE_MPC_PSRAM                        (1 << 12)
#define SEC_CFG_NONSEC_BYPASS_MPC_PSRAM                     (1 << 13)
#define SEC_CFG_INIT_VALUE_MPC_SPINOR0                      (1 << 14)
#define SEC_CFG_NONSEC_BYPASS_MPC_SPINOR0                   (1 << 15)
#define SEC_CFG_INIT_VALUE_MPC_SPINOR1                      (1 << 16)
#define SEC_CFG_NONSEC_BYPASS_MPC_SPINOR1                   (1 << 17)

// reg_08
#define SEC_IRQ_CLR_PPC_SRAM8                               (1 << 0)
#define SEC_IRQ_EN_PPC_SRAM8                                (1 << 1)
#define SEC_IRQ_CLR_PPC_MCU2SENS                            (1 << 2)
#define SEC_IRQ_EN_PPC_MCU2SENS                             (1 << 3)
#define SEC_IRQ_CLR_PPC_SRAM7                               (1 << 4)
#define SEC_IRQ_EN_PPC_SRAM7                                (1 << 5)
#define SEC_IRQ_CLR_PPC_SRAM6                               (1 << 6)
#define SEC_IRQ_EN_PPC_SRAM6                                (1 << 7)
#define SEC_IRQ_CLR_PPC_SRAM5                               (1 << 8)
#define SEC_IRQ_EN_PPC_SRAM5                                (1 << 9)
#define SEC_IRQ_CLR_PPC_SRAM4                               (1 << 10)
#define SEC_IRQ_EN_PPC_SRAM4                                (1 << 11)
#define SEC_IRQ_CLR_PPC_SRAM3                               (1 << 12)
#define SEC_IRQ_EN_PPC_SRAM3                                (1 << 13)
#define SEC_IRQ_CLR_PPC_SRAM2                               (1 << 14)
#define SEC_IRQ_EN_PPC_SRAM2                                (1 << 15)
#define SEC_IRQ_CLR_PPC_SRAM1                               (1 << 16)
#define SEC_IRQ_EN_PPC_SRAM1                                (1 << 17)
#define SEC_IRQ_CLR_PPC_SRAM0                               (1 << 18)
#define SEC_IRQ_EN_PPC_SRAM0                                (1 << 19)
#define SEC_IRQ_CLR_PPC_ROM0                                (1 << 20)
#define SEC_IRQ_EN_PPC_ROM0                                 (1 << 21)
#define SEC_IRQ_EN_MPC_PSRAM                                (1 << 22)
#define SEC_IRQ_EN_MPC_SPINOR0                              (1 << 23)
#define SEC_IRQ_EN_MPC_SPINOR1                              (1 << 24)

// reg_0c
#define SEC_CFG_NONSEC_ADMA                                 (1 << 0)
#define SEC_CFG_NONSEC_GDMA                                 (1 << 1)
#define SEC_CFG_NONSEC_BCM                                  (1 << 2)
#define SEC_CFG_NONSEC_USB                                  (1 << 3)
#define SEC_CFG_NONSEC_LCDC                                 (1 << 4)
#define SEC_CFG_NONSEC_BT2MCU                               (1 << 5)
#define SEC_CFG_NONSEC_SENS2MCU                             (1 << 6)
#define SEC_CFG_NONSEC_BT_TP_DUMP                           (1 << 7)
#define SEC_CFG_NONSEC_EMMC                                 (1 << 8)

// reg_10
#define SEC_CFG_NONSEC_PROT_AHB1(n)                         (((n) & 0xFFFF) << 0)
#define SEC_CFG_NONSEC_PROT_AHB1_MASK                       (0xFFFF << 0)
#define SEC_CFG_NONSEC_PROT_AHB1_SHIFT                      (0)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB1(n)                  (((n) & 0xFFFF) << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB1_MASK                (0xFFFF << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB1_SHIFT               (16)

// reg_14
#define SEC_CFG_NONSEC_PROT_APB0(n)                         (((n) & 0x3FFFFF) << 0)
#define SEC_CFG_NONSEC_PROT_APB0_MASK                       (0x3FFFFF << 0)
#define SEC_CFG_NONSEC_PROT_APB0_SHIFT                      (0)

// reg_18
#define SEC_CFG_NONSEC_BYPASS_PROT_APB0(n)                  (((n) & 0x3FFFFF) << 0)
#define SEC_CFG_NONSEC_BYPASS_PROT_APB0_MASK                (0x3FFFFF << 0)
#define SEC_CFG_NONSEC_BYPASS_PROT_APB0_SHIFT               (0)

// reg_1c
#define SEC_IDAUEN                                          (1 << 0)
#define SEC_CFG_SEC2NSEC                                    (1 << 1)
#define SEC_SPIDEN_CORE0                                    (1 << 2)
#define SEC_SPNIDEN_CORE0                                   (1 << 3)
#define SEC_SPIDEN_CORE1                                    (1 << 4)
#define SEC_SPNIDEN_CORE1                                   (1 << 5)

// reg_20
#define SEC_IRQ_PPC_SRAM8                                   (1 << 0)
#define SEC_IRQ_PPC_MCU2SENS                                (1 << 1)
#define SEC_IRQ_PPC_SRAM7                                   (1 << 2)
#define SEC_IRQ_PPC_SRAM6                                   (1 << 3)
#define SEC_IRQ_PPC_SRAM5                                   (1 << 4)
#define SEC_IRQ_PPC_SRAM4                                   (1 << 5)
#define SEC_IRQ_PPC_SRAM3                                   (1 << 6)
#define SEC_IRQ_PPC_SRAM2                                   (1 << 7)
#define SEC_IRQ_PPC_SRAM1                                   (1 << 8)
#define SEC_IRQ_PPC_SRAM0                                   (1 << 9)
#define SEC_IRQ_PPC_ROM0                                    (1 << 10)
#define SEC_IRQ_MPC_PSRAM                                   (1 << 11)
#define SEC_IRQ_MPC_SPINOR0                                 (1 << 12)
#define SEC_IRQ_MPC_SPINOR1                                 (1 << 13)

//aon reg_00
#define AON_SEC_CFG_NOSEC_I2C_SLV                           (1 << 0)
#define AON_SEC_CFG_NOSEC_SENS                              (1 << 0)

#endif
