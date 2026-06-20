#ifndef __RPC_PING_H__
#define __RPC_PING_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief      helper backend function for ping command
 * @param[in]  handle, the ping command's handle
 * @param[in]  in_args, the input arguments for ping command
 * @param[in]  complete, the callback function after execute the ping command;
 *             this is not need define by user, call back from rpc layer
 * @return     0, success,
 *             others, fail
 *
 * NOTE:       the ping command should be register as part of normal
 *             user backend commands;
 */
int rpc_ping_back(enum RPC_CORE_T core, uint32_t handle,
                    struct rpc_args *in_args, rpc_backend_complete complete);

/**
 * @brief help function for ping the remote is register all its backend functions
 * @param[in]     try_times, the times for ping, every time is 1 second;
 * @return        RPC_OK, success,
 *                others, fail
 *
 */
rpc_status_t rpc_ping_remote(enum RPC_CORE_T core, int try_times);

#ifdef __cplusplus
}
#endif

#endif /* __RPC_PING_H__ */
