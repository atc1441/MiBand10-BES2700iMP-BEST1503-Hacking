/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include <besbt_string.h>
#include "plat_types.h"
#include "tgt_hardware.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "bt_drv.h"
#include "hal_timer.h"
#include "hal_chipid.h"
#include "hal_psc.h"
#include "bt_drv_1306_internal.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"
#include "bt_1306_reg_map.h"
#include "bt_drv_1306_config.h"
#include "hal_cmu.h"

extern void btdrv_send_cmd(uint16_t opcode,uint8_t cmdlen,const uint8_t *param);
extern void btdrv_write_memory(uint8_t wr_type,uint32_t address,const uint8_t *value,uint8_t length);

typedef struct
{
    uint8_t is_act;
    uint16_t opcode;
    uint8_t parlen;
    const uint8_t *param;

} BTDRV_CFG_TBL_STRUCT;


#define BTDRV_CONFIG_ACTIVE   1
#define BTDRV_CONFIG_INACTIVE 0
/*
[0][0] = 63, [0][1] = 0,[0][2] = (-80),           472d
[1][0] = 51, [2][1] = 0,[2][2] = (-80),          472b
[2][0] = 42, [4][1] = 0,[4][2] = (-75),           4722
[3][0] = 36, [6][1] = 0,[6][2] = (-55),           c712
[4][0] = 30, [8][1] = 0,[8][2] = (-40),           c802
[5][0] = 21,[10][1] = 0,[10][2] = 0x7f,         c102
[6][0] = 12,[11][1] = 0,[11][2] = 0x7f,       c142
[7][0] = 3,[13][1] = 0,[13][2] = 0x7f,        c1c2
[8][0] = -3,[14][1] = 0,[14][2] = 0x7f};      c0c2
*/

const int8_t btdrv_rf_env[]=
{
    0x01,0x00,  //rf api
    0x01,   //rf env
    185,     //rf length
    BT_MAX_TX_PWR_IDX,     //txpwr_max
    -1,    ///rssi high thr
    -2,   //rssi low thr
    -100,  //rssi interf thr
    0xf,  //rssi interf gain thr
    2,  //wakeup delay
    0xe, 0, //skew
    0xe8,0x3,    //ble agc inv thr
    BTC_HW_AGC_ENABLE_FLAG,

    0xff,//sw gain set
    0xff,    //sw gain set
    -85,//bt_inq_page_iscan_pscan_dbm
    0x7f,//ble_scan_adv_dbm
    1,      //sw gain reset factor
    1,    //bt sw gain cntl enable
    1,   //ble sw gain cntl en
    1,  //bt interfere  detector en
    0,  //ble interfere detector en

    0,0,0,
    3,3,12,
    6,6,28,
    9,9,28,
    12,12,28,
    15,15,28,
    18,18,28,
    21,21,28,
    0x7f,24,0x7f,
    0x7f,27,0x7f,
    0x7f,30,0x7f,
    0x7f,33,0x7f,
    0x7f,36,0x7f,
    0x7f,39,0x7f,
    0x7f,42,0x7f,  //rx hwgain tbl ptr

    60,0,-80,
    48,0,-80,
    44,0,-80,
    36,0,-80,
    32,0,-80,
    24,0,-80,
    -2,0,-80,
    -18,0,-80,

    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,
    0x7f,0x7f,0x7f,  //rx gain tbl ptr

    -85,-85,
    -82,-82,
    -73,-73,
    -69,-69,
    -63,-63,
    -40,-40,
    -23,-23,
    0x7f,0x7f,

    //  0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //rx gain ths tbl ptr

    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,1,
    0,2,
    0,2,
    0,2,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //flpha filter factor ptr
    -23,-20,-17,-14,-11,-8,-5,-2,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,   //tx pw onv tbl ptr
};

const int8_t btdrv_rxgain_gain_ths_3m[] = {
    -77,-77,
    -73,-73,
    -67,-67,
    -65,-65,
    -57,-57,
    -32,-32,
    -12,-12,

    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //rx gain ths tbl ptr
};

const int8_t btdrv_rxgain_ths_tbl_le[0xf * 2] = {
    -91,-91,
    -88,-88,
    -82,-82,
    -78,-78,
    -72,-72,
    -47,-47,
    -30,-30,
    -20,0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f, //ble rx gain ths tbl ptr
};

const int8_t btdrv_rxgain_ths_tbl_le_2m[0xf * 2] = {
    -84,-84,
    -82,-82,
    -77,-77,
    -74,-74,
    -65,-65,
    -41,-41,
    -24,-24,
    -20,0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f,
    0x7f, 0x7f, //ble rx gain ths tbl ptr
};

const int8_t btdrv_rxgain_ths_tbl_ecc[0xf * 2] = {
    -85,-85,
    -82,-82,
    -73,-73,
    -69,-69,
    -63,-63,
    -40,-40,
    -23,-23,
    -22,0x7f,

    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,
    0x7f,0x7f,    //ECC rx gain ths tbl ptr
};

//ble txpwr convert
int8_t  btdrv_txpwr_conv_tbl[8] = {
        [0] = -4,
        [1] = 0,
        [2] = 3,
        [3] = 7,
        [4] = 11,
        [5] = 16,
        [6] = 16,
        [7] = 16
};

const int8_t btdrv_afh_env[] =
{
    0x02,0x00,   //afh env
    0x00,      //ignore
    33,          //length
    5,   //nb_reass_chnl
    10,  //win_len
    -70,  //rf_rssi_interf_thr
    10,  //per_thres_bad
    20,  //reass_int
    20,   //n_min
    20,   //afh_rep_intv_max
    96,    //ths_min
    2,   //chnl_assess_report_style
    15,  //chnl_assess_diff_thres
    60, // chnl_assess_interfere_per_thres_bad
    9,  //chnl_assess_stat_cnt_max
    -9,  //chnl_assess_stat_cnt_min
    1,2,3,2,1,   //chnl_assess_stat_cnt_inc_mask[5]
    1,2,3,2,1,    //chnl_assess_stat_cnt_dec_mask
    0xd0,0x7,      //chnl_assess_timer
    -48,        //chnl_assess_min_rssi
    0x64,0,   //chnl_assess_nb_pkt
    0x32,0,     //chnl_assess_nb_bad_pkt
    6,    //chnl_reassess_cnt_val
    0x3c,0,     //chnl_assess_interfere_per_thres_bad
};

const uint8_t lpclk_drift_jitter[] =
{
    0xfa,0x00,  //  drift  250ppm
    0x0a,0x00    //jitter  +-10us

};

uint8_t  sleep_param[] =
{
    0,    // sleep_en;
    1,    // exwakeup_en;
    0xd0,0x7,    //  lpo_calib_interval;   lpo calibration interval
    0x32,0,0,0,    // lpo_calib_time;  lpo count lpc times
};

const uint8_t  sync_config[] =
{
    1,1,   //sco path config   0:hci  1:pcm
    0,      //sync use max buff length   0:sync data length= packet length 1:sync data length = host sync buff len
    0,        //cvsd bypass     0:cvsd2pcm   1:cvsd transparent
};

//pcm general ctrl
#define PCM_PCMEN_POS            15
#define PCM_LOOPBCK_POS          14
#define PCM_MIXERDSBPOL_POS      11
#define PCM_MIXERMODE_POS        10
#define PCM_STUTTERDSBPOL_POS    9
#define PCM_STUTTERMODE_POS      8
#define PCM_CHSEL_POS            6
#define PCM_MSTSLV_POS           5
#define PCM_PCMIRQEN_POS         4
#define PCM_DATASRC_POS          0


//pcm phy ctrl
#define PCM_LRCHPOL_POS     15
#define PCM_CLKINV_POS      14
#define PCM_IOM_PCM_POS     13
#define PCM_BUSSPEED_LSB    10
#define PCM_SLOTEN_MASK     ((uint32_t)0x00000380)
#define PCM_SLOTEN_LSB      7
#define PCM_WORDSIZE_MASK   ((uint32_t)0x00000060)
#define PCM_WORDSIZE_LSB    5
#define PCM_DOUTCFG_MASK    ((uint32_t)0x00000018)
#define PCM_DOUTCFG_LSB     3
#define PCM_FSYNCSHP_MASK   ((uint32_t)0x00000007)
#define PCM_FSYNCSHP_LSB    0

/// Enumeration of PCM status
enum PCM_STAT
{
    PCM_DISABLE = 0,
    PCM_ENABLE
};

/// Enumeration of PCM channel selection
enum PCM_CHANNEL
{
    PCM_CH_0 = 0,
    PCM_CH_1
};

/// Enumeration of PCM role
enum PCM_MSTSLV
{
    PCM_SLAVE = 0,
    PCM_MASTER
};

/// Enumeration of PCM data source
enum PCM_SRC
{
    PCM_SRC_DPV = 0,
    PCM_SRC_REG
};

