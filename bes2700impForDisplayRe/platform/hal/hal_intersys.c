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
#include "plat_addr_map.h"

#if defined(BT_CMU_BASE) && !defined(NO_BT_INTERSYS)

#include "cmsis_nvic.h"
#ifdef TX_RX_PCM_MASK
#include "hal_chipid.h"
#endif
#include "hal_intersys.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "string.h"
#include CHIP_SPECIFIC_HDR(reg_btcmu)
#ifdef CHIP_SUBSYS_BTH
#include CHIP_SPECIFIC_HDR(reg_bthcmu)
#elif defined(CHIP_SUBSYS_SENS)
#include CHIP_SPECIFIC_HDR(reg_senscmu)
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)
#endif

#define MAX_SEND_RECORD_COUNT               3

#if defined (CHIP_SUBSYS_SENS) || defined(__SYS_AS_MAIN__)
#define BTCMU_INTERSYS_BIT_OFFSET           16
#else
#define BTCMU_INTERSYS_BIT_OFFSET           0
#endif

// BT-CMU ISIRQ_SET
#define BTCMU_MCU2BT_DATA_DONE_SET          (1 << (0 + BTCMU_INTERSYS_BIT_OFFSET))
#define BTCMU_MCU2BT_DATA1_DONE_SET         (1 << (1 + BTCMU_INTERSYS_BIT_OFFSET))
#define BTCMU_BT2MCU_DATA_IND_SET           (1 << (2 + BTCMU_INTERSYS_BIT_OFFSET))
#define BTCMU_BT2MCU_DATA1_IND_SET          (1 << (3 + BTCMU_INTERSYS_BIT_OFFSET))

// BT-CMU ISIRQ_CLR
#define BTCMU_MCU2BT_DATA_DONE_CLR          (1 << (0 + BTCMU_INTERSYS_BIT_OFFSET))
#define BTCMU_MCU2BT_DATA1_DONE_CLR         (1 << (1 + BTCMU_INTERSYS_BIT_OFFSET))
#define BTCMU_BT2MCU_DATA_IND_CLR           (1 << (2 + BTCMU_INTERSYS_BIT_OFFSET))
#define BTCMU_BT2MCU_DATA1_IND_CLR          (1 << (3 + BTCMU_INTERSYS_BIT_OFFSET))


#define BTCMU_BT2MCU_DATA2_IND_CLR           (1 << 2)
#define BTCMU_BT2MCU_DATA3_IND_CLR          (1 << 3 )

enum HAL_INTERSYS_IRQ_TYPE_T {
    HAL_INTERSYS_IRQ_SEND_IND,
    HAL_INTERSYS_IRQ_RECV_DONE,

    HAL_INTERSYS_IRQ_TYPE_QTY
};

struct HAL_INTERSYS_MSG_T {
    struct HAL_INTERSYS_MSG_T *next;        // pointer to next element in the list
    enum HAL_INTERSYS_MSG_TYPE_T type;      // message type
    unsigned int len;                       // message data length in bytes
    const unsigned char *data;              // pointer to the message data
};

struct HAL_INTERSYS_SEND_RECORD_T {
    struct HAL_INTERSYS_MSG_T msg;
    bool in_use;
};

static const IRQn_Type rx_irq_id[HAL_INTERSYS_ID_QTY] = {
    ISDATA_IRQn,
    ISDATA1_IRQn,
#if defined(__FPGA_1501P__)
    ISDATA2_IRQn,
    ISDATA3_IRQn,
#endif


};

static const IRQn_Type tx_irq_id[HAL_INTERSYS_ID_QTY] = {
    ISDONE_IRQn,
    ISDONE1_IRQn,
#if defined(__FPGA_1501P__)
    ISDONE2_IRQn,
    ISDONE3_IRQn,
#endif



};

static struct HAL_INTERSYS_MSG_T *** const bt_recv_msg_list_ppp[HAL_INTERSYS_ID_QTY] = {
    (struct HAL_INTERSYS_MSG_T ***)(BT_RAM_BASE + BT_INTESYS_MEM_OFFSET + 0x4),
    (struct HAL_INTERSYS_MSG_T ***)(BT_RAM_BASE+ BT_INTESYS_MEM_OFFSET + 0xC),
#if defined(__FPGA_1501P__)
    (struct HAL_INTERSYS_MSG_T ***)(BT_RAM_BASE+ BT_INTESYS_MEM_OFFSET + 0x14),
    (struct HAL_INTERSYS_MSG_T ***)(BT_RAM_BASE+ BT_INTESYS_MEM_OFFSET + 0x1C),
#endif

};

