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
#ifndef __REG_SDIO_DEVICE_H__
#define __REG_SDIO_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

struct SDIO_DEVICE_T {
    __IO uint32_t CONTROL;                  //0x00
    __I  uint32_t COMMAND;                  //0x04
    __I  uint32_t ARGUMENT;                 //0x08
    __I  uint32_t BLOCK_COUNT;              //0x0C
    __IO uint32_t DMA1_ADDRESS;             //0x10
    __IO uint32_t DMA1_CONTROL;             //0x14
    __IO uint32_t DMA2_ADDRESS;             //0x18
    __IO uint32_t DMA2_CONTROL;             //0x1C
    __I  uint32_t ERASE_BLOCK_START_ADDRESS;//0x20
    __I  uint32_t ERASE_BLOCK_END_ADDRESS;  //0x24
    __IO uint32_t PASSWORD_LENGTH;          //0x28
    __IO uint32_t SECURE_BLOCK_COUNT;       //0x2C
    __IO uint32_t TPLMANID;                 //0x30
    __I  uint32_t RESERVED1[2];             //0x34~0x38
    __IO uint32_t INTERRUPT_STATUS;         //0x3C
    __IO uint32_t INTERRUPT_STATUS_ENABLE;  //0x40
    __IO uint32_t INTERRUPT_SIGNAL_ENABLE;  //0x44
    __IO uint32_t CARD_ADDRESS;             //0x48
    __IO uint32_t CARD_DATA;                //0x4C
    __IO uint32_t IO_READY;                 //0x50
    __IO uint32_t FUNCTION1_CONTROL;        //0x54
    __IO uint32_t FUNCTION2_CONTROL;        //0x58
    __IO uint32_t SDIO_CCCR_CONTROL;        //0x5C
    __IO uint32_t SDIO_FBR1_CONTROL;        //0x60
    __IO uint32_t SDIO_FBR2_CONTROL;        //0x64
    __IO uint32_t SDIO_FBR3_CONTROL;        //0x68
    __IO uint32_t SDIO_FBR4_CONTROL;        //0x6C
    __IO uint32_t SDIO_FBR5_CONTROL;        //0x70
    __IO uint32_t SDIO_FBR6_CONTROL;        //0x74
    __IO uint32_t SDIO_FBR7_CONTROL;        //0x78
    __I  uint32_t RESERVED2;                //0x7C
    __IO uint32_t CARD_SIZE;                //0x80
    __IO uint32_t CARD_OCR;                 //0x84
    __IO uint32_t CONTROL2;                 //0x88
    __IO uint32_t DEG_MODE;                 //0x8C
    __IO uint32_t FUNCTION3_CONTROL;        //0x90
    __IO uint32_t FUNCTION4_CONTROL;        //0x94
    __IO uint32_t FUNCTION5_CONTROL;        //0x98
    __IO uint32_t INTERRUPT_STATUS2;        //0x9C
    __IO uint32_t INTERRUPT_STATUS2_ENABLE; //0xA0
    __IO uint32_t INTERRUPT_SIGNAL2_ENABLE; //0xA4
    __IO uint32_t PASSWORD_127_96;          //0xA8
    __IO uint32_t PASSWORD_95_64;           //0xAC
    __IO uint32_t PASSWORD_63_32;           //0xB0
    __IO uint32_t PASSWORD_31_0;            //0xB4
    __IO uint32_t ADMA_ERROR_STATUS;        //0xB8
    __I  uint32_t RCA;                      //0xBC
    __I  uint32_t DBG0;                     //0xC0
    __I  uint32_t DBG1;                     //0xC4
    __I  uint32_t DBG2;                     //0xC8
    __I  uint32_t DBG3;                     //0xCC
    __I  uint32_t DBG4;                     //0xD0
    __I  uint32_t DBG5;                     //0xD4
    __I  uint32_t DBG6;                     //0xD8
    __IO uint32_t AHB_MASTER_BURST_SIZE;    //0xDC
    __I  uint32_t ARGUMENT2;                //0xE0
    __I  uint32_t RESERVED3[7];             //0xE4~0xFF
    __IO uint32_t HOST_INT_CLR;             //0x100,default value is 0
    __IO uint32_t RESERVED4;                //0x104,default value is 0
    __IO uint32_t RX_BUF_CFG;               //0x108,default value is 0
};

