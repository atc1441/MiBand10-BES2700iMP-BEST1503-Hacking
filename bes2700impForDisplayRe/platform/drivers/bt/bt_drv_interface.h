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
#ifndef __BT_DRV_INTERFACE_H__
#define  __BT_DRV_INTERFACE_H__

#include "stdint.h"
#include "stdbool.h"

//LMP Opcodes
#define LMP_NAME_REQ_OPCODE                1
#define LMP_NAME_RES_OPCODE                2
#define LMP_ACCEPTED_OPCODE                3
#define LMP_NOT_ACCEPTED_OPCODE            4
#define LMP_CLK_OFF_REQ_OPCODE             5
#define LMP_CLK_OFF_RES_OPCODE             6
#define LMP_DETACH_OPCODE                  7
#define LMP_INRAND_OPCODE                  8
#define LMP_COMBKEY_OPCODE                 9
#define LMP_UNITKEY_OPCODE                10
#define LMP_AURAND_OPCODE                 11
#define LMP_SRES_OPCODE                   12
#define LMP_TEMPRAND_OPCODE               13
#define LMP_TEMPKEY_OPCODE                14
#define LMP_ENC_MODE_REQ_OPCODE           15
#define LMP_ENC_KEY_SIZE_REQ_OPCODE       16
#define LMP_START_ENC_REQ_OPCODE          17
#define LMP_STOP_ENC_REQ_OPCODE           18
#define LMP_SWITCH_REQ_OPCODE             19
#define LMP_HOLD_OPCODE                   20
#define LMP_HOLD_REQ_OPCODE               21
#define LMP_SNIFF_REQ_OPCODE              23
#define LMP_UNSNIFF_REQ_OPCODE            24
#define LMP_PARK_REQ_OPCODE               25
#define LMP_SET_BSWIN_OPCODE              27
#define LMP_MODIF_BEACON_OPCODE           28
#define LMP_UNPARK_BD_REQ_OPCODE          29
#define LMP_UNPARK_PM_REQ_OPCODE          30
#define LMP_INCR_PWR_REQ_OPCODE           31
#define LMP_DECR_PWR_REQ_OPCODE           32
#define LMP_MAX_PWR_OPCODE                33
#define LMP_MIN_PWR_OPCODE                34
#define LMP_AUTO_RATE_OPCODE              35
#define LMP_PREF_RATE_OPCODE              36
#define LMP_VER_REQ_OPCODE                37
#define LMP_VER_RES_OPCODE                38
#define LMP_FEATS_REQ_OPCODE              39
#define LMP_FEATS_RES_OPCODE              40
#define LMP_QOS_OPCODE                    41
#define LMP_QOS_REQ_OPCODE                42
#define LMP_SCO_LINK_REQ_OPCODE           43
#define LMP_RMV_SCO_LINK_REQ_OPCODE       44
#define LMP_MAX_SLOT_OPCODE               45
#define LMP_MAX_SLOT_REQ_OPCODE           46
#define LMP_TIMING_ACCU_REQ_OPCODE        47
#define LMP_TIMING_ACCU_RES_OPCODE        48
#define LMP_SETUP_CMP_OPCODE              49
#define LMP_USE_SEMI_PERM_KEY_OPCODE      50
#define LMP_HOST_CON_REQ_OPCODE           51
#define LMP_SLOT_OFF_OPCODE               52
#define LMP_PAGE_MODE_REQ_OPCODE          53
#define LMP_PAGE_SCAN_MODE_REQ_OPCODE     54
#define LMP_SUPV_TO_OPCODE                55
#define LMP_TEST_ACTIVATE_OPCODE          56
#define LMP_TEST_CTRL_OPCODE              57
#define LMP_ENC_KEY_SIZE_MASK_REQ_OPCODE  58
#define LMP_ENC_KEY_SIZE_MASK_RES_OPCODE  59
#define LMP_SET_AFH_OPCODE                60
#define LMP_ENCAPS_HDR_OPCODE             61
#define LMP_ENCAPS_PAYL_OPCODE            62
#define LMP_SP_CFM_OPCODE                 63
#define LMP_SP_NB_OPCODE                  64
#define LMP_DHKEY_CHK_OPCODE              65
#define LMP_PAUSE_ENC_AES_REQ_OPCODE      66