static struct HAL_INTERSYS_MSG_T ** const recv_msg_list_p[HAL_INTERSYS_ID_QTY] = {
    (struct HAL_INTERSYS_MSG_T **)(BT_RAM_BASE + BT_INTESYS_MEM_OFFSET),
    (struct HAL_INTERSYS_MSG_T **)(BT_RAM_BASE + BT_INTESYS_MEM_OFFSET + 0x8),
#if defined(__FPGA_1501P__)
    (struct HAL_INTERSYS_MSG_T **)(BT_RAM_BASE + BT_INTESYS_MEM_OFFSET + 0x10),
    (struct HAL_INTERSYS_MSG_T **)(BT_RAM_BASE + BT_INTESYS_MEM_OFFSET + 0x18),
#endif
};

SYNC_FLAGS_LOC static struct HAL_INTERSYS_MSG_T * send_msg_list_p[HAL_INTERSYS_ID_QTY];

static struct HAL_INTERSYS_MSG_T * send_pending_list_p[HAL_INTERSYS_ID_QTY];
static struct HAL_INTERSYS_MSG_T recv_pending_head[HAL_INTERSYS_ID_QTY];

SYNC_FLAGS_LOC static struct HAL_INTERSYS_SEND_RECORD_T send_msgs[HAL_INTERSYS_ID_QTY][MAX_SEND_RECORD_COUNT];

static HAL_INTERSYS_RX_IRQ_HANDLER rx_irq_handler[HAL_INTERSYS_ID_QTY][HAL_INTERSYS_MSG_TYPE_QTY];
static HAL_INTERSYS_TX_IRQ_HANDLER tx_irq_handler[HAL_INTERSYS_ID_QTY][HAL_INTERSYS_MSG_TYPE_QTY];

#if defined(__FPGA_1501P__)
static uint8_t chan_opened[HAL_INTERSYS_ID_QTY] = { 0, 0, 0,0};
#else
static uint8_t chan_opened[HAL_INTERSYS_ID_QTY] = { 0, 0, };
#endif

STATIC_ASSERT(sizeof(chan_opened[0]) * 8 >= HAL_INTERSYS_MSG_TYPE_QTY, "chan_opened size too small");

#if defined(__FPGA_1501P__)

static bool need_flow_ctrl[HAL_INTERSYS_ID_QTY] = { false, false, false,false};

static bool chan_busy[HAL_INTERSYS_ID_QTY] = { false, false,false,false};
#else
static bool need_flow_ctrl[HAL_INTERSYS_ID_QTY] = { false, false, };

static bool chan_busy[HAL_INTERSYS_ID_QTY] = { false, false, };

#endif
static bool busy_now = false;

#ifdef CHIP_SUBSYS_SENS
static struct SENSCMU_T * const cmu = (struct SENSCMU_T *)SENS_CMU_BASE;
#elif defined(CHIP_SUBSYS_BTH)
static struct BTHCMU_T * const cmu = (struct BTHCMU_T *)BTH_CMU_BASE;
#else
static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif

static struct BTCMU_T * const btcmu = (struct BTCMU_T *)BT_CMU_BASE;

static void hal_intersys_busy(enum HAL_INTERSYS_ID_T id, bool busy)
{
    int i;
    bool new_state;

    if (chan_busy[id] == busy) {
        return;
    }

    chan_busy[id] = busy;

    if (busy_now == busy) {
        return;
    }

    if (busy) {
        hal_sys_wake_lock(HAL_SYS_WAKE_LOCK_USER_INTERSYS);
        busy_now = true;
    } else {
        new_state = false;
        for (i = 0; i < HAL_INTERSYS_ID_QTY; i++) {
            if (chan_busy[i]) {
                new_state = true;
                break;
            }
        }
        if (!new_state) {
            hal_sys_wake_unlock(HAL_SYS_WAKE_LOCK_USER_INTERSYS);
            busy_now = false;
        }
    }
}

static int hal_intersys_peer_irq_set(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_IRQ_TYPE_T type)
{
    uint32_t value;

#if (CHIP_INTERSYS_VER <= 1)
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_MCU2BT_DATA_IND_SET;
        } else {
            value = CMU_BT2MCU_DATA_DONE_SET;
        }
    } else {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_MCU2BT_DATA1_IND_SET;
        } else {
            value = CMU_BT2MCU_DATA1_DONE_SET;
        }
    }

    cmu->ISIRQ_SET = value;