// reg_000
#define SDIO_PROGRAM_DONE                                   (1 << 0)
#define SDIO_RESERVED                                       (1 << 1)
#define SDIO_CARD_INIT_DONE                                 (1 << 2)
#define SDIO_ADDR_OUT_OF_RANGE                              (1 << 3)
#define SDIO_ADDR_MISALIGN                                  (1 << 4)
#define SDIO_RPMB_DIS_EN                                    (1 << 5)
#define SDIO_ERASE_PARAM                                    (1 << 6)
#define SDIO_CARD_ECC_FAIL                                  (1 << 7)
#define SDIO_CC_ERROR                                       (1 << 8)
#define SDIO_ERROR                                          (1 << 9)
#define SDIO_MMC_IRQ_TRIGGER                                (1 << 10)
#define SDIO_CMD_DATA_OUTPUT_EDGE                           (1 << 11)
#define SDIO_CMD32_CMD33_EN                                 (1 << 12)
#define SDIO_BOOT_SEQ_SUPPORT                               (1 << 13)
#define SDIO_SWITCH_ERROR                                   (1 << 14)
#define SDIO_BOOT_ACK                                       (1 << 15)
#define SDIO_WP_VIOLATION                                   (1 << 16)
#define SDIO_WP_ERASE_SKIP                                  (1 << 17)
#define SDIO_CID_CSD_OVERWRITE                              (1 << 18)
#define SDIO_AKE_SEQ_ERROR                                  (1 << 19)
#define SDIO_CARD_ECC_DISABLE                               (1 << 20)
#define SDIO_STREAM_THRSH_SIZE(n)                           (((n) & 0x7) << 21)
#define SDIO_STREAM_THRSH_SIZE_MASK                         (0x7 << 21)
#define SDIO_STREAM_THRSH_SIZE_SHIFT                        (21)
#define SDIO_PERM_WR_PROT                                   (1 << 24)
#define SDIO_TEMP_WR_PROT                                   (1 << 25)
#define SDIO_WP_COM_ENABLED                                 (1 << 26)
#define SDIO_ALLOW_AKE                                      (1 << 27)
#define SDIO_SECURED_MODE                                   (1 << 28)
#define SDIO_AKE_SEQ_OK                                     (1 << 29)
#define SDIO_ASSD_DIS_EN                                    (1 << 30)
#define SDIO_BOOT_DATA_RDY                                  (1 << 31)

// reg_004
#define SDIO_APPLICATION                                    (1 << 0)
#define SDIO_BLOCK_SIZE(n)                                  (((n) & 0xFFF) << 1)
#define SDIO_BLOCK_SIZE_MASK                                (0xFFF << 1)
#define SDIO_BLOCK_SIZE_SHIFT                               (1)
#define SDIO_COMMAND_INDEX(n)                               (((n) & 0x3F) << 13)
#define SDIO_COMMAND_INDEX_MASK                             (0x3F << 13)
#define SDIO_COMMAND_INDEX_SHIFT                            (13)
#define SDIO_CURRENT_BUS_WIDTH(n)                           (((n) & 0x3) << 19)
#define SDIO_CURRENT_BUS_WIDTH_MASK                         (0x3 << 19)
#define SDIO_CURRENT_BUS_WIDTH_SHIFT                        (19)
#define SDIO_CURRENT_SPEED(n)                               (((n) & 0x7) << 21)
#define SDIO_CURRENT_SPEED_MASK                             (0x7 << 21)
#define SDIO_CURRENT_SPEED_SHIFT                            (21)
#define SDIO_CARD_STATE(n)                                  (((n) & 0xF) << 24)
#define SDIO_CARD_STATE_MASK                                (0xF << 24)
#define SDIO_CARD_STATE_SHIFT                               (24)
#define SDIO_ERASE_SEQUENCE                                 (1 << 28)

// reg_008
#define SDIO_ARGUMENT(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define SDIO_ARGUMENT_MASK                                  (0xFFFFFFFF << 0)
#define SDIO_ARGUMENT_SHIFT                                 (0)

// reg_00c
#define SDIO_BLOCK_COUNT(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define SDIO_BLOCK_COUNT_MASK                               (0xFFFFFFFF << 0)
#define SDIO_BLOCK_COUNT_SHIFT                              (0)

// reg_010
#define SDIO_DMA1_ADDR(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DMA1_ADDR_MASK                                 (0xFFFFFFFF << 0)
#define SDIO_DMA1_ADDR_SHIFT                                (0)

// reg_014
#define SDIO_DMA1_ADDR_VALID                                (1 << 0)
#define SDIO_DMA1_BUF_SIZE(n)                               (((n) & 0x7) << 1)
#define SDIO_DMA1_BUF_SIZE_MASK                             (0x7 << 1)
#define SDIO_DMA1_BUF_SIZE_SHIFT                            (1)

