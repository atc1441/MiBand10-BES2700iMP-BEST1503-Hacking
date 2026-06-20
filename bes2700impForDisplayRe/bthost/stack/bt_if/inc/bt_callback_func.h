/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BT_CALLBACK_FUNC_H__
#define __BT_CALLBACK_FUNC_H__
#include "bt_common_define.h"
#include "me_common_define.h"
#include "adapter_service.h"
#ifdef __cplusplus
extern "C" {
#endif

struct bt_hf_custom_id_t {
    uint16_t hf_custom_vendor_id;
    uint16_t hf_custom_product_id;
    uint16_t hf_custom_version_id;
    uint16_t hf_custom_feature_id;
};

struct BT_CALLBACK_FUNC_T {
    struct coheap_global_t *(*coheap_get_global)(void);
    struct BT_DEVICE_T* (*bt_get_device)(int i);
    uint8_t (*get_device_id_byaddr)(const bt_bdaddr_t *remote);
    bool (*is_bt_thread)(void);
    int (*call_func_in_bt_thread)(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t func);
    int (*defer_call_in_bt_thread)(uintptr_t func, struct bt_alloc_param_t *param);
    btif_remote_device_t* (*get_remote_dev_by_address)(const bt_bdaddr_t *remote);
    btif_remote_device_t* (*get_remote_dev_by_handle)(uint16_t hci_handle);
    bool (*is_support_multipoint_ibrt)(void);
    ibrt_rx_data_filter_func ibrt_rx_data_filter_cb;
    ibrt_cmd_complete_callback hci_cmd_complete_callback[HCI_CMD_COMPLETE_USER_NUM];
    uint8_t (*tws_get_ibrt_role)(const bt_bdaddr_t *);
    l2cap_sdp_disconnect_callback l2cap_sdp_disconnect_cb;
    bt_get_ibrt_role_callback get_ibrt_role_cb;
    bt_get_ui_role_callback get_ui_role_cb;
    bt_get_tss_state_callback get_tss_state_cb;
    void (*ibrt_profile_callback)(uint8_t device_id, uint64_t profile,void *param1,void *param2,void *param3);
    uint8_t (*middleware_get_ui_role)(void);
    void (*ble_start_disconnect)(uint8_t conIdx);
    void (*btdrv_reconn)(bool en);
    void (*load_sleep_config)(uint8_t* config);
    bool (*ibrt_io_capbility_callback)(void *bdaddr);
    void (*read_le_host_chnl_map_cmpl)(uint8 *data);
    bt_iocap_request_callback_func iocap_request_callback;
    bt_cmgr_sniff_timeout_ext_handler sniff_timeout_handler_ext_fn;
    bt_remote_is_mobile_callback_t conn_remote_is_mobile;
    bt_gather_global_srv_uuids gather_uuid_func_lst[BT_EIR_GLOBAL_SRV_UUIDS_GATHER_CALLBACK_COUNT];
    bt_eir_fill_manufacture_data eir_fill_manufactrue_data_func;
    l2cap_process_bredr_smp_req_callback_func bredr_smp_req_callback;
    btif_hci_vendor_event_callback_func hci_vendor_event_handler;
    bt_spp_callback_t ibrt_spp_app_callback;
    bt_hci_cmd_status_callback_t ibrt_cmd_status_callback;
    btif_event_callback_t tws_ibrt_disconnect_callback;
    bt_get_ibrt_handle_callback_t get_ibrt_hci_handle_callback;
    bt_a2dp_stream_command_pack_callback_t avdtp_stream_command_accept_pack;
    bt_sco_codec_info_sync_callback_t hf_sco_codec_info_sync;
    bt_report_ibrt_slave_spp_closed_t spp_report_close_to_ibrt_slave;
    bt_avrcp_register_notify_callback_t avrcp_register_notify_send_check_callback;
    bt_avrcp_register_notify_response_callback_t avrcp_register_notify_resp_check_callback;
    btif_hci_sync_airmode_check_ind_func hci_sync_airmode_check_ind_callback;
    bt_stack_create_acl_failed_callback_t me_stack_create_acl_failed_cb;
    extra_acl_conn_req_callback extra_acl_conn_req_update;
    void (*set_device_support_le_audio)(const bt_bdaddr_t *remote);
    struct BT_SOURCE_DEVICE_T *(*bt_source_find_device)(bt_bdaddr_t *remote);
    void (*bt_report_source_link_connected)(btif_remote_device_t *dev, uint8_t errcode);
    struct bt_hf_custom_id_t *(*bt_get_hf_custom_id)(void);
    unsigned char *(*bt_get_address)(void);
    unsigned char *(*bt_get_ble_address)(void);
    bt_bdaddr_t *(*bt_get_pts_address)(void);
    void (*bt_set_ble_local_name)(const char* name);
    const char *(*bt_get_ble_local_name)(void);
    uint32_t (*bt_get_class_of_device)(void);
    // ISO buf malloc
    void *(*iso_rx_buf_malloc)(uint32_t size);
    void (*iso_rx_buf_free)(void *rmem);
#ifndef BT_REDUCE_CALLBACK_FUNC
    bool (*lhdc_v3)(void);
    bool (*a2dp_sink_enable)(void);
    bool (*bt_source_enable)(void);
    bool (*bt_sink_enable)(void);
    bool (*avdtp_cp_enable)(void);
    bool (*bt_source_48k)(void);
    bool (*source_unknown_cmd_flag)(void);
    bool (*source_suspend_err_flag)(void);
    bool (*source_get_all_cap_flag)(void);
    bool (*mark_some_code_for_fuzz_test)(void);
    bool (*use_page_scan_repetition_mode_r1)(void);
    bool (*bt_hid_cod_enable)(void);
    bool (*disc_acl_after_auth_key_missing)(void);
    bool (*hfp_support_lc3_swb_en)(void);
    bool (*vendor_codec_en)(void);
    bool (*normal_test_mode_switch)(void);
    bool (*hfp_hf_pts_acs_bv_09_i)(void);
    bool (*sniff)(void);
    uint16_t (*dip_vendor_id)(void);
    uint16_t (*dip_product_id)(void);
    uint16_t (*dip_product_version)(void);
    uint16_t (*dip_vendor_id_source)(void);
    bool (*apple_hf_at_support)(void);
    bool (*hf_support_hf_ind_feature)(void);
    bool (*hf_dont_support_cli_feature)(void);
    bool (*hf_dont_support_enhanced_call)(void);
    bool (*hf_dont_support_3way_call)(void);
    bool (*force_use_cvsd)(void);
    bool (*hfp_ag_pts_ecs_02)(void);
    bool (*hfp_ag_pts_ecc)(void);
    bool (*hfp_ag_pts_ecs_01)(void);
    bool (*hfp_ag_pts_enable)(void);
    bool (*send_l2cap_echo_req)(void);
    bool (*support_enre_mode)(void);
    bool (*le_audio_enabled)(void);
    bool (*hsp_enable)(void);
#endif /* BT_REDUCE_CALLBACK_FUNC */
    // add new bt callbacks below if needed
};

void bt_register_callback_func(struct BT_CALLBACK_FUNC_T* fn);
struct coheap_global_t *bt_callback_coheap_get_global(void);
struct BT_DEVICE_T *bt_callback_get_device(int i);
uint8_t bt_callback_get_device_id_byaddr(const bt_bdaddr_t *remote);
void bt_callback_report_source_link_connected(btif_remote_device_t *dev, uint8_t errcode);
struct BT_SOURCE_DEVICE_T *bt_callback_source_find_device(bt_bdaddr_t *remote);
bool bt_callback_is_bt_thread(void);
struct bt_hf_custom_id_t *bt_callback_get_hf_custom_id(void);
unsigned char *bt_callback_get_address(void);
unsigned char *bt_callback_get_ble_address(void);
bt_bdaddr_t *bt_callback_get_pts_address(void);
void bt_callback_set_ble_local_name(const char* name);
const char *bt_callback_get_ble_local_name(void);
void *bt_callback_iso_rx_buf_malloc(uint32_t size);
void bt_callback_iso_rx_buf_free(void *rmem);
int bt_callback_call_func_in_bt_thread(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t func);
int bt_callback_defer_call_in_bt_thread(uintptr_t func, struct bt_alloc_param_t *param);
btif_remote_device_t* bt_callback_get_remote_dev_by_address(const bt_bdaddr_t *remote);
btif_remote_device_t* bt_callback_get_remote_dev_by_handle(uint16_t hci_handle);
bool bt_callback_is_support_multipoint_ibrt(void);
bool bt_callback_ibrt_rx_data_filter(const bt_bdaddr_t *remote,uint8_t rx_filter_type,void*para);
void bt_callback_ibrt_hci_cmd_complete(const uint8_t *para);
bool bt_callback_tws_get_ibrt_role_slave(const bt_bdaddr_t *remote);
bool bt_callback_tws_get_ibrt_role_master(const bt_bdaddr_t *remote);
void bt_callback_l2cap_sdp_disconnect(const bt_bdaddr_t *remote);
bool bt_callback_get_ibrt_role_slave(const bt_bdaddr_t *remote);
bool bt_callback_get_ibrt_role_master(const bt_bdaddr_t *remote);
bool bt_callback_has_get_ibrt_role_func(void);
uint8_t bt_callback_get_ibrt_role(const bt_bdaddr_t *remote);
bool bt_callback_get_ui_role_slave(void);
bool bt_callback_get_ui_role_master(void);
bool bt_callback_has_get_ui_role_func(void);
uint8_t bt_callback_get_ui_role(void);
uint8_t bt_callback_get_tss_state(const bt_bdaddr_t *remote);
void bt_callback_ibrt_profile_callback(uint8_t device_id, uint64_t profile,void *param1,void *param2,void* param3);
uint8_t bt_callback_ibrt_middleware_get_ui_role(void);
void bt_callback_ble_start_disconnect(uint8_t conIdx);
void bt_callback_btdrv_reconn(bool en);
void bt_callback_load_sleep_config(uint8_t* config);
bool bt_callback_ibrt_io_capbility(void *bdaddr);
void bt_callback_read_le_host_chnl_map_cmpl(uint8 *data);
bt_iocap_requirement_t *bt_callback_iocap_request_handler(uint8_t device_id, uint16_t connhdl, const bt_bdaddr_t *remote, const bt_iocap_requirement_t *remote_initiate);
void bt_callback_sniff_timeout_ext_handler(evm_timer_t * timer, unsigned int* skipInternalHandler);
bool bt_callback_conn_remote_is_mobile(const bt_bdaddr_t *remote);
uint32_t bt_callback_gather_uuid_handler(int func_idx, bool only_check, uint8_t in_uuid_size, uint8_t *out_buff,
        uint32_t out_buff_len, uint32_t *out_len, uint32_t *out_real_len);
bool bt_callback_eir_fill_manufacture_data(uint8_t *buff, uint32_t* offset);
void bt_callback_set_device_support_le_audio(const bt_bdaddr_t *remote);
void bt_callback_bredr_smp_req_callback_func(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);
void bt_callback_hci_vendor_event_handler(uint8_t* pbuf, uint32_t length);
int bt_callback_ibrt_spp_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param);
void bt_callback_ibrt_hci_cmd_status(const void *para);
void bt_callback_ibrt_disconnect_handler(const btif_event_t *);
uint16_t bt_callback_get_ibrt_hci_handle(const bt_bdaddr_t* remote);
int bt_callback_ibrt_a2dp_stream_command_pack(void* remote, uint8_t transaction, uint8_t signal_id);
void bt_callback_hf_sco_codec_info_sync_callback(const bt_bdaddr_t* remote, uint8_t codec);
void bt_callback_report_ibrt_slave_spp_closed(const bt_bdaddr_t *remote, uint8_t dlci);
bool bt_callback_avrcp_register_notify_callback(uint8_t event);
void bt_callback_avrcp_register_notify_response_callback(uint8_t event);
uint32_t bt_callback_hci_sync_airmode_ind_check(uint8_t status, bt_bdaddr_t *bdaddr);
void bt_callback_stack_create_acl_failed(const bt_bdaddr_t *);
int bt_callback_extra_acl_conn_req_callback(uint8_t *remote, uint8_t *cod);
uint32_t bt_callback_get_class_of_device(void);