/// Enumeration of PCM left/right channel selection versus frame sync polarity
enum PCM_LR_CH_POL
{
    PCM_LR_CH_POL_RIGHT_LEFT = 0,
    PCM_LR_CH_POL_LEFT_RIGHT
};

/// Enumeration of PCM clock inversion
enum PCM_CLK_INV
{
    PCM_CLK_RISING_EDGE = 0,
    PCM_CLK_FALLING_EDGE
};

/// Enumeration of PCM mode selection
enum PCM_MODE
{
    PCM_MODE_PCM = 0,
    PCM_MODE_IOM
};

/// Enumeration of PCM bus speed
enum PCM_BUS_SPEED
{
    PCM_BUS_SPEED_128k = 0,
    PCM_BUS_SPEED_256k,
    PCM_BUS_SPEED_512k,
    PCM_BUS_SPEED_1024k,
    PCM_BUS_SPEED_2048k
};

/// Enumeration of PCM slot enable
enum PCM_SLOT
{
    PCM_SLOT_NONE = 0,
    PCM_SLOT_0,
    PCM_SLOT_0_1,
    PCM_SLOT_0_2,
    PCM_SLOT_0_3
};

/// Enumeration of PCM word size
enum PCM_WORD_SIZE
{
    PCM_8_BITS = 0,
    PCM_13_BITS,
    PCM_14_BITS,
    PCM_16_BITS
};

/// Enumeration of PCM DOUT pad configuration
enum PCM_DOUT_CFG
{
    PCM_OPEN_DRAIN = 0,
    PCM_PUSH_PULL_HZ,
    PCM_PUSH_PULL_0
};

/// Enumeration of PCM FSYNC physical shape
enum PCM_FSYNC
{
    PCM_FSYNC_LF = 0,
    PCM_FSYNC_FR,
    PCM_FSYNC_FF,
    PCM_FSYNC_LONG,
    PCM_FSYNC_LONG_16
};

const uint32_t pcm_setting[] =
{
//pcm_general_ctrl
    (PCM_DISABLE<<PCM_PCMEN_POS) |                      //enable auto
    (PCM_DISABLE << PCM_LOOPBCK_POS)  |                 //LOOPBACK test
    (PCM_DISABLE << PCM_MIXERDSBPOL_POS)  |
    (PCM_DISABLE << PCM_MIXERMODE_POS)  |
    (PCM_DISABLE <<PCM_STUTTERDSBPOL_POS) |
    (PCM_DISABLE <<PCM_STUTTERMODE_POS) |
    (PCM_CH_0<< PCM_CHSEL_POS) |
    (PCM_MASTER<<PCM_MSTSLV_POS) |                      //BT clock
    (PCM_DISABLE << PCM_PCMIRQEN_POS) |
    (PCM_SRC_DPV<<PCM_DATASRC_POS),

//pcm_phy_ctrl
    (PCM_LR_CH_POL_RIGHT_LEFT << PCM_LRCHPOL_POS) |
    (PCM_CLK_FALLING_EDGE << PCM_CLKINV_POS) |
    (PCM_MODE_PCM << PCM_IOM_PCM_POS) |
    (PCM_BUS_SPEED_2048k << PCM_BUSSPEED_LSB) |         //8k sample rate; 2048k = slot_num * sample_rate * bit= 16 * 8k * 16
    (PCM_SLOT_0_1 << PCM_SLOTEN_LSB) |
    (PCM_16_BITS << PCM_WORDSIZE_LSB) |
    (PCM_PUSH_PULL_0 << PCM_DOUTCFG_LSB) |
    (PCM_FSYNC_LF << PCM_FSYNCSHP_LSB),
};

const uint8_t local_feature[] =
{
#if defined(__3M_PACK__)
    0xBF, 0xeE, 0xCD,0xFe,0xdb,0xFd,0x7b,0x87
#else
    0xBF, 0xeE, 0xCD,0xFa,0xdb,0xbd,0x7b,0x87

    //0xBF,0xFE,0xCD,0xFa,0xDB,0xFd,0x73,0x87   // disable simple pairing
#endif
};

const uint8_t local_ex_feature_page2[] =
{
    2,   //page
    0x1f,0x03,0x00,0x00,0x00,0x00,0x00,0x00,   //page 2 feature
};

const uint8_t bt_rf_timing[] =
{
    0x37,// rxpwrupct;
    0x0C,// txpwrdnct;
    0x2B,// txpwrupct;
    0x00,// rxpathdly;
    0x10,// txpathdly;
    0x00,// sync_position;
    0x12,// edr_rxgrd_timeout;
};

// LE 1M:uncoded PHY at 1Mbps
// LE 2M:uncoded PHY at 2Mbps
const uint8_t ble_rf_timing[] =
{
    0x00,  //LE 1M syncposition0
    0x37,  //LE 1M rxpwrup0
    0x07,  //LE 1M txpwrdn0
    0x32,  //LE 1M txpwrup0

    0x00,  //LE 2M syncposition1
    0x37,  //LE 2M rxpwrup1
    0x07,  //LE 2M txpwrdn1
    0x32,  //LE 2M txpwrup1

    0x00,  //coded PHY at 125kbps and 500kbps syncposition2
    0x41,  //coded PHY at 125kbps and 500kbps rxpwrup2
    0x0C,  //coded PHY at 125kbps txpwrdn2
    0x32,  //coded PHY at 125kbp txpwrup2

    0x07,  //coded PHY at 500kbps txpwrdn3
    0x32,  //coded PHY at 500kbps txpwrup3

    0x00,  //LE 1M rfrxtmda0
    0x04,  //LE 1M rxpathdly0
    0x0c,  //LE 1M txpathdly0

    0x00,  //LE 2M rfrxtmda1
    0x02,  //LE 2M rxpathdly1
    0x0d,  //LE 2M txpathdly1

    0x85,  //coded PHY at 125kbps rxflushpathdly2
    0xa0,  //coded PHY at 125kbps rfrxtmda2
    0x14,  //coded PHY at 125kbps rxpathdly2
    0x0c,  //coded PHY at 125kbps txpathdly2

    0x26,  //coded PHY at 500kbps rxflushpathdly3
    0x00,  //coded PHY at 500kbps rfrxtmda3
    0x0c,  //coded PHY at 500kbps txpathdly3

    /*   AoA/AoD Registers */
    0x08,  //rxsampstinst01us
    0x18,  //rxswstinst01us
    0x19,  //txswstinst01us

    0x08,  //rxsampstinst02us
    0x18,  //rxswstinst02us
    0x19,  //txswstinst02us

    0x08,  //rxsampstinst11us
    0x18,  //rxswstinst11us
    0x19,  //txswstinst11us

    0x08,  //rxsampstinst12us
    0x18,  //rxswstinst12us
    0x19,  //txswstinst12us

    0x00,  //rxprimidcntlen
    0x00,  //rxprimantid
    0x00,  //txprimidcntlen
    0x00,  //txprimantid
};

const uint8_t bt_common_setting_1306[64] =
{
    0x03, //trace_level
    0x01, //trace_output
    0x00,0x00, //tports_level
    0x01, //power_control_type
    0x00, //close_loopbacken_flag
    0x00, //force_max_slot
    0x06, //wesco_nego
    0x05, //force_max_slot_in_sco
    0x01, //esco_retx_after_establish
    0x00,0x00, //sco_start_delay
    0x01, //msbc_pcmdout_zero_flag
    0x01, //master_2_poll
    0x01, //pca_disable_in_nosync
    BT53_VERSION, //version_major
    0x01, //version_minor
    0x15, //version_build
    0xb0,0x02, //comp_id
    0x00, //ptt_check
    0x00, //address_reset
    0x00,0x00, //hci_timeout_sleep
    0x00, //key_gen_after_reset
    0x01, //intersys_1_wakeup
    0x05,0x00, //lmp_to_before_complete
    0x08, //fastpcm_interval
    0x01, //lm_env_reset_local_name
    0x00, //detach_directly
    0x00, //clk_off_force_even
    0xc8, //seq_error_num
    0x03, //delay_process_lmp_type
    0x0a, //delay_process_lmp_to 10*100 halt slot
    0x01, //ignore_pwr_ctrl_sm_state
    0x01, //support_hiaudio (sco audio policy)
    0x01, //pscan_coex_en
    0x01, //iscan_coex_en
    0x00, //drift_limited
    0x00,0x7d,0x00,0x00, //init_lsto
    0x1e,0x00,0x00,0x00, //lmp_rsp_to
    0x01, //hec_error_no_update_last_ts
    0x96, //acl_rssi_avg_nb_pkt
    0xff,0xff, //sniff_win_lim
    0x01, //iso_host_to_controller_flow
    0x03, //enable_assert
    0x00, //sw_seq_filter_en
    0x00, //ble_aux_adv_ind_update
    0x00, //sco_start_delay_flag
    0x00, //send_evt_when_wakeup
    0x00, //signal_test_en
    0x06, //ble_cis_conn_event_cnt_distance (in connect interval)
    0x0c, //ble_ci_alarm_init_distance (in half slots)
    0x01, //sync_force_max_slot
    0x00,0x08, //max_hdc_adv_dur in slots
};