// reg_018
#define SDIO_DMA2_ADDR(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DMA2_ADDR_MASK                                 (0xFFFFFFFF << 0)
#define SDIO_DMA2_ADDR_SHIFT                                (0)

// reg_01c
#define SDIO_DMA2_ADDR_VALID                                (1 << 0)
#define SDIO_DMA2_BUF_SIZE(n)                               (((n) & 0x7) << 1)
#define SDIO_DMA2_BUF_SIZE_MASK                             (0x7 << 1)
#define SDIO_DMA2_BUF_SIZE_SHIFT                            (1)

// reg_020
#define SDIO_ERASE_BLOCK_START(n)                           (((n) & 0xFFFFFFFF) << 0)
#define SDIO_ERASE_BLOCK_START_MASK                         (0xFFFFFFFF << 0)
#define SDIO_ERASE_BLOCK_START_SHIFT                        (0)

// reg_024
#define SDIO_ERASE_BLOCK_END(n)                             (((n) & 0xFFFFFFFF) << 0)
#define SDIO_ERASE_BLOCK_END_MASK                           (0xFFFFFFFF << 0)
#define SDIO_ERASE_BLOCK_END_SHIFT                          (0)

// reg_028
#define SDIO_PWDS_LEN(n)                                    (((n) & 0xFF) << 0)
#define SDIO_PWDS_LEN_MASK                                  (0xFF << 0)
#define SDIO_PWDS_LEN_SHIFT                                 (0)
#define SDIO_DISABLE_LOCK_UNLOCK                            (1 << 8)

// reg_02c
#define SDIO_SECURE_BLOCK_COUNT(n)                          (((n) & 0xFF) << 0)
#define SDIO_SECURE_BLOCK_COUNT_MASK                        (0xFF << 0)
#define SDIO_SECURE_BLOCK_COUNT_SHIFT                       (0)

// reg_030
#define SDIO_TPLMID_MANF(n)                                 (((n) & 0xFFFF) << 0)
#define SDIO_TPLMID_MANF_MASK                               (0xFFFF << 0)
#define SDIO_TPLMID_MANF_SHIFT                              (0)
#define SDIO_TPLMID_CARD(n)                                 (((n) & 0xFFFF) << 16)
#define SDIO_TPLMID_CARD_MASK                               (0xFFFF << 16)
#define SDIO_TPLMID_CARD_SHIFT                              (16)

// reg_03c
#define SDIO_TRX_COMPLETE_INT                               (1 << 0)
#define SDIO_DMA1_INT                                       (1 << 1)
#define SDIO_SLEEP_AWAKE_INT                                (1 << 2)
#define SDIO_WR_START_INT                                   (1 << 3)
#define SDIO_RD_START_INT                                   (1 << 4)
#define SDIO_PASSWORD_SET_INT                               (1 << 5)
#define SDIO_PASSWORD_RESET_INT                             (1 << 6)
#define SDIO_LOCK_CARD_INT                                  (1 << 7)
#define SDIO_UNLOCK_CARD_INT                                (1 << 8)
#define SDIO_FORCE_ERASE_INT                                (1 << 9)
#define SDIO_ERASE_INT                                      (1 << 10)
#define SDIO_CMD11_INT                                      (1 << 11)
#define SDIO_CMD0_CMD52_SOFTRST_INT                         (1 << 12)
#define SDIO_CMD6_CHECK_DONE_INT                            (1 << 13)
#define SDIO_CMD6_SWITCH_DONE_INT                           (1 << 14)
#define SDIO_PROGRAM_CSD_INT                                (1 << 15)
#define SDIO_ACMD23_INT                                     (1 << 16)
#define SDIO_CMD20_INT                                      (1 << 17)
#define SDIO_HOST_INT                                       (1 << 18)
#define SDIO_CMD4_INT                                       (1 << 19)
#define SDIO_BOOT_START_INT                                 (1 << 20)
#define SDIO_FUNC1_RESET_INT                                (1 << 21)
#define SDIO_FUNC2_RESET_INT                                (1 << 22)
#define SDIO_CMD11_CLK_STOP_INT                             (1 << 23)
#define SDIO_CMD11_CLK_START_INT                            (1 << 24)
#define SDIO_PROGRAM_START_INT                              (1 << 25)
#define SDIO_CMD40_INT                                      (1 << 26)
#define SDIO_CMDR1B_INT                                     (1 << 27)
#define SDIO_FUNCX_CRC_END_ERROR_INT                        (1 << 28)
#define SDIO_FUNCX_ABORT_INT                                (1 << 29)
#define SDIO_LRST_INT                                       (1 << 30)
#define SDIO_BOOT_COMPLETE_INT                              (1 << 31)

