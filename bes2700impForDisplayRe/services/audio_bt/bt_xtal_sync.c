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
#include "plat_types.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "bt_drv_interface.h"
#include "bt_xtal_sync.h"
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "me_api.h"
#include "btm_i.h"
#include "a2dp_api.h"
#include "btapp.h"

//#define FIXED_XTAL_TUNE_FACTOR

// TODO: Bit offset might be invalid when adjusting xtal too much ???
#ifdef CHIP_BEST1000_BEFORE_VER_G
#define FIXED_BIT_OFFSET_TARGET
#endif

#if defined(IBRT)
#ifndef FIXED_BIT_OFFSET_TARGET
#define FIXED_BIT_OFFSET_TARGET
#endif
#define TARGET_TO_INIT_BIT_OFFSET
#define TWS_MASTER_USE_AFC
//#define TWS_SYNC_SYNC_1_STEP
#endif

//#define BTDRV_XTAL_FCAP_DIRECT_UPDATE

#define SYNC_TRACE(str, ...)    //TRACE(str, ##__VA_ARGS__)

#define BIT_OFFSET_BIT_WIDTH    10
#define BT_TICK_BIT_WIDTH       22

#define BT_TICK_HZ              1600
#define BT_TICK_MASK            ((1 << BT_TICK_BIT_WIDTH) - 1)

#define DIFF_CNT                3
#define MAX_BIT_OFFSET          624
#define BAD_BIT_OFFSET          (-1)

// Max valid consecutive bit offset diff
// In theory the max possible diff in 100ms is (((MAX_VALID_PPB + IN_RANGE_PPB) * 100 + 1000 - 1) / 1000) ~= 1
#define MAX_VALID_CONSEC_DIFF   4
#define MAX_CONSEC_DIFF_ERR     5

// NOTE: If the threshold is too small, the calculated ppb value might have a large deviation
#define BIT_OFFSET_SYNC_THRESH  10

#define BIT_OFFSET_RANGE_LOW    144
#define BIT_OFFSET_RANGE_HIGH   288

#define BIT_OFFSET_CUSHION_LOW  30
#define BIT_OFFSET_CUSHION_HIGH 400

#define BIT_OFFSET_TARGET_CUSHION 30

#define BAD_PPB                 ((int)0x80000000)
#define IN_RANGE_PPB            5000 //20000
#define COMMON_PPB_COEFF        4

#if defined(IBRT)
// The max allowed freq offset: 6*2.4=14.4K Hz
#define MAX_VALID_PPB           6000
#else
// The max allowed freq offset: 20*2.4=48K Hz
#define MAX_VALID_PPB           20000
#endif
#define MAX_VALID_PPB_DIFF      1000
#define MAX_PPB_ERR             5

#define STABLE_STATE_PPB        300
#define STABLE_CHECK_COUNT      3
#define MAX_PPB_AFTER_STABLE    2000

#if (MAX_PPB_AFTER_STABLE > MAX_VALID_PPB)
#error "MAX_PPB_AFTER_STABLE should <= MAX_VALID_PPB"
#endif

#define MAX_TUNE_FACTOR         2000
#define MIN_TUNE_FACTOR         20

#define IN_RANGE_TIMEOUT_MS     3000

#define FIX_TARTGET_RXBIT 300

enum IN_RANGE_STATE_T {
    IN_RANGE_NO,
    IN_RANGE_YES_INIT,
    IN_RANGE_YES
};

enum SYNC_STATE_T {
    SYNC_STATE_NULL = 0,
    SYNC_STATE_ENABLED,
    SYNC_STATE_DISABLED
};

union BIT_OFFSET_AND_CLOCK_T {
    struct {
        uint32_t offset : BIT_OFFSET_BIT_WIDTH;
        uint32_t clock  : BT_TICK_BIT_WIDTH;
    };
    uint32_t reg;
};

static uint32_t last_check_offset;
static uint32_t prev_value;
static uint32_t prev_time;
static uint32_t last_check_time;
static uint16_t timer_wrap_count;
static enum SYNC_STATE_T sync_enabled;
static uint8_t prev_valid;
static uint8_t update_factor;
static uint8_t bit_offset_cnt;
static uint32_t xtal_bit_offset;
static uint8_t err_consec_diff;
static uint8_t err_ppb;
static uint8_t stable_count;
static uint8_t stable_state;
static int diff_with_prev;
static int last_valid_ppb;
static int last_tune_ppb;
static int prev_real_ppb;
static int prev_tune_offset;
#ifdef FIXED_BIT_OFFSET_TARGET
static uint8_t fixed_target_enabled;
static int diff_with_target;
static uint16_t target_bit_offset;
static uint8_t target_valid;
static enum IN_RANGE_STATE_T in_range;
static uint32_t target_valid_time;
static uint32_t old_xtal_fcap;
#ifdef TARGET_TO_INIT_BIT_OFFSET
static uint32_t init_bit_offset;
static uint32_t target_to_init_enabled;
#endif
#endif

static int xtal_fcap_min = 20;
static int xtal_fcap_max = 235;
static int fcap_max_val = 0xFF;