const uint8_t bt_sche_setting_1306[37] =
{
    0x14, //ld_sco_switch_timeout
    0x01, //two_slave_sched_en
    0xff, //music_playing_link
    0x00, //dual_slave_tws_en
    0x00, //reconnecting_flag
    0x08, //default_tpoll
    0x08, //acl_slot_in_ibrt_mode
    0x34, //ibrt_start_estb_interval
    0x9c,0x00, //acl_interv_in_ibrt_sco_mode
    0x68,0x00, //acl_interv_in_ibrt_normal_mode
    0x40,0x06, //acl_switch_to_threshold
    0x90,0x01, //stopbitoffthd
    0x0a, //delay_schd_min
    0x02, //stopthrmin
    0x78, //acl_margin
    0x50, //sco_margin
    0x01, //adv_after_sco_retx
    0x01, //ble_wrong_packet_lantency
    0x60,0x00, //sniff_priority_interv_thd
    0x08, //scan_hslot_in_sco 8 half slot in 2ev3
    0x02, //double_pscan_in_sco
    0x06, //reduce_rext_for_sniff_thd
    0x06, //reduce_att_for_sco_thd
    0x01, //reduce_att_space_adjust
    0x01, //bandwidth_check_ignore_retx
    0x00, //check_acl_ble_en
    0x01, //check_sco_ble_en
    0x01, //sch_plan_slave_disable
    0x0a, //sco_anchor_start_add
    0x02, //ble_slot
    0x02, //sniff_max_frm_time
    0x06, //ble_con_in_sco_duration
};

const uint8_t bt_sche_setting_1306_t1[40] =
{
    0x00, //ld_sco_switch_timeout
    0x01, //two_slave_sched_en
    0xff, //music_playing_link
    0x00, //dual_slave_tws_en
    0x00, //reconnecting_flag
    0x08, //default_tpoll
    0x08, //acl_slot_in_ibrt_mode
    0x34, //ibrt_start_estb_interval
    0x9c,0x00, //acl_interv_in_ibrt_sco_mode
    0x68,0x00, //acl_interv_in_ibrt_normal_mode
    0x40,0x06, //acl_switch_to_threshold
    0x90,0x01, //stopbitoffthd
    0x0a, //delay_schd_min
    0x02, //stopthrmin
    0x78, //acl_margin
    0x50, //sco_margin
    0x01, //adv_after_sco_retx
    0x01, //ble_wrong_packet_lantency
    0x60,0x00, //sniff_priority_interv_thd
    0x08, //scan_hslot_in_sco 8 half slot in 2ev3
    0x02, //double_pscan_in_sco
    0x06, //reduce_rext_for_sniff_thd
    0x06, //reduce_att_for_sco_thd
    0x01, //reduce_att_space_adjust
    0x01, //bandwidth_check_ignore_retx
    0x00, //check_acl_ble_en
    0x01, //check_sco_ble_en
    0x01, //sch_plan_slave_disable
    0x0a, //sco_anchor_start_add
    0x02, //ble_slot
    0x02, //sniff_max_frm_time
    0x06, //ble_con_in_sco_duration
    0x01, //unsniff_schdule_more;
    0x64,0x00, //unsniff_trans_interval;
};

const uint8_t bt_ibrt_setting_1306[29] =
{
    0x01, //hci_auto_accept_tws_link_en
    0x00, //send_profile_via_ble
    0x00, //force_rx_error
    0x6c, //sync_win_size hus
    0x16, //magic_cal_bitoff
    0x3f, //role_switch_packet_br
    0x2b, //role_switch_packet_edr
    0x01, //only_support_slave_role
    0x01, //msg_filter_en
    0x00, //relay_sam_info_in_start_snoop
    0x00, //snoop_switch_interval_num
    0x02, //slave_rx_traffic_siam
    0x05, //ibrt_lmp_to
    0x00, //fa_use_twslink_table
    0x00, //fa_use_fix_channel
    0x06, //ibrt_afh_instant_adjust
    0x0c, //ibrt_detach_send_instant
    0x03, //ibrt_detach_receive_instant
    0x03, //ibrt_detach_txcfm_instant
    0x01, //mobile_enc_change
    0x01, //ibrt_auto_accept_sco
    0x01, //ibrt_second_sco_decision
    0x02, //fa_ok_thr_in_sco
    0x02, //fa_ok_thr_for_acl
    0x01, //accep_remote_bt_roleswitch
    0x01, //accept_remote_enter_sniff
    0x0f, //start_ibrt_tpoll
    0x00, //update_all_saved_queue
    0x00, //not_pro_saved_msg_in_ibrt_switch
};

const uint8_t bt_hw_feat_setting_1306[39] =
{
    0x01, //rxheader_int_en
    0x00, //rxdone_bt_int_en
    0x00, //txdone_bt_int_en
    0x01, //rxsync_bt_int_en
    0x01, //rxsync_ble_int_en
    0x00, //pscan_hwagc_flag
    0x00, //iscan_hwagc_flag
    0x00, //sniff_hwagc_flag
    0x00, //blescan_hwagc_flag
    0x00, //bleinit_hwagc_flag
#ifdef __NEW_SWAGC_MODE__
    0x01, //bt_sync_swagc_en
#else
    0x00, //bt_sync_swagc_en
#endif
    0x00, //le_sync_swagc_en
    0x00, //fa_to_en
#ifdef __BES_FA_MODE__
    0x01, //fa_dsb_en
#else
    0x00, //fa_dsb_en
#endif
    0x0F, //fa_to_num
#ifdef __FIX_FA_RX_GAIN___
    0x01, //fa_rxgain
#else
    0xff, //fa_rxgain
#endif
    0x55,0x00,0x00,0x00, //fa_to_type
    0x50,0x00,0x00,0x00, //fa_disable_type
    0xff, //fa_txpwr
    0x00, //ecc_data_flag
    0x00, //softbit_data_flag
    0x00, //rx_noise_chnl_assess_en
    0x14, //rx_noise_thr_good
    0x0a, //rx_noise_thr_bad
    0x00, //snr_chnl_assess_en
    0x14, //snr_good_thr_value
    0x28, //snr_bad_thr_value
    0x00, //rssi_maxhold_record_en
    0x00, //new_agc_adjust_dbm
    0x0f, //ble_rssi_noise_thr
    0x01, //ble_rxgain_adjust_once
    0x00, //trig_open_pcm_flag
    0x00, //sco_sw_mute_en
};

const uint8_t bt_common_setting_1306_t1[12] =
{
    120,//tws_acl_prio_in_sco
    144,//tws_acl_prio_in_normal
    10,//scan_evt_win_slot_in_a2dp
    32,//pscan_gap_slot_in_a2dp
    20,//page_gap_slot_in_a2dp
    0,//send_tws_interval_to_peer
    0,//softbit_enable
    1,//ble_adv_buf_malloc
#ifdef __AFH_ASSESS__
    0,//afh_assess_old
#else
    1,//afh_assess_old
#endif
    1,//cpu_idle_set
    0x40,0x05,//sniff_interval_max
};

const uint8_t bt_txrx_gain_setting[] =
{
    BT_INIT_TX_PWR_IDX,//bt_init_txpwr
    0x01,//bt_inq_rxgain
    0x01,//bt_page_rxgain
    0x06,//bt_page_txpwr
    0x01,//bt_iscan_rxgain
    0xff,//bt_iscan_txpwr
    0x01,//bt_pscan_rxgain
    0xff,//bt_pscan_txpwr
    0x00,//bt_ibrt_rxgain
    0xff,//ble_adv_txpwr
    0x00,//ble_adv_rxgain
    0xff,//ble_bis_txpwr
    0x00,//ble_bis_rxgain
    0xff,//ble_adv_per_txpwr
    0x00,//ble_adv_per_rxgain
    0xff,//ble_testmode_txpwr
    0x00,//ble_testmode_rxgain
    0xff,//ble_con_txpwr
    0x00,//ble_con_rxgain
    0xff,//ble_con_init_txpwr
    0x00,//ble_con_init_rxgain
    0xff,//ble_scan_txpwr
    0x00,//ble_scan_rxgain
};

const uint8_t bt_peer_txpwr_dft_thr[]=
{
    0x64,00,//uint16_t rssi_avg_nb_pkt;
    -1,//rssi_high_thr;
    -2,//rssi_low_thr;
    5,//rssi_below_low_thr;
    50,//unused rssi_interf_thr;
};

const struct rssi_txpower_link_thd tws_link_txpwr_thd =
{
    0x32,//uint16_t rssi_avg_nb_pkt;
    -40,//rssi_high_thr;
    -50,//rssi_low_thr;
    5,//rssi_below_low_thr;
    50,//unused rssi_interf_thr;
};

