/***************************************************************************
 *
 * Copyright (c) 2015-2024 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __CS_SERVICE_H__
#define __CS_SERVICE_H__

#include "adapter_service.h"
#ifdef __cplusplus
extern "C" {
#endif

#define CS_BYTE_BIT_NUM         (8)
#define CS_MAX_CONFIG_NUM       (4)
#define CS_INVALID_CONFIG_ID    CS_MAX_CONFIG_NUM
#define CS_MAX_ANTENNA_PATH_NUM (4)
#define CS_CHANNEL_NUM          (72)
#define CS_CHANNEL_MAP_BIT_NUM  (80)
#define CS_CHANNEL_MAP_SIZE     (10)        // CS_CHANNEL_MAP_BIT_NUM / CS_BYTE_BIT_NUM
#define CS_SYNC_PAYLOAD_SIZE    (16)
#define CS_TEST_CONN_HANDLE     (0x0FFF)    // connection handle used for cs test
#define CS_INVLAID_CONIDX       GAP_INVALID_CONIDX

typedef uint8_t cs_proc_able_type_t;
enum cs_proc_enable_cmpl_type
{
    CS_PROC_DISABLE,
    CS_PROC_ENABLE,
};

enum cs_action_type
{
    CS_CONFIG_REMOVED,
    CS_CONFIG_CREATED,
};

typedef uint8_t cs_aa_qty_type_t;
enum cs_aa_qty_type
{
    CS_AA_CHECK_NO_ERROR,
    CS_AA_CHECK_ERROR,
    CS_AA_NOT_FOUND,
};

typedef uint8_t cs_step_mode_type_t;
enum cs_step_mode_type
{
    CS_STEP_MODE_0,
    CS_STEP_MODE_1,
    CS_STEP_MODE_2,
    CS_STEP_MODE_3,
};

typedef uint8_t cs_role_type_t;
enum cs_role_type
{
    CS_INITIATOR,
    CS_REFLECTOR,
};

typedef uint8_t cs_role_enable_type_t;
enum cs_role_enable_type
{
    CS_INITIATOR_ENABLE = 1,
    CS_REFLECTOR_ENABLE,
    CS_BOTH_ROLE_ENABLE,
};

typedef uint8_t cs_rtt_type_t;
enum cs_rtt_type
{
    /// RTT Access Address Only
    CS_RTT_AA_ONLY,
    /// RTT with 32-bit Sounding Sequence
    CS_RTT_32_SS,
    /// RTT with 96-bit Sounding Sequence
    CS_RTT_96_SS,
    /// RTT with 32-bit Random Sequence
    CS_RTT_32_RS,
    /// RTT with 64-bit Random Sequence
    CS_RTT_96_RS,
    /// RTT with 128-bit Random Sequence
    CS_RTT_128_RS,
};

typedef uint8_t cs_channel_selection_type_t;
enum cs_channel_selection_type
{
    CS_CHANNEL_SELECTION_3B,
    CS_CHANNEL_SELECTION_3C,
};

typedef uint8_t cs_ch3c_shape_type_t;
enum cs_ch3c_shape_type
{
    CS_CH3C_HAT_SHAPE,
    CS_CH3C_X_SHAPE,
};

typedef uint8_t cs_proc_done_type_t;
enum cs_proc_done_type
{
    /// all results complete for the CS procedure
    CS_PROC_DONE_ALL,
    /// partial results with more to follow for the CS procedure
    CS_PROC_DONE_PARTIAL,
    /// all subsequent CS procedure aborted
    CS_PROC_DONE_ABORT = 0x0F,
};

typedef uint8_t cs_subevt_done_type_t;
enum cs_subevent_done_type
{
    /// all results complete for the CS subevent
    CS_SUBEVT_DONE_ALL,
    /// partial results with more to follow for the CS subevent
    CS_SUBEVT_DONE_PARTIAL,
    /// all subsequent CS subevent aborted
    CS_SUBEVT_DONE_ABORT = 0x0F,
};

typedef uint8_t cs_proc_abort_type_t;
enum cs_proc_abort_type
{
    /// report with no abort
    CS_PROC_NO_ABORT,
    /// abort because of Local host or Remote request
    CS_PROC_LR_ABORT,
    /// abort because of Filtered Channel Map (FCM) has less than 15 channels
    CS_PROC_FCM_ABORT,
    /// abort because of the Channel Map Update Instant (CMUI) has passed
    CS_PROC_CMUI_ABORT,
    /// abort because of unspecified reasons
    CS_PROC_UNSPEC_ABORT = 0x0F,
};

typedef uint8_t cs_subevt_abort_type_t;
enum cs_subevt_abort_type
{
    /// report with no abort
    CS_SUBEVT_NO_ABORT,
    /// abort because of Local host or Remote request
    CS_SUBEVT_LR_ABORT,
    /// abort because of No CS Sync (mode-0) received
    CS_SUBEVT_NCSS_ABORT,
    /// abort because of Scheduling Conflicts or Limited Resources
    CS_SUBEVT_SCLR_ABORT,
    /// abort because of unspecified reasons
    CS_SUBEVT_UNSPEC_ABORT = 0x0F,
};

typedef uint8_t pkt_nadm_type_t;
enum pkt_nadm_type
{
    CS_PKT_NADM_ATTACK_EX_UNLIKELY,
    CS_PKT_NADM_ATTACK_VERY_UNLIKELY,
    CS_PKT_NADM_ATTACK_UNLIKELY,
    CS_PKT_NADM_ATTACK_POSSIBLE,
    CS_PKT_NADM_ATTACK_LIKELY,
    CS_PKT_NADM_ATTACK_VERY_LIKELY,
    CS_PKT_NADM_ATTACK_EX_LIKELY,
    CS_PKT_NADM_ATTACK_UNKNOWN = 0xFF,
};

enum tone_quality_type
{
    CS_TONE_QUALITY_HIGH,
    CS_TONE_QUALITY_MEDIUM,
    CS_TONE_QUALITY_LOW,
    CS_TONE_QUALITY_UNAVAILABLE,
};

enum tone_extension_slot_type
{
    /// not tone extension slot
    CS_NOT_TONE_EXTENSION_SLOT,
    /// tone extension slot, but tone not expected to be present
    CS_TONE_EXTENSION_SLOT_NOT_PRESENT,
    /// tone extension slot, and tone expected to present
    CS_TONE_EXTENTION_SLOT_PRESENT,
};

typedef uint8_t cs_config_context_t;
enum cs_create_config_context
{
    /// write CS configuration in local controller only
    CS_CONFIG_LOCAL_ONLY,
    /// write CS configuration in both local and remote controller using Channel Sounding Configuration procedure
    CS_CONFIG_BOTH_SIDE,
};

typedef uint8_t cs_spec_info_type_t;
enum cs_spec_info_type
{
    CS_MODE_0_ROLE_I,
    CS_MODE_0_ROLE_R,
    CS_MODE_1_ROLE_I,
    CS_MODE_1_ROLE_I_SPBR_SS,
    CS_MODE_1_ROLE_R,
    CS_MODE_1_ROLE_R_SPBR_SS,
    CS_MODE_2_ROLE_E,
    CS_MODE_3_ROLE_I,
    CS_MODE_3_ROLE_I_SPBR_SS,
    CS_MODE_3_ROLE_R,
    CS_MODE_3_ROLE_R_SPBR_SS,
    CS_MODE_ROLE_UNKNOWN,
};

typedef uint8_t cs_sync_phy_type_t;
enum cs_sync_phy_type
{
    CS_LE_1M_PHY,
    CS_LE_2M_PHY,
    CS_LE_2M_BT_PHY,
};

typedef struct
{
    /// @brief Number of CS configurations supported per connection
    /// Range: 0x01 ~ 0x04
    uint8_t             num_config_supp;
    /// cnsv -> consecutive
    uint16_t            max_cnsv_proc_supp;
    uint8_t             num_antennas_supp;
    uint8_t             max_antenna_paths_supp;
    cs_role_type_t      roles_supp;
    cs_step_mode_type_t modes_supp;
    /// bit field
    uint8_t             rtt_capas;
    uint8_t             rtt_aa_only_n;
    uint8_t             rtt_sounding_n;
    uint8_t             rtt_random_payload_n;
    uint16_t            nadm_sounding_capability;
    uint16_t            nadm_random_capability;
    uint8_t             cs_sync_phys_supp;
    uint16_t            subfeatures_supp;
    uint16_t            t_ip1_times_supp;
    uint16_t            t_ip2_times_supp;
    uint16_t            t_fcs_times_supp;
    uint16_t            t_pm_times_supp;
    uint8_t             t_sw_time_supp;
    uint8_t             tx_snr_capability;
} cs_supp_capas_t;

/**
 * @brief the procedure configuration reported by Controller
 *
 */
