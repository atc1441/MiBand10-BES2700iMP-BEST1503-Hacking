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
#ifndef __BLE_GAP_I_H__
#define __BLE_GAP_I_H__
#include "bluetooth.h"
#include "gap_service.h"
#include "hci_i.h"
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    bool bonding_type;
    bool require_mitm_protection;
    bool secure_connection_support;
    bool keypress_notify;
    bool ct2; // security h7 function support or not
} smp_auth_requirements_t;

typedef struct {
    uint8_t opcode;
    uint8_t io_cap; // RFU if SC pairing intiated over BR/EDR
    uint8_t oob_flag; // 0x00 OOB auth data not present, 0x01 present, RFU if SC pairing intiated over BR/EDR
    uint8_t auth_req; // RFU if SC pairing intiated over BR/EDR except the CT2 bit
    uint8_t max_enc_key_size; // max encryption key size the device can support, 7 to 16 octets
    uint8_t init_key_dist; // which keys the initiator is requesting to distribute / generate or use during the Transport Specific Key Distribution phase
    uint8_t resp_key_dist; // which keys the initiator is requesting the responder to distribute / generate or use during the TSKD phase
} __attribute__ ((packed)) smp_pairing_req_t;

typedef struct
{
    bt_bdaddr_t factory_bt_addr;
    bt_bdaddr_t factory_le_addr;
    bt_bdaddr_t le_static_addr;
    const bt_bdaddr_t *hci_bt_addr;
    const bt_bdaddr_t *hci_le_addr;
    const bt_bdaddr_t *hci_random_addr;
    // Used for speed up smp pairing
    // when using DHkey generation
    const uint8_t *secret_key_256;
    const uint8_t *public_key_256;
    uint32_t sign_counter;
    uint8_t irk[16];
    uint8_t csrk[16];
    uint8_t local_database_hash[16];
    uint8_t max_filter_list_size;
    uint8_t max_resolving_list_size;
    uint8_t max_pa_list_size;
    uint8_t max_adv_monitored_list_size;
    uint8_t max_num_adv_sets;
    uint8_t max_size_att_prep_q;
    uint8_t gatt_self_prf_id;
    uint16_t max_adv_data_len;
    uint32_t le_feat_page_0[2];
    uint32_t le_feat_page_1[1];
    uint16_t data_sign_support: 1;
    uint16_t use_random_identity_address: 1;
    uint16_t reso_address_by_host: 1;
    uint16_t address_reso_support: 1;
    uint16_t aes_encrypt_by_host: 1;
} gap_local_info_t;

typedef void (*gap_user_confirm_func_t)(void *priv, bool user_confirmed);
typedef void (*gap_data_func_t)(void *priv, const uint8_t *data);
typedef void (*gap_passkey_func_t)(void *priv, uint32_t passkey);
typedef void (*gap_oob_auth_data_callback_t)(void *priv, const gap_smp_oob_auth_data_t *data);

bt_status_t gap_start_aes_128_encrypt(const uint8_t *key, const uint8_t *data,
        hci_cmd_evt_func_t cmd_cb, void *priv, void *cont, void *cmpl);
bt_status_t gap_send_hci_le_encrypt(const uint8_t *key, const uint8_t *data,
        hci_cmd_evt_func_t cmd_cb, void *priv, void *cont, void *cmpl);
bt_status_t gap_send_hci_le_rand(hci_cmd_evt_func_t cmpl_status_cb, void *priv, void *cont);
bt_status_t gap_send_hci_le_read_local_p256_public_key(hci_cmd_evt_func_t cmpl_status_cb, void *priv, void *cont);
bt_status_t gap_send_hci_le_gen_dhkey(const uint8_t *pkx, const uint8_t *pky, bool use_debug_private_key,
        hci_cmd_evt_func_t cmpl_status_cb, void *priv, void *cont);
bt_status_t gap_send_hci_le_enable_encryption(gap_conn_item_t *conn, const uint8_t *rand, const uint8_t *ediv, const uint8_t *ltk,
        hci_cmd_evt_func_t cmd_cb, void *priv, void *cont);