const uint8_t bt_sw_rssi_setting[] =
{
     0,  //.sw_rssi_en = false
    80,00,00,00,//.link_agc_thd_mobile = 80,
    100,00,00,00,//.link_agc_thd_mobile_time = 100,
    80,00,00,00,//.link_agc_thd_tws = 80,
    100,00,00,00,//.link_agc_thd_tws_time = 100,
    3,//.rssi_mobile_step = 3,
    3,//.rssi_tws_step = 3,
    -100,//.rssi_min_value_mobile = -100,
    -100,//.rssi_min_value_tws = -100,

    0,//.ble_sw_rssi_en = 0,
    80,00,00,00,//.ble_link_agc_thd = 80,
    100,00,00,00,//.ble_link_agc_thd_time = 100,//(in BT half-slots)
    3,//.ble_rssi_step = 3,
    -100,//.ble_rssidbm_min_value = -100,

    1,//.bt_no_sync_en = 1,
    -90,//.bt_link_no_sync_rssi= -90,
    80,00,//.bt_link_no_snyc_thd = 0x50,
    200,00,//.bt_link_no_sync_timeout = 200,

    1,//.ble_no_sync_en = 1,
    -90,//.ble_link_no_sync_rssi= -90,
    20,00,//.ble_link_no_snyc_thd = 20,
    0x20,0x03,//.ble_link_no_sync_timeout = 800,
};

struct bt_cmd_chip_config_t g_bt_drv_btstack_chip_config = {
    HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE,
    HCI_DBG_SET_SCO_SWITCH_CMD_OPCODE,
};

static BTDRV_CFG_TBL_STRUCT  btdrv_cfg_tbl[] = {
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE,sizeof(local_feature),local_feature},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_SETTING_CMD_OPCODE,sizeof(bt_common_setting_1306),bt_common_setting_1306},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_IBRT_SETTING_CMD_OPCODE,sizeof(bt_ibrt_setting_1306),bt_ibrt_setting_1306},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_HW_FEAT_SETTING_CMD_OPCODE,sizeof(bt_hw_feat_setting_1306),bt_hw_feat_setting_1306},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_CUSTOM_PARAM_CMD_OPCODE,189,(uint8_t *)&btdrv_rf_env},
#ifdef _SCO_BTPCM_CHANNEL_
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE,sizeof(sync_config),(uint8_t *)&sync_config},
    {BTDRV_CONFIG_INACTIVE,HCI_DBG_SET_PCM_SETTING_CMD_OPCODE,sizeof(pcm_setting),(uint8_t *)&pcm_setting},
#endif
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_LOCAL_EX_FEATURE_CMD_OPCODE,sizeof(local_ex_feature_page2),(uint8_t *)&local_ex_feature_page2},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_RF_TIMING_CMD_OPCODE,sizeof(bt_rf_timing),(uint8_t *)&bt_rf_timing},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BLE_RF_TIMING_CMD_OPCODE,sizeof(ble_rf_timing),(uint8_t *)&ble_rf_timing},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_RSSI_TX_POWER_DFT_THR_CMD_OPCODE,sizeof(bt_peer_txpwr_dft_thr),(uint8_t *)&bt_peer_txpwr_dft_thr},
    //{BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_SW_RSSI_CMD_OPCODE,sizeof(bt_sw_rssi_setting),(uint8_t *)&bt_sw_rssi_setting},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_BT_COMMON_SETTING_T1_CMD_OPCODE,sizeof(bt_common_setting_1306_t1),(uint8_t *)&bt_common_setting_1306_t1},
    {BTDRV_CONFIG_ACTIVE,HCI_DBG_SET_BT_BLE_TXRX_GAIN_CMD_OPCODE,sizeof(bt_txrx_gain_setting),(uint8_t *)bt_txrx_gain_setting},
};

static void btdrv_hci_init_sleep_wakeup_param(void)
{
    uint32_t wait_26m_cnt = hal_cmu_get_26m_ready_timeout_us();
    uint32_t twosc_cnt = wait_26m_cnt + 200;//us

    struct hci_dbg_set_sleep_para_cmd sleep_wakeup_param;
    sleep_wakeup_param.twrm = twosc_cnt;
    sleep_wakeup_param.twosc = twosc_cnt;
    sleep_wakeup_param.twext = twosc_cnt;
    sleep_wakeup_param.rwip_prog_delay = IP_PROG_DELAY_DFT;
    sleep_wakeup_param.clk_corr = 2;
    sleep_wakeup_param.wait_26m_cnt = wait_26m_cnt;
    sleep_wakeup_param.cpu_idle_en = 1;
    sleep_wakeup_param.wait_26m_cycle = 0;//no use
    sleep_wakeup_param.poweroff_flag = BTC_LP_MODE;

    btdrv_send_cmd(HCI_DBG_SET_WAKEUP_TIME_CMD_OPCODE, sizeof(struct hci_dbg_set_sleep_para_cmd),(const uint8_t *)&sleep_wakeup_param);

    BT_DRV_TRACE(1, "%s,wait24m cycle=%d,twosc=%d",__func__, wait_26m_cnt, twosc_cnt);
}

void btdrv_fa_txpwrup_timing_setting(uint8_t txpwrup)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL2_ADDR, 0xff, 24, txpwrup);
}

void btdrv_fa_rxpwrup_timig_setting(uint8_t rxpwrup)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL2_ADDR, 0xff, 16, rxpwrup);
}

void btdrv_fa_margin_timig_setting(uint8_t margin)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL2_ADDR, 0x1f, 11, margin);
}

void bt_fa_sync_invert_en_setf(uint8_t faacinven)
{
    BTDIGITAL_REG_WR(BT_BES_FACNTL0_ADDR,
                     (BTDIGITAL_REG(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x00020000)) | ((uint32_t)faacinven << 17));
}

void btdrv_fast_lock_en_setf(uint8_t rxonextenden)
{
    BTDIGITAL_REG_WR(BT_BES_FACNTL0_ADDR,
                     (BTDIGITAL_REG(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x40000000)) | ((uint32_t)rxonextenden << 30));
}

void btdrv_fatx_pll_pre_on_en(uint8_t en)
{
    BTDIGITAL_REG_WR(BT_BES_CNTL5_ADDR,
                     (BTDIGITAL_REG(BT_BES_CNTL5_ADDR) & ~((uint32_t)0x80000000)) | ((uint32_t)en << 31));
}

void btdrv_2m_band_wide_sel(uint8_t fa_2m_mode)
{
   BTDIGITAL_REG_WR(BT_BES_FACNTL0_ADDR,
                    (BTDIGITAL_REG(BT_BES_FACNTL0_ADDR) & ~((uint32_t)0x00000001)) | ((uint32_t)fa_2m_mode << 0));
}

void bt_bes_cntl5_null_ack_stop_sco_retx_setf(uint8_t nullackstopscoretx)
{
    BTDIGITAL_REG_WR(BT_BES_CNTL5_ADDR, (BTDIGITAL_REG(BT_BES_CNTL5_ADDR) & ~((uint32_t)0x10000000)) | ((uint32_t)nullackstopscoretx << 28));
}

void btdrv_48m_sys_enable(void)
{
    //BT select 48M system,osc x2
    BTDIGITAL_REG(CMU_CLKREG_ADDR)|=(1<<7);
}

void btdrv_24m_sys_enable(void)
{
    //BT select 24M system,osc
    BTDIGITAL_REG(CMU_CLKREG_ADDR) &= ~(1<<7);
}

void btdrv_fa_corr_mode_setting(bool is_new_corr)
{
    //[1]: corr_preamble
    if(is_new_corr == true)
    {
        BTDIGITAL_REG(0xd0350228) |= (1<<24);//enable new corr
    }
    else
    {
        BTDIGITAL_REG(0xd0350228) &= ~(1<<24);//enable old corr
    }
}

void btdrv_fa_new_mode_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd0350228, 0x7f, 16, (corr_thr & 0x7f));
}

void btdrv_fa_old_mode_corr_thr(uint8_t corr_thr)
{
    BTDIGITAL_REG_SET_FIELD(0xd0350368, 0xf, 4, (corr_thr & 0xf));
}


void btdrv_fa_config_vendor_syncword_en(bool enable)
{
    if(enable)
    {
        BTDIGITAL_REG(BT_BES_FACNTL1_ADDR) |= (1<<31);
    }
    else
    {
        BTDIGITAL_REG(BT_BES_FACNTL1_ADDR) &= ~(1<<31);
    }
}