typedef struct
{
    /// @see enum cs_proc_type
    uint8_t  state;
    uint8_t  tone_antenna_config_selection;
    uint8_t  selected_tx_power;
    /// actually 3 bytes
    uint32_t subevent_len;
    uint8_t  subevents_per_evt;
    uint16_t subevent_interval;
    uint16_t event_interval;
    uint16_t proc_interval;
    uint16_t proc_count;
    uint16_t max_proc_len;
} cs_proc_config_t;

/**
 * @brief the procedure configuration set by Host
 *
 */
typedef struct
{
    uint16_t max_proc_len;
    uint16_t min_proc_interval;
    uint16_t max_proc_interval;
    uint16_t max_proc_count;
    /// actually 3 bytes
    uint32_t min_subevent_len;
    /// actually 3 bytes
    uint32_t max_subevent_len;
    uint8_t  tone_antenna_config_selection;
    uint8_t  phy;
    uint8_t  tx_power_delta;
    uint8_t  preferred_peer_antenna;
    uint8_t  snr_control_initiator;
    uint8_t  snr_control_reflector;
} cs_set_proc_config_t;

// cs tone params -- i.e. `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
typedef struct
{
    /// actually 3 bytes
    uint32_t tone_pct;
    uint8_t  tone_qty_indicator;
} cs_tone_params_t;