// reg_040
#define SDIO_TX_COMPLETE_EN                                 (1 << 0)
#define SDIO_DMA1_EN                                        (1 << 1)
#define SDIO_SLEEP_AWAKE_EN                                 (1 << 2)
#define SDIO_WR_START_EN                                    (1 << 3)
#define SDIO_RD_START_EN                                    (1 << 4)
#define SDIO_PASSWORD_SET_EN                                (1 << 5)
#define SDIO_PASSWORD_RESET_EN                              (1 << 6)
#define SDIO_LOCK_CARD_EN                                   (1 << 7)
#define SDIO_UNLOCK_CARD_EN                                 (1 << 8)
#define SDIO_FORCE_ERASE_EN                                 (1 << 9)
#define SDIO_ERASE_EN                                       (1 << 10)
#define SDIO_CMD11_EN                                       (1 << 11)
#define SDIO_CMD0_CMD52_SOFTRST_EN                          (1 << 12)
#define SDIO_CMD6_CHECK_DONE_EN                             (1 << 13)
#define SDIO_CMD6_SWITCH_DONE_EN                            (1 << 14)
#define SDIO_PROGRAM_CSD_EN                                 (1 << 15)
#define SDIO_ACMD23_EN                                      (1 << 16)
#define SDIO_CMD20_EN                                       (1 << 17)
#define SDIO_HOST_EN                                        (1 << 18)
#define SDIO_CMD4_EN                                        (1 << 19)
#define SDIO_BOOT_START_EN                                  (1 << 20)
#define SDIO_FUNC1_RESET_EN                                 (1 << 21)
#define SDIO_FUNC2_RESET_EN                                 (1 << 22)
#define SDIO_CMD11_CLK_STOP_EN                              (1 << 23)
#define SDIO_CMD11_CLK_START_EN                             (1 << 24)
#define SDIO_PROGRAM_START_EN                               (1 << 25)
#define SDIO_CMD40_EN                                       (1 << 26)
#define SDIO_CMDR1B_EN                                      (1 << 27)
#define SDIO_FUNCX_CRC_END_ERROR_EN                         (1 << 28)
#define SDIO_FUNCX_ABORT_EN                                 (1 << 29)
#define SDIO_LRST_EN                                        (1 << 30)
#define SDIO_BOOT_COMPLETE_EN                               (1 << 31)

// reg_044
#define SDIO_TX_COMPLETE_SIG_EN                             (1 << 0)
#define SDIO_DMA1_SIG_EN                                    (1 << 1)
#define SDIO_SLEEP_AWAKE_SIG_EN                             (1 << 2)
#define SDIO_WR_START_SIG_EN                                (1 << 3)
#define SDIO_RD_START_SIG_EN                                (1 << 4)
#define SDIO_PASSWORD_SET_SIG_EN                            (1 << 5)
#define SDIO_PASSWORD_RESET_SIG_EN                          (1 << 6)
#define SDIO_LOCK_CARD_SIG_EN                               (1 << 7)
#define SDIO_UNLOCK_CARD_SIG_EN                             (1 << 8)
#define SDIO_FORCE_ERASE_SIG_EN                             (1 << 9)
#define SDIO_ERASE_SIG_EN                                   (1 << 10)
#define SDIO_CMD11_SIG_EN                                   (1 << 11)
#define SDIO_CMD0_CMD52_SOFTRST_SIG_EN                      (1 << 12)
#define SDIO_CMD6_CHECK_DONE_SIG_EN                         (1 << 13)
#define SDIO_CMD6_SWITCH_DONE_SIG_EN                        (1 << 14)
#define SDIO_PROGRAM_CSD_SIG_EN                             (1 << 15)
#define SDIO_ACMD23_SIG_EN                                  (1 << 16)
#define SDIO_CMD20_SIG_EN                                   (1 << 17)
#define SDIO_HOST_SIG_EN                                    (1 << 18)
#define SDIO_CMD4_SIG_EN                                    (1 << 19)
#define SDIO_BOOT_START_SIG_EN                              (1 << 20)
#define SDIO_FUNC1_RESET_SIG_EN                             (1 << 21)
#define SDIO_FUNC2_RESET_SIG_EN                             (1 << 22)
#define SDIO_CMD11_CLK_STOP_SIG_EN                          (1 << 23)
#define SDIO_CMD11_CLK_START_SIG_EN                         (1 << 24)
#define SDIO_PROGRAM_START_SIG_EN                           (1 << 25)
#define SDIO_CMD40_SIG_EN                                   (1 << 26)
#define SDIO_CMDR1B_SIG_EN                                  (1 << 27)
#define SDIO_FUNCX_CRC_END_ERROR_SIG_EN                     (1 << 28)
#define SDIO_FUNCX_ABORT_SIG_EN                             (1 << 29)
#define SDIO_LRST_SIG_EN                                    (1 << 30)
#define SDIO_BOOT_COMPLETE_SIG_EN                           (1 << 31)