void btdrv_fa_config_vendor_syncword_content(uint32_t syncword_high, uint32_t syncword_low, uint8_t syncword_len)
{
    if(syncword_low == INVALID_SYNC_WORD || syncword_high == INVALID_SYNC_WORD)
    {
        BT_DRV_TRACE(0,"BT_DRV:vendor syncword invalid");
        return;
    }

    if(syncword_len == FA_SYNCWORD_32BIT)
    {
        BTDIGITAL_REG(BT_BES_FA_SWREG1_ADDR) = syncword_low;
    }
    else if(syncword_len == FA_SYNCWORD_64BIT)
    {
        BTDIGITAL_REG(BT_BES_FA_SWREG1_ADDR) = syncword_high;
        BTDIGITAL_REG(BT_BES_FA_SWREG0_ADDR) = syncword_low;
    }
}

void btdrv_fa_syncword_phy_setting(uint8_t syncword_len)
{
    //set FA RX delay in modem
    BTDIGITAL_REG_SET_FIELD(0xD035020C, 0XFF, 8, PHY_RX_DELAY);
    //set FA TX delay in modem
    BTDIGITAL_REG_SET_FIELD(0xD0350210, 0XFF, 8, PHY_TX_DELAY);

    if (syncword_len == FA_SYNCWORD_32BIT)
    {
        //PHY using 32 bit FA
        BTDIGITAL_REG_SET_FIELD(0xD0350228, 0x3, 29, 0x3);
        BTDIGITAL_REG_SET_FIELD(0xD0350048, 0xf, 24, 0x2);
        BTDIGITAL_REG_SET_FIELD(0xD0350048, 0x1ff, 15, 0x4b);
    }
    else if (syncword_len == FA_SYNCWORD_64BIT)
    {
        //PHY using 64 bit FA
        BTDIGITAL_REG_SET_FIELD(0xD0350228, 0x3, 29, 0x0);
        BTDIGITAL_REG_SET_FIELD(0xD0350048, 0xf, 24, 0x8);
        BTDIGITAL_REG_SET_FIELD(0xD0350048, 0x1ff, 15, 0x64);
    }
    else
    {
        ASSERT(0, "[%s] len=%d", __func__, syncword_len);
    }
}

void btdrv_fa_config_syncword_mode(uint8_t syncword_len)
{
    if(syncword_len == FA_SYNCWORD_32BIT)
    {
        BTDIGITAL_REG(0xd0220c8c) &= ~(1<<28);//syncword 32bit
    }
    else if(syncword_len == FA_SYNCWORD_64BIT)
    {
        BTDIGITAL_REG(0xd0220c8c) |= (1<<28);//syncword 64bit
    }
}

void btdrv_fa_config_tx_gain(bool ecc_tx_gain_en, uint8_t ecc_tx_gain_idx)//false :disable tx gain
{
    if(ecc_tx_gain_en == true)
    {
        BTDIGITAL_REG(BT_BES_CNTL3_ADDR) |= (1<<29);
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL3_ADDR, 0xf, 25, (ecc_tx_gain_idx & 0xf));
    }
    else
    {
        BTDIGITAL_REG(BT_BES_CNTL3_ADDR) &= ~(1<<29);
    }
}

void btdrv_fa_config_rx_gain(bool ecc_rx_gain_en, uint8_t ecc_rx_gain_idx)//false: disable rx gain
{
    if(ecc_rx_gain_en == true)
    {
        BTDIGITAL_REG(BT_BES_CNTL3_ADDR) |= (1<<30);
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL3_ADDR, 0xf, 21, (ecc_rx_gain_idx & 0xf));
    }
    else
    {
        BTDIGITAL_REG(BT_BES_CNTL3_ADDR) &= ~(1<<30);
    }
}

void btdrv_fa_rx_winsz(uint8_t ecc_rx_winsz)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7f, 24, (ecc_rx_winsz & 0x7f));
}

void btdrv_fa_tx_preamble_enable(bool fa_tx_preamble_en)
{
    if(fa_tx_preamble_en)
    {
        BTDIGITAL_REG(0xD0220C18) |= (1<<16); // fa tx 8 bit preamble
    }
    else
    {
        BTDIGITAL_REG(0xD0220C18) &= ~(1<<16);
    }
}

void btdrv_fa_freq_table_enable(bool fa_freq_table_en)
{
    if(fa_freq_table_en)
    {
        BTDIGITAL_REG(0xD0220C80) |= (1<<14);
    }
    else
    {
        BTDIGITAL_REG(0xD0220C80) &= ~(1<<14);
    }
}

void btdrv_fa_multi_mode0_enable(bool fa_multi_mode0_en)
{
    if(fa_multi_mode0_en)
    {
        BTDIGITAL_REG(0xD0220C18) |= (1<<24);
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7f, 24, (0x66 & 0x7f));
    }
    else
    {
        BTDIGITAL_REG(0xD0220C18) &= ~(1<<24);
    }
}

void btdrv_fa_multi_mode1_enable(bool fa_multi_mode1_en, uint8_t fa_multi_tx_count)
{
    if(fa_multi_mode1_en)
    {
        BTDIGITAL_REG(0xD0220C18) |= (1<<25);
        if(fa_multi_tx_count == 2)
        {
            BTDIGITAL_REG(0xD0220C18) |= 0x00800000;
            BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7f, 24, (0x31 & 0x7f));
        }

        if(fa_multi_tx_count == 3)
        {
            BTDIGITAL_REG(0xD0220C18) |= 0x00c00000;
            BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7f, 24, (0x5b & 0x7f));
        }
    }
    else
    {
        BTDIGITAL_REG(0xD0220C18) &= ~(1<<25);
    }
}

void btdrv_fa_freq_table_config(uint32_t fa_freq_table0,uint32_t fa_freq_table1,uint32_t fa_freq_table2)
{
    BTDIGITAL_REG(0xD0220C40)  = fa_freq_table0; // freq 0~31
    BTDIGITAL_REG(0xD0220C44)  = fa_freq_table1; // freq 32~63
    BTDIGITAL_REG(0xD0220C48) |= fa_freq_table2; // freq 64~78
}

void btdrv_fa_basic_config(btdrv_fa_basic_config_t* p_fa_basic_config)
{
    if(p_fa_basic_config != NULL)
    {
        //fast ack TX power gain set
        btdrv_fa_config_tx_gain(p_fa_basic_config->fa_tx_gain_en, p_fa_basic_config->fa_tx_gain_idx);
#ifdef __FIX_FA_RX_GAIN___
        //fix fast ack rx gain
        btdrv_fa_config_rx_gain(p_fa_basic_config->fa_rx_gain_en, p_fa_basic_config->fa_rx_gain_idx);
#endif
        // fa 2M mode select
        btdrv_2m_band_wide_sel(p_fa_basic_config->fa_2m_mode);
        if(p_fa_basic_config->fa_2m_mode)
        {
           btdrv_fa_txpwrup_timing_setting(FA_BW2M_TXPWRUP_TIMING);
           btdrv_fa_rxpwrup_timig_setting(FA_BW2M_RXPWRUP_TIMING);
        }else
        {
           btdrv_fa_txpwrup_timing_setting(FA_TXPWRUP_TIMING);
           btdrv_fa_rxpwrup_timig_setting(FA_RXPWRUP_TIMING);
        }
        //fa syncword mode
        btdrv_fa_config_syncword_mode(p_fa_basic_config->syncword_len);
        //fa phy setting
        btdrv_fa_syncword_phy_setting(p_fa_basic_config->syncword_len);

        //ECC vendor syncword mode
        if(p_fa_basic_config->fa_vendor_syncword_en)
        {
            btdrv_fa_config_vendor_syncword_en(true);
            btdrv_fa_config_vendor_syncword_content(p_fa_basic_config->syncword_high,
                                                    p_fa_basic_config->syncword_low,
                                                    p_fa_basic_config->syncword_len);
        }
        else
        {
            btdrv_fa_config_vendor_syncword_en(false);
        }
        //fa win size
        btdrv_fa_rx_winsz(p_fa_basic_config->fa_rx_winsz);
        //fa corr mode
        btdrv_fa_corr_mode_setting(p_fa_basic_config->is_new_corr);

        if(p_fa_basic_config->is_new_corr)
        {
            btdrv_fa_new_mode_corr_thr(p_fa_basic_config->new_mode_corr_thr);
        }
        else
        {
            btdrv_fa_old_mode_corr_thr(p_fa_basic_config->old_mode_corr_thr);
        }

        if(p_fa_basic_config->fa_tx_preamble_en)
        {
            btdrv_fa_tx_preamble_enable(true);
        }
        else
        {
            btdrv_fa_tx_preamble_enable(false);
        }

        if(p_fa_basic_config->fa_freq_table_en)
        {
            btdrv_fa_freq_table_enable(true);
            btdrv_fa_freq_table_config(p_fa_basic_config->fa_freq_table0,
                                       p_fa_basic_config->fa_freq_table1,
                                       p_fa_basic_config->fa_freq_table2);
        }
        else
        {
            btdrv_fa_freq_table_enable(false);
        }

        if(p_fa_basic_config->fa_multi_mode0_en)
        {
            btdrv_fa_multi_mode0_enable(true);
        }
        else
        {
            btdrv_fa_multi_mode0_enable(false);
        }

        if(p_fa_basic_config->fa_multi_mode1_en)
        {
            btdrv_fa_multi_mode1_enable(true, p_fa_basic_config->fa_multi_tx_count);
        }
        else
        {
            btdrv_fa_multi_mode1_enable(false,p_fa_basic_config->fa_multi_tx_count);
        }
    }
    //enable fa invert
    bt_fa_sync_invert_en_setf(BT_FA_INVERT_EN);
}