#else // (CHIP_INTERSYS_VER >= 2)
    if (type != HAL_INTERSYS_IRQ_SEND_IND) {
        return 1;
    }
#if defined(__FPGA_1501P__)
    if (id == HAL_INTERSYS_ID_0) {
        value = CMU_MCU2BT_DATA_IND_SET;
    } else if(id == HAL_INTERSYS_ID_1){
        value = CMU_MCU2BT_DATA1_IND_SET;
    }else if(id == HAL_INTERSYS_ID_2){
        value = CMU_MCU2BT_DATA2_IND_SET;
    }else if(id == HAL_INTERSYS_ID_3){
        value = CMU_MCU2BT_DATA3_IND_SET;
    }
    if(id < HAL_INTERSYS_ID_2)
    {
        cmu->ISIRQ_SET = value;
        cmu->ISIRQ_SET;
    }
    else
    {
        cmu->ISIRQ_SET1 = value;
        cmu->ISIRQ_SET1;
    }
#else
    if (id == HAL_INTERSYS_ID_0) {
        value = CMU_MCU2BT_DATA_IND_SET;
    } else {
        value = CMU_MCU2BT_DATA1_IND_SET;
    }
    cmu->ISIRQ_SET = value;
    cmu->ISIRQ_SET;
#endif

#endif // (CHIP_INTERSYS_VER >= 2)

    return 0;
}

static inline void btcmu_reg_update_wait(void)
{
    // Make sure BTCMU (26M clock domain) write opertions finish before return
    btcmu->ISIRQ_CLR;
}

static inline void btcmu_reg_update_wait1(void)
{
#if defined(__FPGA_1501P__)
    // Make sure BTCMU (26M clock domain) write opertions finish before return
    btcmu->BT2MCUIRQ_CLR1;
#endif
}


static int hal_intersys_local_irq_clear(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_IRQ_TYPE_T type)
{
    uint32_t value;

#if (CHIP_INTERSYS_VER <= 1)
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA_IND_CLR;
        } else {
            value = BTCMU_MCU2BT_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA1_IND_CLR;
        } else {
            value = BTCMU_MCU2BT_DATA1_DONE_CLR;
        }
    }

    btcmu->ISIRQ_CLR = value;
    btcmu_reg_update_wait();
#else

#if defined(__FPGA_1501P__)
    value = 0;
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA_DONE_CLR;
        }
    } else if(id == HAL_INTERSYS_ID_1){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA1_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA1_DONE_CLR;
        }
    }else if(id == HAL_INTERSYS_ID_2){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA2_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA2_DONE_CLR;
        }
    }else if(id == HAL_INTERSYS_ID_3){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA3_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA3_DONE_CLR;
        }
    }

    if(id < HAL_INTERSYS_ID_2)
    {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            btcmu->ISIRQ_CLR = value;
            btcmu_reg_update_wait();
        } else {
            cmu->ISIRQ_CLR = value;
        }
    }
    else
    {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            btcmu->BT2MCUIRQ_CLR1 = value;
            btcmu_reg_update_wait1();
        } else {
            cmu->ISIRQ_CLR1 = value;
        }
    }
#else
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA_DONE_CLR;
        }
    } else {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = BTCMU_BT2MCU_DATA1_IND_CLR;
        } else {
            value = CMU_BT2MCU_DATA1_DONE_CLR;
        }
    }

    if (type == HAL_INTERSYS_IRQ_SEND_IND) {
        btcmu->ISIRQ_CLR = value;
        btcmu_reg_update_wait();
    } else {
        cmu->ISIRQ_CLR = value;
    }
#endif
#endif

    return 0;
}

#if (CHIP_INTERSYS_VER <= 1)
static int hal_intersys_local_irq_set(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_IRQ_TYPE_T type)
{
    uint32_t value;

    if (type != HAL_INTERSYS_IRQ_SEND_IND) {
        return 1;
    }

    if (id == HAL_INTERSYS_ID_0) {
        value = BTCMU_BT2MCU_DATA_IND_SET;
    } else {
        value = BTCMU_BT2MCU_DATA1_IND_SET;
    }

    btcmu->ISIRQ_SET = value;
    btcmu_reg_update_wait();
    return 0;
}
#else
static int hal_intersys_local_irq_mask_set(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_IRQ_TYPE_T type)
{
    uint32_t value;
#if defined(__FPGA_1501P__)
    value = 0;

    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA_MSK_SET;
        }
    } else if (id == HAL_INTERSYS_ID_1) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA1_MSK_SET;
        }
    } else if (id == HAL_INTERSYS_ID_2) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA2_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA2_MSK_SET;
        }
    } else if (id == HAL_INTERSYS_ID_3) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA3_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA3_MSK_SET;
        }
    }
    if(id<HAL_INTERSYS_ID_2)
    {
        cmu->ISIRQ_SET = value;
    }
    else
    {
       TRACE(1,"INTERSY OPEN1 value=%x",value);

        cmu->ISIRQ_SET1 = value;
    }

