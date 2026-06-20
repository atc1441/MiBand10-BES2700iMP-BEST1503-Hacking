#ifndef __RPC_RX_THREAD_H__
#define __RPC_RX_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t rpc_core_id;
} RPC_RX_MAILBOX_PARAM_T;

int32_t rpc_rx_mailbox_put(RPC_RX_MAILBOX_PARAM_T* msg_src);
int32_t rpc_rx_mailbox_get(RPC_RX_MAILBOX_PARAM_T** msg_p);
int32_t rpc_rx_mailbox_free(RPC_RX_MAILBOX_PARAM_T* msg_p);
int32_t rpc_rx_mailbox_init(void);
void rpc_rx_thread_init(void);

#ifdef __cplusplus
extern "C" {
#endif

#endif