#ifdef __FASTACK_ECC_ENABLE__
//ecc usert data
void btdrv_ecc_config_len_mode_sel(uint8_t ecc_len_mode) //only for ecc 1 block
{
    if(ecc_len_mode == ECC_8_BYTE_MODE)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7, 0, 1);
    }
    else if(ecc_len_mode == ECC_16_BYTE_MODE)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7, 0, 0);
    }
    else if(ecc_len_mode == ECC_12_BYTE_MODE)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7, 0, 2);
    }
}

void btdrv_ecc_config_usr_tx_dat_set(ecc_trx_dat_t* p_ecc_trx_dat)
{
    BTDIGITAL_REG(0xd0220cd0) = p_ecc_trx_dat->trx_dat.dat_arr[0];
    BTDIGITAL_REG(0xd0220cd4) = p_ecc_trx_dat->trx_dat.dat_arr[1];
    BTDIGITAL_REG(0xd0220cd8) = p_ecc_trx_dat->trx_dat.dat_arr[2];
    BTDIGITAL_REG(0xd0220cdc) = p_ecc_trx_dat->trx_dat.dat_arr[3];
}

void btdrv_ecc_config_usr_rx_dat_get(ecc_trx_dat_t* p_ecc_trx_dat)
{
    p_ecc_trx_dat->trx_dat.dat_arr[0] = BTDIGITAL_REG(0xd0220cd0);
    p_ecc_trx_dat->trx_dat.dat_arr[1] = BTDIGITAL_REG(0xd0220cd4);
    p_ecc_trx_dat->trx_dat.dat_arr[2] = BTDIGITAL_REG(0xd0220cd8);
    p_ecc_trx_dat->trx_dat.dat_arr[3] = BTDIGITAL_REG(0xd0220cdc);
}


void btdrv_ecc_disable_spec_pkt_type(uint32_t ptk_type) // 1 -> disable FA, ptk type enum in bt_drv_1600_internal.h
{
    BTDIGITAL_REG(0xd0220c30) |= (1<<20);
    BTDIGITAL_REG(0xd0220c30) = (ptk_type & 0xfffff);
}


void btdrv_ecc_config_blk_mode(uint8_t ecc_blk_mode)
{
    if (ecc_blk_mode == ECC_1BLOCK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7ff, 5, 0xef);
    }
    else if (ecc_blk_mode == ECC_2BLOCK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7ff, 5, 0x1de);
    }
    else if (ecc_blk_mode == ECC_3BLOCK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7ff, 5, 0x2cd);
    }
}

void btdrv_ecc_config_modu_mode_acl(uint8_t ecc_modu_mode_acl)
{
    if(ecc_modu_mode_acl == ECC_8PSK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTLX_ADDR, 3, 20, 3); //ECC 8PSK
    }
    else if(ecc_modu_mode_acl == ECC_DPSK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTLX_ADDR, 3, 20, 2); //ECC DPSK
    }
    else if(ecc_modu_mode_acl == ECC_GFSK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTLX_ADDR, 3, 20, 1); //ECC GFSK
    }
}

void btdrv_ecc_config_modu_mode_sco(uint8_t ecc_modu_mode_sco)
{
    if(ecc_modu_mode_sco == ECC_8PSK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 3, 1, 3); //ECC 8PSK
    }
    else if(ecc_modu_mode_sco == ECC_DPSK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 3, 1, 2); //ECC DPSK
    }
    else if(ecc_modu_mode_sco == ECC_GFSK)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 3, 1, 1); //ECC GFSK
    }
}

void btdrv_sco_ecc_enable(bool enable)
{
    if(enable == true)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 1, 8, 1);//sco ecc flag, disabled by default
    }
    else
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 1, 8, 0);//sco ecc flag, disabled by default
    }
}

void btdrv_acl_ecc_enable(bool enable)
{
    if(enable == true)
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL5_ADDR, 1, 1, 1);//acl ecc flag, disabled by default
    }
    else
    {
        BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL5_ADDR, 1, 1, 0);//acl ecc flag, disabled by default
    }
}


void btdrv_ecc_enable(bool enable)
{
    //ecc all flag, initialize all configurations
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL5_ADDR, 1, 1, 0);//acl ecc flag, disabled by default
    //to enable acl ecc, please use btdrv_acl_ecc_enable later
    BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 1, 8, 0);//sco ecc flag, disabled by default
    //to enable sco ecc, please use btdrv_sco_ecc_enable later

    if(enable == true)
    {
        BTDIGITAL_REG(BT_BES_CNTL5_ADDR) |= 1;//ecc enable
#ifdef __FASTACK_SCO_ECC_ENABLE__
        btdrv_sco_ecc_enable(true);
#endif
#ifdef __FASTACK_ACL_ECC_ENABLE__
        btdrv_acl_ecc_enable(true);
#endif
    }
    else
    {
        BTDIGITAL_REG(BT_BES_CNTL5_ADDR) &= ~1;//disable
    }
}

void btdrv_ecc_basic_config(btdrv_ecc_basic_config_t* p_ecc_basic_config)
{
    if((p_ecc_basic_config != NULL) && (p_ecc_basic_config->ecc_mode_enable == true))
    {
        btdrv_ecc_enable(true);
        btdrv_ecc_config_modu_mode_acl(p_ecc_basic_config->ecc_modu_mode_acl);
        btdrv_ecc_config_modu_mode_sco(p_ecc_basic_config->ecc_modu_mode_sco);
        btdrv_ecc_config_blk_mode(p_ecc_basic_config->ecc_blk_mode);
        btdrv_ecc_config_len_mode_sel(p_ecc_basic_config->ecc_len_mode_sel);
    }
}


void btdrv_bid_ecc_2dh5_enable(void)
{
    BTDIGITAL_REG_SET_FIELD(BT_TRIGREG_ADDR, 1, 15, 1);
}

void btdrv_bid_ecc_cnt_act_1(uint8_t regcnteccact1)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL3_ADDR, 0xff, 0, regcnteccact1);
}

void btdrv_bid_ecc_cnt_act_2(uint8_t regcnteccact2)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL3_ADDR, 0xff, 8, regcnteccact2);
}

void btdrv_bid_ecc_frame_len(uint16_t regeccframelen)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL1_ADDR, 0x7ff, 5, regeccframelen);
}

void btdrv_bid_ecc_bid_ecc_blk_en(uint8_t bideccen)
{
    BTDIGITAL_REG_SET_FIELD(BT_BES_ENHPCM_CNTL_ADDR, 1, 5, bideccen);
}

void btdrv_ecc_bid_enable(void)
{
    BTDIGITAL_REG_SET_FIELD(0xd0220c8c, 1, 29, 1);
}

void btdrv_ecc_bid_pkt_2dh_5_reduce_byte(uint8_t pkt2dh5reducebyte)
{
    BTDIGITAL_REG_SET_FIELD(BT_TRIGREG_ADDR, 0x7f, 18, pkt2dh5reducebyte);
}

void btdrv_ecc_bid_fa_rate_mode_acl(uint8_t regfaratemodeacl)
{

    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTLX_ADDR, 3, 20, regfaratemodeacl);
}

void btdrv_bid_ecc_basic_config(void)
{
    btdrv_bid_ecc_2dh5_enable();
    btdrv_bid_ecc_cnt_act_1(205);
    btdrv_bid_ecc_cnt_act_2(250);
    btdrv_bid_ecc_frame_len(690);
    btdrv_ecc_bid_pkt_2dh_5_reduce_byte(78);
    btdrv_bid_ecc_bid_ecc_blk_en(1);
    btdrv_ecc_bid_fa_rate_mode_acl(3);
    btdrv_ecc_bid_enable();
}

void btdrv_ecc_content_config(void)
{
    btdrv_ecc_basic_config_t ecc_config;
    //ECC  config
    ecc_config.ecc_mode_enable = true;
    ecc_config.ecc_modu_mode_acl = ECC_MODU_MODE;
    ecc_config.ecc_modu_mode_sco = ECC_8PSK;
    ecc_config.ecc_blk_mode = ECC_BLK_MODE;
    ecc_config.ecc_len_mode_sel = ECC_8_BYTE_MODE;
    //setting
    btdrv_ecc_basic_config(&ecc_config);

#if __BT_BID_ECC_EN__
    btdrv_bid_ecc_basic_config();
#endif
}
#endif