#else
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA_MSK_SET;
        }
    } else {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_MSK_SET;
        } else {
            value = CMU_MCU2BT_DATA1_MSK_SET;
        }
    }

    cmu->ISIRQ_SET = value;
#endif
    return 0;
}

static int hal_intersys_local_irq_mask_clear(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_IRQ_TYPE_T type)
{
    uint32_t value;
#if defined(__FPGA_1501P__)
    value = 0;

    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA_MSK_CLR;
        }
    } else if(id == HAL_INTERSYS_ID_1){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA1_MSK_CLR;
        }
    }else if(id == HAL_INTERSYS_ID_2){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA2_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA2_MSK_CLR;
        }
    }else if(id == HAL_INTERSYS_ID_3){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA3_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA3_MSK_CLR;
        }
    }
    if(id<HAL_INTERSYS_ID_2)
    {
        cmu->ISIRQ_CLR = value;
    }
    else
    {
         cmu->ISIRQ_CLR1 = value;
    }

#else
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA_MSK_CLR;
        }
    } else {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_MSK_CLR;
        } else {
            value = CMU_MCU2BT_DATA1_MSK_CLR;
        }
    }

    cmu->ISIRQ_CLR = value;
#endif
    return 0;
}
#endif

//static void hal_intersys_wait_done_idle(void)
//{
//    while(hal_cmu_get_address()->ISIRQ_SET & (CMU_BT2MCU_DATA_DONE_SET |CMU_BT2MCU_DATA1_DONE_SET));
//}


//static void hal_intersys_wait_data_idle(void)
//{
//    while(hal_cmu_get_address()->ISIRQ_SET & (CMU_MCU2BT_DATA_IND_SET |CMU_MCU2BT_DATA1_IND_SET));
//}

debug_intersys_type g_debug_intersys;

static int hal_intersys_irq_active(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_IRQ_TYPE_T type)
{
#if (CHIP_INTERSYS_VER <= 1)
    IRQn_Type irq;

    if (type == HAL_INTERSYS_IRQ_SEND_IND) {
        irq = rx_irq_id[id];
    } else {
        irq = tx_irq_id[id];
    }

    return NVIC_GetActive(irq);
#else
    uint32_t value;

#if defined(__FPGA_1501P__)
    value = 0;

    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA_INTR_MSK;
        }
    } else if(id == HAL_INTERSYS_ID_1){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA1_INTR_MSK;
        }
    }else if(id == HAL_INTERSYS_ID_2){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA2_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA2_INTR_MSK;
        }
    }else if(id == HAL_INTERSYS_ID_3){
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA3_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA3_INTR_MSK;
        }
    }
    if(id<HAL_INTERSYS_ID_2)
    {
        return !!(cmu->ISIRQ_SET & value);
    }
    else
    {
        return !!(cmu->ISIRQ_SET1 & value);
    }
#else
    if (id == HAL_INTERSYS_ID_0) {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA_INTR_MSK;
        }
    } else {
        if (type == HAL_INTERSYS_IRQ_SEND_IND) {
            value = CMU_BT2MCU_DATA1_INTR_MSK;
        } else {
            value = CMU_MCU2BT_DATA1_INTR_MSK;
        }
    }

    return !!(cmu->ISIRQ_SET & value);
#endif
#endif
}