#ifdef BT_REDUCE_CALLBACK_FUNC
#define bt_callback_cfg_lhdc_v3() besbt_cfg.lhdc_v3
#define bt_callback_cfg_a2dp_sink_enable() besbt_cfg.a2dp_sink_enable
#define bt_callback_cfg_bt_source_enable() besbt_cfg.bt_source_enable
#define bt_callback_cfg_bt_sink_enable() besbt_cfg.bt_sink_enable
#define bt_callback_cfg_avdtp_cp_enable() besbt_cfg.avdtp_cp_enable
#define bt_callback_cfg_bt_source_48k() besbt_cfg.bt_source_48k
#define bt_callback_cfg_source_unknown_cmd_flag() besbt_cfg.source_unknown_cmd_flag
#define bt_callback_cfg_source_suspend_err_flag() besbt_cfg.source_suspend_err_flag
#define bt_callback_cfg_source_get_all_cap_flag() besbt_cfg.source_get_all_cap_flag
#define bt_callback_cfg_mark_some_code_for_fuzz_test() besbt_cfg.mark_some_code_for_fuzz_test
#define bt_callback_cfg_use_page_scan_repetition_mode_r1(void) besbt_cfg.use_page_scan_repetition_mode_r1
#define bt_callback_cfg_bt_hid_cod_enable() besbt_cfg.bt_hid_cod_enable
#define bt_callback_cfg_disc_acl_after_auth_key_missing() besbt_cfg.disc_acl_after_auth_key_missing
#define bt_callback_cfg_hfp_support_lc3_swb_en() besbt_cfg.hfp_support_lc3_swb_en
#define bt_callback_cfg_vendor_codec_en() besbt_cfg.vendor_codec_en
#define bt_callback_cfg_normal_test_mode_switch() besbt_cfg.normal_test_mode_switch
#define bt_callback_cfg_hfp_hf_pts_acs_bv_09_i() besbt_cfg.hfp_hf_pts_acs_bv_09_i
#define bt_callback_cfg_sniff() besbt_cfg.sniff
#define bt_callback_cfg_dip_vendor_id() besbt_cfg.dip_vendor_id
#define bt_callback_cfg_dip_product_id() besbt_cfg.dip_product_id
#define bt_callback_cfg_dip_product_version() besbt_cfg.dip_product_version
#define bt_callback_cfg_dip_vendor_id_source() besbt_cfg.dip_vendor_id_source
#define bt_callback_cfg_apple_hf_at_support() besbt_cfg.apple_hf_at_support
#define bt_callback_cfg_hf_support_hf_ind_feature() besbt_cfg.hf_support_hf_ind_feature
#define bt_callback_cfg_hf_dont_support_cli_feature() besbt_cfg.hf_dont_support_cli_feature
#define bt_callback_cfg_hf_dont_support_enhanced_call() besbt_cfg.hf_dont_support_enhanced_call
#define bt_callback_cfg_hf_dont_support_3way_call() besbt_cfg.hf_dont_support_3way_call
#define bt_callback_cfg_force_use_cvsd() besbt_cfg.force_use_cvsd
#define bt_callback_cfg_hfp_ag_pts_ecs_02() besbt_cfg.hfp_ag_pts_ecs_02
#define bt_callback_cfg_hfp_ag_pts_ecc() besbt_cfg.hfp_ag_pts_ecc
#define bt_callback_cfg_hfp_ag_pts_ecs_01() besbt_cfg.hfp_ag_pts_ecs_01
#define bt_callback_cfg_hfp_ag_pts_enable() besbt_cfg.hfp_ag_pts_enable
#define bt_callback_cfg_send_l2cap_echo_req() besbt_cfg.send_l2cap_echo_req
#define bt_callback_cfg_support_enre_mode() besbt_cfg.support_enre_mode
#define bt_callback_cfg_le_audio_enabled() besbt_cfg.le_audio_enabled
#define bt_callback_cfg_hsp_enable() besbt_cfg.hsp_enable
#else
bool bt_callback_cfg_lhdc_v3(void);
bool bt_callback_cfg_a2dp_sink_enable(void);
bool bt_callback_cfg_bt_source_enable(void);
bool bt_callback_cfg_bt_sink_enable(void);
bool bt_callback_cfg_avdtp_cp_enable(void);
bool bt_callback_cfg_bt_source_48k(void);
bool bt_callback_cfg_source_unknown_cmd_flag(void);
bool bt_callback_cfg_source_suspend_err_flag(void);
bool bt_callback_cfg_source_get_all_cap_flag(void);
bool bt_callback_cfg_mark_some_code_for_fuzz_test(void);
bool bt_callback_cfg_use_page_scan_repetition_mode_r1(void);
bool bt_callback_cfg_bt_hid_cod_enable(void);
bool bt_callback_cfg_disc_acl_after_auth_key_missing(void);
bool bt_callback_cfg_hfp_support_lc3_swb_en(void);
bool bt_callback_cfg_vendor_codec_en(void);
bool bt_callback_cfg_normal_test_mode_switch(void);
bool bt_callback_cfg_hfp_hf_pts_acs_bv_09_i(void);
bool bt_callback_cfg_sniff(void);
uint16_t bt_callback_cfg_dip_vendor_id(void);
uint16_t bt_callback_cfg_dip_product_id(void);
uint16_t bt_callback_cfg_dip_product_version(void);
uint16_t bt_callback_cfg_dip_vendor_id_source(void);
bool bt_callback_cfg_apple_hf_at_support(void);
bool bt_callback_cfg_hf_support_hf_ind_feature(void);
bool bt_callback_cfg_hf_dont_support_cli_feature(void);
bool bt_callback_cfg_hf_dont_support_enhanced_call(void);
bool bt_callback_cfg_hf_dont_support_3way_call(void);
bool bt_callback_cfg_force_use_cvsd(void);
bool bt_callback_cfg_hfp_ag_pts_ecs_02(void);
bool bt_callback_cfg_hfp_ag_pts_ecc(void);
bool bt_callback_cfg_hfp_ag_pts_ecs_01(void);
bool bt_callback_cfg_hfp_ag_pts_enable(void);
bool bt_callback_cfg_send_l2cap_echo_req(void);
bool bt_callback_cfg_support_enre_mode(void);
bool bt_callback_cfg_le_audio_enabled(void);
bool bt_callback_cfg_hsp_enable(void);
#endif /*BT_REDUCE_CALLBACK_FUNC*/

#ifdef __cplusplus
}
#endif
#endif /* __BT_CALLBACK_FUNC_H__ */