void btdrv_fast_ack_config(void)
{
    btdrv_fa_basic_config_t fa_config;
    //fast ack config
    fa_config.fa_2m_mode = false;
    fa_config.fa_vendor_syncword_en = false;
    fa_config.syncword_high = INVALID_SYNC_WORD;
    fa_config.syncword_low = INVALID_SYNC_WORD;
    #if defined (CTKD_ENABLE)|| defined (__3M_PACK__)||defined(__FASTACK_ECC_ENABLE__)
    fa_config.syncword_len = FA_SYNCWORD_32BIT;
    #else
    fa_config.syncword_len = FA_SYNCWORD_64BIT;
    #endif
    fa_config.is_new_corr = false;
    fa_config.old_mode_corr_thr = FA_OLD_CORR_VALUE;
    fa_config.new_mode_corr_thr = FA_NEW_CORR_VALUE;
    fa_config.fa_tx_gain_en = true;
    fa_config.fa_tx_gain_idx = FA_FIX_TX_GIAN_IDX;
    fa_config.fa_rx_winsz = FA_RX_WIN_SIZE;
#ifdef __FIX_FA_RX_GAIN___
    fa_config.fa_rx_gain_en = true;
    fa_config.fa_rx_gain_idx = FA_FIX_RX_GIAN_IDX;
#endif
    fa_config.enhance_fa_mode_en = false;
    fa_config.fa_tx_preamble_en = true;
    fa_config.fa_freq_table_en = false;
    fa_config.fa_freq_table0 = INVALID_FA_FREQ_TABLE;
    fa_config.fa_freq_table1 = INVALID_FA_FREQ_TABLE;
    fa_config.fa_freq_table2 = INVALID_FA_FREQ_TABLE;
    fa_config.fa_multi_mode0_en = false;
    fa_config.fa_multi_mode1_en = false;
    fa_config.fa_multi_tx_count = FA_MULTI_TX_COUNT;

    //setting
    btdrv_fa_basic_config(&fa_config);
}

void btdrv_fa_timing_init(void)
{
    btdrv_fa_margin_timig_setting(FA_CNT_PKT_US);
    BTDIGITAL_REG(BT_BES_CNTL5_ADDR) &= ~1;//disable
    BTDIGITAL_REG_SET_FIELD(BT_TRIGREG_ADDR, 7, 15, 0);//disable bid ecc en
    BTDIGITAL_REG_SET_FIELD(BT_TRIGREG_ADDR, 1, 1, 1);//trig_lock_mode
    BTDIGITAL_REG_SET_FIELD(BT_TRIGREG_ADDR, 7, 9, 0);//clear fa mt en
    BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 1, 29, 0);//clear bid ecc en
    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL5_ADDR, 1, 8, 0);//clear sbc en
    BTDIGITAL_REG_SET_FIELD(BT_BES_FACNTL0_ADDR, 1, 30, 0);//clear farx always on
    TRACE(0,"fa timing=0x%x",BTDIGITAL_REG(BT_TRIGREG_ADDR));
}

void btdrv_ecc_config(void)
{
    btdrv_fa_timing_init();

    btdrv_fast_ack_config();
#ifdef __FASTACK_ECC_ENABLE__
    btdrv_ecc_content_config();
#endif
}

void btdrv_afh_monitor_config(void)
{
    BTDIGITAL_REG_SET_FIELD(0xD035035C,3,0,1);//afh_avg_window[1:0]
    BTDIGITAL_REG_SET_FIELD(0xD0350240,1,12,1);//cpx_bypass[12]

    BTDIGITAL_REG_SET_FIELD(0xD035034C,0xff,0,0x40);//afh_settle_down0[7:0]
    BTDIGITAL_REG_SET_FIELD(0xD0350350,0xff,0,0x40);//afh_settle_down1[7:0]

    BTDIGITAL_REG_SET_FIELD(0xD0350354,0xff,0,0x80);//The front_end latency time, bit[8:5] which unit is us,and bit[4:0] which unit is 1/overramp_rate
    BTDIGITAL_REG_SET_FIELD(0xD0350358,0xf,0,6);//The number of channel which estimate the RSSI afh_chan_num[3:0]
    //BTDIGITAL_REG_SET_FIELD(0xD0220C04,1,23,1); //bit23 afh en
    //BTDIGITAL_REG_SET_FIELD(0xD0220C04,1,22,1); //bit22 rxgain dr
    //BTDIGITAL_REG_SET_FIELD(0xD0220C04,0xf,12,3); //bit22 rxgain dr

    //BTDIGITAL_REG_SET_FIELD(0xD0350348,0x3ff,0,0x1a4);//afh ramp up delay
    //BTDIGITAL_REG_SET_FIELD(0xD035020C,0xff,0,0x2e);//rx_startupdel[7:0]
    //btdrv_btradiowrupdn_set_rxpwrup(0x5a);
    //btdrv_fa_rxpwrup_timig_setting(0x5a);
}
static void btdrv_enable_mcu2bt1_isr_en(void)
{
    //[3]:mcu2bt1 isr en
    BTDIGITAL_REG_SET_FIELD(0xD03300a0,1,3,1);
}

static void btdrv_enable_slave_loop_p_en(void)
{
    //[30]:crash after skipping CS update in slave sniff
    BTDIGITAL_REG_SET_FIELD(0xD0220C38,1,30,1);//enable slave loop avoid controll crash
}

static inline void btdrv_rampdn_cnt(void)
{
    BTDIGITAL_REG_SET_FIELD(0xD0220C14, 0x7F, 0, 0x6);
}

void btdrv_digital_config_init(void)
{
    //[21] tx pwr dr
    BTDIGITAL_REG_SET_FIELD(0xD0220C04,1,21,0);//disable tx power dr
    //[22] swagc gain dr
    BTDIGITAL_REG_SET_FIELD(0xD0220C04,1,22,0);//disable swagc gain dr
    // bit[0]:bit[6] rampdn_cnt
    btdrv_rampdn_cnt();

#ifdef __NEW_SWAGC_MODE__
    //sync agc guard update en
    BTDIGITAL_REG_SET_FIELD(0xD0220C2C,1,25,1);
    //sync agc guard update time
    BTDIGITAL_REG_SET_FIELD(0xD0220C2C,0x3f,26,20);
#endif

#ifdef BT_LOG_POWEROFF
    hal_psc_bt_enable_auto_power_down();
#endif
    btdrv_enable_mcu2bt1_isr_en();

    //enable slave loop avoid controll crash
    btdrv_enable_slave_loop_p_en();

    btdrv_afh_monitor_config();

    bt_bes_cntl5_null_ack_stop_sco_retx_setf(1);
    //unlock btc lpo clk & set to manual mode
    BTDIGITAL_REG(0x40080020) = 0xcafe0001;
    BTDIGITAL_REG_SET_FIELD(0x400800e8, 0x1, 9, 1);
#if defined(__HOST_GEN_ECDH_KEY__)
    BTDIGITAL_REG(0xd0220050) = 0x99b1;//close BTC gerate public key
#endif

}

void btdrv_ble_modem_config(void)
{
    //add ble modem config here
}