// reg_048
#define SDIO_STARTING_ADDR(n)                               (((n) & 0x3FF) << 0)
#define SDIO_STARTING_ADDR_MASK                             (0x3FF << 0)
#define SDIO_STARTING_ADDR_SHIFT                            (0)

// reg_04c
#define SDIO_CARD_DATA(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define SDIO_CARD_DATA_MASK                                 (0xFFFFFFFF << 0)
#define SDIO_CARD_DATA_SHIFT                                (0)

// reg_050
#define SDIO_RESERVED0500                                   (1 << 0)
#define SDIO_FUNC1_READY                                    (1 << 1)
#define SDIO_FUNC2_READY                                    (1 << 2)
#define SDIO_FUNC3_READY                                    (1 << 3)
#define SDIO_FUNC4_READY                                    (1 << 4)
#define SDIO_FUNC5_READY                                    (1 << 5)

// reg_054
#define SDIO_FUNC1_READ_COUNT(n)                            (((n) & 0xFFFF) << 0)
#define SDIO_FUNC1_READ_COUNT_MASK                          (0xFFFF << 0)
#define SDIO_FUNC1_READ_COUNT_SHIFT                         (0)

// reg_058
#define SDIO_FUNC2_READ_COUNT(n)                            (((n) & 0xFFFF) << 0)
#define SDIO_FUNC2_READ_COUNT_MASK                          (0xFFFF << 0)
#define SDIO_FUNC2_READ_COUNT_SHIFT                         (0)

// reg_05c
#define SDIO_CCCR_VERSION(n)                                (((n) & 0xF) << 0)
#define SDIO_CCCR_VERSION_MASK                              (0xF << 0)
#define SDIO_CCCR_VERSION_SHIFT                             (0)
#define SDIO_SDIO_SPEC_REVISION(n)                          (((n) & 0xF) << 4)
#define SDIO_SDIO_SPEC_REVISION_MASK                        (0xF << 4)
#define SDIO_SDIO_SPEC_REVISION_SHIFT                       (4)
#define SDIO_SDX(n)                                         (((n) & 0xF) << 8)
#define SDIO_SDX_MASK                                       (0xF << 8)
#define SDIO_SDX_SHIFT                                      (8)
#define SDIO_S8B                                            (1 << 12)
#define SDIO_SCSI                                           (1 << 13)
#define SDIO_SDC                                            (1 << 14)
#define SDIO_SMB                                            (1 << 15)
#define SDIO_SRW                                            (1 << 16)
#define SDIO_SBS                                            (1 << 17)
#define SDIO_S4MI                                           (1 << 18)
#define SDIO_LSC                                            (1 << 19)
#define SDIO_4BLS                                           (1 << 20)
#define SDIO_SMPC                                           (1 << 21)
#define SDIO_SHS                                            (1 << 22)
#define SDIO_SDR50                                          (1 << 23)
#define SDIO_SDR104                                         (1 << 24)
#define SDIO_DDR50                                          (1 << 25)
#define SDIO_SDTA                                           (1 << 26)
#define SDIO_SDTC                                           (1 << 27)
#define SDIO_SDTD                                           (1 << 28)
#define SDIO_SAI                                            (1 << 29)

// reg_060
#define SDIO_FUNC1_STANDARD_IF_CODE(n)                      (((n) & 0xF) << 0)
#define SDIO_FUNC1_STANDARD_IF_CODE_MASK                    (0xF << 0)
#define SDIO_FUNC1_STANDARD_IF_CODE_SHIFT                   (0)
#define SDIO_FUNC1_EX_IF_CODE(n)                            (((n) & 0xFF) << 4)
#define SDIO_FUNC1_EX_IF_CODE_MASK                          (0xFF << 4)
#define SDIO_FUNC1_EX_IF_CODE_SHIFT                         (4)
#define SDIO_RESERVED0600(n)                                (((n) & 0x7) << 12)
#define SDIO_RESERVED0600_MASK                              (0x7 << 12)
#define SDIO_RESERVED0600_SHIFT                             (12)
#define SDIO_FUNC1_SUPPORT_CSA                              (1 << 15)
#define SDIO_FUNC1_SPS                                      (1 << 16)