static void hal_intersys_rx_irq_handler(void)
{
    int id;
    struct HAL_INTERSYS_MSG_T *msg_ptr;
    enum HAL_INTERSYS_MSG_TYPE_T type;
    unsigned int processed;

    if (g_debug_intersys.cmd_opcode != 0xFFFF){
        g_debug_intersys.irq_happen += 1;
    }

    for (id = HAL_INTERSYS_ID_0; id < HAL_INTERSYS_ID_QTY; id++) {
        if (hal_intersys_irq_active(id, HAL_INTERSYS_IRQ_SEND_IND)) {
#if (CHIP_INTERSYS_VER <= 1)
            hal_intersys_local_irq_clear(id, HAL_INTERSYS_IRQ_SEND_IND);
#else
            hal_intersys_local_irq_mask_clear(id, HAL_INTERSYS_IRQ_SEND_IND);
#endif

            if (recv_pending_head[id].data) {
                // Previous unprocessed message
                msg_ptr = &recv_pending_head[id];
            } else {
                // New message
                msg_ptr = *recv_msg_list_p[id];
            }
            while (msg_ptr) {
                type = msg_ptr->type;
                if (type >= HAL_INTERSYS_MSG_TYPE_QTY) {
                    // Error
                    ASSERT(false, "INTERSYS-RX: Invalid msg type: %d", type);
                    break;
                }
                if (rx_irq_handler[id][type]) {
                    processed = rx_irq_handler[id][type](msg_ptr->data, msg_ptr->len);
                    // Check if flow control needed
                    if (processed < msg_ptr->len) {
                        recv_pending_head[id].next = msg_ptr->next;
                        recv_pending_head[id].type = msg_ptr->type;
                        recv_pending_head[id].len = msg_ptr->len - processed;
                        recv_pending_head[id].data = msg_ptr->data + processed;
                        break;
                    }
                } else {
                    // Error
                    TRACE(1,"INTERSYS-RX current id=%x",id);
                    TRACE(1,"INTERSYS-RX current type=%x",type);
                    ASSERT(false, "INTERSYS-RX: Handler missing");
                    break;
                }
                msg_ptr = msg_ptr->next;
            }

            if (msg_ptr == NULL) {
                if (!need_flow_ctrl[id]){
                    hal_intersys_rx_done(id);
                }
                recv_pending_head[id].data = NULL;
            }
        }
    }
}

static void hal_intersys_tx_irq_handler(void)
{
    int id;
    struct HAL_INTERSYS_MSG_T *msg_ptr;
    enum HAL_INTERSYS_MSG_TYPE_T type;

    for (id = HAL_INTERSYS_ID_0; id < HAL_INTERSYS_ID_QTY; id++) {
        if (hal_intersys_irq_active(id, HAL_INTERSYS_IRQ_RECV_DONE)) {
            hal_intersys_local_irq_clear(id, HAL_INTERSYS_IRQ_RECV_DONE);

            msg_ptr = send_msg_list_p[id];
            while (msg_ptr) {
                type = msg_ptr->type;
                if (type >= HAL_INTERSYS_MSG_TYPE_QTY) {
                    // Error
                    ASSERT(false, "INTERSYS-TX: Invalid msg type: %d", type);
                    break;
                }
                if (tx_irq_handler[id][type]) {
                    tx_irq_handler[id][type](msg_ptr->data, msg_ptr->len);
                };
                CONTAINER_OF(msg_ptr, struct HAL_INTERSYS_SEND_RECORD_T, msg)->in_use = false;
                msg_ptr = msg_ptr->next;
            }

            if (send_pending_list_p[id]) {
                send_msg_list_p[id] = send_pending_list_p[id];
                send_pending_list_p[id] = NULL;
                hal_intersys_peer_irq_set(id, HAL_INTERSYS_IRQ_SEND_IND);
            } else {
                send_msg_list_p[id] = NULL;
                // Allow sleep
                hal_intersys_busy(id, false);
            }
        }
    }
}

#ifdef CORE_SLEEP_POWER_DOWN
#if (CHIP_INTERSYS_VER >= 2)
static int hal_intersys_pm_notif_handler(enum HAL_PM_STATE_T state)
{
    if (state == HAL_PM_STATE_POWER_DOWN_WAKEUP) {
        for (int i = 0; i < HAL_INTERSYS_ID_QTY; i++) {
            if (chan_opened[i]) {
                hal_intersys_local_irq_mask_set(i, HAL_INTERSYS_IRQ_SEND_IND);
                hal_intersys_local_irq_mask_set(i, HAL_INTERSYS_IRQ_RECV_DONE);
            }
        }
    }

    return 0;
}
#endif
#endif