bt_status_t gap_send_le_ltk_request_reply(gap_conn_item_t *conn, const uint8_t *ltk, bool positive_reply);
bt_status_t gap_ask_user_numeric_comparison(gap_conn_item_t *conn, uint32_t user_confirm_value, void *priv);
bt_status_t gap_ask_user_ltk_req_reply(gap_conn_item_t *conn, const gap_ltk_enc_info_t *p_req_enc);
bt_status_t gap_ask_input_6_digit_passkey(gap_conn_item_t *conn, gap_passkey_func_t passkey_cb, void *priv);
bt_status_t gap_ask_display_6_digit_passkey(gap_conn_item_t *conn, uint32_t passkey);
bt_status_t gap_get_tk_from_oob_data(gap_conn_item_t *conn, gap_data_func_t tk_cb, void *priv);
bt_status_t gap_get_peer_oob_auth_data(gap_conn_item_t *conn, gap_oob_auth_data_callback_t oob_cb, void *priv);
bt_status_t gap_get_local_oob_auth_data(gap_conn_item_t *conn, gap_oob_auth_data_callback_t oob_cb, void *priv);
bt_status_t gap_start_tx_block_authentication(gap_conn_item_t *conn, gap_who_started_auth_t who);
bt_status_t gap_start_rx_block_authentication(gap_conn_item_t *conn, gap_who_started_auth_t who);
bt_status_t gap_tx_rx_block_authentication(gap_conn_item_t *conn, gap_who_started_auth_t who);
bt_status_t gap_start_eatt_block_authentication(gap_conn_item_t *conn, gap_who_started_auth_t who);
void gap_get_smp_own_specific_ia(gap_conn_item_t *conn, ble_bdaddr_t *ia);
bool gap_is_gatt_need_start_mtu_exchange(gap_conn_item_t *conn);
void gap_report_le_conn_disconnected(gap_conn_item_t *conn, uint8_t reason);
void gap_report_le_conn_cache_data(gap_conn_item_t *conn, const gatt_client_cache_t *cli_cache, const gatt_server_cache_t *srv_cache);
void gap_report_le_conn_restore_done(gap_conn_item_t *conn);

gap_local_info_t *gap_local_info(void);
void gap_prepare_ble_stack(void);
void gap_gen_local_random(uint8_t *random, uint8_t bytes);
bool gap_is_pairing_method_accept(const gap_bond_sec_t *sec, uint8_t accept_method);
void gap_get_smp_own_specific_irk(gap_conn_item_t *conn, uint8_t *irk);
smp_requirements_t gap_local_smp_requirements(gap_conn_item_t *conn, smp_pairing_req_t *peer_req);
int gatt_conn_event_handler(uintptr_t connhdl, gap_conn_event_t event, gap_conn_callback_param_t param);
bt_status_t gatt_conn_ready_handler(gap_conn_item_t *conn);
void gatt_continue_rx_packet(gap_conn_item_t *item);
void gatt_continue_tx_packet(gap_conn_item_t *item);
bool gap_is_gatt_data_allowed_send(gap_conn_item_t *conn);
void gap_report_mtu_is_exchanged(gap_conn_item_t *conn, uint16_t mtu);
void gap_report_smp_pairing_complete(gap_conn_item_t *conn, uint8_t err_code);
void data_path_init(uint8_t init_type);
void bap_audio_evt_handle(uint8_t subcode, const uint8_t *evt_data, uint8_t len);
void gap_get_local_secret_public_key_pair(const uint8_t **pp_sec_key_256, const uint8_t **pp_pub_key_256);
void gap_ctkd_notify_ltk_derived(gap_conn_item_t *bredr_conn, bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr, bool wait_peer_kdist);
void gap_ctkd_notify_link_key_derived(gap_conn_item_t *conn, const bt_bdaddr_t *bt_addr, const uint8_t *link_key, bool wait_peer_ia);
void gap_ltk_from_lk_generated(gap_conn_item_t *conn, const uint8_t *ltk, bool waiting_peer_kdist);
void gap_lk_from_ltk_generated(gap_conn_item_t *conn, const uint8_t *link_key, bool waiting_peer_ia);
void gap_recv_smp_encrypted(gap_conn_item_t *conn, bool new_pair, uint8_t error_code);
bool gap_recv_smp_security_request(gap_conn_item_t *conn, uint8_t auth_req);
bool gap_recv_smp_pairing_requirements(gap_conn_item_t *conn, bool is_pairing_request, const smp_requirements_t *p_smp_req);
void gap_legacy_ltk_ediv_rand_generated(gap_conn_item_t *conn);
void gap_recv_peer_legacy_ltk_ediv_rand(gap_conn_item_t *conn);
void gap_recv_peer_irk_ia(gap_conn_item_t *conn);
void gap_recv_peer_csrk(gap_conn_item_t *conn);
bool gap_is_directly_send_sec_error_rsp(void);
bool gap_is_directly_report_sec_error(void);
void gap_set_default_key_size(uint8_t key_size);
void gap_set_default_auth_req(uint8_t mitm_auth);
void gap_pts_set_ble_l2cap_test(bool test);
bool gap_is_pts_ble_l2cap_test(void);
void gap_pts_set_use_passkey_entry(void);
void gap_pts_set_use_oob_method(void);
void gap_pts_set_no_mitm_auth(void);
void gap_pts_set_display_only(void);
void gap_pts_set_keyboard_only(void);
void gap_pts_set_no_bonding(void);
void gap_pts_set_dont_start_smp(bool dont_auto_start_smp);
bool gap_pts_get_dont_start_smp(void);
void gap_pts_gen_linkkey_from_ltk(void);
void gap_pts_set_dist_irk_only(void);
void gap_pts_set_dist_csrk(void);