typedef struct
{
    /// @brief actually 12 bits
    uint16_t         pkt_pct_i_sample;
    /// @brief actually 12 bits
    uint16_t         pkt_pct_q_sample;
    /// @brief always zero now
    uint8_t          pkt_pct_reserved;
} cs_pkt_pct_t;

typedef struct
{
    /// @brief lower 4 bits of `Packet_Quality`
    cs_aa_qty_type_t aa_qty;
    /// @brief higner 4 bits of `Packet_Quality`
    uint8_t          rs_ss_qty;
} cs_pkt_qty_t;

/// pkt --> packet, qty --> quality
typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    uint8_t          pkt_antenna;
    cs_pkt_pct_t     pkt_pct_1;
    cs_pkt_pct_t     pkt_pct_2;
    /// @brief range: -100ppm ~ +100ppm (0x58F0 ~ 0x2710), units: 0.01ppm
    uint16_t         measured_freq_offset;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of arrival and
    ///         the time of departure of the CS packets at the initiator during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    uint16_t         toa_tod_initiator;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of departure
    ///         and the time of arrival of the CS packets at the reflector during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    /// @details 0x8000 is not available
    uint16_t         tod_toa_reflector;
    /// @brief Antenna Permutation Index for the chosen Num_Antenna_Paths parameter
    ///         used during the phase measurement stage of the CS step. Range: 0x00 ~ 0x17
    uint8_t          antenna_permutation_idx;
    /// @brief include `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
    cs_tone_params_t tone_params[CS_MAX_ANTENNA_PATH_NUM];
} cs_mode_role_all_info_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    uint8_t          pkt_antenna;
    /// @brief range: -100ppm ~ +100ppm (0x58F0 ~ 0x2710), units: 0.01ppm
    uint16_t         measured_freq_offset;
} cs_mode_0_role_i_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    uint8_t          pkt_antenna;
} cs_mode_0_role_r_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of arrival and
    ///         the time of departure of the CS packets at the initiator during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    uint16_t         toa_tod_initiator;
    uint8_t          pkt_antenna;
} cs_mode_1_role_i_t;

/**
 * @brief Mode type is 1, role is initiator, sounding phase-based ranging is supported, rtt type contains a sounding sequence
 *
 */
typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of arrival and
    ///         the time of departure of the CS packets at the initiator during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    uint16_t         toa_tod_initiator;
    uint8_t          pkt_antenna;
    cs_pkt_pct_t     pkt_pct_1;
    cs_pkt_pct_t     pkt_pct_2;
} cs_mode_1_role_i_spbr_ss_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of departure
    ///         and the time of arrival of the CS packets at the reflector during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    /// @details 0x8000 is not available
    uint16_t         tod_toa_reflector;
    uint8_t          pkt_antenna;
} cs_mode_1_role_r_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of departure
    ///         and the time of arrival of the CS packets at the reflector during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    /// @details 0x8000 is not available
    uint16_t         tod_toa_reflector;
    uint8_t          pkt_antenna;
    cs_pkt_pct_t     pkt_pct_1;
    cs_pkt_pct_t     pkt_pct_2;
} cs_mode_1_role_r_spbr_ss_t;

typedef struct
{
    /// @brief Antenna Permutation Index for the chosen Num_Antenna_Paths parameter
    ///         used during the phase measurement stage of the CS step. Range: 0x00 ~ 0x17
    uint8_t          antenna_permutation_idx;
    /// @brief include `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
    cs_tone_params_t tone_params[CS_MAX_ANTENNA_PATH_NUM];
} cs_mode_2_role_e_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of arrival and
    ///         the time of departure of the CS packets at the initiator during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    uint16_t         toa_tod_initiator;
    uint8_t          pkt_antenna;
    /// @brief Antenna Permutation Index for the chosen Num_Antenna_Paths parameter
    ///         used during the phase measurement stage of the CS step. Range: 0x00 ~ 0x17
    uint8_t          antenna_permutation_idx;
    /// @brief include `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
    cs_tone_params_t tone_params[CS_MAX_ANTENNA_PATH_NUM];
} cs_mode_3_role_i_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of arrival and
    ///         the time of departure of the CS packets at the initiator during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    uint16_t         toa_tod_initiator;
    uint8_t          pkt_antenna;
    cs_pkt_pct_t     pkt_pct_1;
    cs_pkt_pct_t     pkt_pct_2;
    /// @brief Antenna Permutation Index for the chosen Num_Antenna_Paths parameter
    ///         used during the phase measurement stage of the CS step. Range: 0x00 ~ 0x17
    uint8_t          antenna_permutation_idx;
    /// @brief include `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
    cs_tone_params_t tone_params[CS_MAX_ANTENNA_PATH_NUM];
} cs_mode_3_role_i_spbr_ss_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of departure
    ///         and the time of arrival of the CS packets at the reflector during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    /// @details 0x8000 is not available
    uint16_t         tod_toa_reflector;
    uint8_t          pkt_antenna;
    /// @brief Antenna Permutation Index for the chosen Num_Antenna_Paths parameter
    ///         used during the phase measurement stage of the CS step. Range: 0x00 ~ 0x17
    uint8_t          antenna_permutation_idx;
    /// @brief include `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
    cs_tone_params_t tone_params[CS_MAX_ANTENNA_PATH_NUM];
} cs_mode_3_role_r_t;

typedef struct
{
    cs_pkt_qty_t     pkt_qty;
    pkt_nadm_type_t  pkt_nadm;
    /// @brief range: -127 ~ 20, units: dBm
    uint8_t          pkt_rssi;
    /// @brief Time difference in units of 0.5 nanoseconds between the time of departure
    ///         and the time of arrival of the CS packets at the reflector during a CS step
    ///         (16-bit signed integer), where the known nominal offsets are excluded.
    /// @details 0x8000 is not available
    uint16_t         tod_toa_reflector;
    uint8_t          pkt_antenna;
    cs_pkt_pct_t     pkt_pct_1;
    cs_pkt_pct_t     pkt_pct_2;
    /// @brief Antenna Permutation Index for the chosen Num_Antenna_Paths parameter
    ///         used during the phase measurement stage of the CS step. Range: 0x00 ~ 0x17
    uint8_t          antenna_permutation_idx;
    /// @brief include `tone_pct` & `tone_qty_indicator`, whose num is subject to `num_antenna_paths`
    cs_tone_params_t tone_params[CS_MAX_ANTENNA_PATH_NUM];
} cs_mode_3_role_r_spbr_ss_t;