static uint32_t tune_factor;
#ifdef FIXED_XTAL_TUNE_FACTOR
#define TRUST_OPT_TYPE                  const
#else
#define TRUST_OPT_TYPE
#endif
static TRUST_OPT_TYPE uint8_t tune_factor_trust = 0;;
#ifdef BT_XTAL_SYNC_NEW_METHOD
int32_t init_rxbit = -1;
int32_t last_rxbit = -1;
int32_t cross_slot = 0;
uint8_t first_offset_lock = 0;
uint8_t check_cnt = 0;
#define BITOFFSET_CROSS_THRESHOLD     50
pid_para_t afc_mobile_para;
pid_para_t afc_tws_para;
#endif
static int bt_tune_xtal(int ppb)
{
    uint32_t old_val, new_val;
    int offset;
    int round;

    if (ppb == 0) {
        return 0;
    }

    if (ppb < 0) {
        round = - (tune_factor / 2);
    } else {
        round = tune_factor / 2;
    }

    offset = (ppb + round) / (int)tune_factor;
    if (offset == 0) {
        SYNC_TRACE(2,"%s: offset=%d\n\n", __func__, offset);
        return 0;
    }

#ifdef TWS_SYNC_SYNC_1_STEP
    if (offset > 1) {
        offset = 1;
    } else if (offset < -1) {
        offset=-1;
    }
#endif

    old_val = btdrv_rf_get_xtal_fcap();
    if (offset < 0) {
        if (old_val > -offset) {
            new_val = old_val + offset;
        } else {
            new_val = 0;
        }
    } else {
        if (old_val + offset < fcap_max_val) {
            new_val = old_val + offset;
        } else {
            new_val = fcap_max_val;
        }
    }

#ifdef FIXED_BIT_OFFSET_TARGET
    // Not to limit the tune ppb if fixed bit offset target is enabled
    if (fixed_target_enabled == 0)
#endif
    {
        uint32_t init_val;
        init_val = btdrv_rf_get_init_xtal_fcap();
        if (ABS((int)(new_val - init_val)) > (btdrv_rf_get_max_xtal_tune_ppb() / tune_factor)) {
            SYNC_TRACE(4,"%s: BAD new_val=%u init_val=%u tune_factor=%u\n\n", __func__, new_val, init_val, tune_factor);
            return 0;
        }
    }

    if (new_val < xtal_fcap_min){
        new_val = xtal_fcap_min;
    }

    if (new_val > xtal_fcap_max){
        new_val = xtal_fcap_max;
    }

    if (new_val != old_val) {
        btdrv_rf_set_xtal_fcap(new_val, true);
    }

    SYNC_TRACE(4,"%s: offset=%d old=%u new=%u\n\n", __func__, offset, old_val, new_val);
    return new_val - old_val;
}

static int bt_xtal_offset_to_ppb(int offset)
{
    return offset * (int)tune_factor;
}

static void bt_reset_xtal_to_default(void)
{
    uint16_t fcap_val;

    err_consec_diff = 0;
    err_ppb = 0;
#ifdef FIXED_BIT_OFFSET_TARGET
    in_range = IN_RANGE_NO;
#endif
    fcap_val = btdrv_rf_get_init_xtal_fcap();
    SYNC_TRACE(2,"%s: Reset xtal to default %u", __func__, fcap_val);
    btdrv_rf_set_xtal_fcap(fcap_val, true);
}

#ifdef FIXED_BIT_OFFSET_TARGET
#ifdef CHIP_BEST1000_BEFORE_VER_G
static void bt_get_tune_range(int *min_ppb, int *max_ppb)
{
    uint32_t fcap;

    fcap = btdrv_rf_get_xtal_fcap();

    *min_ppb = -fcap * tune_factor;
    *max_ppb = (fcap_max_val - fcap) * tune_factor;
}
#endif

static int bit_offset_in_range(uint32_t bit_offset)
{
#ifdef TARGET_TO_INIT_BIT_OFFSET
    if (target_to_init_enabled) {
        uint32_t low, high;

        if (init_bit_offset >= BIT_OFFSET_TARGET_CUSHION) {
            low = init_bit_offset - BIT_OFFSET_TARGET_CUSHION;
        } else {
            low = MAX_BIT_OFFSET + init_bit_offset - BIT_OFFSET_TARGET_CUSHION;
        }

        if (init_bit_offset <= MAX_BIT_OFFSET - BIT_OFFSET_TARGET_CUSHION) {
            high = init_bit_offset + BIT_OFFSET_TARGET_CUSHION;
        } else {
            high = init_bit_offset + BIT_OFFSET_TARGET_CUSHION - MAX_BIT_OFFSET;
        }

        if (low < high) {
            if (bit_offset < low || bit_offset > high) {
                return 1;
            }
        } else {
            if (bit_offset > high && bit_offset < low) {
                return 1;
            }
        }
    } else
#endif
    {
#ifdef CHIP_BEST1000_BEFORE_VER_G
        if (bit_offset > BIT_OFFSET_RANGE_LOW && bit_offset < BIT_OFFSET_RANGE_HIGH) {
            return 1;
        }
#endif
    }
    return 0;
}

static int diff_out_of_range(int cur_to_target)
{
    if (ABS(cur_to_target) <= BIT_OFFSET_SYNC_THRESH / 2 ||
            (diff_with_target > 0 && cur_to_target < 0) ||
            (diff_with_target < 0 && cur_to_target > 0)) {
        return 1;
    }
    return 0;
}

