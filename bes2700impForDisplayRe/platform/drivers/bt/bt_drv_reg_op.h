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
#ifndef __BT_DRV_REG_OP_H__
#define  __BT_DRV_REG_OP_H__

#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "plat_types.h"



#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int8_t rssi;
    uint8_t rxgain;
} rx_agc_t;

/** Bluetooth Address */
typedef struct {
    uint8_t address[6];
} __attribute__ ((packed)) bt_drv_bdaddr_t;

struct ld_afh_env
{
/// RSSI threshold for interference detection (in dBm)
    int8_t rf_rssi_interf_thr;
/// AFH update period (in sec)
    uint8_t afh_update_period;
/// Maximum duration without receiving a packet before considering a channel quality unknown  (in slots) (2 secs)
    uint16_t afh_access_valid_to;
/// Maximum duration without receiving a packet before reintroducing a channel to the map (in slots) (20 secs)
    uint16_t afh_reaccess_to;
/// Maximum counter value for channel assessment
    int8_t afh_access_count_max;
/// Counter threshold to consider a channel good
    int8_t afh_access_count_thr_good;
/// Counter threshold to consider a channel bad
    int8_t afh_access_count_thr_bad;
/// Minimum counter value for channel assessment
    int8_t afh_access_count_min;
};

/// RSSI threshold for interference detection (in dBm)
#define BT_AFH_RSSI_INTERF_THR       (-70)

/// AFH update period (in sec)
#define BT_AFH_UPDATE_PERIOD         4

/// Maximum duration without receiving a packet before considering a channel quality unknown  (in slots) (2 secs)
#define BT_AFH_ASSESS_VALID_TO       0x1900

/// Maximum duration without receiving a packet before reintroducing a channel to the map (in slots) (20 secs)
#define BT_AFH_REASSESS_TO           0x7D00

/// Maximum counter value for channel assessment
#define BT_AFH_ASSESS_COUNT_MAX      4
/// Counter threshold to consider a channel good
#define BT_AFH_ASSESS_COUNT_THR_GOOD 2
/// Counter threshold to consider a channel bad
#define BT_AFH_ASSESS_COUNT_THR_BAD  -2
/// Minimum counter value for channel assessment
#define BT_AFH_ASSESS_COUNT_MIN      -4

#define RETX_NB_0                 0
#define RETX_NB_1                 1
#define RETX_NB_2                 2
#define RETX_NEGO                 3
#ifdef CHIP_BEST2000
#define MIC_BUFF_ADRR_REG        (0xc0000040)
#define RX_BUFF_ADRR        (0xc0000044)
#elif defined(CHIP_BEST1400) || defined(CHIP_BEST1402) || defined(CHIP_BEST2300P)
#define MIC_BUFF_ADRR_REG        (0xc0000058)
#define RX_BUFF_ADRR        (0xc000005c)
#elif defined(CHIP_BEST2300A) || defined(CHIP_BEST1305)
#define MIC_BUFF_ADRR_REG        (0xc0000058)
#define RX_BUFF_INFO_ADRR        (0xc000005c)
#define RX_BUFF_ADRR        (*(volatile uint32_t *)(RX_BUFF_INFO_ADRR))
#endif

//BT acl tx silence
#define TX_SILENCE_ON                   1
#define TX_SILENCE_OFF                  0

struct dbg_set_ebq_test
{
    uint8_t ebq_testmode;
    uint8_t ble_set_privacy_mode;
    uint8_t ll_con_ini_bv2324;
    uint8_t ll_con_ini_bv2021;
    uint8_t ll_ddi_scn_bv26;
    uint8_t ll_sec_adv_bv05;
    uint8_t ll_sync_win_default;
    uint8_t auth_end_notify_host_lk;
    uint8_t ll_sec_adv_bv09;
    uint8_t ll_sec_adv_bv17;
    uint8_t ll_sec_adv_bv1819;
    uint8_t LocEncKeyRefresh;
    uint8_t publickey_check;
    uint8_t Qos;
    uint8_t hv1;
    uint8_t ssr;
    uint8_t aes_ccm_daycounter;
    uint8_t bb_prot_flh_bv01;
    uint8_t bb_prot_arq_bv43;
    uint8_t pause_aes_generate_rand;
};