int hal_intersys_open(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_MSG_TYPE_T type,
                      HAL_INTERSYS_RX_IRQ_HANDLER rxhandler, HAL_INTERSYS_TX_IRQ_HANDLER txhandler, bool rx_flowctrl)
{
    int i;

    if (id >= HAL_INTERSYS_ID_QTY) {
        return 1;
    }
    if (type >= HAL_INTERSYS_MSG_TYPE_QTY) {
        return 2;
    }

    if (chan_opened[id] == 0) {
        // Stop IRQs by default
        NVIC_DisableIRQ(rx_irq_id[id]);
        NVIC_DisableIRQ(tx_irq_id[id]);

        NVIC_SetVector(rx_irq_id[id], (uint32_t)hal_intersys_rx_irq_handler);
        NVIC_SetPriority(rx_irq_id[id], IRQ_PRIORITY_NORMAL);

        NVIC_SetVector(tx_irq_id[id], (uint32_t)hal_intersys_tx_irq_handler);
        NVIC_SetPriority(tx_irq_id[id], IRQ_PRIORITY_NORMAL);

#if (CHIP_INTERSYS_VER >= 2)
        hal_intersys_local_irq_mask_set(id, HAL_INTERSYS_IRQ_SEND_IND);
        hal_intersys_local_irq_mask_set(id, HAL_INTERSYS_IRQ_RECV_DONE);
#endif
#if defined(__FPGA_1501P__)
        if (id == HAL_INTERSYS_ID_0) {
            cmu->ISIRQ_CLR = CMU_BT2MCU_DATA_DONE_CLR | CMU_MCU2BT_DATA_IND_CLR;
        } else if(id == HAL_INTERSYS_ID_1){
            cmu->ISIRQ_CLR = CMU_BT2MCU_DATA1_DONE_CLR | CMU_MCU2BT_DATA1_IND_CLR;
        } else if(id == HAL_INTERSYS_ID_2){
            cmu->ISIRQ_CLR1 = CMU_BT2MCU_DATA2_DONE_CLR | CMU_MCU2BT_DATA2_IND_CLR;
        } else if(id == HAL_INTERSYS_ID_3){
            cmu->ISIRQ_CLR1 = CMU_BT2MCU_DATA3_DONE_CLR | CMU_MCU2BT_DATA3_IND_CLR;
        }

#else
        if (id == HAL_INTERSYS_ID_0) {
            cmu->ISIRQ_CLR = CMU_BT2MCU_DATA_DONE_CLR | CMU_MCU2BT_DATA_IND_CLR;
        } else {
            cmu->ISIRQ_CLR = CMU_BT2MCU_DATA1_DONE_CLR | CMU_MCU2BT_DATA1_IND_CLR;
        }
#endif
        *bt_recv_msg_list_ppp[id] = &send_msg_list_p[id];

        send_msg_list_p[id] = NULL;
        send_pending_list_p[id] = NULL;
        recv_pending_head[id].data = NULL;
        for (i = 0; i < MAX_SEND_RECORD_COUNT; i++) {
            send_msgs[id][i].in_use = false;
        }
        need_flow_ctrl[id] = rx_flowctrl;

        // Enable TX IRQ
        NVIC_ClearPendingIRQ(tx_irq_id[id]);
        NVIC_EnableIRQ(tx_irq_id[id]);
    } else {
        ASSERT(need_flow_ctrl[id] == rx_flowctrl, "INTERSYS-OPEN: rx_flowctrl=%d (should be %d)", rx_flowctrl, need_flow_ctrl[id]);
        return 3;
    }

#ifdef CORE_SLEEP_POWER_DOWN
#if (CHIP_INTERSYS_VER >= 2)
    for (i = 0; i < HAL_INTERSYS_ID_QTY; i++) {
        if (chan_opened[i]) {
            break;
        }
    }
    if (i >= HAL_INTERSYS_ID_QTY) {
        hal_pm_notif_register(HAL_PM_USER_HAL, hal_intersys_pm_notif_handler);
    }
#endif
#endif

    chan_opened[id] |= (1 << type);

    rx_irq_handler[id][type] = rxhandler;
    tx_irq_handler[id][type] = txhandler;

    return 0;
}

int hal_intersys_close(enum HAL_INTERSYS_ID_T id,enum HAL_INTERSYS_MSG_TYPE_T type)
{
    if (id >= HAL_INTERSYS_ID_QTY) {
        return 1;
    }

    chan_opened[id] &= ~(1 << type);
    rx_irq_handler[id][type] = NULL;
    tx_irq_handler[id][type] = NULL;

    if (chan_opened[id] == 0) {
        // Stop IRQs by default
        NVIC_DisableIRQ(rx_irq_id[id]);
        NVIC_DisableIRQ(tx_irq_id[id]);
#if (CHIP_INTERSYS_VER >= 2)
        hal_intersys_local_irq_mask_clear(id, HAL_INTERSYS_IRQ_SEND_IND);
        hal_intersys_local_irq_mask_clear(id, HAL_INTERSYS_IRQ_RECV_DONE);
#endif

        send_msg_list_p[id] = NULL;
        send_pending_list_p[id] = NULL;
        recv_pending_head[id].data = NULL;
        need_flow_ctrl[id] = false;

#ifdef CORE_SLEEP_POWER_DOWN
#if (CHIP_INTERSYS_VER >= 2)
        int i;

        for (i = 0; i < HAL_INTERSYS_ID_QTY; i++) {
            if (chan_opened[i]) {
                break;
            }
        }
        if (i >= HAL_INTERSYS_ID_QTY) {
            hal_pm_notif_deregister(HAL_PM_USER_HAL, hal_intersys_pm_notif_handler);
        }
#endif
#endif
    }

    return 0;
}

