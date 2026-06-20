#ifndef __LCDC_LP_CMD_H
#define __LCDC_LP_CMD_H

#ifdef cpluscplus
extern "C" {
#endif
#include <stdint.h>

#define LPCMDS_MAX_PARAMS_NUM 4
#define LPCMDS_MAX_CMDS_NUM 20
#define LPCMDS_MAX_GROUP_NUM 30

#define LPCMDS_NONBLOCK 0
#define LPCMDS_BLOCK    1

enum lpcmd_type_e
{
    LPCMD_TYPE_WRITE,
    LPCMD_TYPE_READ,
};

typedef struct lpcmd_node_s
{
  uint8_t cmd;
  uint8_t count;
  int optype;
  uint8_t params[LPCMDS_MAX_PARAMS_NUM];
} lpcmd_node_t;

/**
 * @brief lpcmds_register_cmds - register cmds to cache, will send at framedone irq
 *
 * @param cmds reference struct lpcmd_node_s *
 * @param nums count
 * @return int success 0; failed -1;
 */
int lpcmds_register_cmds(void *cmds, int nums, int flag);

/**
 * @brief lpcmds_flush - send all cmds at cache
 *
 * @return int - success 0; failed -1;
 */
int lpcmds_flush(void);

#ifdef cplusplus
}
#endif

#endif /* __LCDC_LP_CMD_H */