typedef struct{
    uint32_t clkn;
    int8_t rssi;
} __attribute__ ((__packed__)) CLKN_SER_NUM_T;


int bt_drv_reg_op_currentfreeaclbuf_get(void);
void bt_drv_reg_op_afh_bak_reset(void);
void bt_drv_reg_op_afh_bak_save(uint8_t role, uint16_t mobileHciHandle);
void bt_drv_reg_op_connection_checker(void);
void bt_drv_reg_op_bt_info_checker(void);
uint8_t bt_drv_reg_op_get_controller_tx_free_buffer(void);
uint8_t bt_drv_reg_op_get_controller_ble_tx_free_buffer(void);
bool bt_drv_reg_op_get_dbg_state(void);
void bt_drv_reg_op_crash_dump(void);
void bt_drv_reg_op_lsto_hack(uint16_t hciHandle, uint16_t lsto);
uint16_t bt_drv_reg_op_get_lsto(uint16_t hciHandle);
void bt_drv_reg_op_force_sco_retrans(bool enable);
bool bt_drv_reg_op_read_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val);
uint8_t bt_drv_reg_op_read_bt_3m_rxgain(void);
bool bt_drv_reg_op_read_ble_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val);
void bt_drv_reg_op_set_swagc_mode(uint8_t mode);

uint16_t bt_drv_reg_op_bitoff_getf(int elt_idx);
void bt_drv_reg_op_set_tpoll(uint8_t linkid,uint16_t poll_interval);
uint16_t bt_drv_reg_op_get_tpoll(uint8_t linkid);
uint8_t  bt_drv_reg_op_get_role(uint8_t linkid);
int8_t  bt_drv_reg_op_rssi_correction(int8_t rssi);
void bt_drv_reg_set_rssi_seed(uint32_t seed);
void bt_drv_reg_op_piconet_clk_offset_get(uint16_t connHandle, int32_t *clock_offset, uint16_t *bit_offset);
void bt_drv_reg_op_sw_seq_filter(uint16_t connHandle);
bool bt_drv_reg_op_check_bt_controller_state(void);
void bt_drv_reg_op_dma_tc_clkcnt_get(uint32_t *btclk, uint16_t *btcnt);
void bt_drv_reg_op_enable_dma_tc(uint8_t adma_ch, uint32_t dma_base);
void bt_drv_reg_op_disable_dma_tc(uint8_t adma_ch, uint32_t dma_base);
void bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(uint32_t *btclk, uint16_t *btcnt, uint8_t adma_ch, uint32_t dma_base);