typedef union
{
    cs_mode_role_all_info_t    mode_role_all_info;
    cs_mode_0_role_i_t         mode_0_role_i;
    cs_mode_0_role_r_t         mode_0_role_r;
    cs_mode_1_role_i_t         mode_1_role_i;
    cs_mode_1_role_i_spbr_ss_t mode_1_role_i_spbr_ss;
    cs_mode_1_role_r_t         mode_1_role_r;
    cs_mode_1_role_r_spbr_ss_t mode_1_role_r_spbr_ss;
    cs_mode_2_role_e_t         mode_2_role_e;
    cs_mode_3_role_i_t         mode_3_role_i;
    cs_mode_3_role_i_spbr_ss_t mode_3_role_i_spbr_ss;
    cs_mode_3_role_r_t         mode_3_role_r;
    cs_mode_3_role_r_spbr_ss_t mode_3_role_r_spbr_ss;
} cs_mode_role_specific_info_t;

typedef struct
{
    cs_spec_info_type_t           spec_info_type;
    cs_step_mode_type_t           step_mode;
    uint8_t                       step_channel;
    /// @brief Length for mode- and role-specific information being reported
    uint8_t                       step_data_len;
    /// content is subject to `mode_type` & `role`
    cs_mode_role_specific_info_t  step_data;
} cs_step_params_t;

typedef struct
{
    uint16_t               start_acl_conn_evt_counter;
    uint16_t               proc_counter;
    /// @brief Frequency compensation value in units of 0.01 ppm (15-bit signed integer),
    /// range: -100ppm ~ 100ppm, 0x58F0 ~ 0x2710
    uint16_t               freq_compensation;
    /// @brief range: -127 ~ 20, unit: dBm
    uint8_t                reference_power_level;
    cs_proc_done_type_t    proc_done_status;
    cs_subevt_done_type_t  subevent_done_status;
    cs_proc_abort_type_t   proc_abort_reason;
    cs_subevt_abort_type_t subevt_abort_reason;
    uint8_t                num_antenna_paths;
    uint8_t                num_steps_reported;
    /// `step_params` num is subject to `num_steps_reported`
    /// don't worry about this, the lower layer will free the whole `subevent_result` data, include the step_params at the tail
    /// so just access step_params by the num_steps_reported
    cs_step_params_t       step_params[];
} cs_subevent_result_t;

typedef struct
{
    /// @see `enum cs_role_enable_type`
    cs_role_enable_type_t role_enable;
    uint8_t               cs_sync_antenna_selection;
    /// range: -127 ~ 20, units: dBm
    uint8_t               max_tx_power;
} cs_default_settings_t;

typedef struct
{
    uint8_t  channel_len;
    uint8_t *channel;
} cs_test_opd_b0_true_t;
typedef struct
{
    uint8_t  channel_map[CS_CHANNEL_MAP_SIZE];
    uint8_t  cahnnel_selection_type;
    uint8_t  ch3c_shape;
    uint8_t  ch3c_jump;
} cs_test_opd_b0_false_t;
typedef union
{
    cs_test_opd_b0_true_t  opd_b0_true;
    cs_test_opd_b0_false_t opd_b0_false;
} cs_test_opd_b0_t;

typedef struct
{
    uint8_t  main_mode_steps;
} cs_test_opd_b2_true_t;
typedef struct { } cs_test_opd_b2_false_t;
typedef union
{
    cs_test_opd_b2_true_t  opd_b2_true;
    cs_test_opd_b2_false_t opd_b2_false;
} cs_test_opd_b2_t;

typedef struct
{
    uint8_t  t_pm_tone_ext;
} cs_test_opd_b3_true_t;
typedef struct { } cs_test_opd_b3_false_t;
typedef union
{
    cs_test_opd_b3_true_t  opd_b3_true;
    cs_test_opd_b3_false_t opd_b3_false;
} cs_test_opd_b3_t;

typedef struct
{
    uint8_t  tone_antenna_permutation;
} cs_test_opd_b4_true_t;
typedef struct { } cs_test_opd_b4_false_t;
typedef union
{
    cs_test_opd_b4_true_t  opd_b4_true;
    cs_test_opd_b4_false_t opd_b4_false;
} cs_test_opd_b4_t;

typedef struct
{
    uint32_t cs_sync_aa_initiator;
    uint32_t cs_sync_aa_reflector;
} cs_test_opd_b5_true_t;
typedef struct { } cs_test_opd_b5_false_t;
typedef union
{
    cs_test_opd_b5_true_t  opd_b5_true;
    cs_test_opd_b5_false_t opd_b5_false;
} cs_test_opd_b5_t;