///LMP Escape 4 Extended Opcodes
#define LMP_ACCEPTED_EXT_EXTOPCODE         1
#define LMP_NOT_ACCEPTED_EXT_EXTOPCODE     2
#define LMP_FEATS_REQ_EXT_EXTOPCODE        3
#define LMP_FEATS_RES_EXT_EXTOPCODE        4
#define LMP_CLK_ADJ_EXTOPCODE              5
#define LMP_CLK_ADJ_ACK_EXTOPCODE          6
#define LMP_CLK_ADJ_REQ_EXTOPCODE          7
#define LMP_PKT_TYPE_TBL_REQ_EXTOPCODE    11
#define LMP_ESCO_LINK_REQ_EXTOPCODE       12
#define LMP_RMV_ESCO_LINK_REQ_EXTOPCODE   13
#define LMP_CH_CLASS_REQ_EXTOPCODE        16
#define LMP_CH_CLASS_EXTOPCODE            17
#define LMP_SSR_REQ_EXTOPCODE             21
#define LMP_SSR_RES_EXTOPCODE             22
#define LMP_PAUSE_ENC_REQ_EXTOPCODE       23
#define LMP_RESUME_ENC_REQ_EXTOPCODE      24
#define LMP_IO_CAP_REQ_EXTOPCODE          25
#define LMP_IO_CAP_RES_EXTOPCODE          26
#define LMP_NUM_COMPARISON_FAIL_EXTOPCODE 27
#define LMP_PASSKEY_FAIL_EXTOPCODE        28
#define LMP_OOB_FAIL_EXTOPCODE            29
#define LMP_KEYPRESS_NOTIF_EXTOPCODE      30
#define LMP_PWR_CTRL_REQ_EXTOPCODE        31
#define LMP_PWR_CTRL_RES_EXTOPCODE        32
#define LMP_PING_REQ_EXTOPCODE            33
#define LMP_PING_RES_EXTOPCODE            34

// CARE: round-off error
#define BT_CLK_UNIT 312.5       //us
#define BT_CLK_UNIT_2X 625       //us
#define BT_CLK_UNIT_10X 3125       //us
#define US_TO_BTCLKS(us)    ((uint64_t)(us) * 2 / BT_CLK_UNIT_2X)
#define BTCLKS_TO_US(n)     ((uint64_t)(n) * BT_CLK_UNIT_2X / 2)
#define HALF_SLOT_INV(x)  (HALF_SLOT_SIZE - (x) - 1)

#define BD_NAME_SIZE        0xF8

#define DUT_CONNECT_STATUS_CONNECTED (1)
#define DUT_CONNECT_STATUS_DISCONNECTED (2)

#define BT_TRIG_NONE_ROLE      (0)
#define BT_TRIG_MASTER_ROLE    (1)
#define BT_TRIG_SLAVE_ROLE     (2)

//LE RX test PHY type
#define LE_RX_TEST_1M_PHY 0x01
#define LE_RX_TEST_2M_PHY 0x02
#define LE_RX_TEST_LE_CODED_PHY 0x03
//LE RX test  Modulation_Index
#define LE_TEST_STANDARD_IDX 0x00
#define LE_TEST_STABLE_IDX 0x01

//LE TX test PHY type
#define LE_TX_TEST_1M_PHY 0x01
#define LE_TX_TEST_2M_PHY 0x02
#define LE_TX_TEST_LE_CODED_S_8_PHY 0x03
#define LE_TX_TEST_LE_CODED_S_2_PHY 0x04
//LE TX test Packet_Payload type
#define LE_TX_TEST_PRBS9_PAYLOAD 0x00
#define LE_TX_TEST_11110000_PAYLOAD 0x01
#define LE_TX_TEST_10101010_PAYLOAD 0x02
#define LE_TX_TEST_PRBS15_PAYLOAD 0x03
#define LE_TX_TEST_11111111_PAYLOAD 0x04
#define LE_TX_TEST_00000000_PAYLOAD 0x05
#define LE_TX_TEST_00001111_PAYLOAD 0x06
#define LE_TX_TEST_01010101_PAYLOAD 0x07