static int get_bit_offset_target(uint32_t bit_offset, int cur_ppb)
{
#ifdef TARGET_TO_INIT_BIT_OFFSET
    if (target_to_init_enabled) {
        if (init_bit_offset == BAD_BIT_OFFSET) {
            init_bit_offset = bit_offset;
            SYNC_TRACE(2,"%s: init_bit_offset=%u", __func__, init_bit_offset);
        }
        return init_bit_offset;
    }
#endif

#ifdef CHIP_BEST1000_BEFORE_VER_G
    int max_ppb, min_ppb;
    int time_to_high, time_to_low;
    int ppb;

    if (bit_offset > BIT_OFFSET_CUSHION_LOW && bit_offset < BIT_OFFSET_CUSHION_HIGH) {
        bt_get_tune_range(&min_ppb, &max_ppb);
        // min_ppb always <= 0
        // max_ppb always >= 0
        // ppb < 0 means bit offset is decreasing
        // ppb > 0 means bit offset is increasing
        if (cur_ppb - min_ppb <= 0) {
            return BIT_OFFSET_CUSHION_LOW;
        }
        if (cur_ppb - max_ppb >= 0) {
            return BIT_OFFSET_CUSHION_HIGH;
        }

        ppb = cur_ppb - min_ppb;
        if (ppb > IN_RANGE_PPB) {
            ppb = IN_RANGE_PPB;
        }
        time_to_high = (int)(BIT_OFFSET_CUSHION_HIGH - bit_offset) * 1000 * 10 / ppb;

        ppb = cur_ppb - max_ppb;
        if (ppb < -IN_RANGE_PPB) {
            ppb = -IN_RANGE_PPB;
        }
        time_to_low = (int)(BIT_OFFSET_CUSHION_LOW - bit_offset) * 1000 * 10 / ppb;

        if (time_to_low > time_to_high) {
            return BIT_OFFSET_CUSHION_HIGH;
        } else {
            return BIT_OFFSET_CUSHION_LOW;
        }
    }
#endif
    return bit_offset;
}
#endif // FIXED_BIT_OFFSET_TARGET

static void reset_xtal_sync(void)
{
    prev_valid = 0;
    bit_offset_cnt = 0;
    xtal_bit_offset = 0;
    diff_with_prev = 0;
#ifdef BT_XTAL_SYNC_NEW_METHOD
    init_rxbit = -1;
    last_rxbit = -1;
    cross_slot = 0;
    first_offset_lock = 0;
    check_cnt = 0;
    afc_mobile_para.Kp = 0.1;//affect response speed
    afc_mobile_para.Kd = 1;//suppress oversize adjust
    afc_tws_para.Kp = 0.5;
    afc_tws_para.Kd = 1;
#endif
}

static int diff_value_valid(int diff, uint32_t size)
{
    int i_size = size;

    if (diff >= -i_size / 2 && diff < i_size / 2) {
        return 1;
    }

    return 0;
}

static int get_diff_value(int a, int b, uint32_t size)
{
    int diff = a - b;

    if (diff > 0) {
        if (diff >= (int)size / 2) {
            diff -= size;
        }
    } else if (diff < 0) {
        if (-diff > (int)size / 2) {
            diff += size;
        }
    }

    if (diff_value_valid(diff, size)) {
        return diff;
    } else {
        SYNC_TRACE(3,"%s: Bad diff=%d size=%u\n\n", __func__, diff, size);
        return 0;
    }
}

static int time_to_ppb(int diff_us, uint32_t ticks)
{
    int ppb;
    int round;

    if (diff_us < 0) {
        round = - (ticks / 2);
    } else {
        round = ticks / 2;
    }
#ifdef BTDRV_XTAL_FCAP_DIRECT_UPDATE
    ppb = ((int64_t)diff_us * CONFIG_SYSTICK_HZ * 1000 + round) / ticks;
#else
    ppb = ((int64_t)diff_us * BT_TICK_HZ * 1000 + round) / ticks;
#endif
    return ppb;
}

static void update_xtal_tune_factor(int offset, int prev_ppb, int cur_ppb)
{
#ifndef FIXED_XTAL_TUNE_FACTOR
    int factor;
    int diff;
    //uint32_t old_factor;

    if (ABS(offset) < 4) {
        SYNC_TRACE(2,"%s: Offset too small: %d\n\n", __func__, offset);
        return;
    }

    diff = prev_ppb - cur_ppb;
    if (diff == 0) {
        SYNC_TRACE(2,"%s: Same PPB: ppb=%d\n\n", __func__, offset, prev_ppb);
        return;
    }

    if ((offset < 0 && diff > 0) ||
            (offset > 0 && diff < 0)) {
        SYNC_TRACE(3,"%s: Incompatible factor offset %d and ppb diff %d\n\n", __func__, offset, diff);
        return;
    }

    if (ABS(diff) < 1000) {
        SYNC_TRACE(2,"%s: Deviation might be large: diff=%d\n\n", __func__, diff);
        return;
    }

    factor = (diff + offset / 2) / offset;
    if (MIN_TUNE_FACTOR <= factor && factor <= MAX_TUNE_FACTOR) {
        //old_factor = tune_factor;
        tune_factor = (tune_factor + factor) / 2;
        if (tune_factor_trust < 2) {
            tune_factor_trust++;
        }
        SYNC_TRACE(6,"%s: offset=%d prev=%d cur=%d factor=%u/%u/\n\n", __func__, offset, prev_ppb, cur_ppb, factor, tune_factor);
    } else {
        SYNC_TRACE(5,"%s: BAD FACTOR: offset=%d prev=%d cur=%d factor=%u\n\n", __func__, offset, prev_ppb, cur_ppb, factor);
    }
#endif
}

static uint32_t get_real_ticks(uint32_t ticks)
{
#ifndef BTDRV_XTAL_FCAP_DIRECT_UPDATE
    if (prev_valid) {
        ticks = (timer_wrap_count << BT_TICK_BIT_WIDTH) | (ticks & BT_TICK_MASK);
    }
#endif
    return ticks;
}

static uint32_t POSSIBLY_UNUSED ticks_to_ms(uint32_t ticks)
{
#ifdef BTDRV_XTAL_FCAP_DIRECT_UPDATE
    return TICKS_TO_MS(ticks);
#else
    return (uint32_t)((uint64_t)ticks * 1000 / BT_TICK_HZ);
#endif
}