/*SMP INTF*/
void smp_pairing_end(gap_conn_item_t *conn, uint8_t error_code);
bt_status_t smp_start_authentication(gap_conn_item_t *conn, gap_who_started_auth_t who, uint32_t ca);
bt_status_t smp_send_pairing_request(gap_conn_item_t *conn, const smp_requirements_t *p_requirements);
bt_status_t smp_send_security_request(gap_conn_item_t *conn, uint8_t auth_req);
bt_status_t smp_bredr_ctkd_request(uint16_t connhdl);
void smp_continue_bredr_pairing(gap_conn_item_t *conn);
void smp_receive_peer_ltk_req(gap_conn_item_t *conn, struct hci_ev_le_ltk_request *p);
bt_status_t smp_check_send_ltk_key_reply(gap_conn_item_t *conn, bool negative_reply, const uint8_t *ltk_reply);
bt_status_t smp_check_enable_encryption(gap_conn_item_t *conn, const gap_ltk_enc_info_t *enc_info, const uint8_t *ltk);
bt_status_t smp_check_send_pairing_rsp(gap_conn_item_t *conn, bool pairing_failed, smp_error_code_t err_code);
void smp_receive_enc_change(gap_conn_item_t *conn, uint8_t opcode, struct hci_ev_encryption_change_v2 *p);
bool smp_random(void *priv, gap_key_callback_t cmpl);
bool smp_e(const uint8_t *key_128_le, const uint8_t *plain_128_le, gap_key_callback_t cmpl, void *priv);
bool smp_gen_irk(gap_key_callback_t func, void *priv);
bool smp_gen_csrk(gap_key_callback_t func, void *priv);
bool smp_ah(const uint8_t *k_128_le, const uint8_t *r_24_le, gap_key_callback_t func, void* priv);
bool smp_aes_ccm(const uint8_t *key_128_le, const uint8_t *nonce_le, const uint8_t *m, uint16_t m_len, bool encrypt, uint8_t add_auth_data, gap_key_callback_t func, void *priv);
bool smp_aes_cmac(const uint8_t *k_128_le, const uint8_t *m_le, uint16_t m_len, gap_key_callback_t func, void *priv);
bool smp_f5_gen_key_T(const uint8_t *DHKey_256_le, gap_key_callback_t func, void *priv);
bool smp_f5_gen_mackey_ltk(const uint8_t *key_T_128_le, const uint8_t *Ra_128_le, const uint8_t *Rb_128_le,
        const uint8_t *A, const uint8_t *B, gap_key_callback_t mackey, gap_key_callback_t ltk, void *priv);