//BT nonsignaling test
#define NONSIGNALING_TEST_OK 0x00
#define TX_NUM_UNDEFINE 0xffffffff
#define HOPSINGLE               0x00
#define HOPUSA                  0x01

#define WHITENING_MODE_ON 0x00
#define WHITENING_MODE_OFF 0x01

#define BT_EDR_ENABLE 1
#define BT_EDR_DISABLE 0

#define BT_NONSIGNALING_00_PAYLOAD 0x00
#define BT_NONSIGNALING_FF_PAYLOAD 0x01
#define BT_NONSIGNALING_55_PAYLOAD 0x02
#define BT_NONSIGNALING_0F_PAYLOAD 0x03
#define BT_NONSIGNALING_PRBS9_PAYLOAD 0x04

/// Packet type code interpretation possibilities BB:6.5
#define ID_NUL_TYPE     0x0
#define POLL_TYPE       0x1
#define FHS_TYPE        0x2
#define DM1_TYPE        0x3
#define DH1_TYPE        0x4
#define DH1_2_TYPE      0x4
#define DH1_3_TYPE      0x8
#define HV1_TYPE        0x5
#define HV2_TYPE        0x6
#define EV3_2_TYPE      0x6
#define HV3_TYPE        0x7
#define EV3_TYPE        0x7
#define EV3_3_TYPE      0x7
#define DV_TYPE         0x8
#define AUX1_TYPE       0x9
#define DM3_TYPE        0xA
#define DH3_TYPE        0xB
#define DH3_2_TYPE      0xA
#define DH3_3_TYPE      0xB
#define EV4_TYPE        0xC
#define EV5_2_TYPE      0xC
#define EV5_TYPE        0xD
#define EV5_3_TYPE      0xD
#define DM5_TYPE        0xE
#define DH5_TYPE        0xF
#define DH5_2_TYPE      0xE
#define DH5_3_TYPE      0xF

#define FHS_PACKET_SIZE         18
#define DM1_PACKET_SIZE         17
#define DH1_PACKET_SIZE         27
#define DH1_2_PACKET_SIZE       54
#define DH1_3_PACKET_SIZE       83
#define DV_ACL_PACKET_SIZE      9
#define DM3_PACKET_SIZE         121
#define DH3_PACKET_SIZE         183
#define DH3_2_PACKET_SIZE       367
#define DH3_3_PACKET_SIZE       552
#define DM5_PACKET_SIZE         224
#define DH5_PACKET_SIZE         339
#define DH5_2_PACKET_SIZE       679
#define DH5_3_PACKET_SIZE       1021
#define AUX1_PACKET_SIZE        29

#define HV1_PACKET_SIZE         10
#define HV2_PACKET_SIZE         20
#define HV3_PACKET_SIZE         30
#define EV3_PACKET_SIZE         30
#define EV3_2_PACKET_SIZE       60
#define EV3_3_PACKET_SIZE       90
#define EV4_PACKET_SIZE        120
#define EV5_PACKET_SIZE        180
#define EV5_2_PACKET_SIZE      360
#define EV5_3_PACKET_SIZE      540


enum BT_WORK_MODE_T{
   BT_A2DP_WORK_MODE = 0,
   BT_HFP_WORK_MODE,
   BT_IDLE_MODE,
   BT_NONE_MODE,
};

enum BT_AGC_MODE_T{
   BT_AGC_MODE_SW = 0,
   BT_AGC_MODE_HW,
   BT_AGC_MODE_NONE,
};

enum BT_RSSI_THR_TYPE_T{
   READ_MOBILE_TYPE = 0,
   READ_TWS_TYPE,
};

/// BT driver customer config structure
struct btdrv_customer_rf_config_t
{
    bool config_xtal_en;
    bool config_tx_pwr_en;
    uint16_t xtal_cap_val;
    int8_t bt_tx_idx7_pwr;
    int8_t le_tx_idx7_pwr;
    int8_t bt_tx_page_pwr;
    int8_t le_tx_idx6_pwr;
    int8_t bt_tx_page_high_pwr;
    int8_t bt_tx_max_pwr;
    int8_t le_tx_max_pwr;
    int8_t bt_tx_idx4_pwr;
    int8_t le_tx_idx4_pwr;
    int8_t bt_tx_idx3_pwr;
    int8_t le_tx_idx3_pwr;
    int8_t bt_tx_idx2_pwr;
    int8_t le_tx_idx2_pwr;
    int8_t bt_tx_idx1_pwr;
    int8_t le_tx_idx1_pwr;
    int8_t bt_tx_idx0_pwr;
    int8_t le_tx_idx0_pwr;
};

