/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "nuttx/irq.h"

#if 1//defined(CONFIG_BES_HAVE_DVFS) || defined(CONFIG_PM)
#define CONFIG_IRQ_TO_CORE0
#endif

__STATIC_FORCEINLINE void __NVIC_SetPriorityGrouping(uint32_t PriorityGroup);
__STATIC_FORCEINLINE uint32_t __NVIC_GetPriorityGrouping(void);
__STATIC_FORCEINLINE uint32_t __NVIC_GetEnableIRQ(IRQn_Type IRQn);
__STATIC_FORCEINLINE uint32_t __NVIC_GetPendingIRQ(IRQn_Type IRQn);
__STATIC_FORCEINLINE void __NVIC_SetPendingIRQ(IRQn_Type IRQn);
__STATIC_FORCEINLINE void __NVIC_ClearPendingIRQ(IRQn_Type IRQn);
__STATIC_FORCEINLINE uint32_t __NVIC_GetActive(IRQn_Type IRQn);
__STATIC_FORCEINLINE uint32_t __NVIC_GetPriority(IRQn_Type IRQn);
__NO_RETURN __STATIC_FORCEINLINE void __NVIC_SystemReset(void);
__STATIC_FORCEINLINE void __NVIC_EnableIRQ(IRQn_Type IRQn);
__STATIC_FORCEINLINE void __NVIC_DisableIRQ(IRQn_Type IRQn);
__STATIC_FORCEINLINE void __NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority);
__STATIC_FORCEINLINE void __NVIC_SetVector(IRQn_Type IRQn, uint32_t vector);

extern uint32_t get_cpu_id(void);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
extern int  nuttx_smp_irqvec_cp2mcu(int irq, xcpt_t isr, FAR void *arg);
extern uint32_t  nuttx_smp_irq_cp2mcu(uint16_t api_id, uint16_t irq, uint32_t arg);
extern void nuttx_smp_irq_cp2mcu_noret(uint16_t api_id, uint16_t irq, uint32_t arg);
#endif
void nuttx_irq_attach(IRQn_Type IRQn, xcpt_t isr, uint32_t arg);
extern int up_irq_handler(int irq, FAR void *context, FAR void *arg);
extern bool crash_occurs[2];

#ifndef isValidIrq
#ifdef CHIP_SUBSYS_SENS
#define NUM_IPC_IRQ 1
#else
#define NUM_IPC_IRQ 4
#endif
#define IRQ_IND_START (CP2MCU_DATA_IRQn+NVIC_USER_IRQ_OFFSET)
#define IRQ_IND_END (CP2MCU_DATA_IRQn+NVIC_USER_IRQ_OFFSET+NUM_IPC_IRQ)

#define IRQ_DONE_START (MCU2CP_DONE_IRQn+NVIC_USER_IRQ_OFFSET)
#define IRQ_DONE_END (MCU2CP_DONE_IRQn+NVIC_USER_IRQ_OFFSET+NUM_IPC_IRQ)

#define isValidIrq(irq) (!crash_occurs[1] && irq != (WDT_IRQn+NVIC_USER_IRQ_OFFSET) && (irq < IRQ_IND_START || irq > IRQ_IND_END) && (irq < IRQ_DONE_START || irq > IRQ_DONE_END))
#if defined(CONFIG_IRQ_TO_CORE0)
#define isHiproIrq(irq) (irq == WDT_IRQn || irq == CP2MCU_DATA1_IRQn || irq == MCU2CP_DONE1_IRQn)
#else
#define isHiproIrq(irq) (irq == WDT_IRQn)
#endif
#endif

//api_id:0
extern int NXIC_IrqAttach(int irq, xcpt_t isr, FAR void *arg);

//api_id:1
__STATIC_FORCEINLINE void NXIC_SetPriorityGrouping(uint32_t PriorityGroup)
{
    //TRACE(0,"%s:%d,PriorityGroup:%d",__FUNCTION__,__LINE__,PriorityGroup);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if (get_cpu_id() == 1)
    {
        nuttx_smp_irq_cp2mcu_noret(1,0,PriorityGroup);
    }
    else
#endif
    __NVIC_SetPriorityGrouping(PriorityGroup);
}

//api_id:2
__STATIC_FORCEINLINE uint32_t NXIC_GetPriorityGrouping(void)
{
    //TRACE(0,"%s:%d",__FUNCTION__,__LINE__);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if (get_cpu_id() == 1)
    {
        return nuttx_smp_irq_cp2mcu(2,1,0);
    }
    else
#endif
    return __NVIC_GetPriorityGrouping();
}

//api_id:3
__STATIC_FORCEINLINE void NXIC_EnableIRQ(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        nuttx_smp_irq_cp2mcu_noret(3,irq,0);
    }
    else