bool smp_f6(const uint8_t *W_128_le, const uint8_t *N1_128_le, const uint8_t *N2_128_le,
        const uint8_t *R_128_le, const uint8_t *IOcap_24_le, const uint8_t *A1_56_le, const uint8_t *A2_56_le,
        gap_key_callback_t func, void *priv);
bool smp_g2(const uint8_t *PKax_U_256_le, const uint8_t *PKbx_V_256_le, const uint8_t *Ra_X_128_le,
        const uint8_t *Rb_Y_128_le, gap_key_callback_t func, void *priv);
bool smp_h6(const uint8_t *W_128_le, const uint8_t *key_id_32_le, gap_key_callback_t func, void *priv);
bool smp_h7(const uint8_t *salt_128_le, const uint8_t *W_128_le, gap_key_callback_t func, void *priv);
bool smp_h8(const uint8_t *k_128_le, const uint8_t *s_128_le, const uint8_t *key_id_32_le, gap_key_callback_t func, void *priv);
bool smp_big_gsk_gen(const uint8_t *gltk_128_le, const uint8_t *rand_gskd_128_le, gap_key_callback_t func, void *priv);
bool smp_signature(uint8_t *m_le, uint16_t len, uint32_t sign_counter, gap_key_callback_t func, void *priv);
bool smp_linkkey_to_iltk(const uint8_t *linkkey, bool ct2, gap_key_callback_t func, void *priv);
bool smp_iltk_to_ltk(const uint8_t *iltk, gap_key_callback_t func, void *priv);
bool smp_ltk_to_ilk(const uint8_t *ltk, bool ct2, gap_key_callback_t func, void *priv);
bool smp_ilk_to_linkkey(const uint8_t *ilk, gap_key_callback_t func, void *priv);
void smp_aes_key_xor(const uint8_t *a, const uint8_t *b, gap_key_callback_t func, void *priv);
void smp_aes_key_shift(const uint8_t *key, gap_key_callback_t func, void *priv);
void smp_gen_secure_oob_auth_data(gap_oob_auth_data_callback_t cb, void *priv);
void smp_input_oob_legacy_tk(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, const uint8_t *tk);
void smp_input_6_digit_passkey(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, uint32_t passkey);
void smp_input_numeric_confirm(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, bool user_confirmed);
void smp_input_peer_oob_auth_data(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, const gap_smp_oob_auth_data_t *data);
void smp_input_local_oob_auth_data(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, const gap_smp_oob_auth_data_t *data);

struct attr_context_t;

typedef struct {
    uint8_t error_code;
    uint8_t req_opcode;
    uint16_t err_handle;
} att_rsp_header_t;

typedef struct {
    att_rsp_header_t head;
    const uint8_t *pdu_data;
    uint16_t pdu_len;
} att_response_t;

typedef enum {
    SMP_PAIRING_REQ             = 0x01, // le-u acl-u
    SMP_PAIRING_RSP             = 0x02, // le-u acl-u
    SMP_PAIRING_CONFIRM         = 0x03, // le-u
    SMP_PAIRING_RANDOM          = 0x04, // le-u
    SMP_PAIRING_FAILED          = 0x05, // le-u acl-u
    SMP_ENCRYPTION_INFO         = 0x06, // le-u
    SMP_CENTRAL_IDENTIFICATION  = 0x07, // le-u
    SMP_IDENTITY_INFO           = 0x08, // le-u acl-u
    SMP_IDENTITY_ADDR_INFO      = 0x09, // le-u acl-u
    SMP_SIGNING_INFO            = 0x0a, // le-u acl-u
    SMP_SECURITY_REQ            = 0x0b, // le-u
    SMP_PAIRING_PUBLIC_KEY      = 0x0c, // le-u
    SMP_PAIRING_DHKEY_CHECK     = 0x0d, // le-u
    SMP_PAIRING_KEYPRESS_NOTIFY = 0x0e, // le-u
} smp_opcode_t;

typedef smp_pairing_req_t smp_pairing_rsp_t;