//add bt magic call back function here
struct bt_cb_tag
{
    ///switch various agc call back function
    void (*bt_switch_agc)(enum BT_WORK_MODE_T);
};

struct rssi_txpower_link_thd
{
    uint16_t    rssi_avg_nb_pkt;
    int8_t      rssi_high_thr;
    int8_t      rssi_low_thr;
    int8_t      rssi_below_low_thr;
    int8_t      rssi_interf_thr;
};
typedef int (* btstack_send_hci_cmd_t)(uint16_t , uint8_t *, uint8_t );

#define btdrv_enable_sleep_checker(isEnable)   beshci_enable_sleep_checker(isEnable)

#define HCI_LINK_INDEX_INVALID  (0xFF)
#define btdrv_is_link_index_valid(index)  (index != HCI_LINK_INDEX_INVALID)

#ifdef __cplusplus
extern "C" {
#endif
void btdrv_hciopen(void);
void btdrv_hcioff(void);
void btdrv_start_bt(void);
void btdrv_SendData(const uint8_t *buff,uint8_t len);
void bt_drv_config_after_hci_reset(void);
void btdrv_rf_init_ext(void);

void btdrv_stop_bt(void);
void btdrv_testmode_start(void);
void btdrv_testmode_end(void);
void btdrv_enable_dut(void);
void btdrv_hci_reset(void);
void btdrv_enable_nonsig_tx(uint8_t index);
void btdrv_enable_nonsig_rx(uint8_t index);
void btdrv_vco_test_start(uint8_t chnl);
void btdrv_vco_test_stop(void);
void bt_drv_calib_open(void);
void bt_drv_check_calib(void);
void bt_drv_calib_close(void);
int bt_drv_calib_result_porc(uint32_t *capval);
void bt_drv_calib_rxonly_porc(void);
void btdrv_write_localinfo(const char *name, uint8_t len, const uint8_t *addr);
void btdrv_disable_scan(void);

void btdrv_sleep_config(uint8_t sleep_en);
void btdrv_load_sleep_config(uint8_t* config);
void btdrv_feature_default(void);
void btdrv_test_mode_addr_set(void);

uint32_t btdrv_rf_get_max_xtal_tune_ppb(void);
uint32_t btdrv_rf_get_xtal_tune_factor(void);
void btdrv_rf_init_xtal_fcap(uint32_t fcap);
uint32_t btdrv_rf_get_init_xtal_fcap(void);
uint32_t btdrv_rf_get_xtal_fcap(void);
void btdrv_rf_set_xtal_fcap(uint32_t fcap, uint8_t is_direct);
int btdrv_rf_xtal_fcap_busy(uint8_t is_direct);
uint32_t btdrv_rf_bit_offset_get(void);
void btdrv_uart_bridge_loop(void);
void btdrv_testmode_data_overide(void);
uint16_t btdrv_rf_bitoffset_get(uint8_t conidx);
void  btdrv_syn_clr_trigger(uint8_t trig_route);
uint32_t btdrv_rf_bit_offset_get(void);
uint32_t btdrv_syn_get_curr_ticks(void);
uint32_t btdrv_syn_get_cis_curr_time(void);
uint32_t bt_syn_get_curr_ticks(uint16_t conhdl);
int32_t bt_syn_get_offset_ticks(uint16_t conhdl);
void bt_syn_cancel_tg_ticks(uint8_t trig_route);
void bt_syn_set_tg_ticks(uint32_t val,uint16_t conhdl, uint8_t mode, uint8_t trig_route, bool no_link_trig);
void bt_syn_ble_set_tg_ticks(uint32_t val,uint8_t trig_route);
uint32_t bt_syn_ble_bt_time_to_bts(uint32_t hs, uint16_t hus);
void bt_syn_trig_checker(uint16_t conhdl);
uint32_t btdrv_get_syn_trigger_codec_en(void);
uint32_t btdrv_get_trigger_ticks(void);

#define ACL_TRIGGLE_MODE       1
#define SCO_TRIGGLE_MODE      2
void btdrv_enable_playback_triggler(uint8_t triggle_mode);
void btdrv_disable_playback_triggler(void);
void btdrv_enable_tports(void);
uint8_t btdrv_conhdl_to_linkid(uint16_t connect_hdl);
void btdrv_ins_patch_test_init(void);
uint32_t btdrv_dut_get_connect_status(void);

void btdrv_enable_one_packet_more_head(bool enable);
void btdrv_reconn(bool en);

void btdrv_pcm_enable(void);
void btdrv_pcm_disable(void);
void btdrv_spi_trig_data_change(uint8_t spi_sel, uint8_t index, uint32_t value);
void btdrv_spi_trig_data_set(uint8_t spi_sel);
uint8_t btdrv_get_spi_trig_enable(uint8_t spi_sel);
void btdrv_set_spi_trig_enable(uint8_t spi_sel);
void btdrv_clear_spi_trig_enable(uint8_t spi_sel);
void btdrv_spi_trig_init(void);
void btdrv_spi_trig_select(uint8_t link_id, bool spi_set);
void bt_drv_reg_op_cs_monitor(void);
const struct rssi_txpower_link_thd* btdrv_get_tws_link_txpwr_thd_ptr(void);
void bt_drv_hwspi_select(uint8_t link_id, uint8_t spi_idx);
void bt_drv_enable_hw_spi(uint8_t elt_idx, bool hwspien);
void beshci_enable_sleep_checker(bool isEnable);
void btdrv_send_cmd(uint16_t opcode,uint8_t cmdlen,const uint8_t *param);
void btdrv_write_memory(uint8_t wr_type,uint32_t address,const uint8_t *value,uint8_t length);
void bt_drv_reg_op_music_link_config(uint16_t active_link,uint8_t active_role,uint16_t inactive_link,uint8_t inactive_role);
struct bt_cb_tag* bt_drv_get_func_cb_ptr(void);

void btdrv_memory_copy(uint32_t *dest,const uint32_t *src,uint16_t length);
void *btdrv_memcpy_word(void * d, const void * s, size_t n);
#if defined(TX_RX_PCM_MASK)
uint8_t  btdrv_is_pcm_mask_enable(void);
#endif
void btdrv_set_lpo_times(void);

#if defined(PCM_FAST_MODE)
void btdrv_open_pcm_fast_mode_enable(void);
void btdrv_open_pcm_fast_mode_disable(void);
#endif

void btdrv_set_btpcm_mute_pattern(uint16_t pattern);

#if defined(CVSD_BYPASS)
void btdrv_cvsd_bypass_enable(void);
#endif
void bt_drv_reg_op_write_private_public_key(uint8_t* private_key,uint8_t* public_key);
void bt_drv_reg_op_write_private_public_192_256_key(uint8_t* private_key,uint8_t* public_key,bool p192_or_p256);

#define BTCLK_STATUS_PRINT()    do{ \
    TRACE("[%s] %d: curr bt time= %d", __func__, __LINE__, bt_syn_get_curr_ticks(app_tws_get_tws_conhdl())); \
                                }while(0)