static uint32_t POSSIBLY_UNUSED ms_to_ticks(uint32_t ms)
{
#ifdef BTDRV_XTAL_FCAP_DIRECT_UPDATE
    return MS_TO_TICKS(ms);
#else
    return ms * BT_TICK_HZ / 1000;
#endif
}

#ifdef BT_XTAL_SYNC_NEW_METHOD
#ifndef ABS
#define ABS(x) ((x) < 0 ? (-(x)) : (x))      /**< Absolute integer value. */
#endif
uint8_t check_init_rxbit(int32_t rxbit)
{
    if(last_rxbit == -1){
        last_rxbit = rxbit;
        return 0;
    }else{
        int32_t diff = 0;
        if((last_rxbit>622)&&(rxbit<3)){
            rxbit+=624;
            SYNC_TRACE(2,"xtal_sync: last_rxbit %d rxbit %d",last_rxbit,rxbit);
        }else if((last_rxbit<3)&&(rxbit>622)){
            rxbit-=624;
            SYNC_TRACE(2,"xtal_sync: last_rxbit %d rxbit %d",last_rxbit,rxbit);
        }
        diff = rxbit - last_rxbit;
        if(ABS(diff)<5){
            check_cnt++;
        }else{
            check_cnt = 0;
        }
        last_rxbit = rxbit;
        if(check_cnt>3){
            return 1;
        }else{
            return 0;
        }
    }
}

#if 0
void bt_xtal_sync_new(int32_t rxbit)
{
    int32_t old_val, new_val;
    if (sync_enabled != SYNC_STATE_ENABLED){
        return;
    }
    if (btdrv_rf_xtal_fcap_busy(true)) {
        return;
    }
    if (init_rxbit == -1)
    {
        if(first_offset_lock == 0){
            if(!check_init_rxbit(rxbit)){
                return;
            }
        }
        init_rxbit = rxbit;
        last_rxbit = rxbit;
        first_offset_lock = 1;
        SYNC_TRACE(1,"xtal_sync: init_rxbit %d",init_rxbit);
    }
    else
    {
        int32_t rxbit_diff = 0;
        int32_t last_rxbit_diff = last_rxbit-init_rxbit+cross_slot*625;
        if((last_rxbit>622)&&(rxbit<3))
        {
            cross_slot += 1;
            SYNC_TRACE(2,"xtal_sync: last_rxbit %d rxbit %d",last_rxbit,rxbit);
        }
        else if((last_rxbit<3)&&(rxbit>622))
        {
            cross_slot -= 1;
            SYNC_TRACE(2,"xtal_sync: last_rxbit %d rxbit %d",last_rxbit,rxbit);
        }
        rxbit_diff = rxbit-init_rxbit+cross_slot*625;
        if(init_rxbit != rxbit)
        {
            old_val = btdrv_rf_get_xtal_fcap();
            new_val = old_val+rxbit_diff/2+(rxbit_diff-last_rxbit_diff);
            if(new_val<0){
                new_val = 0;
            }
            else if(new_val>fcap_max_val){
                new_val = fcap_max_val;
            }
            btdrv_rf_set_xtal_fcap(new_val,true);
            old_xtal_fcap = new_val;
            SYNC_TRACE(3,"xtal_sync: old_val %d new_val %d rxbit %d",old_val,new_val,rxbit);
        }
        last_rxbit = rxbit;
    }
}
#else
void bt_xtal_sync_new(int32_t rxbit, bool fix_rxbit_en, enum BT_XTAL_SYNC_MODE_T mode)
{
    int32_t old_val, new_val;
    if (sync_enabled != SYNC_STATE_ENABLED){
        return;
    }
    if (btdrv_rf_xtal_fcap_busy(true)) {
        return;
    }
    if (init_rxbit == -1)
    {
        if(first_offset_lock == 0){
            if(!check_init_rxbit(rxbit)){
                return;
            }
        }
        
        if(fix_rxbit_en)
        {
            init_rxbit = FIX_TARTGET_RXBIT;
        }
        else
        {
            init_rxbit = rxbit;
        }
        last_rxbit = rxbit;
        first_offset_lock = 1;
        SYNC_TRACE(2,"xtal_sync: init_rxbit %d,fix rxbit en=%d",init_rxbit,fix_rxbit_en);
    }
    else
    {
        int32_t rxbit_diff = rxbit-last_rxbit;
        int32_t last_rxbit_diff = last_rxbit-init_rxbit+cross_slot*625;
        if(rxbit_diff > 312){
            rxbit_diff = ABS(rxbit_diff-625);
        }else if(rxbit_diff < -312){
            rxbit_diff = ABS(rxbit_diff+625);
        }else{
            rxbit_diff = ABS(rxbit_diff);
        }
        if(rxbit_diff>20){
            return;
        }
        if((last_rxbit>624-BITOFFSET_CROSS_THRESHOLD)&&(rxbit<BITOFFSET_CROSS_THRESHOLD))
        {
            cross_slot += 1;
            SYNC_TRACE(2,"xtal_sync: last_rxbit %d rxbit %d",last_rxbit,rxbit);
        }
        else if((last_rxbit<BITOFFSET_CROSS_THRESHOLD)&&(rxbit>624-BITOFFSET_CROSS_THRESHOLD))
        {
            cross_slot -= 1;
            SYNC_TRACE(2,"xtal_sync: last_rxbit %d rxbit %d",last_rxbit,rxbit);
        }
        rxbit_diff = rxbit-init_rxbit+cross_slot*625;
        if(init_rxbit != rxbit)
        {
            pid_para_t * pid_para_ptr = NULL;
            if(mode == BT_XTAL_SYNC_MODE_WITH_MOBILE){
                pid_para_ptr = &afc_mobile_para;
            }else if(mode == BT_XTAL_SYNC_MODE_WITH_TWS){
                pid_para_ptr = &afc_tws_para;
            }
            else{
                return;
            }
            old_val = btdrv_rf_get_xtal_fcap();
            new_val = (int32_t)(old_val+rxbit_diff*pid_para_ptr->Kp+(rxbit_diff-last_rxbit_diff)*pid_para_ptr->Kd);
            
            if(new_val < xtal_fcap_min){
                new_val = xtal_fcap_min;
            }
            else if(new_val > xtal_fcap_max){
                new_val = xtal_fcap_max;
            }
            
            btdrv_rf_set_xtal_fcap(new_val,true);
            old_xtal_fcap = new_val;
            SYNC_TRACE(5,"xtal_sync: old_val %d new_val %d rxbit %d [%d:%d]",old_val,new_val,rxbit, xtal_fcap_min, xtal_fcap_max);
        }
        last_rxbit = rxbit;
    }
}