void btdrv_bt_modem_config(void)
{
    BT_DRV_TRACE(1,"%s",__func__);
    //[16]:iqswap
    BTDIGITAL_REG_SET_FIELD(0xd0350214, 0x1, 16, 1);
    //[18]:txqsigned
    BTDIGITAL_REG_SET_FIELD(0xd0350214, 0x1, 18, 0);
    //[19]:txisigned
    BTDIGITAL_REG_SET_FIELD(0xd0350214, 0x1, 19, 0);
    //bit[3]=1'b1 txdac_fifo_bypass
    BTDIGITAL_REG_SET_FIELD(0xd0350324, 0x1, 3, 1);

    BTDIGITAL_REG_SET_FIELD(0xD0350210, 0xFF, 0, 0x20);
    BTDIGITAL_REG_SET_FIELD(0xD0350254, 0x1F, 0, 0xB);

    BTDIGITAL_REG_SET_FIELD(0xD03500D0, 0xFFFF, 16, 0x1AAA);
    // iq_swap
    BTDIGITAL_REG_SET_FIELD(0xD0350240, 0x1, 2, 1);

    BTDIGITAL_REG_SET_FIELD(0xD0350010, 0xFFFF, 0, 0x0032);
    BTDIGITAL_REG_SET_FIELD(0xD0350010, 0xFFFF, 16, 0x3710);

    BTDIGITAL_REG_SET_FIELD(0xD0350004, 0xFFFF, 0, 0x55);
    BTDIGITAL_REG_SET_FIELD(0xD0350004, 0xFFFF, 16, 0x0291);

    BTDIGITAL_REG_SET_FIELD(0xD035036C, 0xffff, 0, 0x1000);
    BTDIGITAL_REG_SET_FIELD(0xD035036C, 0xffff, 16, 0x0480);

    //digital_tx_gfsk_power_config
    BTDIGITAL_REG_SET_FIELD(0xD0350300, 0x7, 0, 0x5);                //[3:0]     gsg_edr_den
    BTDIGITAL_REG_SET_FIELD(0xd0350308, 0x3ff, 0,  0x1D);            //[9:0]     gsg_psk_nom
    BTDIGITAL_REG_SET_FIELD(0xd0350308, 0x3ff, 10, 0x1D);            //[19:10]   gsg_psk_nom
    //digital_tx_psk_power_config
    BTDIGITAL_REG_SET_FIELD(0xD0350340, 0x7, 0, 0x5);                //[3:0] dsg_edr_den
    BTDIGITAL_REG_SET_FIELD(0xD0350344, 0x1F, 0, 0x1D);              //[9:0] dsg_psk_nom
    //digital_tx_2M3M_gfsk_power_config
    BTDIGITAL_REG_SET_FIELD(0xD0350370, 0x7, 20, 0x5);               //[22:20] gsg_edr_den
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3ff,  0, 0x1D);            //[9:0]   gsg_edr_nom_i
    BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3ff, 10, 0x1D);            //[19:10] gsg_edr_nom_q

    BTDIGITAL_REG_SET_FIELD(0xD0350014, 0xF, 20, 0x8);
    BTDIGITAL_REG_SET_FIELD(0xD0350014, 0x1FF, 0, 0xC8);             // fsync_par_th, 0xD0350014[8:0]=0xC8;
    BTDIGITAL_REG_WR(0xD0350044, 0x33320032);                        // fsync_par_th_ble, 0xD0350044[23:15]=0x64;
    BTDIGITAL_REG_WR(0xD03500DC, 0x04B0A2CC);                        // fsync_par_th_ble2m, 0xD03500DC[28:20]=0x4B;
    // ahi
    BTDIGITAL_REG_WR(0xD0350004, 0x42910065);                        // ahi_k1_ble, 0xD0350004[31:28]=0x4;
    BTDIGITAL_REG_WR(0xD035000C, 0x0200201F);                        // ahi compdl offset, 0xD035000C[6:2]=0x7; ahi lock th, 0xD035000C[13:10]=0x8；
    // psd
    BTDIGITAL_REG_WR(0xD03500C0, 0x00000007);                        // psd filter enable，0xD03500C0[3:0]=0x7;
    BTDIGITAL_REG_WR(0xD03502C4, 0x08E1FAD9);                        // BW1M EDR time，0xD03502C4[10:0]=0x2D9;
    // fa
    BTDIGITAL_REG_WR(0xD0350048, 0x08640064);                        // if 64bit FA: 0xD0350048[31:0]=0x08640064; else if 32bit FA: 0xD0350048[31:0]=0x02320032;

    //IQmis
    BTDIGITAL_REG(0xD0350040) = 0x00810115;
    BTDIGITAL_REG(0xD035005C) = 0x014E1010;
    BTDIGITAL_REG(0xD0350120) = 0x00C80032;

#ifdef __HW_AGC__
    // [HWAGC-DIG]
    BTDIGITAL_REG(0xD03500D8) = 0x00000001;

    BTDIGITAL_REG(0xD03502C0) = 0x00100087;
    BTDIGITAL_REG(0xD03500E0) = 0x01FFFC87;
    BTDIGITAL_REG(0xD0350138) = 0x0FFF0FFF;
    BTDIGITAL_REG(0xD035013C) = 0x00000FFF;
    BTDIGITAL_REG(0xD03500E4) = 0x10000060;
    BTDIGITAL_REG(0xD03500E8) = 0x10004000;
    BTDIGITAL_REG(0xD03500EC) = 0x20000060;
    BTDIGITAL_REG(0xD03500F0) = 0x20004000;
    BTDIGITAL_REG(0xD03500F4) = 0x20000060;
    BTDIGITAL_REG(0xD03500F8) = 0x20004000;
    BTDIGITAL_REG(0xD03500FC) = 0x08000060;
    BTDIGITAL_REG(0xD0350100) = 0x08004000;
    BTDIGITAL_REG(0xD0350104) = 0x001441D4;
    BTDIGITAL_REG(0xD0350108) = 0x005E8120;
    BTDIGITAL_REG(0xD035020C) = 0x27012727;

    // SNR_K=2
    BTDIGITAL_REG(0xD0350018) = 0x00000002;

    // validgen sel&adj, 0xD0350230[19:16]=0x9;
    BTDIGITAL_REG(0xD0350230) = 0x1C0940F0;

    // NEW FSC ON
    BTDIGITAL_REG(0xD03500D8) = 0x00000001;
#endif  //__HW_AGC__
}

uint32_t data_backup_tbl[] =
{
    0xd0350210,
//  0xd0350214,
//  0xD0350240,
    0xD0350254,
    0xd0350324,
};

void btdrv_config_end(void)
{
#ifdef BT_SYSTEM_52M
    btdrv_48m_sys_enable();
#else
    btdrv_24m_sys_enable();
#endif

    BTDIGITAL_REG(0xD0330024) &= (~(1<<5));
    BTDIGITAL_REG(0xD0330024) |= (1<<18);
#ifdef BT_LOG_POWEROFF
    bt_drv_reg_op_data_bakeup_init();
    bt_drv_reg_op_data_backup_write(&data_backup_tbl[0],sizeof(data_backup_tbl)/sizeof(data_backup_tbl[0]));
#endif
}

bool bt_drv_is_support_hci_set_tws_link(void)
{
    bool ret = false;

    enum HAL_CHIP_METAL_ID_T metal_id = hal_get_chip_metal_id();
    if(metal_id >= HAL_CHIP_METAL_ID_1)
    {
        ret = true;
    }

    return ret;
}

void btdrv_hciprocess(void)
{
    enum HAL_CHIP_METAL_ID_T metal_id = hal_get_chip_metal_id();
    BT_DRV_TRACE(1,"%s,metal id=%d", __func__, metal_id);

    btdrv_hci_init_sleep_wakeup_param();

    for(uint8_t i=0; i<sizeof(btdrv_cfg_tbl)/sizeof(btdrv_cfg_tbl[0]); i++){
        //BT other config
        if(btdrv_cfg_tbl[i].is_act == BTDRV_CONFIG_ACTIVE){
            btdrv_send_cmd(btdrv_cfg_tbl[i].opcode,btdrv_cfg_tbl[i].parlen,btdrv_cfg_tbl[i].param);
#ifdef NORMAL_TEST_MODE_SWITCH
            btdrv_delay(20);
#else
            btdrv_delay(1);
#endif
        }
    }

    if(metal_id == HAL_CHIP_METAL_ID_0)
    {
        btdrv_send_cmd(HCI_DBG_SET_BT_SCHE_SETTING_CMD_OPCODE, sizeof(bt_sche_setting_1306), bt_sche_setting_1306);
        btdrv_delay(1);
    }
    else if(metal_id >= HAL_CHIP_METAL_ID_1)
    {
        btdrv_send_cmd(HCI_DBG_SET_BT_SCHE_SETTING_CMD_OPCODE, sizeof(bt_sche_setting_1306_t1), bt_sche_setting_1306_t1);
        btdrv_delay(1);
    }
}

void btdrv_config_init(void)
{
    BT_DRV_TRACE(1,"%s", __func__);

    btdrv_digital_config_init();

    btdrv_bt_modem_config();

    btdrv_ble_modem_config();

    btdrv_ecc_config();
}

bool btdrv_is_ecc_enable(void)
{
    bool ret = false;
#ifdef  __FASTACK_ECC_ENABLE__
    ret = true;
#endif
    return ret;
}

////////////////////////////////////////test mode////////////////////////////////////////////

void btdrv_sleep_config(uint8_t sleep_en)
{
    sleep_param[0] = sleep_en;
    btdrv_send_cmd(HCI_DBG_SET_SLEEP_SETTING_CMD_OPCODE,8,sleep_param);
    btdrv_delay(1);
}

void btdrv_feature_default(void)
{
#ifdef __EBQ_TEST__
    const uint8_t feature[] = {0xBF, 0xFE, 0xCF,0xFe,0xdb,0xFF,0x5b,0x87};
#else
    const uint8_t feature[] = {0xBF, 0xeE, 0xCD,0xFe,0xdb,0xFf,0x7b,0x87};
#endif
    btdrv_send_cmd(HCI_DBG_SET_LOCAL_FEATURE_CMD_OPCODE,8,feature);
    btdrv_delay(1);
}

const struct rssi_txpower_link_thd* btdrv_get_tws_link_txpwr_thd_ptr(void)
{
    return &tws_link_txpwr_thd;
}

void bt_drv_bt_tport_type_config(void)
{
    uint32_t tport_type = 0xb1b1;
#ifdef __BT_DEBUG_TPORTS__
    tport_type = TPORT_TYPE;
#endif
    BTDIGITAL_REG(0xd0220050) = tport_type;

    BTDIGITAL_REG(0xd0340000) = 0xa2220200;

    BT_DRV_TRACE(1,"BT_DRV: tport type=0x%x",tport_type);
}