int btdrv_slave2master_clkcnt_convert(uint32_t local_clk, uint16_t local_cnt,
                                               int32_t clk_offset, uint16_t bit_offset,
                                               uint32_t *master_clk, uint16_t *master_cnt);

int btdrv_clkcnt_diff(int32_t clk1, int16_t cnt1,
                         int32_t clk2, int16_t cnt2,
                         int32_t *diff_clk, uint16_t *diff_bit);
int64_t btdrv_clkcnt_to_us(uint32_t clk, uint16_t cnt);
int64_t btdrv_iso_clkcnt_to_us(uint32_t clk, uint16_t cnt);
bool bt_drv_reg_op_set_ble_conn_latency(uint16_t ble_conhdl, uint16_t conn_latency);
bool bt_drv_is_enhanced_ibrt_rom(void);
bool bt_drv_is_bes_fa_mode_en(void);
bool bt_drv_is_support_multipoint_ibrt(void);

uint16_t bt_drv_reg_op_get_ibrt_sco_hdl(uint16_t acl_hdl);
void bt_drv_reg_op_get_ibrt_address(uint8_t *addr);

void btdrv_enable_rf_sw(int rx_on, int tx_on);
void btdrv_trigger_coredump(void);
bool btdrv_is_ecc_enable(void);
void* bt_drv_get_btstack_chip_config(void);
bool bt_drv_is_esco_auto_accept_support(void);