#endif

#endif

void bt_xtal_sync(enum BT_XTAL_SYNC_MODE_T mode)
{
    uint32_t avg;
    uint32_t cur_offset;
    uint32_t cur_time;
    int diff;
    int ppb;
    int POSSIBLY_UNUSED old_ppb;
    union BIT_OFFSET_AND_CLOCK_T oc;
    int16_t consec_diff;
    uint8_t ppb_valid;
    uint8_t tune_xtal;
#ifdef FIXED_BIT_OFFSET_TARGET
    int8_t cur_in_range = 0;
    int8_t cur_out_of_range = 0;
    int cur_to_target = 0;
    static enum IN_RANGE_STATE_T saved_in_range;
#endif

    if (sync_enabled == SYNC_STATE_NULL) {
        bt_init_xtal_sync(mode, xtal_fcap_min, xtal_fcap_max, fcap_max_val);
    } else if (sync_enabled == SYNC_STATE_DISABLED) {
        return;
    }

    if (btdrv_rf_xtal_fcap_busy(true)) {
        return;
    }

    oc.reg = btdrv_rf_bit_offset_get();
    cur_offset = oc.offset;
#ifdef BTDRV_XTAL_FCAP_DIRECT_UPDATE
    cur_time = hal_sys_timer_get();
#else
    cur_time = oc.clock;
    if (last_check_time == cur_time && last_check_offset == cur_offset) {
        return;
    }
    if (prev_valid) {
        if (cur_time <= last_check_time) {
            timer_wrap_count++;
        }
    }
    last_check_time = cur_time;
#endif
    if (last_check_offset != cur_offset) {
        SYNC_TRACE(3,"%s: ms=%u bit_offset=%u", __func__, ticks_to_ms(get_real_ticks(cur_time)), cur_offset);
        if (last_check_offset == BAD_BIT_OFFSET) {
            consec_diff = 0;
        } else {
            consec_diff = get_diff_value(cur_offset, last_check_offset, (MAX_BIT_OFFSET + 1));
        }
        // Save current bit offset
        last_check_offset = cur_offset;
        // Check bit offset validity
        if (ABS(consec_diff) > MAX_VALID_CONSEC_DIFF) {
            err_consec_diff++;
            SYNC_TRACE(3,"%s: Bad consec_diff=%d (%u)", __func__, consec_diff, err_consec_diff);
            if (err_consec_diff > MAX_CONSEC_DIFF_ERR) {
                bt_reset_xtal_to_default();
            }
            // Restart ppb calculation
            reset_xtal_sync();
            return;
        } else {
            err_consec_diff = 0;
        }
    }

#ifdef FIXED_BIT_OFFSET_TARGET
    if (fixed_target_enabled) {
        if (target_valid == 0) {
            target_valid = 1;
            target_valid_time = hal_sys_timer_get();
            target_bit_offset = get_bit_offset_target(cur_offset, 0);
            if (
#ifdef CHIP_BEST1000_BEFORE_VER_G
                    target_bit_offset != cur_offset
#else
                    bit_offset_in_range(cur_offset)
#endif
                    ) {
                diff_with_target = get_diff_value(cur_offset, target_bit_offset, (MAX_BIT_OFFSET + 1));
                in_range = IN_RANGE_YES_INIT;
                old_xtal_fcap = btdrv_rf_get_xtal_fcap();
                ppb = (diff_with_target > 0) ? IN_RANGE_PPB : -IN_RANGE_PPB;
                bt_tune_xtal(ppb);
                //reset_xtal_sync();
                //update_factor = 0;
                SYNC_TRACE(1,"\n\nBit offset in range or near range: try_ppb=%d\n\n", ppb);
            } else {
                SYNC_TRACE(0,"\n\nBit offset is good\n\n");
            }
            SYNC_TRACE(3,"Init tgt=%u cur=%u in_range=%d\n\n", target_bit_offset, cur_offset, in_range);
        } else if (in_range == IN_RANGE_YES_INIT) {
            // Check if bit offset is unchanged for a long time
            if (hal_sys_timer_get() - target_valid_time >= MS_TO_TICKS(IN_RANGE_TIMEOUT_MS)) {
                ppb = (diff_with_target > 0) ? -IN_RANGE_PPB : IN_RANGE_PPB;
                bt_tune_xtal(ppb);
                reset_xtal_sync();
                update_factor = 0;
                SYNC_TRACE(1,"\n\nIn range init timeout: try_ppb=%d\n\n", ppb);
                // Update the target bit offset
                target_bit_offset = get_bit_offset_target(cur_offset, 0);
                diff_with_target = get_diff_value(cur_offset, target_bit_offset, (MAX_BIT_OFFSET + 1));
                target_valid_time = hal_sys_timer_get();
                if (bit_offset_in_range(cur_offset)) {
                    in_range = IN_RANGE_YES;
                } else {
                    in_range = IN_RANGE_NO;
                }
                SYNC_TRACE(3,"Update tgt=%u cur=%d in_range=%d\n\n", target_bit_offset, cur_offset, in_range);
            }
        }
    }
#endif

    if (bit_offset_cnt == 0) {
        xtal_bit_offset = cur_offset;
    } else {
        avg = xtal_bit_offset / bit_offset_cnt;
        if (avg < ((MAX_BIT_OFFSET + 1) / 2) && cur_offset >= ((MAX_BIT_OFFSET + 1) / 2)) {
            // Bit offset just wraps from 0 to MAX_BIT_OFFSET
            xtal_bit_offset += (MAX_BIT_OFFSET + 1) * bit_offset_cnt;
        } else if (avg >= ((MAX_BIT_OFFSET + 1) / 2) && cur_offset < ((MAX_BIT_OFFSET + 1) / 2)) {
            // Bit offset just wraps from MAX_BIT_OFFSET to 0
            cur_offset += (MAX_BIT_OFFSET + 1);
        }
        xtal_bit_offset += cur_offset;
    }
    bit_offset_cnt++;

    if (bit_offset_cnt >= DIFF_CNT) {
        avg = xtal_bit_offset / bit_offset_cnt;
        if (avg >= (MAX_BIT_OFFSET + 1)) {
            avg -= (MAX_BIT_OFFSET + 1);
        }
        xtal_bit_offset = 0;
        bit_offset_cnt = 0;

        if (prev_valid == 0) {
            prev_valid = 1;
            prev_value = avg;
            prev_time = cur_time;
            timer_wrap_count = 0;
        } else {
            diff = get_diff_value(avg, prev_value, (MAX_BIT_OFFSET + 1));

            // Check diff direction
            if (diff_with_prev == 0) {
                diff_with_prev = diff;
            } else if ((diff_with_prev > 0 && diff < 0) ||
                    (diff_with_prev < 0 && diff > 0)) {
                // Restart ppb calculation
                reset_xtal_sync();
                return;
            }

            // Check whether to tune crystal
            tune_xtal = 0;
            if (ABS(diff) >= BIT_OFFSET_SYNC_THRESH) {
                tune_xtal = 1;
            }
#ifdef FIXED_BIT_OFFSET_TARGET
            if (fixed_target_enabled) {
                cur_to_target = get_diff_value(avg, target_bit_offset, (MAX_BIT_OFFSET + 1));
                if (in_range == IN_RANGE_NO) {
                    cur_in_range = bit_offset_in_range(avg);
                    if (cur_in_range) {
                        tune_xtal = 1;
                    }
                } else {
                    cur_out_of_range = diff_out_of_range(cur_to_target);
                    if (cur_out_of_range) {
                        tune_xtal = 1;
                    } else {
                        // Check whether current ppb is less than IN_RANGE_PPB / 5
                        if (get_real_ticks(cur_time) - prev_time >= ms_to_ticks(IN_RANGE_TIMEOUT_MS) &&
                                ABS(diff) < ((IN_RANGE_PPB / 1000) * (IN_RANGE_TIMEOUT_MS / 1000) + 4) / 5) {
                            tune_xtal = 1;
                        }
                    }
                }
            }
#endif
            if (tune_xtal) {
                // Calculate current ppb
                if (ABS(diff) >= BIT_OFFSET_SYNC_THRESH ||
                        get_real_ticks(cur_time) - prev_time >= ms_to_ticks(IN_RANGE_TIMEOUT_MS)) {
                    ppb = time_to_ppb(diff, get_real_ticks(cur_time) - prev_time);
                } else {
                    if (last_valid_ppb == BAD_PPB) {
                        ppb = 0;
                    } else {
                        ppb = last_valid_ppb - last_tune_ppb;
                    }
                    // Current ppb is not accurate
                    update_factor = 0;
                }
                SYNC_TRACE(3,"\n\nUpdate 26M: diff=%d ms=%u ppb=%d\n\n", diff, ticks_to_ms(get_real_ticks(cur_time) - prev_time), ppb);

                // Check ppb validity
                ppb_valid = 1;
#ifdef FIXED_BIT_OFFSET_TARGET
                if (fixed_target_enabled) {
                    if (in_range == IN_RANGE_NO) {
                        if (ABS(ppb) > MAX_VALID_PPB) {
                            ppb_valid = 0;
                        }
                    } else {
                        // When in range at init time, the max possible ppb is (MAX_VALID_PPB + IN_RANGE_PPB);
                        // when in range at other time, the max possible ppb is (MAX_VALID_PPB_DIFF + MAX(MAX_VALID_PPB, IN_RANGE_PPB))
                        if (ABS(ppb) > MAX_VALID_PPB + IN_RANGE_PPB) {
                            ppb_valid = 0;
                        }
                    }
                } else
#endif
                {
                    if (ABS(ppb) > MAX_VALID_PPB) {
                        ppb_valid = 0;
                    }
                }
                if (last_valid_ppb != BAD_PPB) {
                    if (ABS(ppb - (last_valid_ppb - last_tune_ppb)) > MAX_VALID_PPB_DIFF + ABS(last_tune_ppb) / (2 + tune_factor_trust)) {
                        ppb_valid = 0;
                    }
                }
                if (ppb_valid) {
                    err_ppb = 0;
                    // Save valid ppb
                    last_valid_ppb = ppb;
                } else {
                    err_ppb++;
                    SYNC_TRACE(5,"%s: Bad ppb=%d last_valid_ppb=%d last_tune_ppb=%d (%u)", __func__, ppb, last_valid_ppb, last_tune_ppb, err_ppb);
                    if (err_ppb > MAX_PPB_ERR) {
                        bt_reset_xtal_to_default();
                    }
                    // Save valid ppb
                    last_valid_ppb = ppb;
                    // Save tune ppb
                    last_tune_ppb = 0;
                    // Restart ppb calculation
                    reset_xtal_sync();
                    return;
                }

                // Update tune factor and check whether in range
                if (update_factor) {
                    update_xtal_tune_factor(prev_tune_offset, prev_real_ppb, ppb);
                }
                // Save real ppb
                prev_real_ppb = ppb;

#ifdef FIXED_BIT_OFFSET_TARGET
                // Check whether bit offset in range
                if (fixed_target_enabled) {
                    saved_in_range = in_range;
                    if (in_range == IN_RANGE_NO) {
                        if (cur_in_range) {
                            target_bit_offset = get_bit_offset_target(avg, ppb);
                            diff_with_target = get_diff_value(avg, target_bit_offset, (MAX_BIT_OFFSET + 1));
                            target_valid_time = hal_sys_timer_get();
                            old_xtal_fcap = btdrv_rf_get_xtal_fcap();
                            in_range = IN_RANGE_YES;
                            cur_to_target = diff_with_target;
                            SYNC_TRACE(4,"In range: tgt=%u avg=%u diff2tgt=%d cur2tgt=%d",
                                target_bit_offset, avg, diff_with_target, cur_to_target);
                        }
                    } else {
                        if (cur_out_of_range) {
                            in_range = IN_RANGE_NO;
                            SYNC_TRACE(4,"Out of range: tgt=%u avg=%u diff2tgt=%d cur2tgt=%d",
                                target_bit_offset, avg, diff_with_target, cur_to_target);
                        } else if (in_range == IN_RANGE_YES_INIT) {
                            in_range = IN_RANGE_YES;
                            // Update the target bit offset with current frequency shift
                            target_bit_offset = get_bit_offset_target(avg, ppb);
                            diff_with_target = get_diff_value(avg, target_bit_offset, (MAX_BIT_OFFSET + 1));
                            cur_to_target = diff_with_target;
                            SYNC_TRACE(1,"Update tgt=%u with cur ppb\n\n", target_bit_offset);
                        }
                    }
                    if (in_range == IN_RANGE_NO) {
                        if (saved_in_range == IN_RANGE_NO && cur_to_target != 0) {
                            if ((cur_to_target > 0 && ppb > 0) || (cur_to_target < 0 && ppb < 0)) {
                                ppb += ppb / COMMON_PPB_COEFF;
                            } else {
                                ppb -= ppb / COMMON_PPB_COEFF;
                            }
                        }
                        // Otherwise using the real ppb
                    } else {
                        stable_count = 0;
                        stable_state = 0;
                        // Here cur_to_target has the same sign with diff_with_target
                        if (cur_to_target > 0) {
                            if (ppb > -IN_RANGE_PPB) {
                                ppb += IN_RANGE_PPB;
                                if (ppb > IN_RANGE_PPB) {
                                    ppb = IN_RANGE_PPB;
                                }
                            } else {
                                // Otherwise keep current ppb
                                ppb = 0;
                            }
                        } else {
                            if (ppb < IN_RANGE_PPB) {
                                ppb -= IN_RANGE_PPB;
                                if (ppb < -IN_RANGE_PPB) {
                                    ppb = -IN_RANGE_PPB;
                                }
                            } else {
                                // Otherwise keep current ppb
                                ppb = 0;
                            }
                        }
                    }
                    SYNC_TRACE(4,"real_ppb=%d try_ppb=%d cur2tgt=%d in_range=%d\n\n", prev_real_ppb, ppb, cur_to_target, in_range);
                }
#endif // FIXED_BIT_OFFSET_TARGET

                // Set flag to update factor in next run
                update_factor = 1;

                // Stable state check
                // Reduce the impact if the ppb is derived from instant invalid bit offset values
                if (stable_state && ABS(ppb) > MAX_PPB_AFTER_STABLE) {
                    old_ppb = ppb;
                    ppb = (ppb > 0) ? MAX_PPB_AFTER_STABLE : -MAX_PPB_AFTER_STABLE;
                    SYNC_TRACE(2,"Bad ppb=%d in stable state. Set to %d\n\n", old_ppb, ppb);
                    update_factor = 0;
                } else {
                    if (stable_state == 0) {
                        if (ABS(ppb) < STABLE_STATE_PPB) {
                            if (stable_count < STABLE_CHECK_COUNT) {
                                stable_count++;
                            } else {
                                stable_state = 1;
                            }
                        } else {
                            stable_count = 0;
                        }
                    }
                }

                // Tune crystal
                prev_tune_offset = bt_tune_xtal(ppb);
                // Save tune ppb
                last_tune_ppb = bt_xtal_offset_to_ppb(prev_tune_offset);
                // Restart ppb calculation
                reset_xtal_sync();
                SYNC_TRACE(1,"tune_ppb=%d\n\n", last_tune_ppb);
            }
        }
    }
}