// reg_064
#define SDIO_FUNC2_STANDARD_IF_CODE(n)                      (((n) & 0xF) << 0)
#define SDIO_FUNC2_STANDARD_IF_CODE_MASK                    (0xF << 0)
#define SDIO_FUNC2_STANDARD_IF_CODE_SHIFT                   (0)
#define SDIO_FUNC2_EX_IF_CODE(n)                            (((n) & 0xFF) << 4)
#define SDIO_FUNC2_EX_IF_CODE_MASK                          (0xFF << 4)
#define SDIO_FUNC2_EX_IF_CODE_SHIFT                         (4)
#define SDIO_RESERVED0640(n)                                (((n) & 0x7) << 12)
#define SDIO_RESERVED0640_MASK                              (0x7 << 12)
#define SDIO_RESERVED0640_SHIFT                             (12)
#define SDIO_FUNC2_SUPPORT_CSA                              (1 << 15)
#define SDIO_FUNC2_SPS                                      (1 << 16)

// reg_080
#define SDIO_CARD_SIZE(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define SDIO_CARD_SIZE_MASK                                 (0xFFFFFFFF << 0)
#define SDIO_CARD_SIZE_SHIFT                                (0)

// reg_084
#define SDIO_CARD_OCR(n)                                    (((n) & 0xFFFFFF) << 0)
#define SDIO_CARD_OCR_MASK                                  (0xFFFFFF << 0)
#define SDIO_CARD_OCR_SHIFT                                 (0)
#define SDIO_SWITCH_TO_1V8                                  (1 << 24)
#define SDIO_RESERVED0840(n)                                (((n) & 0x3) << 25)
#define SDIO_RESERVED0840_MASK                              (0x3 << 25)
#define SDIO_RESERVED0840_SHIFT                             (25)
#define SDIO_MEM_PRESENT                                    (1 << 27)
#define SDIO_NUM_FUNCTIONS(n)                               (((n) & 0x7) << 28)
#define SDIO_NUM_FUNCTIONS_MASK                             (0x7 << 28)
#define SDIO_NUM_FUNCTIONS_SHIFT                            (28)

// reg_088
#define SDIO_CMD60_BUSY_EN_DIS                              (1 << 0)
#define SDIO_RESERVED0880                                   (1 << 1)
#define SDIO_ADMA_ENABLE                                    (1 << 2)

// reg_090
#define SDIO_FUNC3_READ_COUNT(n)                            (((n) & 0xFFFF) << 0)
#define SDIO_FUNC3_READ_COUNT_MASK                          (0xFFFF << 0)
#define SDIO_FUNC3_READ_COUNT_SHIFT                         (0)

// reg_094
#define SDIO_FUNC4_READ_COUNT(n)                            (((n) & 0xFFFF) << 0)
#define SDIO_FUNC4_READ_COUNT_MASK                          (0xFFFF << 0)
#define SDIO_FUNC4_READ_COUNT_SHIFT                         (0)

// reg_098
#define SDIO_FUNC5_READ_COUNT(n)                            (((n) & 0xFFFF) << 0)
#define SDIO_FUNC5_READ_COUNT_MASK                          (0xFFFF << 0)
#define SDIO_FUNC5_READ_COUNT_SHIFT                         (0)

// reg_09c
#define SDIO_FUNC3_RESET_INT                                (1 << 0)
#define SDIO_FUNC4_RESET_INT                                (1 << 1)
#define SDIO_FUNC5_RESET_INT                                (1 << 2)
#define SDIO_RESERVED09C0(n)                                (((n) & 0x3) << 3)
#define SDIO_RESERVED09C0_MASK                              (0x3 << 3)
#define SDIO_RESERVED09C0_SHIFT                             (3)
#define SDIO_PACKED_INT                                     (1 << 5)
#define SDIO_ADMA_ERROR_INT                                 (1 << 6)

// reg_0a0
#define SDIO_FUNC3_RESET_EN                                 (1 << 0)
#define SDIO_FUNC4_RESET_EN                                 (1 << 1)
#define SDIO_FUNC5_RESET_EN                                 (1 << 2)
#define SDIO_RESERVED0A00(n)                                (((n) & 0x3) << 3)
#define SDIO_RESERVED0A00_MASK                              (0x3 << 3)
#define SDIO_RESERVED0A00_SHIFT                             (3)
#define SDIO_PACKED_EN                                      (1 << 5)
#define SDIO_ADMA_ERROR_EN                                  (1 << 6)