#if defined(IBRT)
bool btdrv_is_acl_ecc_softbit_support(void);
void btdrv_softbit_enable(uint16_t connhdl, uint8_t type1,uint8_t type2,uint8_t type3, uint8_t num);
#endif

void btdrv_play_trig_mode_set(uint8_t mode);
#ifdef __SW_TRIG__
void btdrv_sw_trig_master_set(uint32_t Tclk_M, int16_t Tbit_M_h_ori, uint8_t trig_route);
void btdrv_sw_trig_slave_calculate_and_set(uint16_t conhdl, uint32_t Tclk_M, int16_t Tbit_M_h_ori, uint8_t trig_route);
#endif
#if defined(PCM_FAST_MODE) && defined(PCM_PRIVATE_DATA_FLAG)
void bt_drv_reg_op_set_pcm_flag();
#endif

#ifdef __SELF_TRIG__
void btdrv_self_trig_gen_set(uint8_t en);
void btdrv_trig_playtime_stamp_en(uint8_t en);
#endif
void bt_drv_productiontest_settings_checker(void);

#if (defined(SOFTBIT_EN))
void btdrv_softbit_config(uint16_t connhdl, uint8_t type1,uint8_t type2,uint8_t type3, uint8_t num);
#endif


#ifdef __IBRT_IBRT_TESTMODE__
void btdrv_enable_ibrt_test(void);
void btdrv_connect_ibrt_device(uint8_t *addr);
#endif
void bt_drv_select_agc_mode(enum BT_WORK_MODE_T mode);

bool bt_drv_error_check_handler(void);
void bt_drv_rssi_dump_handler(void);
void bt_drv_testmode_adaptive_adjust_param(void);
void bt_drv_reg_op_ble_sup_timeout_set(uint16_t ble_conhdl, uint16_t sup_to);
void btdrv_delay_for_wakeup(void);

#ifdef VCO_TEST_TOOL
//vco test callback
bool btdrv_vco_test_bridge_intsys_callback(const unsigned char *data);
void btdrv_vco_test_process(uint8_t op);
unsigned short btdrv_get_vco_test_process_flag(void);
bool btdrv_vco_test_set_xtal_fcap_bridge_intsys_callback(const unsigned char *data);
void btdrv_vco_test_set_xtal_fcap_process(uint8_t op);
unsigned short btdrv_get_vco_test_set_xtal_fcap_process_flag(void);
unsigned short btdrv_pmu_reboot_hci_process_flag(void);
bool btdrv_pmu_reboot_hci_intsys_callback(const unsigned char *data);
void btdrv_pmu_reboot_hci_process(uint8_t op);
#endif

void btdrv_le_rx_test_v2(uint8_t rx_channel, uint8_t phy, uint8_t mod_idx);
void btdrv_le_tx_test_v2(uint8_t tx_channel, uint8_t data_len, uint8_t pkt_payload, uint8_t phy);
void btdrv_le_test_end(void);

void btdrv_le_rx_test_v2_bystack(uint8_t rx_channel, uint8_t phy, uint8_t mod_idx);
void btdrv_le_tx_test_v2_bystack(uint8_t tx_channel, uint8_t data_len, uint8_t pkt_payload, uint8_t phy);
void btdrv_le_test_end_bystack(void);

typedef bool (*le_test_end_handler_func)(uint8_t status, uint16_t nb_packet_received);
typedef bool (*bt_nonsig_test_end_handler_func)(uint8_t status, uint16_t nb_packet_received, uint16_t hec_nb, uint16_t crc_nb);

void btdrv_regist_le_teset_end_callback(le_test_end_handler_func cb);
void btdrv_regist_bt_nonsignalint_test_end_callback(bt_nonsig_test_end_handler_func cb);