#endif
    __NVIC_EnableIRQ((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:4
__STATIC_FORCEINLINE void NXIC_DisableIRQ(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        nuttx_smp_irq_cp2mcu_noret(4,irq,0);
    }
    else
#endif
    __NVIC_DisableIRQ((IRQn_Type)(IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:5
__STATIC_FORCEINLINE uint32_t NXIC_GetEnableIRQ(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        return nuttx_smp_irq_cp2mcu(5,irq,0);
    }
    else
#endif
    return __NVIC_GetEnableIRQ((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:6
__STATIC_FORCEINLINE uint32_t NXIC_GetPendingIRQ(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        return nuttx_smp_irq_cp2mcu(6,irq,0);
    }
    else
#endif
    return __NVIC_GetPendingIRQ((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:7
__STATIC_FORCEINLINE void NXIC_SetPendingIRQ(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        nuttx_smp_irq_cp2mcu_noret(7,irq,0);
    }
    else
#endif
    __NVIC_SetPendingIRQ((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:8
__STATIC_FORCEINLINE void NXIC_ClearPendingIRQ(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        nuttx_smp_irq_cp2mcu_noret(8,irq,0);
    }
    else
#endif
    __NVIC_ClearPendingIRQ((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:9
__STATIC_FORCEINLINE uint32_t NXIC_GetActive(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        return nuttx_smp_irq_cp2mcu(9,irq,0);
    }
    else
#endif
    return __NVIC_GetActive((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:10
__STATIC_FORCEINLINE uint32_t NXIC_GetPriority(int irq)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        return nuttx_smp_irq_cp2mcu(10,irq,0);
    }
    else
#endif
    return __NVIC_GetPriority((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET));
}

//api_id:11
__STATIC_FORCEINLINE void NXIC_SetPriority(int irq, uint32_t priority)
{
    //TRACE(0,"%s:%d,irq:%d",__FUNCTION__,__LINE__,irq);
#if defined(CONFIG_SMP)
    if (isHiproIrq(irq-NVIC_USER_IRQ_OFFSET)) {
        __NVIC_SetPriority((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET),0);
        return;
    }
#elif (RAMCP_SIZE != 0)
    if (get_cpu_id() == 1) {
        __NVIC_SetPriority((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET),priority);
        return;
    }
#endif

#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if ((irq >= NVIC_USER_IRQ_OFFSET) && (get_cpu_id() == 1) && isValidIrq(irq))
    {
        nuttx_smp_irq_cp2mcu_noret(11,irq,5);
    }
    else
#endif
    __NVIC_SetPriority((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET),5);
}

__STATIC_FORCEINLINE void NXIC_SetVector(int irq, uint32_t vector)
{
#if defined(CONFIG_SMP) || (RAMCP_SIZE != 0)
#if defined(CONFIG_SMP)
    if (isHiproIrq(irq-NVIC_USER_IRQ_OFFSET))
#else
    if (get_cpu_id() == 1)
#endif
    {
        __NVIC_SetVector((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET),vector);
        return;
    }
#endif
    nuttx_irq_attach((IRQn_Type)(irq-NVIC_USER_IRQ_OFFSET), up_irq_handler, vector);
}

//api_id:12
__STATIC_FORCEINLINE void NXIC_SystemReset(void)
{
    //TRACE(0,"%s:%d",__FUNCTION__,__LINE__);
#if defined(CONFIG_SMP) && defined(CONFIG_IRQ_TO_CORE0)
    if (get_cpu_id() == 1)
    {
        nuttx_smp_irq_cp2mcu_noret(12,2,0);
    }
    else
#endif
    __NVIC_SystemReset();
}

#define NVIC_SetPriorityGrouping      NXIC_SetPriorityGrouping
#define NVIC_GetPriorityGrouping      NXIC_GetPriorityGrouping
#define NVIC_GetEnableIRQ(a)          NXIC_GetEnableIRQ(a+NVIC_USER_IRQ_OFFSET)
#define NVIC_GetPendingIRQ(a)         NXIC_GetPendingIRQ(a+NVIC_USER_IRQ_OFFSET)
#define NVIC_SetPendingIRQ(a)         NXIC_SetPendingIRQ(a+NVIC_USER_IRQ_OFFSET)
#define NVIC_ClearPendingIRQ(a)       NXIC_ClearPendingIRQ(a+NVIC_USER_IRQ_OFFSET)
#define NVIC_GetActive(a)             NXIC_GetActive(a+NVIC_USER_IRQ_OFFSET)
#define NVIC_GetPriority(a)           NXIC_GetPriority(a+NVIC_USER_IRQ_OFFSET)
#define NVIC_SystemReset              NXIC_SystemReset