void bt_drv_reg_op_clean_flags_of_ble_and_sco(void);
void bt_drv_reg_op_sco_txfifo_reset(uint16_t codec_id);
void bt_drv_reg_op_ibrt_retx_att_nb_set(uint8_t retx_nb);
void bt_drv_reg_op_acl_tx_type_set(uint16_t hciHandle, uint8_t slot_sel);
void bt_drv_reg_op_acl_tx_type_trace(uint16_t hciHandle);
uint8_t bt_drv_reg_op_acl_tx_type_get(uint16_t hciHandle, uint8_t* br_type, uint8_t* edr_type);
void bt_drv_reg_op_ibrt_env_reset(void);
void btdrv_reg_op_set_private_tws_poll_interval(uint16_t poll_interval, uint16_t poll_interval_in_sco);
void btdrv_reg_op_enable_private_tws_schedule(bool enable);
void btdrv_reg_op_set_tws_link_duration(uint8_t slot_num);
void bt_drv_reg_op_decrease_tx_pwr_when_reconnect(bool enable);
void bt_drv_reg_op_controller_mem_log_config(void);
void bt_drv_reg_op_clear_ibrt_snoop_status(void);
uint8_t bt_drv_reg_op_linkcntl_aclltaddr_getf(uint16_t conhdl);
uint8_t bt_drv_reg_op_sync_bt_param(uint8_t* param, uint16_t len);
uint32_t bt_drv_reg_op_get_lc_env_ptr(uint16_t conhdl, uint8_t type);
void bt_drv_reg_op_set_tws_link_id(uint8_t link_id);
void bt_drv_reg_op_hack_max_slot(uint8_t link_id,uint8_t slot);
void bt_drv_reg_op_fa_set(uint8_t en);
bool bt_drv_reg_op_check_seq_pending_status(void);
void bt_drv_reg_op_clear_seq_pending_status(void);
void bt_dev_clear_verison_receive_flag(uint8_t linkid);
void bt_drv_reg_op_fastack_status_checker(uint16_t conhdl);
void bt_drv_reg_op_set_link_policy(uint8_t linkid, uint8_t policy);
uint8_t bt_drv_reg_op_lmp_sco_hdl_get(uint16_t sco_handle);
bool bt_drv_reg_op_lmp_sco_hdl_set(uint16_t sco_handle, uint8_t lmp_hdl);
void bt_drv_reg_op_controller_state_checker(void);
void bt_drv_reg_op_controller_ble_flow_checker(void);
void bt_drv_reg_op_force_set_lc_state(uint16_t conhdl, uint8_t state);
uint8_t bt_drv_reg_op_force_get_lc_state(uint16_t conhdl);
void bt_drv_digital_config_for_ble_adv(bool en);
void bt_drv_reg_op_fa_gain_direct_set(uint8_t gain_idx);
uint8_t bt_drv_reg_op_fa_gain_direct_get(void);
void bt_drv_reg_op_dgb_link_gain_ctrl_set(uint16_t connHandle, uint8_t bt_ble_mode, uint8_t gain_idx, uint8_t enable);
void bt_drv_reg_op_dgb_link_gain_ctrl_clear(uint16_t connHandle, uint8_t bt_ble_mode);
void bt_drv_reg_op_dgb_link_gain_ctrl_init(void);
void bt_drv_reg_op_rx_gain_fix(uint16_t connHandle, uint8_t bt_ble_mode, uint8_t gain_idx, uint8_t enable, uint8_t table_idx);
void bt_drv_reg_op_afh_follow_en(bool enable, uint8_t be_followed_link_id, uint8_t follow_link_id);
/// Index for DM1 packet type (used in both BR and EDR)
#define DM1_IDX         0
/// Enhanced Data Rate packet types indexes (classified by packet size)
#define DH1_2_IDX       1
#define DH1_3_IDX       2
#define DH3_2_IDX       3
#define DH3_3_IDX       4
#define DH5_2_IDX       5
#define DH5_3_IDX       6

#define USE_1_SLOT_PACKET       0x02
#define USE_3_SLOT_PACKET       0x04
#define USE_5_SLOT_PACKET       0x06

///Sniffer sco status
#define SNIFFER_SCO_STOP                  0
#define SNIFFER_SCO_START                 1
#define SNIFFER_SCO_RESTART               2
#define SNIFFER_SCO_RESUME_AFTER_TO       3
#define SNIFFER_ACL_DISCONNECT_STOP_SCO   4
#define SNIFFER_IBRT_START                5
#define SNIFFER_IBRT_STOP                 6
#define IDEL_CHECK                        0xff

enum REG_OP
{
    READ_OP,
    WRITE_OP,
};

enum LC_ENV_TAG
{
    LC_LINK_TAG,
    LC_INFO_TAG,
    LC_ENC_TAG,
    LC_AFH_TAG,
    LC_SP_TAG,
    LC_EPR_TAG,
    LC_EPC_TAG,
};