int bt_drv_register_send_hci_cmd_hander(btstack_send_hci_cmd_t hander);
int btdrv_send_hci_cmd_bystack(uint16_t opcode, uint8_t *param, uint8_t len);
void btdrv_stack_send_hci_cmd_data_bystack(uint8_t *hci_ptr);
void btdrv_nosig_tx_test_set(uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length, uint32_t tx_packet_num);
void btdrv_nosig_rx_test_set(uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                const uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type,
                                uint8_t payload_pattern, uint16_t payload_length);
void btdrv_nosig_stop_rx_test_set(void);
void btdrv_nosig_tx_test_set_for_1300(uint16_t test_period, uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length);
void btdrv_nosig_rx_test_set_for_1300(uint16_t test_period, uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length);

void bt_drv_ble_adv_txpwr_via_advhdl(uint8_t adv_hdl, uint8_t idx, int8_t txpwr_dbm);
void btdrv_regop_get_ble_event_info(uint16_t ble_conhdl, uint16_t *evt_count, uint32_t *interval, uint32_t *anchor_ts, uint16_t *bit_off);

typedef bool (*freqcalib_mode_get_handler_func)(void);
void btdrv_register_freqcalib_mode_get_callback(freqcalib_mode_get_handler_func cb);
bool btdrv_freqcalib_mode_get(void);
void bt_drv_rf_high_efficency_tx_pwr_ctrl(bool en, bool reset);
uint8_t btdrv_reg_op_isohci_in_nb_buffer(uint8_t link_id);
void trace_2_uart(uint8_t debug_level,const char *format, ...);
void bt_drv_rf_set_customer_config(struct btdrv_customer_rf_config_t* config);

void bt_drv_reg_op_crash_dump_2_uart(void);
void btdrv_dump_mem_2_uart(uint8_t *dump_mem_start, uint32_t dump_length, uint8_t dump_type);
int hal_uart_dump(const char *fmt, unsigned int size,  unsigned int count, const void *buffer);
void bt_uart_debug_send(unsigned char *data, unsigned int len);

void btdrv_ins_patch_close(void);
void bt_drv_reg_op_write_ble_p256(uint8_t* private_key);
uint8_t bt_drv_reg_op_fa_agc_get(void);
void btdrv_set_bdr_ble_txpower(uint8_t txpwr_idx, uint16_t n);;
#ifdef NORMAL_TEST_MODE_SWITCH
typedef int (*btdrv_testmode_nonsig_rx_process_func)(const unsigned char *data, unsigned int len);
void btdrv_testmode_register_nonsig_rx_process(btdrv_testmode_nonsig_rx_process_func cb);
#endif

#define UART_DUMP8(str, buf, cnt)            hal_uart_dump(str, sizeof(uint8_t), cnt, buf)
#define UART_DUMP16(str, buf, cnt)           hal_uart_dump(str, sizeof(uint16_t), cnt, buf)
#define UART_DUMP32(str, buf, cnt)           hal_uart_dump(str, sizeof(uint32_t), cnt, buf)

void iomux_restore_function_on_tport_disabled(void);
void bt_drv_ctrl_tports(bool en);

typedef void (*btdrv_temp_compensation_handler)(void);
void btdrv_register_temp_compensation_callback(btdrv_temp_compensation_handler cb);
void btdrv_compensate_pwr_based_on_temp(void);

unsigned char *bt_get_local_address(void);
void bt_set_local_address(unsigned char* btaddr);
unsigned char *bt_get_ble_local_address(void);
const char *bt_get_local_name(void);
void bt_set_local_name(const char* name);
const char *bt_get_ble_local_name(void);
void bt_set_ble_local_address(uint8_t* bleAddr);
bool bt_drv_work_in_ramrun_mode(void);
bool bt_drv_is_support_hci_set_tws_link(void);
bool bt_drv_is_support_hci_enable_btpcm(void);
bool bt_drv_is_support_set_bt_ble_active_link_switch(void);
void bt_drv_rx_status_dump(uint16_t conhdl);
uint32_t bt_drv_reg_op_get_multi_ibrt_slice(uint8_t active);
bool bt_drv_is_support_hci_config_multi_ibrt_sche(void);
void bt_drv_only_wakeup_btcore(void);
void bt_drv_dump_rf_reg(void);
#ifdef __cplusplus
}
#endif

#endif