bool bt_xtal_sync_allow(void)
{
#ifndef BT_BUILD_WITH_CUSTOMER_HOST
#if defined(IBRT)
#if !defined(TWS_MASTER_USE_AFC) && !defined(FPGA)
    btif_remote_device_t *remDev = 0;
    if(!app_tws_mode_is_slave())
        return false;
    remDev = btif_a2dp_get_remote_device(app_tws_get_sink_stream());
    if (btif_me_get_remote_device_state(remDev) != BTIF_BDS_CONNECTED)
        return false;
    if (btif_me_get_current_role(remDev) != BTIF_BCR_SLAVE)
        return false;
#endif
#else
    btif_remote_device_t *remDev = NULL;
    if(bt_adapter_count_mobile_link() == 1) {
        for (uint8_t i = 0; i < BT_DEVICE_NUM; i++) {
            remDev = btif_me_enumerate_remote_devices(i);
            if(NULL == remDev){
                continue;
            }

            if (btif_me_get_remote_device_state(remDev) == BTIF_BDS_CONNECTED)
                break;
            else
                remDev = NULL;
        }
        if (remDev == NULL)
            return false;
        if (btif_me_get_current_role(remDev) != BTIF_BCR_SLAVE)
            return false;
    }
#endif
#endif
    return true;
}

