/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#ifndef CHIP_SUBSYS_SENS

#ifndef __TGT_HARDWARE_CAPSENSOR__
#define __TGT_HARDWARE_CAPSENSOR__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CAPSENSOR_SLIDE
#define CAP_SAMP_FS          10  /* ms */
#else
#define CAP_SAMP_FS          50  /* ms */
#endif
#define CAP_CHNUM            7   /* channel numbers, CAP_USED_TOUCH_NUM+CAP_USED_WEAR_NUM */
#define CAP_REPNUM           1   /* rep numbers */

#define CAP_USED_TOUCH_NUM   3      /* slide_num */
#define CAP_USED_WEAR_NUM    4      /* wear_num */
#define CAP_DETECT_WEAR_NUM  2      /* detect_num */

/*customers can be modified parameters*/
// touch key
#define CAP_KEYDOWN_TH              (-4000)  // key down
#define CAP_KEYUP_TH                (-3500)  // key up
#define CAP_KEY_SLIDE_MS_PER_CNT    (25)
#define CAP_KEY_TOUCH_MS_PER_CNT    (65)
#define CAP_KEY_CLICK_MAX_DURATION  (450)   // 450m
#ifdef CAPSENSOR_SLIDE
#define CAP_KEY_CLICK_MIN_DURATION  (CAP_KEY_SLIDE_MS_PER_CNT-5)
#else
#define CAP_KEY_CLICK_MIN_DURATION  (CAP_KEY_TOUCH_MS_PER_CNT-5)
#endif
#define CAP_KEY_DOUBLE_INTERVAL     (400)   // double key interval
#define CAP_KEY_TRIPLE_INTERVAL     (400)   // triple key interval
#define CAP_KEY_ULTRA_INTERVAL      (400)   // ultra  key interval
#define CAP_KEY_RAMPAGE_INTERVAL    (400)   // rampage key interval
#define CAP_KEY_LONG_INTERVAL       (1000)  // 1000ms
#define CAP_KEY_LONGLONG_INTERVAL   (5000)  // 5000ms
#define CAP_DC_RESET_INTERVAL       (20000) // 20000ms
#define CAP_SECOND_CLICK_TH_FACTOR  (0.7)
// end touch key

// wear detect
#define CAP_WEAR_THRESHOLD_HIGH1            4000
#define CAP_WEAR_THRESHOLD_HIGH2            4000
#define CAP_WEAR_THRESHOLD_LOW1             3000
#define CAP_WEAR_THRESHOLD_LOW2             3000
#define CAP_WEAR_TWOPAD_THRESHOLD1          5000
#define CAP_WEAR_OFFSET0_UPDATE_THRESHOLD   600
#define CAP_WEAR_OFFSET1_UPDATE_THRESHOLD   600
#define CAP_WEAR_JUMP_THRESHOLD1            50000
#define CAP_WEAR_JUMP_THRESHOLD2            50000
#define CAP_WEAR_TWOPAD_STATE_WIN1          12
#define CAP_WEAR_EAR_ON_STATE_WIN           15
#define CAP_WEAR_EAR_OFF_STATE_WIN          12
#define CAP_WEAR_ON_DELAY_MSE_TH            200
#define CAP_WEAR_ON_DELAY_COUNT             50
#define CAP_WEAR_OUT_BOX_COUNT              30
#define CAP_WEAR_IN_BOX_COUNT               30
#define CAP_WEAR_IN_BOX_DIFF1               1000
#define CAP_WEAR_IN_BOX_DIFF2               1000
#define CAP_WEAR_OFFSET_MAX                 30000
#define CAP_WEAR_OFFSET0_UPDATE_STD         100
#define CAP_WEAR_OFFSET1_UPDATE_STD         100
// end wear detect
/*customers can be modified parameters*/

#define CAP_CH0             (1 << 0)
#define CAP_CH1             (1 << 1)
#define CAP_CH2             (1 << 2)
#define CAP_CH3             (1 << 3)
#define CAP_CH4             (1 << 4)
#define CAP_CH5             (1 << 5)
#define CAP_CH6             (1 << 6)
#define CAP_CH7             (1 << 7)

#ifdef __cplusplus
}
#endif

#endif

#endif