int hal_intersys_start_recv(enum HAL_INTERSYS_ID_T id)
{
    if (id >= HAL_INTERSYS_ID_QTY) {
        return 1;
    }

    NVIC_ClearPendingIRQ(rx_irq_id[id]);
    NVIC_EnableIRQ(rx_irq_id[id]);

    // Check if there is any previous unprocessed message
    if (recv_pending_head[id].data) {
#if (CHIP_INTERSYS_VER <= 1)
        hal_intersys_local_irq_set(id, HAL_INTERSYS_IRQ_SEND_IND);
#else
        hal_intersys_local_irq_mask_set(id, HAL_INTERSYS_IRQ_SEND_IND);
#endif
    }

    return 0;
}

int hal_intersys_stop_recv(enum HAL_INTERSYS_ID_T id)
{
    if (id >= HAL_INTERSYS_ID_QTY) {
        return 1;
    }

    NVIC_DisableIRQ(rx_irq_id[id]);

    return 0;
}

#if (defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE)) || \
        (defined(CHIP_HAS_PSRAMUHS) && defined(PSRAMUHS_ENABLE)) ||\
        defined(CHIP_BEST1502X)
#define INTERSYS_BUF_LEN_MAX 1024
SYNC_FLAGS_LOC static uint8_t intersys_buf_sram[HAL_INTERSYS_ID_QTY][MAX_SEND_RECORD_COUNT][INTERSYS_BUF_LEN_MAX];
#endif

int hal_intersys_send(enum HAL_INTERSYS_ID_T id, enum HAL_INTERSYS_MSG_TYPE_T type,
                      const unsigned char *data, unsigned int len)
{
    uint32_t lock;
    int ret;
    struct HAL_INTERSYS_SEND_RECORD_T *record;
    struct HAL_INTERSYS_MSG_T *next;
    int i;
    if (id >= HAL_INTERSYS_ID_QTY) {
        return 1;
    }
    if (type >= HAL_INTERSYS_MSG_TYPE_QTY) {
        return 2;
    }

    if ((chan_opened[id] & (1 << type)) == 0) {
        return 3;
    }

    ret = -1;
    record = &send_msgs[id][0];

    lock = int_lock();
    for (i = 0; i < MAX_SEND_RECORD_COUNT; i++) {
        if (record->in_use) {
            record++;
            continue;
        }

#if (defined(CHIP_HAS_PSRAM) && defined(PSRAM_ENABLE)) || \
        (defined(CHIP_HAS_PSRAMUHS) && defined(PSRAMUHS_ENABLE))
        if (!((uint32_t)data > RAM0_BASE && (uint32_t)data < (RAM0_BASE + RAM_TOTAL_SIZE)) &&
                len <= INTERSYS_BUF_LEN_MAX) {
            memcpy(intersys_buf_sram[id][i], data, len);
            data = (const unsigned char *)intersys_buf_sram[id][i];
        }
#elif defined(CHIP_BEST1502X)
        if(len <= INTERSYS_BUF_LEN_MAX)
        {
            memcpy(intersys_buf_sram[id][i], data, len);
            data = (const unsigned char *)intersys_buf_sram[id][i];
        }
        else
        {
            ASSERT(0,"err hci length");
        }
#endif

        record->in_use = true;
        record->msg.next = NULL;
        record->msg.type = type;
        record->msg.len = len;
        record->msg.data = data;
        if (send_msg_list_p[id] == NULL) {
            send_msg_list_p[id] = &record->msg;
            hal_intersys_peer_irq_set(id, HAL_INTERSYS_IRQ_SEND_IND);
        } else if (send_pending_list_p[id] == NULL) {
            send_pending_list_p[id] = &record->msg;
        } else {
            next = send_pending_list_p[id];
            while (next->next) {
                next = next->next;
            }
            next->next = &record->msg;
        }
        ret = 0;
        // Prohibit sleep here
        hal_intersys_busy(id, true);
        break;
    }
    int_unlock(lock);

    return ret;
}