// reg_0a4
#define SDIO_FUNC3_RESET_SIG_EN                             (1 << 0)
#define SDIO_FUNC4_RESET_SIG_EN                             (1 << 1)
#define SDIO_FUNC5_RESET_SIG_EN                             (1 << 2)
#define SDIO_RESERVED0A40(n)                                (((n) & 0x3) << 3)
#define SDIO_RESERVED0A40_MASK                              (0x3 << 3)
#define SDIO_RESERVED0A40_SHIFT                             (3)
#define SDIO_PACKED_SIG_EN                                  (1 << 5)
#define SDIO_ADMA_ERROR_SIG_EN                              (1 << 6)

// reg_0a8
#define SDIO_PASSWORD_127_96(n)                             (((n) & 0xFFFFFFFF) << 0)
#define SDIO_PASSWORD_127_96_MASK                           (0xFFFFFFFF << 0)
#define SDIO_PASSWORD_127_96_SHIFT                          (0)

// reg_0ac
#define SDIO_PASSWORD_95_64(n)                              (((n) & 0xFFFFFFFF) << 0)
#define SDIO_PASSWORD_95_64_MASK                            (0xFFFFFFFF << 0)
#define SDIO_PASSWORD_95_64_SHIFT                           (0)

// reg_0b0
#define SDIO_PASSWORD_63_32(n)                              (((n) & 0xFFFFFFFF) << 0)
#define SDIO_PASSWORD_63_32_MASK                            (0xFFFFFFFF << 0)
#define SDIO_PASSWORD_63_32_SHIFT                           (0)

// reg_0b4
#define SDIO_PASSWORD_31_0(n)                               (((n) & 0xFFFFFFFF) << 0)
#define SDIO_PASSWORD_31_0_MASK                             (0xFFFFFFFF << 0)
#define SDIO_PASSWORD_31_0_SHIFT                            (0)

// reg_0b8
#define SDIO_ADMA_ERROR_STATE(n)                            (((n) & 0x3) << 0)
#define SDIO_ADMA_ERROR_STATE_MASK                          (0x3 << 0)
#define SDIO_ADMA_ERROR_STATE_SHIFT                         (0)
#define SDIO_ADMA_LENGTH_ERROR                              (1 << 2)

// reg_0bc
#define SDIO_RCA(n)                                         (((n) & 0xFFFF) << 0)
#define SDIO_RCA_MASK                                       (0xFFFF << 0)
#define SDIO_RCA_SHIFT                                      (0)

// reg_0c0
#define SDIO_DBG0(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG0_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG0_SHIFT                                     (0)

// reg_0c4
#define SDIO_DBG1(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG1_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG1_SHIFT                                     (0)

// reg_0c8
#define SDIO_DBG2(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG2_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG2_SHIFT                                     (0)

// reg_0cc
#define SDIO_DBG3(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG3_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG3_SHIFT                                     (0)

// reg_0d0
#define SDIO_DBG4(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG4_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG4_SHIFT                                     (0)

// reg_0d4
#define SDIO_DBG5(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG5_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG5_SHIFT                                     (0)

// reg_0d8
#define SDIO_DBG6(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define SDIO_DBG6_MASK                                      (0xFFFFFFFF << 0)
#define SDIO_DBG6_SHIFT                                     (0)

// reg_0dc
#define SDIO_BURST_SIZE_EN(n)                               (((n) & 0x7F) << 0)
#define SDIO_BURST_SIZE_EN_MASK                             (0x7F << 0)
#define SDIO_BURST_SIZE_EN_SHIFT                            (0)

// reg_0e0
#define SDIO_ARGUMENT2(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define SDIO_ARGUMENT2_MASK                                 (0xFFFFFFFF << 0)
#define SDIO_ARGUMENT2_SHIFT                                (0)

// reg_100
#define SDIO_HOST_INT_CLR                                   (1 << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG0(n)                     (((n) & 0x7FFFFFFF) << 1)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG0_MASK                   (0x7FFFFFFF << 1)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG0_SHIFT                  (1)

// reg_104
#define SDIO_FUNC1_SDIO_R_AHB_W_REG1(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG1_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG1_SHIFT                  (0)

// reg_108
#define SDIO_RX_BUF_CNT(n)                                  (((n) & 0xFF) << 0)
#define SDIO_RX_BUF_CNT_MASK                                (0xFF << 0)
#define SDIO_RX_BUF_CNT_SHIFT                               (0)
#define SDIO_RX_BUF_LEN(n)                                  (((n) & 0xFFFF) << 8)
#define SDIO_RX_BUF_LEN_MASK                                (0xFFFF << 8)
#define SDIO_RX_BUF_LEN_SHIFT                               (8)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG2(n)                     (((n) & 0xFF) << 24)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG2_MASK                   (0xFF << 24)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG2_SHIFT                  (24)

