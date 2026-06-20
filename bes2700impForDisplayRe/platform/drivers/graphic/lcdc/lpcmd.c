#include <stdio.h>
#include "string.h"
#include "lpcmd.h"
#include "hal_dsi.h"
#include "hal_trace.h"
#include "local_sem.h"
#include "plat_types.h"
#include "cmsis.h"
#ifdef __NuttX__
#include <arch/board/board.h>
#include <debug.h>
#include <semaphore.h>
#include <nuttx/arch.h>
#else
#include "fb.h"
#ifdef RTOS
#include "semaphore.h"
#endif
#endif

#define LCDC_WARN(...) TR_WARN(TR_MOD(TEST), "//" __VA_ARGS__)
#define LCDC_ERR(...) TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)
#define LCDC_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)

typedef struct lpcmd_cmds_s
{
  uint8_t cmd;
  uint8_t count;
  int optype;
  uint8_t* pbuf;
  uint8_t params[LPCMDS_MAX_PARAMS_NUM];
} lpcmd_cmds_t;

struct lpcmd_group_s {
    int count;
    int flag;
    sem_t sem;
    lpcmd_cmds_t cmds[LPCMDS_MAX_CMDS_NUM];
};

struct lpcmd_s {
    sem_t* lock;
    volatile int groups;
    struct lpcmd_group_s gcmds[LPCMDS_MAX_GROUP_NUM];
};

static sem_t g_sem;

static int g_initalize = 0;

static struct lpcmd_s lpcmds = {
    .lock = &g_sem,
    .groups = 0,
};

static struct lpcmd_s* lpcmds_getdev(void)
{
    if (!g_initalize) {
        memset(&lpcmds.gcmds, 0, sizeof(lpcmds.gcmds));

        lcdc_sem_init(lpcmds.lock, 0, 1);

        for (int i = 0; i < LPCMDS_MAX_GROUP_NUM; i++) {
            lcdc_sem_init(&lpcmds.gcmds[i].sem, 0, 0);
        }

        g_initalize = 1;
    }

    return &lpcmds;
}

/**
 * @brief lpcmds_register_cmds - register cmds to cache, will send at framedone
 * irq
 *
 * @param cmds reference struct lpcmd_node_s *
 * @param nums count
 * @return int
 */
int lpcmds_register_cmds(void* cmds, int nums, int flag)
{
    struct lpcmd_node_s* pcmds = cmds;
    int i;
    struct lpcmd_s* priv = lpcmds_getdev();

    if (!cmds || nums <= 0) {
        return -1;
    }

    /* lock */

    lcdc_sem_wait(priv->lock);
    /* too many params, support max num LPCMDS_MAX_PARAMS_NUM, default 4  */

    if (pcmds->count > LPCMDS_MAX_PARAMS_NUM) {
        LCDC_ERR("lpcmd mode register lpcmds failed, cmds too many params.");
        goto _exit;
    }

    /* busy */

    if (priv->groups >= LPCMDS_MAX_GROUP_NUM) {
        LCDC_ERR("lpcmd mode register failed, no memory to store new cmds.");
        goto _exit;
    }

    i = priv->groups;
    priv->gcmds[i].flag = flag;
    if (flag == LPCMDS_BLOCK) {
        lcdc_sem_reset(&priv->gcmds[i].sem, 0);
    }

    priv->groups++;
    for (int j = 0; j< nums; j++) {

        priv->gcmds[i].cmds[j].cmd = pcmds[j].cmd;
        priv->gcmds[i].cmds[j].count = pcmds[j].count;
        priv->gcmds[i].cmds[j].optype = pcmds[j].optype;
        if (pcmds[j].optype == LPCMD_TYPE_READ && flag == LPCMDS_NONBLOCK) {
            priv->gcmds[i].cmds[j].pbuf = NULL;
        }
        else {
            priv->gcmds[i].cmds[j].pbuf = pcmds[j].params;
        }

        for (int k = 0; k < pcmds[j].count; k++) {
            priv->gcmds[i].cmds[j].params[k] = pcmds[j].params[k];
        }
    }

    priv->gcmds[i].count = nums;

    /* unlock */
    lcdc_sem_post(priv->lock);

    if (flag == LPCMDS_BLOCK) {
        lcdc_sem_wait(&priv->gcmds[i].sem);
    }

    return 0;

_exit:
    /* unlock */
    lcdc_sem_post(priv->lock);

    return -1;
}

/**
 * @brief lpcmds_flush - send all cmds at cache
 *
 * @return int - success 0; failed -1;
 */
int lpcmds_flush(void)
{
    struct lpcmd_s* priv = lpcmds_getdev();
    struct lpcmd_group_s* gcmds;
    int ret = 0;
    int i;
    static int count = 0;

    /* lock */
    if (in_isr()) {
        lcdc_sem_get_value(priv->lock, &ret);
        if (ret <= 0)
            return -1;
    } else {
        ret = lcdc_sem_trywait(priv->lock);
        if (ret != OK)
            return -1;
    }

    for (i = 0; i < priv->groups; i++) {
        gcmds = &priv->gcmds[i];
        hal_dsi_reset(BOARD_LCDC_WIDTH);
        for (int j = 0; j < gcmds->count; j++) {

            switch (gcmds->cmds[j].optype) {
            case LPCMD_TYPE_WRITE:
                hal_dsi_send_cmd_list(gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                                      gcmds->cmds[j].params);
                LCDC_INFO("count = %d, cmd write cmd = 0x%02x, count = %d, "
                          "param = 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", count,
                          gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                          gcmds->cmds[j].params[0], gcmds->cmds[j].params[1],
                          gcmds->cmds[j].params[2], gcmds->cmds[j].params[3]);
                break;
            case LPCMD_TYPE_READ:
                if (!gcmds->cmds[j].pbuf) {
                    LCDC_WARN("cmd read cmd = 0x%02x, count = %d, "
                            "read buffer is NULL.\n",
                            gcmds->cmds[j].cmd, gcmds->cmds[j].count);
                    break;
                }

                hal_dsi_read_cmd(gcmds->cmds[j].cmd, gcmds->cmds[j].pbuf,
                                 gcmds->cmds[j].count);
                LCDC_INFO("count = %d, cmd read cmd = 0x%02x, count = %d, "
                          "param = 0x%02x, 0x%02x\n", count,
                          gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                          gcmds->cmds[j].pbuf[0], gcmds->cmds[j].pbuf[1]);
                break;
            default:
                LCDC_WARN("cmd optype error, type = %d, "
                          "cmd = 0x%02x, count = %d, "
                          "param = 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                          gcmds->cmds[j].optype, gcmds->cmds[j].cmd,
                          gcmds->cmds[j].count, gcmds->cmds[j].params[0],
                          gcmds->cmds[j].params[1], gcmds->cmds[j].params[2],
                          gcmds->cmds[j].params[3]);
                break;
            }
            count++;
        }

        gcmds->count = 0;

        if (gcmds->flag == LPCMDS_BLOCK) {
            /* notify wait tasks */
            lcdc_sem_post(&gcmds->sem);
        }
    }

    priv->groups = 0;

    /* unlock */
    lcdc_sem_post(priv->lock);

    return 0;
}