typedef struct
{
    uint8_t  ss_marker1_position;
    uint8_t  ss_marker2_position;
} cs_test_opd_b6_true_t;
typedef struct { } cs_test_opd_b6_false_t;
typedef union
{
    cs_test_opd_b6_true_t  opd_b6_true;
    cs_test_opd_b6_false_t opd_b6_false;
} cs_test_opd_b6_t;

typedef struct
{
    uint8_t  ss_marker_value;
} cs_test_opd_b7_true_t;
typedef struct { } cs_test_opd_b7_false_t;
typedef union
{
    cs_test_opd_b7_true_t  opd_b7_true;
    cs_test_opd_b7_false_t opd_b7_false;
} cs_test_opd_b7_t;

typedef struct
{
    uint8_t  cs_sync_payload_pattern;
    uint8_t  cs_sync_user_payload[CS_SYNC_PAYLOAD_SIZE];
} cs_test_opd_b8_true_t;
typedef struct { } cs_test_opd_b8_false_t;
typedef union
{
    cs_test_opd_b8_true_t  opd_b8_true;
    cs_test_opd_b8_false_t opd_b8_false;
} cs_test_opd_b8_t;

typedef struct
{
    cs_test_opd_b0_t opd_b0;
    cs_test_opd_b2_t opd_b2;
    cs_test_opd_b3_t opd_b3;
    cs_test_opd_b4_t opd_b4;
    cs_test_opd_b5_t opd_b5;
    cs_test_opd_b6_t opd_b6;
    cs_test_opd_b7_t opd_b7;
    cs_test_opd_b8_t opd_b8;
} cs_test_opd_t;

typedef struct
{
    uint8_t                     main_mode_type;
    uint8_t                     sub_mode_type;
    uint8_t                     main_mode_repetition;
    uint8_t                     mode_0_steps;
    cs_role_type_t              role;
    cs_rtt_type_t               rtt_type;
    cs_sync_phy_type_t          cs_sync_phy;
    uint8_t                     cs_sync_antenna_selection;
    /// @brief only 3 bytes used
    uint32_t                    subevent_len;
    uint16_t                    subevent_interval;
    uint8_t                     max_num_subevents;
    uint8_t                     transmit_power_level;
    uint8_t                     t_ip1_time;
    uint8_t                     t_ip2_time;
    uint8_t                     t_fcs_time;
    uint8_t                     t_pm_time;
    uint8_t                     t_sw_time;
    uint8_t                     tone_antenna_config_selection;
    uint8_t                     reserved;
    uint8_t                     snr_control_initiator;
    uint8_t                     snr_control_reflector;
    uint8_t                     drbg_nonce;
    uint8_t                     channel_map_repetition;
    uint16_t                    override_config;
    /// @brief you'd better assure that your override parameters length is really matched with your override parameters data
    ///     i.e. the override params length means the length of valid dat, not the size of `cs_test_opd_t`
    uint8_t                     override_params_len;
    cs_test_opd_t               override_params_data;
} cs_test_params_t;

/**
 * @brief struct used for related CS configuration
 *
 * @details When used in CS config command, `t_ip1_time` `t_ip2_time` `t_fcs_time` `t_pm_time` is unused
 *
 * @member: action: 0 -> cs config is removed; 1 -> cs config is created
 * @member: main_mode_type: 0x00 ~ 0x03
 * @member: sub_mode_type: 0x01 ~ 0x03, 0xFF means unused
 */
typedef struct
{
    bool                        is_valid;
    uint8_t                     main_mode_type;
    uint8_t                     sub_mode_type;
    uint8_t                     min_main_mode_steps;
    uint8_t                     max_main_mode_steps;
    uint8_t                     main_mode_repetition;
    uint8_t                     mode_0_steps;
    cs_role_type_t              role;
    cs_rtt_type_t               rtt_type;
    cs_sync_phy_type_t          cs_sync_phy;
    uint8_t                     channel_map[CS_CHANNEL_MAP_SIZE];
    uint8_t                     channel_map_repetition;
    cs_channel_selection_type_t channel_selection_type;
    cs_ch3c_shape_type_t        ch3c_shape;
    uint8_t                     ch3c_jump;
    uint8_t                     reserved;
    uint8_t                     t_ip1_time;
    uint8_t                     t_ip2_time;
    uint8_t                     t_fcs_time;
    uint8_t                     t_pm_time;
} cs_basic_config_t;

