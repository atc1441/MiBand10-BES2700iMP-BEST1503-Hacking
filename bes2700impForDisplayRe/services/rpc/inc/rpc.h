#ifndef __RPC_H__
#define __RPC_H__

#ifdef __cplusplus
extern "C" {
#endif

enum RPC_CORE_T {
    RPC_CORE_BTH_M55,
    RPC_CORE_BTH_DSP,
    RPC_CORE_BTH_SESHUB,
    RPC_CORE_BTH_BTC,

    RPC_CORE_QTY,
};

typedef enum RPC_STATUS_T {
    RPC_OK = 0,
    RPC_CMD_ERR,
    RPC_CMD_ARG_ERR,
    RPC_TIMEOUT,
    RPC_NO_DEV,
    RPC_NO_MEM,
    RPC_NO_CONNECT,
    RPC_RMT_HUNG,

    RPC_QTY,
} rpc_status_t;

#define RPC_WAIT_FOREVER        (-1U)
#define INVALID_RPC_HANDLE      (-1U)

struct rpc_args {
    uint32_t args_len;
    void    *args;
};

typedef int (*rpc_frontend_complete)(uint32_t rpc, void *out_args);

typedef int (*rpc_backend_complete)(int32_t core, uint32_t handle, int cmd_rsp,
                                        struct rpc_args *out_args);

typedef int (*rpc_backend_handler)(uint32_t handle, struct rpc_args *in_args,
                                                    rpc_backend_complete fn);

struct rpc_backend_cmd {
    const char   *cmd_name;
    rpc_backend_handler handler;
};

/**
 * @brief open rpc
 * @param[in]     core, rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_open_frontend(enum RPC_CORE_T core);

/**
 * @brief close rpc
 * @param[in]     core, rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_close(enum RPC_CORE_T core);

/**
 * @brief register commands for rpc
 * @param[in]     core, rpc core
 * @param[in]     cmds, commands for register
 * @param[in]     cmds_num, commands number
 * @return        RPC_OK, success,
 *                others, fail
 *
 * NOTE: the command's name should be not large than 16,
 *       which currently controlled by RPC_CMD_NAME_LEN;
 */
/* rpc_status_t rpc_register_cmd(enum RPC_CORE_T core, */
/*                             const struct rpc_cli_cmd *cmds, uint32_t cmds_num); */

/**
 * @brief unregister a command for rpc
 * @param[in]     core, rpc core
 * @param[in]     handle, the command's handle
 * @return        RPC_OK, success,
 *                others, fail
 */
/* rpc_status_t rpc_unregister_cmd(uint32_t handle); */

/**
 * @brief         send a commond to remote cpu for executing synchronously
 * @param[in]     core, rpc core
 * @param[in]     handle, the command's handle
 * @param[in]     in_args: the input arguments
 * @param[in]     timeout:  the timeout(ms) value for the remote cpu response,
 *                          osWaitForever for wait for ever
 * @param[out]    out_args: the output arguments received from remote cpu
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_exec_cmd_sync(enum RPC_CORE_T core, const char *cmd_name,
                struct rpc_args *in_args, struct rpc_args *out_args, uint32_t timeout);

/**
 * @brief         send a commond to remote cpu asynchronously
 * @param[in]     core, rpc core
 * @param[in]     handle, the command's handle
 * @param[in]     in_args: the input arguments
 * @param[in]     complete: after the command issue, call back this function when
 *                          there is response
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_exec_cmd_async(enum RPC_CORE_T core, const char *cmd_name,
                        struct rpc_args *in_args, rpc_frontend_complete complete);

/**
 * @brief         send a commond to remote, and dont need any response
 * @param[in]     core, rpc core
 * @param[in]     cmd_name, rpc commnad's name
 * @param[in]     in_args: the input arguments
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_notify_cmd(enum RPC_CORE_T core, const char *cmd_name,
                                                struct rpc_args *in_args);

/**
 * @brief open server side rpc
 * @param[in]     rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_open_backend(enum RPC_CORE_T core);

/**
 * @brief  close server side rpc
 * @param[in]     core, rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_close_backend(enum RPC_CORE_T core);

/**
 * @brief register a command for rpc server
 * @param[in]     core, the rpc core
 * @param[in]     cmds, the array of the server side commands
 * @param[in]     cmds_num, the number of commands
 * @return        RPC_OK, success,
 *                others, fail
 * NOTE: the command's name should be not large than 16,
 *       which currently controlled by RPC_CMD_NAME_LEN;
 */
rpc_status_t rpc_register_backend_cmd(enum RPC_CORE_T core,
                    const struct rpc_backend_cmd *cmds, uint32_t cmds_num);

/**
 * @brief unregister a command for rpc server
 * @param[in]     rpc core
 * @param[in]     cmd_name, string for command
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_unregister_backend_cmd(enum RPC_CORE_T core,
                                        const char cmd_name[]);

#if !defined(RTOS)
void rpc_start_loop(enum RPC_CORE_T core);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __RPC_H__ */