void bt_drv_reg_op_hw_sw_agc_select(uint8_t agc_mode);
void bt_drv_reg_op_force_set_sniff_att(uint16_t conhdle);
void bt_drv_reg_op_set_music_link(uint8_t link_id);
void bt_drv_reg_op_sco_fifo_reset(void);
void bt_drv_reg_sco_fifo_dump(void);
uint8_t bt_drv_reg_op_get_sync_id_op(uint8_t op, uint8_t sync_id);
void bt_drv_reg_op_set_agc_thd(bool isMaster, bool isSco);
void bt_drv_reg_op_reset_agc_thd(void);
void bt_drv_reg_op_global_symbols_init(void);
void bt_drv_tx_pwr_init(void);
void bt_drv_tx_pwr_init_for_testmode(void);
void bt_drv_reg_op_set_ibrt_auto_accept_sco(bool en);
void btdrv_reg_op_disable_bt_wifi_coex_signal(bool disable);
void btdrv_reg_op_set_bt_acl_tdd_percent(uint8_t percent,uint8_t duration);
void bt_drv_reg_op_update_dbg_state(void);
void bt_drv_reg_op_set_ibrt_reject_sniff_req(bool en);
void bt_drv_reg_op_set_ibrt_second_sco_decision(uint8_t value);
uint8_t bt_drv_reg_op_get_esco_nego_airmode(uint8_t sco_link_id);
uint8_t bt_drv_reg_op_get_sync_id_by_conhdl(uint16_t conhdl);
void bt_drv_reg_op_hci_vender_ibrt_ll_monitor(uint8_t* ptr, uint16_t* p_sum_err,uint16_t* p_rx_total);
uint32_t bt_drv_reg_op_get_host_ref_clk(void);
void bt_drv_reg_op_ecc_softbit_process(uint16_t* p_conn_handle1,uint16_t* p_conn_handle2, uint16_t length, uint8_t *data);
void bt_drv_reg_op_ebq_test_setting(void);
void bt_drv_reg_op_hw_spi_en_setf(int elt_idx, uint8_t hwspien);
void bt_drv_enhance_fa_mode(bool enable);
void bt_drv_reg_op_swagc_mode_set(uint8_t mode);
void bt_drv_reg_op_key_gen_after_reset(bool enable);
uint8_t bt_drv_reg_op_bt_sync_swagc_en_get(void);
#ifdef PCM_PRIVATE_DATA_FLAG
void bt_drv_reg_op_sco_pri_data_init();
#endif
void bt_drv_reg_op_init_rssi_setting(void);
void bt_drv_reg_op_init_sleep_para(void);
void bt_drv_reg_op_data_bakeup_init(void);
void bt_drv_reg_op_data_backup_write(const uint32_t *ptr,uint32_t cnt);
void bt_drv_reg_op_init_hwagc_corr_gain(void);
void bt_drv_reg_op_low_txpwr_set(uint8_t enable, uint8_t factor, uint8_t bt_or_ble, uint8_t link_id);

#ifdef __SW_TRIG__
uint16_t bt_drv_reg_op_rxbit_1us_get(uint16_t conhdl);
void btdrv_reg_op_sw_trig_tg_finecnt_set(uint16_t tg_bitcnt, uint8_t trig_route);
uint16_t btdrv_reg_op_sw_trig_tg_finecnt_get(uint8_t trig_route);
void btdrv_reg_op_sw_trig_tg_clkncnt_set(uint32_t num, uint8_t trig_route);
void btdrv_sw_trig_en_set(uint8_t trig_route);
void btdrv_sw_trig_disable_set(uint8_t trig_route);
#endif
void bt_drv_reg_op_set_btpcm_trig_flag(bool flag);
void bt_drv_reg_op_multi_ibrt_music_config(uint8_t* link_id, uint8_t* active, uint8_t num);
void bt_drv_iq_tab_set_normalmode(void);
void bt_drv_iq_tab_set_value(uint32_t val);
void bt_drv_reg_op_power_adjust_onoff(uint8_t en);
uint32_t bt_drv_lp_clk_get();