typedef enum {
    SMP_PHASE_IDLE                          = 0x00,
    SMP_PHASE_PAIRING_START                 = 0x01,
    SMP_PHASE_GEN_TK_RAND                   = 0x02,
    SMP_PHASE_GEN_CFM_VALUE                 = 0x03,
    SMP_PHASE_WAIT_PAIRING_CONFIRM          = 0x04,
    SMP_PHASE_WAIT_PAIRING_RANDOM           = 0x05,
    SMP_PHASE_VERIFY_CONFIRM                = 0x06,
    SMP_PHASE_GEN_STK                       = 0x07,
    SMP_PHASE_GEN_PUB_KEY                   = 0x08,
    SMP_PHASE_WAIT_PAIRING_PUBLIC_KEY       = 0x09,
    SMP_PHASE_GEN_DHKEY                     = 0x0a,
    SMP_PHASE_GET_PASSKEY                   = 0x0b,
    SMP_PHASE_GET_L_OOB_AUTH_DATA           = 0x0c,
    SMP_PHASE_GET_P_OOB_AUTH_DATA           = 0x0d,
    SMP_PHASE_GEN_RANDOM                    = 0x0e,
    SMP_PHASE_GEN_USER_VALUE                = 0x0f,
    SMP_PHASE_START_STAGE_2                 = 0x10,
    SMP_PHASE_GEN_LTK                       = 0x11,
    SMP_PHASE_GEN_CHECK_VALUE               = 0x12,
    SMP_PHASE_WAIT_PAIRING_DHKEY_CHECK      = 0x13,
    SMP_PHASE_VERIFY_CHECK                  = 0x14,
    SMP_PHASE_WAIT_PEER_LTK_REQ             = 0x15,
    SMP_PHASE_WAIT_ENC_CHANGE               = 0x16,
    SMP_PHASE_START_PHASE_3                 = 0x17,
    SMP_PHASE_WAIT_DIST_KEY                 = 0x18,
    SMP_PHASE_FINISHED                      = 0x19,
    SMP_PHASE_MAX_NUM                       = 0x1a,
} smp_phase_t;

typedef enum {
    SMP_EVENT_NONE                      = 0x00,
    SMP_EVENT_PAIRING_REQ               = SMP_PAIRING_REQ,              // 0x01
    SMP_EVENT_PAIRING_RSP               = SMP_PAIRING_RSP,              // 0x02
    SMP_EVENT_PAIRING_CONFIRM           = SMP_PAIRING_CONFIRM,          // 0x03
    SMP_EVENT_PAIRING_RANDOM            = SMP_PAIRING_RANDOM,           // 0x04
    SMP_EVENT_PAIRING_FAILED            = SMP_PAIRING_FAILED,           // 0x05
    SMP_EVENT_ENCRYPTION_INFO           = SMP_ENCRYPTION_INFO,          // 0x06
    SMP_EVENT_CENTRAL_IDENTIFICATION    = SMP_CENTRAL_IDENTIFICATION,   // 0x07
    SMP_EVENT_IDENTITY_INFO             = SMP_IDENTITY_INFO,            // 0x08
    SMP_EVENT_IDENTITY_ADDR_INFO        = SMP_IDENTITY_ADDR_INFO,       // 0x09
    SMP_EVENT_SIGNING_INFO              = SMP_SIGNING_INFO,             // 0x0a
    SMP_EVENT_SECURITY_REQ              = SMP_SECURITY_REQ,             // 0x0b
    SMP_EVENT_PAIRING_PUBLIC_KEY        = SMP_PAIRING_PUBLIC_KEY,       // 0x0c
    SMP_EVENT_PAIRING_DHKEY_CHECK       = SMP_PAIRING_DHKEY_CHECK,      // 0x0d
    SMP_EVENT_PAIRING_KEYPRESS_NOTIFY   = SMP_PAIRING_KEYPRESS_NOTIFY,  // 0x0e
    SMP_EVENT_RECV_PASSKEY              = 0x11,
    SMP_EVENT_RECV_L_OOB_AUTH_DATA      = 0x12,
    SMP_EVENT_RECV_P_OOB_AUTH_DATA      = 0x13,
    SMP_EVENT_RECV_RANDOM               = 0x14,
    SMP_EVENT_RECV_STK                  = 0x15,
    SMP_EVENT_RECV_CFM_VALUE            = 0x16,
    SMP_EVENT_RECV_PUB_KEY              = 0x17,
    SMP_EVENT_RECV_DHKEY                = 0x18,
    SMP_EVENT_RECV_USER_VALUE           = 0x19,
    SMP_EVENT_RECV_LTK                  = 0x1a,
    SMP_EVENT_RECV_CHECK_VALUE          = 0x1b,
    SMP_EVENT_RECV_PEER_LTK_REQ         = 0x1c,
    SMP_EVENT_RECV_ENC_CHANGE           = 0x1d,
} smp_event_t;