typedef struct
{
    uint8_t         err_code;
    /// Local capabilities
    cs_supp_capas_t capas;
} cs_read_local_capas_cmpl_t;

typedef struct
{
    uint8_t                 err_code;
    uint16_t                connhdl;
    /// Remote capabilities
    cs_supp_capas_t         capas;
} cs_read_remote_capas_cmpl_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_write_cached_remote_capas_cmpl_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_set_default_settings_cmpl_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
    /// @brief 72 bytes
    uint8_t  fae_table[CS_CHANNEL_NUM];
} cs_read_remote_fae_table_cmpl_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_write_cached_remote_fae_table_cmpl_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_security_enable_cmpl_t;

typedef struct
{
    uint8_t            err_code;
    uint16_t           connhdl;
    uint8_t            config_id;
    /// @brief create or remove config
    uint8_t            action;
    cs_basic_config_t  basic_config;
} cs_config_update_cmpl_t;

typedef struct
{
    uint8_t           err_code;
    uint16_t          connhdl;
    uint8_t           config_id;
    cs_proc_config_t  proc_config;
} cs_proc_enable_cmpl_t;

typedef struct
{
    uint8_t err_code;
} cs_set_channel_classification_cmpl_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_set_proc_params_cmpl_t;

typedef struct
{
    uint8_t                     err_code;
    uint16_t                    connhdl;
    uint8_t                     config_id;
    const cs_subevent_result_t *subevent_result;
} cs_recv_subevent_result_t;

typedef struct
{
    uint8_t err_code;
} cs_start_test_cmpl_t;

typedef struct
{
    uint16_t connhdl;
} cs_connected_t;

typedef struct
{
    uint16_t connhdl;
} cs_disconnected_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_read_remote_capas_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_security_enable_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_read_remote_fae_table_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
    uint8_t  config_id;
    uint8_t  basic_config;
} cs_create_config_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
    uint8_t  config_id;
} cs_remove_config_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
    uint8_t  config_id;
} cs_proc_enable_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
    uint8_t  config_id;
} cs_proc_disable_status_t;

typedef struct
{
    uint8_t  err_code;
    uint16_t connhdl;
} cs_end_test_status_t;

typedef struct
{
    uint8_t err_code;
} cs_end_test_cmpl_t;

typedef union
{
    void *param_ptr;
    cs_read_remote_capas_status_t           *read_remote_capas_status;
    cs_security_enable_status_t             *security_enable_status;
    cs_read_remote_fae_table_status_t       *read_remote_fae_table_status;
    cs_create_config_status_t               *create_config_status;
    cs_remove_config_status_t               *remove_config_status;
    cs_proc_enable_status_t                 *proc_enable_status;
    cs_proc_disable_status_t                *proc_disable_status;
    cs_end_test_status_t                    *test_end_status;
    cs_read_local_capas_cmpl_t              *read_local_capas_cmpl;
    cs_read_remote_capas_cmpl_t             *read_remote_capas_cmpl;
    cs_write_cached_remote_capas_cmpl_t     *write_remote_cached_capas_cmpl;
    cs_set_default_settings_cmpl_t          *set_default_settings_cmpl;
    cs_read_remote_fae_table_cmpl_t         *read_fae_table_cmpl;
    cs_write_cached_remote_fae_table_cmpl_t *write_cached_fae_table_cmpl;
    cs_security_enable_cmpl_t               *security_enable_cmpl;
    cs_config_update_cmpl_t                 *config_update_cmpl;
    cs_proc_enable_cmpl_t                   *proc_enable_cmpl;
    cs_set_channel_classification_cmpl_t    *set_channel_classification_cmpl;
    cs_set_proc_params_cmpl_t               *set_proc_params_cmpl;
    // cs_recv_subevent_result_t               *recv_subevent_result;
    cs_start_test_cmpl_t                    *start_test_cmpl;
    cs_end_test_cmpl_t                      *end_test_cmpl;
    cs_connected_t                          *conn_connected;
    cs_disconnected_t                       *conn_disconnected;
} cs_event_param_t;