void bt_init_xtal_sync(enum BT_XTAL_SYNC_MODE_T mode, int range_min, int range_max, int fcap_range)
{
    if (!bt_xtal_sync_allow()) {
        sync_enabled = SYNC_STATE_DISABLED;
        return;
    }

    SYNC_TRACE(1,"%s",__FUNCTION__);

    if (tune_factor == 0) {
        tune_factor = btdrv_rf_get_xtal_tune_factor();
    }

    reset_xtal_sync();
    last_check_offset = BAD_BIT_OFFSET;
    last_check_time = 0;
    update_factor = 0;
    err_consec_diff = 0;
    err_ppb = 0;
    stable_count = 0;
    stable_state = 0;
    last_valid_ppb = BAD_PPB;
    last_tune_ppb = 0;
    sync_enabled = SYNC_STATE_ENABLED;
    xtal_fcap_min = range_min;
    xtal_fcap_max = range_max;
    fcap_max_val = fcap_range;
    ASSERT(fcap_range >= range_max, "%s [%d:%d] range:%d", __func__, range_min, range_max, fcap_range);

#ifdef FIXED_BIT_OFFSET_TARGET
    fixed_target_enabled = 0;
#if defined(IBRT)
#if !defined(TWS_MASTER_USE_AFC) && !defined(FPGA)
    if (app_tws_mode_is_slave())
#endif
    {
        fixed_target_enabled = 1;
    }
#endif
#ifdef CHIP_BEST1000_BEFORE_VER_G
    // Fixed since version G
    if (hal_get_chip_metal_id() < HAL_CHIP_METAL_ID_4 &&
            mode == BT_XTAL_SYNC_MODE_VOICE) {
        fixed_target_enabled = 1;
    }
#endif

    in_range = IN_RANGE_NO;
    target_valid = 0;

#ifdef TARGET_TO_INIT_BIT_OFFSET
    target_to_init_enabled = 1;
#if defined(IBRT)
#if !defined(TWS_MASTER_USE_AFC) && !defined(FPGA)
    if (!app_tws_mode_is_slave()) {
        target_to_init_enabled = 0;
    }
#endif
#endif

    init_bit_offset = BAD_BIT_OFFSET;
#endif // TARGET_TO_INIT_BIT_OFFSET
#endif // FIXED_BIT_OFFSET_TARGET
}