void hal_intersys_rx_done(enum HAL_INTERSYS_ID_T id)
{
#if (CHIP_INTERSYS_VER <= 1)
    hal_intersys_peer_irq_set(id, HAL_INTERSYS_IRQ_RECV_DONE);
#else
    hal_intersys_local_irq_clear(id, HAL_INTERSYS_IRQ_SEND_IND);
    hal_intersys_local_irq_mask_set(id, HAL_INTERSYS_IRQ_SEND_IND);
#endif
}

#ifdef TX_RX_PCM_MASK
static FRAME2BUFF_HANDLER DecQ;

void hal_intersys_mic_rx_irq()
{
    int id;
    id = HAL_INTERSYS_ID_1;
    if(NVIC_GetActive(rx_irq_id[id]))
    {
        hal_intersys_local_irq_clear(id, HAL_INTERSYS_IRQ_SEND_IND);
        //TRACE(0,"HAL_INTERSYS_ID_1 CLEAR");
        //output data to buff
        DecQ();
    }
}

int hal_intersys_mic_open(enum HAL_INTERSYS_ID_T id, FRAME2BUFF_HANDLER dch)
{
   // if(btdrv_is_pcm_mask_enable()==1)
    {
        DecQ= dch;
        NVIC_EnableIRQ(rx_irq_id[id]);
        if(id == HAL_INTERSYS_ID_1)
        {
            cmu->ISIRQ_CLR = CMU_BT2MCU_DATA1_DONE_CLR | CMU_MCU2BT_DATA1_IND_CLR;
            NVIC_SetVector(rx_irq_id[id], (uint32_t)hal_intersys_mic_rx_irq);
            NVIC_SetPriority(rx_irq_id[id], IRQ_PRIORITY_NORMAL);
        }
    }
    return 0;
}
#endif

#ifdef SIMULTANEOUS_AWAKEN
static WAKEUP_HANDLER WakeUp;

void hal_intersys_wakeup_rx_irq()
{
    int id;
    id = HAL_INTERSYS_ID_1;
    if(NVIC_GetActive(rx_irq_id[id]))
    {
        hal_cmu_bt_sys_clock_force_on();
        hal_intersys_local_irq_clear(id, HAL_INTERSYS_IRQ_SEND_IND);
        WakeUp();
        hal_cmu_bt_sys_clock_auto();
    }
}

int hal_intersys_wakeup_open(enum HAL_INTERSYS_ID_T id, WAKEUP_HANDLER dch)
{
    WakeUp= dch;
    NVIC_EnableIRQ(rx_irq_id[id]);
    if(id == HAL_INTERSYS_ID_1)
    {
        cmu->ISIRQ_CLR = CMU_BT2MCU_DATA1_DONE_CLR | CMU_MCU2BT_DATA1_IND_CLR;
        NVIC_SetVector(rx_irq_id[id], (uint32_t)hal_intersys_wakeup_rx_irq);
        NVIC_SetPriority(rx_irq_id[id], IRQ_PRIORITY_NORMAL);
        hal_intersys_local_irq_mask_set(id, HAL_INTERSYS_IRQ_SEND_IND);
    }
    return 0;
}
#endif

void hal_intersys_wakeup_btcore(void)
{
    hal_intersys_peer_irq_set(HAL_INTERSYS_ID_1, HAL_INTERSYS_IRQ_SEND_IND);
}

bool hal_intersys_get_wakeup_flag(void)
{
    if(cmu->ISIRQ_SET & CMU_MCU2BT_DATA1_IND_SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void hal_intersys_dump_register(void)
{
    uint32_t cmu_irq_set = cmu->ISIRQ_SET;
    uint32_t cmu_irq_clr = cmu->ISIRQ_CLR;
    uint32_t bt_cmu_irq_set = btcmu->ISIRQ_SET;
    uint32_t bt_cmu_irq_clr = btcmu->ISIRQ_CLR;

    TRACE(2,"CMU addr=%p [set]:0x%x,[clr]:0x%x", cmu, cmu_irq_set, cmu_irq_clr);
    TRACE(2,"BT CMU addr=%p [set]:0x%x,[clr]:0x%x", btcmu, bt_cmu_irq_set, bt_cmu_irq_clr);
}
#endif // BT_CMU_BASE