typedef enum
{
    CS_EVENT_CONN_CONNECTED = BT_EVENT_CS_EVENT_START,
    CS_EVENT_CONN_DISCONNECTED,
    CS_EVENT_READ_REMOTE_CAPAS_STATUS,
    CS_EVENT_SECURITY_ENABLE_STATUS,
    CS_EVENT_READ_REMOTE_FAE_TABLE_STATUS,
    CS_EVENT_CREATE_CONFIG_STATUS,
    CS_EVENT_REMOVE_CONFIG_STATUS,
    CS_EVENT_PROC_ENABLE_STATUS,
    CS_EVENT_PROC_DISABLE_STATUS,
    CS_EVENT_TEST_END_STATUS,
    CS_EVENT_READ_LOCAL_CAPAS_CMPL,
    CS_EVENT_READ_REMOTE_CAPAS_CMPL,
    CS_EVENT_WRITE_CACHED_REMOTE_CAPAS_CMPL,
    CS_EVENT_SET_DEFAULT_SETTINGS_CMPL,
    CS_EVENT_READ_REMOTE_FAE_TABLE_CMPL,
    CS_EVENT_WRITE_CACHED_REMOTE_FAE_TABLE_CMPL,
    CS_EVENT_ENABLE_SECURITY_CMPL,
    CS_EVENT_CREATE_CONFIG_CMPL,
    CS_EVENT_REMOVE_CONFIG_CMPL,
    CS_EVENT_ENABLE_PROC_CMPL,
    CS_EVENT_DISABLE_PROC_CMPL,
    CS_EVENT_SET_CHANNEL_CLASSIFICATION_CMPL,
    CS_EVENT_SET_PROC_PARAMS_CMPL,
    // CS_EVENT_RECV_SUBEVENT_RESULT,
    CS_EVENT_START_TEST_CMPL,
    CS_EVENT_END_TEST_CMPL,
} cs_event_t;

typedef void (*cs_event_handle_func_t)(const cs_event_param_t param);

cs_supp_capas_t *cs_get_local_supp_capas();
cs_supp_capas_t *cs_get_remote_supp_capas(uint16_t connhdl);
cs_basic_config_t *cs_get_remote_basic_config(uint16_t connhdl, uint8_t config_id);
cs_proc_config_t *cs_get_remote_proc_config(uint16_t connhdl, uint8_t config_id);
cs_default_settings_t *cs_get_remote_default_settings(uint16_t connhdl);
bool cs_is_security_enable(uint16_t connhdl);
bool cs_is_proc_enable(uint16_t connhdl);

/**
 * @brief get the config id that procedure enable command enables
 *
 * @param connhdl connection handle
 * @return uint8_t : config id that procedure enabled, exact config id if procedure enabled, else `CS_INVALID_CONFIG_ID`
 */
uint8_t cs_get_proc_enabled_config_id(uint16_t connhdl);

uint8_t cs_get_conidx_by_connhdl(uint16_t connhdl);

typedef int (*cs_ctrl_evt_cb_t)(cs_event_t event, cs_event_param_t param);

bt_status_t cs_register_ctrl_evt_callback(cs_ctrl_evt_cb_t cs_evt_cb);

bt_status_t cs_read_local_supp_capas(void);
bt_status_t cs_read_remote_supp_capas(uint16_t connhdl);
bt_status_t cs_write_cached_remote_supp_capas(uint16_t connhdl, const cs_supp_capas_t *supp_capas);
bt_status_t cs_enable_security(uint16_t connhdl);
bt_status_t cs_set_default_settings(uint8_t connhdl, cs_default_settings_t *default_settings);
bt_status_t cs_read_remote_fae_table(uint16_t connhdl);
bt_status_t cs_write_cached_remote_fae_table(uint8_t connhdl, uint8_t *remote_fae_table);
bt_status_t cs_create_config(uint16_t connhdl, uint8_t config_id, cs_config_context_t create_context, cs_basic_config_t *basic_config);
bt_status_t cs_remove_config(uint16_t connhdl, uint8_t config_id);
bt_status_t cs_set_channel_classification(uint8_t *channel_classification);
bt_status_t cs_set_procedure_parameters(uint16_t connhdl, uint8_t config_id, cs_set_proc_config_t *set_proc_config);
bt_status_t cs_enable_procedure(uint8_t connhdl, uint8_t config_id, cs_proc_able_type_t proc_enable);
bt_status_t cs_start_test(cs_test_params_t *params);
bt_status_t cs_end_test();

#ifdef __cplusplus
}
#endif

#endif /* __CS_SERVICE_H__ */