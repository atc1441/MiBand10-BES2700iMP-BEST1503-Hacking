#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "cqueue.h"

#include "hal_sys2bth.h"
#include "app_rpc_api.h"
#include "rpc_queue.h"
#include "rpc_bth_dsp.h"
#include "rpc_rx_thread.h"

#ifndef RPC_RX_MAILBOX_MAX
#define RPC_RX_MAILBOX_MAX          (10)
#endif

osMailQDef (rpc_rx_mailbox, RPC_RX_MAILBOX_MAX, RPC_RX_MAILBOX_PARAM_T);
static osMailQId rpc_rx_mailbox_id = NULL;

static osThreadId rpc_rx_thread_id = NULL;
static void rpc_rx_thread(const void *arg);
osThreadDef(rpc_rx_thread, osPriorityHigh, 1, (4096), "rpc_rx_thread");

int32_t rpc_rx_mailbox_put(RPC_RX_MAILBOX_PARAM_T* msg_src)
{
    RPC_RX_MAILBOX_PARAM_T *msg_p = NULL;
    osStatus status;

    msg_p = (RPC_RX_MAILBOX_PARAM_T*)osMailAlloc(rpc_rx_mailbox_id, 0);
    ASSERT(msg_p, "rpc rx mailbox Alloc error");

    msg_p->rpc_core_id = msg_src->rpc_core_id;
    status = osMailPut(rpc_rx_mailbox_id, msg_p);

    return (int32_t)status;
}

int32_t rpc_rx_mailbox_get(RPC_RX_MAILBOX_PARAM_T** msg_p)
{
    osEvent evt;

    evt = osMailGet(rpc_rx_mailbox_id, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (RPC_RX_MAILBOX_PARAM_T *)evt.value.p;
        return 0;
    }
    return -1;
}

int32_t rpc_rx_mailbox_free(RPC_RX_MAILBOX_PARAM_T* msg_p)
{
    osStatus status;
    status = osMailFree(rpc_rx_mailbox_id, msg_p);
    return (int32_t)status;
}

int32_t rpc_rx_mailbox_init(void)
{
    if (NULL == rpc_rx_mailbox_id)
    {
        rpc_rx_mailbox_id = osMailCreate(osMailQ(rpc_rx_mailbox), NULL);
        if (NULL == rpc_rx_mailbox_id) {
            TRACE(0,"Failed to Create rpc mailbox\n");
            return -1;
        }
    }
    return 0;
}

static void rpc_rx_thread(const void *arg)
{
    RPC_RX_MAILBOX_PARAM_T *msg_p = NULL;

    for(;;) {
        if (!rpc_rx_mailbox_get(&msg_p)) {
            switch (msg_p->rpc_core_id)
            {
            case APP_RPC_CORE_BTH_M55:
                break;
            case APP_RPC_CORE_BTH_DSP:
                rpc_bth_dsp_rx_queue_data_process_handler();
                break;
            case APP_RPC_CORE_DSP_M55:
                break;
            default:
                ASSERT(false, "Wrong rpc core id 0x%x", msg_p->rpc_core_id);
                break;
            }
            rpc_rx_mailbox_free(msg_p);
        }
    }
}

void rpc_rx_thread_init(void)
{
    rpc_rx_mailbox_init();
    TRACE(0, "%s", __func__);
    if (rpc_rx_thread_id == NULL) {
        rpc_rx_thread_id = osThreadCreate(osThread(rpc_rx_thread), NULL);
        ASSERT(rpc_rx_thread_id, "creatte rx thread fail!");
    }
}