// reg_10c
#define SDIO_FUNC1_SDIO_R_AHB_W_REG3(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG3_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG3_SHIFT                  (0)

// reg_110
#define SDIO_FUNC1_SDIO_R_AHB_W_REG4(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG4_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG4_SHIFT                  (0)

// reg_114
#define SDIO_FUNC1_SDIO_R_AHB_W_REG5(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG5_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG5_SHIFT                  (0)

// reg_118
#define SDIO_FUNC1_SDIO_R_AHB_W_REG6(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG6_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG6_SHIFT                  (0)

// reg_11c
#define SDIO_FUNC1_SDIO_R_AHB_W_REG7(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG7_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_R_AHB_W_REG7_SHIFT                  (0)

// reg_120
#define SDIO_HOST_INT_SET                                   (1 << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG0(n)                     (((n) & 0x7FFFFFFF) << 1)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG0_MASK                   (0x7FFFFFFF << 1)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG0_SHIFT                  (1)

// reg_124
#define SDIO_FUNC1_SDIO_W_AHB_R_REG1(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG1_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG1_SHIFT                  (0)

// reg_128
#define SDIO_FUNC1_SDIO_W_AHB_R_REG2(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG2_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG2_SHIFT                  (0)

// reg_12c
#define SDIO_FUNC1_SDIO_W_AHB_R_REG3(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG3_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG3_SHIFT                  (0)

// reg_130
#define SDIO_FUNC1_SDIO_W_AHB_R_REG4(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG4_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG4_SHIFT                  (0)

// reg_134
#define SDIO_FUNC1_SDIO_W_AHB_R_REG5(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG5_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG5_SHIFT                  (0)

// reg_138
#define SDIO_FUNC1_SDIO_W_AHB_R_REG6(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG6_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG6_SHIFT                  (0)

// reg_13c
#define SDIO_FUNC1_SDIO_W_AHB_R_REG7(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG7_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC1_SDIO_W_AHB_R_REG7_SHIFT                  (0)

// reg_140
#define SDIO_FUNC2_SDIO_R_AHB_W_REG0(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG0_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG0_SHIFT                  (0)

// reg_144
#define SDIO_FUNC2_SDIO_R_AHB_W_REG1(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG1_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG1_SHIFT                  (0)

// reg_148
#define SDIO_FUNC2_SDIO_R_AHB_W_REG2(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG2_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG2_SHIFT                  (0)

// reg_14c
#define SDIO_FUNC2_SDIO_R_AHB_W_REG3(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG3_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG3_SHIFT                  (0)

// reg_150
#define SDIO_FUNC2_SDIO_R_AHB_W_REG4(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG4_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG4_SHIFT                  (0)

// reg_154
#define SDIO_FUNC2_SDIO_R_AHB_W_REG5(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG5_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG5_SHIFT                  (0)

// reg_158
#define SDIO_FUNC2_SDIO_R_AHB_W_REG6(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG6_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG6_SHIFT                  (0)

// reg_15c
#define SDIO_FUNC2_SDIO_R_AHB_W_REG7(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG7_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_R_AHB_W_REG7_SHIFT                  (0)

// reg_160
#define SDIO_FUNC2_SDIO_W_AHB_R_REG0(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG0_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG0_SHIFT                  (0)

// reg_164
#define SDIO_FUNC2_SDIO_W_AHB_R_REG1(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG1_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG1_SHIFT                  (0)

// reg_168
#define SDIO_FUNC2_SDIO_W_AHB_R_REG2(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG2_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG2_SHIFT                  (0)

// reg_16c
#define SDIO_FUNC2_SDIO_W_AHB_R_REG3(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG3_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG3_SHIFT                  (0)

// reg_170
#define SDIO_FUNC2_SDIO_W_AHB_R_REG4(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG4_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG4_SHIFT                  (0)

// reg_174
#define SDIO_FUNC2_SDIO_W_AHB_R_REG5(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG5_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG5_SHIFT                  (0)

// reg_178
#define SDIO_FUNC2_SDIO_W_AHB_R_REG6(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG6_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG6_SHIFT                  (0)

// reg_17c
#define SDIO_FUNC2_SDIO_W_AHB_R_REG7(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG7_MASK                   (0xFFFFFFFF << 0)
#define SDIO_FUNC2_SDIO_W_AHB_R_REG7_SHIFT                  (0)

#endif