typedef struct {
    smp_phase_t curr_phase;
    smp_event_t allow_event;
    smp_phase_t next_phase;
    smp_event_t second_event;
} smp_transfer_t;

typedef struct {
    uint16_t initiator: 1;
    uint16_t secure_debug_mode: 1;
    uint16_t pairing_random_received: 1;
    uint16_t pairing_confirm_received: 1;
    uint16_t pairing_dhkey_check_received: 1;
    uint16_t peer_ltk_req_received: 1;
} smp_conn_flag;

#define SMP_SECURE_PAIRING_FLAG 0x10
#define SMP_PAIRING_METHOD_MASK 0x0f

struct smp_conn_item_t;
struct smp_receive_t;

typedef void (*smp_recv_func_t)(struct smp_conn_item_t *smp, const struct smp_receive_t *recv);
typedef void (*smp_rx_handle_t)(struct smp_conn_item_t *smp, smp_event_t event, const struct smp_receive_t *recv);

typedef struct smp_receive_t {
    uint8_t error_code;
    uint16_t data_len;
    const uint8_t *data;
    smp_recv_func_t cmpl;
} smp_receive_t;

typedef struct smp_conn_item_t {
    smp_conn_flag flag;
    smp_phase_t phase;
    uint8_t smp_rsp_wait_timer;
    uint8_t init_key_dist;
    uint8_t resp_key_dist;
    uint8_t wait_peer_kdist;
    smp_event_t curr_expect_event;
    smp_pairing_method_t method;
    smp_pairing_req_t pair_req;
    smp_pairing_rsp_t pair_rsp;
    smp_pairing_req_t *l_pair;
    smp_pairing_req_t *p_pair;
    uint8_t tk[GAP_KEY_LEN];
    uint8_t l_rand[GAP_KEY_LEN];
    uint8_t p_rand[GAP_KEY_LEN];
    uint8_t c1_p2_le[GAP_KEY_LEN];
    uint8_t l_confirm[GAP_KEY_LEN];
    uint8_t p_confirm[GAP_KEY_LEN];
    uint8_t p_legacy_ediv[2];
    uint8_t p_legacy_rand[8];
    uint8_t iltk[GAP_KEY_LEN];
    uint8_t ilk[GAP_KEY_LEN];
    const uint8_t *l_secret_key;
    uint8_t l_pkx[GAP_PUB_KEY_LEN];
    uint8_t l_pky[GAP_PUB_KEY_LEN];
    uint8_t p_pkx[GAP_PUB_KEY_LEN];
    uint8_t p_pky[GAP_PUB_KEY_LEN];
    uint8_t l_dhkey[GAP_PUB_KEY_LEN];
    uint8_t l_mackey[GAP_KEY_LEN];
    uint8_t l_dhkey_check_value[GAP_KEY_LEN];
    uint8_t p_dhkey_check_value[GAP_KEY_LEN];
    uint8_t ra[GAP_KEY_LEN];
    uint8_t rb[GAP_KEY_LEN];
    uint8_t a_addr[GAP_ADDR_LEN+1];
    uint8_t a_iocap[3];
    uint8_t b_addr[GAP_ADDR_LEN+1];
    uint8_t b_iocap[3];
    const uint8_t *a_rand;
    const uint8_t *b_rand;
    uint32_t user_confirm_value;
    uint32_t passkey;
    uint8_t passkey_entry_repeat;
    uint16_t connhdl;
    gap_conn_item_t *conn;
    struct hci_ev_le_ltk_request ltk_req;
    smp_rx_handle_t curr_rx_handle;
} smp_conn_item_t;

#if defined(__cplusplus)
}
#endif
#endif /* __BLE_GAP_I_H__ */