void btdrv_reg_op_vco_test_start(uint8_t chnl);
void btdrv_reg_op_vco_test_stop(void);
void btdrv_reg_op_syn_set_tg_ticks_master_role(void);
void btdrv_reg_op_syn_set_tg_ticks_slave_role(uint32_t num);
uint32_t btdrv_reg_op_syn_get_curr_ticks(void);
uint32_t btdrv_reg_op_syn_get_cis_curr_time(void);
uint32_t btdrv_reg_op_get_syn_trigger_codec_en(void);
uint32_t btdrv_reg_op_get_trigger_ticks(void);
void btdrv_reg_op_syn_set_tg_ticks_linkid(uint8_t link_id);
void btdrv_reg_op_enable_playback_triggler(uint8_t triggle_mode);
void btdrv_reg_op_play_trig_mode_set(uint8_t mode);
void btdrv_reg_op_disable_playback_triggler(void);
void btdrv_reg_op_pcm_enable(void);
void btdrv_reg_op_pcm_disable(void);
#ifdef PCM_FAST_MODE
void btdrv_reg_op_open_pcm_fast_mode_enable(void);
void btdrv_reg_op_open_pcm_fast_mode_disable(void);
#endif
void btdrv_reg_op_set_btpcm_mute_pattern(uint16_t pattern);
#if defined(CVSD_BYPASS)
void btdrv_reg_op_cvsd_bypass_enable(void);
#endif
int8_t btdrv_reg_op_txpwr_idx_to_rssidbm(uint8_t txpwr_idx);
void bt_drv_reg_op_set_nosig_sch_flag(uint8_t enable);
void bt_drv_reg_op_afh_assess_en(bool en);
void btdrv_reg_op_bts_to_bt_time(uint32_t bts, uint32_t* p_hs, uint16_t* p_hus);
uint32_t btdrv_reg_op_bt_time_to_bts(uint32_t hs, uint16_t hus);
int bt_drv_reg_op_acl_chnmap(uint16_t hciHandle, uint8_t *chnmap, uint8_t chnmap_len);

int btdrv_host_gpio_tport_open(void);
int btdrv_gpio_port_set(int port);
int btdrv_gpio_tport_clr(int port);
int32_t bt_drv_reg_op_get_clkoffset(uint16_t linkid);
void btdrv_regop_dump_btpcm_reg(void);
void btdrv_regop_force_preferred_rate_send(uint16_t conhdl);
void btdrv_regop_host_set_txpwr(uint16_t acl_connhdl, uint8_t txpwr_idx);

void bt_drv_i2v_disable_sleep_for_bt_access(void);
void bt_drv_i2v_enable_sleep_for_bt_access(void);

int8_t bt_drv_reg_op_get_tx_pwr_dbm(uint16_t conhdl);
void bt_drv_reg_op_get_pkt_ts_rssi(uint16_t connhdl, CLKN_SER_NUM_T* buf);
void bt_drv_reg_op_rssi_adjust_mode_setting_for_test_mode(uint8_t enable);
bool bt_drv_reg_op_check_link_exist(uint16_t con_hdl);
void bt_drv_reg_op_setting_wesco(uint8_t wesco);
uint32_t btdrv_reg_op_big_anchor_timestamp(uint8_t link_id);

void bt_drv_reg_op_ble_audio_config_ramrun_init(void);
bool bt_drv_reg_op_read_fa_rssi_in_dbm(rx_agc_t* rx_val);
void btdrv_reg_op_sw_trig_route_extra_set(uint8_t trig_route);
uint8_t bt_drv_reg_op_get_max_acl_nb(void);
void bt_drv_reg_op_set_btclk_seed(uint32_t seed);
uint8_t bt_drv_reg_op_get_sco_type(uint8_t sco_link_id);

void bt_drv_reg_op_set_page_device_support_le_audio(bool support);
void bt_drv_reg_op_sec_con_base_on_le_aud(bool en);
int8_t btdrv_ultra_lowpwr_rssidbm(int8_t adv_tx_pwr, uint8_t den_step);
void bt_drv_reg_op_set_sleep_duration(uint32_t tws_slp_dura_dec, uint32_t normal_slp_dura_inc);
#ifdef __cplusplus
}
#endif

#endif