static void wait_xtal_tuning_done(uint8_t is_direct)
{
    const int retry_max = 6;
    int retry;

    // Disable SPI channel after ensuring fcap has been set
    retry = 0;
    while (btdrv_rf_xtal_fcap_busy(is_direct) && retry++ < retry_max) {
        osDelay(10);
    }
}

void bt_term_xtal_sync(bool xtal_term_default)
{
    bool need_term = false;
    if (sync_enabled == SYNC_STATE_ENABLED){
        // Always stay in tracking mode (instead of switching to locking mode)

        wait_xtal_tuning_done(true);
#ifdef FIXED_BIT_OFFSET_TARGET
        if (in_range != IN_RANGE_NO) {
            need_term = true;
        }
#endif
    }
    if (need_term || xtal_term_default){
        if (xtal_term_default){
            SYNC_TRACE(1,"%s: Restore init_xtal_fcap", __func__);
            btdrv_rf_set_xtal_fcap(btdrv_rf_get_init_xtal_fcap(),true);
            wait_xtal_tuning_done(true);
        }else{
  #ifdef FIXED_BIT_OFFSET_TARGET
            // Restore a workable xtal fcap setting
            SYNC_TRACE(2,"%s: Restore old_xtal_fcap=%u", __func__, old_xtal_fcap);
            btdrv_rf_set_xtal_fcap(old_xtal_fcap,true);
            wait_xtal_tuning_done(true);
  #endif
        }
    }
    sync_enabled = SYNC_STATE_NULL;
}

void bt_term_xtal_sync_default(void)
{
    uint32_t init_val, curr_val;

    SYNC_TRACE(1,"%s",__func__);
#ifdef FIXED_BIT_OFFSET_TARGET
    in_range = IN_RANGE_NO;
#endif
    sync_enabled = SYNC_STATE_NULL;

    curr_val = btdrv_rf_get_xtal_fcap();
    init_val = btdrv_rf_get_init_xtal_fcap();

    if (init_val != curr_val){
        btdrv_rf_set_xtal_fcap(init_val,true);
        wait_xtal_tuning_done(true);
    }
}