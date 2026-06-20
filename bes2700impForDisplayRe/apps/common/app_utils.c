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
#include "cmsis.h"
#include "hal_trace.h"
#include "app_utils.h"

#define FREQ_FREE    0UL
#define FREQ_26M     26UL
#define FREQ_52M     52UL
#define FREQ_78M     78UL
#define FREQ_104M    104UL
#ifdef HAL_CMU_FREQ_15M
#define FREQ_15M     15UL
#define FREQ_30M     30UL
#define FREQ_120M    120UL
#define FREQ_144M    144UL
#define FREQ_168M    168UL
#endif
#define FREQ_208M    208UL

/*
 * qos_users, quality of services users, this kind of user must run with the
 * minist frequency they requiested, if there are more users, the frequency
 * should be boost for these users
 * e.g. the key word recorgnithion function request 26M Hz, and the music
 * also need 26M, if both of them are running, the cpu freq should boost to
 * 26M + 26M = 52M Hz
 */
/*
 * NOTE:
 * The macro QOS_USERS works only when the APP_SYSFREQ_USER_APP_XXX is not large than
 * 32, currently this works, but if the are more user, another way needed
 */
#define QOS_USERS  ((1 << (APP_SYSFREQ_USER_AI_VOICE))      | \
                    (1 << (APP_SYSFREQ_USER_BT_A2DP))       | \
                    (1 << (APP_SYSFREQ_USER_BT_SCO))        | \
                    (1 << (APP_SYSFREQ_USER_VOICE_ASSIST))  | \
                    (1 << (APP_SYSFREQ_USER_AOB_PLAYBACK))  | \
                    (1 << (APP_SYSFREQ_USER_AOB_CAPTURE)))

static const uint32_t freq_map[] =  {
    [HAL_CMU_FREQ_32K] = FREQ_FREE,
    [HAL_CMU_FREQ_26M] = FREQ_26M,
    [HAL_CMU_FREQ_52M] = FREQ_52M,
    [HAL_CMU_FREQ_78M] = FREQ_78M,
    [HAL_CMU_FREQ_104M] = FREQ_104M,
#ifdef HAL_CMU_FREQ_15M
    [HAL_CMU_FREQ_15M] = FREQ_15M,
    [HAL_CMU_FREQ_30M] = FREQ_30M,
    [HAL_CMU_FREQ_120M] = FREQ_120M,
    [HAL_CMU_FREQ_144M] = FREQ_144M,
    [HAL_CMU_FREQ_168M] = FREQ_168M,
#endif
    [HAL_CMU_FREQ_208M] = FREQ_208M,
};

static const uint32_t user_map[] =  {
    [0] = APP_SYSFREQ_USER_AI_VOICE,
    [1] = APP_SYSFREQ_USER_BT_A2DP,
    [2] = APP_SYSFREQ_USER_BT_SCO,
    [3] = APP_SYSFREQ_USER_VOICE_ASSIST,
    [4] = APP_SYSFREQ_USER_AOB_PLAYBACK,
    [5] = APP_SYSFREQ_USER_AOB_CAPTURE,
};
/*
 * qos_freqs_map
 * filled with user's freq, one user's freq occupy 5bits,
 * that limit the frequecy number is not large than 64
 *
 * bit field structure:
  -----------------------------------------------------
  | user11 |..................|user1  | user 0|
  -----------------------------------------------------
  | 59~55  |..................|9~5    | 4~0   |
  -----------------------------------------------------
 *
 * Ok, this is so ugly, but there is not so much frequecy level,
 * and maybe work for a long time
 */
static uint64_t qos_freqs_map;

/*
 * qos_users_map
 * filled with user's number, one user's occupy 1 bit,
 *
 bit field structure:
  -----------------------------------------------------
  |user31~12(reseverd)| user11 |........| user 1| user 0|
  -----------------------------------------------------
 *
 */
static uint32_t qos_users_map;

static int app_qosfreq_req(enum APP_SYSFREQ_USER_T user, enum APP_SYSFREQ_FREQ_T freq)
{
    int ret;
    int qos_freq_num = 0;
    uint32_t max_qos_freq = 0;
    int user_idx;
    int i;
    uint32_t lock;

    if (freq >= APP_SYSFREQ_FREQ_QTY)
        return -1;

    lock = int_lock();
    for (i = 0; i < ARRAY_SIZE(user_map); i++) {
        if (user == user_map[i]) {
            break;
        }
    }

    if (i >= ARRAY_SIZE(user_map)) {
        int_unlock(lock);
        ASSERT(0, "can not find qos user");
        return 0;
    }

    user_idx = i;
    if ((int)freq != (int)HAL_CMU_FREQ_32K) { // require freq
        qos_freqs_map &= ~(((uint64_t)0x1f) << (5 * i));
        qos_freqs_map |= ((uint64_t)freq) << (5 * i);
        qos_users_map |= 1 << user_idx;
    } else { //release freq
        qos_freqs_map &= ~(((uint64_t)0x1f) << (5 * i));
        qos_users_map &= ~ (1 << user_idx);
    }

    //scan the qos_user_map and sum every user's request freq
    for(i = 0; i < ARRAY_SIZE(user_map); i++) {
        if ((qos_users_map >> i) & 0x1) {
            uint32_t real_freq;
            int freq_num;

            freq_num = (qos_freqs_map >> (5 * i )) & 0x1f;
            real_freq = freq_map[freq_num];
            max_qos_freq += real_freq;
        }
    }

    for (i = 0; i < ARRAY_SIZE(freq_map); i++) {
        if (i) {
            if ((max_qos_freq > freq_map[i-1]) && (max_qos_freq <= freq_map[i])) {
                qos_freq_num  = i;
                break;
            }
        } else {
            if (max_qos_freq == freq_map[i]) {
                qos_freq_num = i;
                break;
            }
        }
    }

    if (i >= ARRAY_SIZE(freq_map)) {
        qos_freq_num = (HAL_CMU_FREQ_QTY - 1);
        TRACE(0, "WARNING: required sysfreq exceed");
        // ASSERT(0, "can not find actual freq");
    }

    user = APP_SYSFREQ_USER_QOS;
    //TRACE(2, "User %d require sysfreq %d", user, qos_freq_num);
    ret = hal_sysfreq_req((enum HAL_SYSFREQ_USER_T)user, (enum HAL_CMU_FREQ_T)qos_freq_num);

    int_unlock(lock);
    return ret;
}

int app_sysfreq_req(enum APP_SYSFREQ_USER_T user, enum APP_SYSFREQ_FREQ_T freq)
{
    int ret;

    // if user is qos user
    if ((1 << user) & QOS_USERS) {
        ret = app_qosfreq_req(user, freq);
    } else { // if user is NOT qos user
        ret = hal_sysfreq_req((enum HAL_SYSFREQ_USER_T)user, (enum HAL_CMU_FREQ_T)freq);
    }

    return ret;
}

uint32_t app_sysfreq_get_value(enum APP_SYSFREQ_FREQ_T freq)
{
    if (freq < ARRAY_SIZE(freq_map))
        return freq_map[freq];
    else
        return 0;
}
