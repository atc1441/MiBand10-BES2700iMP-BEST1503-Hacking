#ifndef __HFP_I_H__
#define __HFP_I_H__
#include "bluetooth.h"
#include "rfcomm_i.h"
#include "sdp_i.h"

#ifdef BT_HFP_SUPPORT
#if defined(__cplusplus)
extern "C" {
#endif

struct hfp_response {
    const char *data;
    unsigned int offset;
};

struct hf_result_code_parse_t {
    int result_code_len;
    const char* content_start;
    char* content_end;
};

enum hfp_result {
    HFP_RESULT_OK       = 0,
    HFP_RESULT_CONNECT  = 1,
    HFP_RESULT_RING     = 2,
    HFP_RESULT_NO_CARRIER   = 3,
    HFP_RESULT_ERROR        = 4,
    HFP_RESULT_NO_DIALTONE  = 6,
    HFP_RESULT_BUSY         = 7,
    HFP_RESULT_NO_ANSWER    = 8,
    HFP_RESULT_DELAYED      = 9,
    HFP_RESULT_BLACKLISTED  = 10,
    HFP_RESULT_CME_ERROR    = 11,
    HFP_RESULT_OESF         = 12,
};

enum hfp_error {
    HFP_ERROR_AG_FAILURE                = 0,
    HFP_ERROR_NO_CONNECTION_TO_PHONE    = 1,
    HFP_ERROR_OPERATION_NOT_ALLOWED     = 3,
    HFP_ERROR_OPERATION_NOT_SUPPORTED   = 4,
    HFP_ERROR_PH_SIM_PIN_REQUIRED       = 5,
    HFP_ERROR_SIM_NOT_INSERTED          = 10,
    HFP_ERROR_SIM_PIN_REQUIRED          = 11,
    HFP_ERROR_SIM_PUK_REQUIRED          = 12,
    HFP_ERROR_SIM_FAILURE               = 13,
    HFP_ERROR_SIM_BUSY                  = 14,
    HFP_ERROR_INCORRECT_PASSWORD        = 16,
    HFP_ERROR_SIM_PIN2_REQUIRED         = 17,
    HFP_ERROR_SIM_PUK2_REQUIRED         = 18,
    HFP_ERROR_MEMORY_FULL               = 20,
    HFP_ERROR_INVALID_INDEX             = 21,
    HFP_ERROR_MEMORY_FAILURE            = 23,
    HFP_ERROR_TEXT_STRING_TOO_LONG      = 24,
    HFP_ERROR_INVALID_CHARS_IN_TEXT_STRING  = 25,
    HFP_ERROR_DIAL_STRING_TO_LONG       = 26,
    HFP_ERROR_INVALID_CHARS_IN_DIAL_STRING  = 27,
    HFP_ERROR_NO_NETWORK_SERVICE        = 30,
    HFP_ERROR_NETWORK_TIMEOUT           = 31,
    HFP_ERROR_NETWORK_NOT_ALLOWED       = 32,
};

enum hshf_tx_status {
    HFP_TX_IDLE = 0,
    HFP_TX_BUSY,
};

/* notify upper layer */
enum hshf_event_t {
    /* user command event*/
    HSHF_ENTER_PAIRING = 1,
    HSHF_EXIT_PAIRING,
    HF_DIAL_NUM_REQ,
    HF_ANSWER_REQ,
    HF_REJECT_REQ,
    HF_ENDCALL_REQ,
    HF_TRANSFER_REQ,
    HF_DIAL_LASTNUM_REQ,
    HF_TRANSMIT_DTMF,
    HF_VOICE_DIAL_REQ, //10
    HF_VOICE_DIAL_CANCEL_REQ,
    HSHF_CONNECT_REQ,
    HF_DISCONNECT_REQ,
    HSHF_SPK_VOL_UP,
    HSHF_SPK_VOL_DOWN,
    HSHF_TOGGLE_MIC,
    HSHF_TOGGLE_LED,
    HSHF_TOGGLE_VOLBTN,
    HF_ANSWER_ENDCALL_CONN,     //  answer @ ring, end @ talking, connect@idle
    HF_REJ_REDIAL_TRANS_CONN,   // reject @ ring, redial @ connected, transfer @ talking
    HF_RELEASE_HOLD_REJECT_WAIT_REQ, //21
    HF_RELEASE_ACTVIE_ACCEPT_OTHER_REQ,
    HF_HOLD_ACTIVE_ACCEPT_OTHER_REQ,
    HF_CONFERENCE_REQ,
    HSHF_SET_PB_STORAGE,
    HSHF_GET_PB_ITEM,

    HF_HCI_RXTX_IND,

    /* internal event */
    HSHF_EVNT_BEGIN,     //28
    HSHF_RFCOMM_OPENED, //29
    HSHF_CONN_OPENED, 
    HSHF_CONN_CLOSED,
    HSHF_CONN_REQ_FAIL,
    HSHF_REMOTE_NOT_SUPPORT,
    HF_AG_SUPPORTED_FEATURE_IND,
    HF_AG_SUPPORTED_INDICATOR_IND,
    HF_AG_CURRENT_INDICATOR_IND,
    HF_INDICATOR_EVENT_IND,
    HF_CIEV_CALL_IND,
    HF_CIEV_SERVICE_IND,
    HF_CIEV_SIGNAL_IND, //40
    HF_CIEV_ROAM_IND,
    HF_CIEV_BATTCHG_IND,
    HF_CIEV_CALLSETUP_IND,
    HF_CALLER_ID_IND,
    HSHF_VOICE_REC_STATUS_IND,
    HSHF_RING_IND,  //46
    HSHF_SPK_VOL_IND,
    HSHF_MIC_VOL_IND,
    HF_IN_BAND_RING_IND,
    HSHF_AT_CMD_RESPONSE,
    HSHF_PAIR_OK,  //53
    HSHF_PAIR_TOUT,
    HSHF_PAIR_FAILED,
    //// NEW for three way call
    HF_CIEW_CALLHELD_IND,
    HF_CCWA_IND,
    HF_VOICE_REQ,
    // for enter pairing and test mode by combkey
    HSHF_ENTER_TESTMODE,
    HF_EVENT_AT_RESULT_DATA,
    HF_EVENT_CNUM_IND,
};

enum hshf_callsetup {
    CALL_SETUP_NONE,
    CALL_SETUP_INCOMING,
    CALL_SETUP_OUTGOING,
    CALL_SETUP_REMOTE_ALERT,
    CALL_SETUP_ESTABLISHED
};

enum hshf_call {
    CALL_NONE = 0,
    CALL_ESTABLISHED
};

enum hshf_profile {
    PRO_BOTH = 0,
    PRO_HEADSET,
    PRO_HANDSFREE,
    PRO_EXIT,
    PRO_SHOW
};

enum hshf_conn_state {
    STOP,
    STANDBY = 1,
    LISTENING ,  //ready
    QUERING,
    CONNECTING,
    AT_EXCHANGING,
    CONNECTED,
    SCOCONNECTED
};

enum hshf_pb_location {
    LOCATION_SM = 0,
    LOCATION_ME,
    LOCATION_MT,
    LOCATION_DC,
    LOCATION_RC,
    LOCATION_MC,
    LOCATION_LD
};

enum hshf_pb_action {
    ACTION_PREV = 0,
    ACTION_NEXT
};

enum hfp_indicator {
    HFP_INDICATOR_SERVICE = 0,
    HFP_INDICATOR_CALLSETUP,
    HFP_INDICATOR_CALLHELD,
    HFP_INDICATOR_CALL,
    HFP_INDICATOR_SIGNAL,
    HFP_INDICATOR_ROAM,
    HFP_INDICATOR_BATTCHG,
    HFP_INDICATOR_LAST,
    HFP_INDICATOR_UNKNOWN = HFP_INDICATOR_LAST,
};

#define AG_MAX_RX_AT_CMD_SIZE (64)

struct ag_at_cmd_rx_buffer {
    char at_cmd_data[AG_MAX_RX_AT_CMD_SIZE+1];
    uint16_t at_cmd_start;
    uint16_t at_cmd_end;
};

struct hshf_control;

typedef void (*ciev_func_t)(struct hshf_control *chan, uint8_t val);

struct indicator {
    uint8_t index;
    uint8_t min;
    uint8_t max;
    uint8_t val;
    bool disable;
    ciev_func_t cb;
};

struct indicator;

#define HFP_HF_IND_ENHANCED_SAFETY 1
#define HFP_HF_IND_BATTERY_LEVEL   2

struct hf_ind_enhanced_safety {
    bool local_support;
    bool remote_support;
    bool enabled;
    uint8_t value; /* 0 or 1 */
};

struct hf_ind_battery_level {
    bool local_support;
    bool remote_support;
    bool enabled;
    uint8_t value; /* 0 ~ 100 */
};

struct hf_indicator {
    struct hf_ind_enhanced_safety enhanced_safety;
    struct hf_ind_battery_level battery_level;
};

#define MAX_DIAL_NUM_SIZE     0x10
#define MAX_SAVED_CALL_NUM    4

#define SCO_VND_CODEC 0x10

#define CODEC_ID_CVSD 0x01
#define CODEC_ID_MSBC 0x02
#define CODEC_ID_LC3_SWB  0x03  // For LC3_SWB

struct hfp_codec {
    uint8_t type;
    bool local_supported;
    bool remote_supported;
};

#define HF_RC_CTX_CIND_STATUS       1
#define HF_RC_CTX_CIND_QUERY        2
#define HF_RC_CTX_BIND_GET_STATE    3
#define HF_RC_CTX_BIND_QUERY        4

#define SIM_CARD_NUMBER_MAX     (2)
struct hshf_control {
    struct bdaddr_t remote;
    uint16_t conn_handle;
    uint32_t rfcomm_handle;
    uint8 device_id;
    uint8 listen_channel;
    uint8 disc_reason;
    uint8 audio_up;
    bool initiator;
    bool at_cmd_upper_handle;
    uint16_t at_cmd_count;
    struct single_link_head_t at_cmd_queue;

#if HFP_CMD_FLOW_CONTROL_ENABLE==1
    unsigned int tx_time;
    uint8_t tx_timeout_timer;
    enum hshf_tx_status tx_status;
#endif

    uint8_t rc_process_context;
    uint8_t negotiated_codec;
    bool is_ag_role;
    bool sco_wait_codec_sync;
#if defined(HFP_SUPPORT_LC3_SWB)
    struct hfp_codec hfp_codecs[3];
#else
    struct hfp_codec hfp_codecs[2];
#endif

    struct indicator ag_ind[HFP_INDICATOR_LAST];

    struct hf_indicator hf_ind;

    uint32_t chld_features;

    uint8 bsir_enable;
    uint8 status_call;              /* Phone status info - call */
    uint8 status_service;           /* Phone status info - service */
    uint8 status_callsetup;         /* Phone status info - callsetup*/
    uint8 status_callheld;          /* Phone status info - callheld*/

    uint32_t ag_slc_completed: 1;
    uint32_t ag_status_report_enable: 1;
    uint32_t ag_calling_line_notify: 1;
    uint32_t ag_call_waiting_notify: 1;
    uint32_t ag_extended_error_enable: 1;
    struct hfp_ag_module_handler* ag_module;

    struct ag_at_cmd_rx_buffer ag_rx_at_cmd;

    uint32_t hf_features;    /*hf supported feature bitmap:            */
                                /* bit 0 - EC/NR function                */
                                /*     1 - Call waiting and 3-way calling */
                                /*     2 - CLI presentation capability   */
                                /*     3 - Voice recognition activation   */
                                /*     4 - Remote volume control         */
                                /*     5 - Enhance call status            */
                                /*     6 - Enhanced call control         */
    uint32_t ag_features;   /* AG supported feature bitmap  */
                                /* bit0 - 3-way calling      */
                                /*    1 - EC/NR function    */
                                /*    2 - Voice recognition  */
                                /*    3 - In-band ring tone */
                                /*    4 - Attach a number to a voice tag*/
                                /*    5 - Ablility to reject a call */
                                /*    6 - Enhanced call status     */
                                /*    7 - Enhanced call control     */
                                /*    8 - extended error result codes*/

    enum hshf_conn_state state;   /*check if it is connecting now,
                                   if it is connecting,the hf_connect_req
                                   will not work
                                   */
    uint8 speak_volume;
    uint8 mic_gain;
    uint8 voice_rec;
    uint8 voice_rec_param;
    uint8 ciev_status;
    uint8 cmd_result;
    uint8 delay_send_speak_volume_timer; //When device continue change volume, just send last change result
    bool is_cust_cmd_done;
    bool client_enabled;
    bool last_dial_number;

    bool is_hsp_profile;
    bool current_is_virtual_call;
    char *ptr;
    struct pp_buff *rx_result_code_ppb;
};

struct _hshf_channel {
    struct hshf_control ctl;
};

struct hfp_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    uint32 rfcomm_handle;
    struct hshf_control *hfp_ctl;
};

struct hfp_ctx_output {
    uint32 rfcomm_handle;
};

typedef struct {
    uint8 length;
    char *caller_id;
} hf_caller_id_ind_t;

typedef struct {
    uint8_t device_id;
    struct bdaddr_t *remote;
    // 0: connect_fail or disconnected, 1:connected
    uint8_t connected;
    uint8_t* connected_codec;
} hf_set_sco_state_param_t;

typedef void (*hfp_destroy_func_t)(void *user_data);
typedef void (*hfp_debug_func_t)(const char *str, void *user_data);
typedef void (*hfp_command_func_t)(const char *command, void *user_data);
typedef void (*hfp_hf_result_func_t)(struct hfp_response *context, void *user_data);
typedef void (*hfp_response_func_t)(enum hfp_result result, enum hfp_error cme_err, void *user_data);

typedef void (*hfp_callback_t)(struct hshf_control *, uint8_t, void *);

enum hshf_conn_state hshf_get_state(struct hshf_control *chan);
void hshf_set_state(struct hshf_control *chan, enum hshf_conn_state state);
const char *hshf_state2str(enum hshf_conn_state state);
const char *hshf_event2str(enum hshf_event_t event);
int8 hf_disconnect_req(struct hshf_control *chan);
int8 hf_disconnect_req_v2(struct hshf_control *chan,uint8 reason);
int8 hshf_exit_sniff(struct hshf_control *chan);
int8 hshf_connect_req (struct hshf_control *hshf_ctl, struct bdaddr_t *remote);
int8 hshf_query_sdp_only(struct hshf_control *hshf_ctl, struct bdaddr_t *remote);
int8 hshf_create_codec_connection(struct bdaddr_t *bdaddr, struct hshf_control *chan);
int8 hf_createSCO(struct bdaddr_t *bdaddr, void *chan);
void hfp_init(hfp_callback_t callback,
        struct hshf_control* (*accept)(uint8_t device_id, const bt_bdaddr_t* addr, uint8 server_channel),
        struct hshf_control* (*search)(uint8_t device_id));

int hf_chan_init(struct hshf_control *chan);
bool hshf_disable_nrec(struct hshf_control *chan);
bool hshf_report_speaker_volume(struct hshf_control *chan, uint8_t gain);
bool hshf_report_mic_volume(struct hshf_control *chan, uint8_t gain);
bool hshf_attach_voice_tag(struct hshf_control *chan);
bool hshf_hf_ind_activation(struct hshf_control *chan);
bool hshf_send_custom_cmd(struct hshf_control *chan, const char *cmd);
bool hshf_hungup_call(struct hshf_control *chan);
bool hshf_dial_number(struct hshf_control *chan, uint8 *number, uint16 len);
bool hshf_dial_memory(struct hshf_control *chan, int location);
bool hshf_answer_call(struct hshf_control *chan);
bool hshf_redial_call(struct hshf_control *chan);
bool hshf_batt_report(struct hshf_control *chan, uint8_t level);
bool hshf_call_hold(struct hshf_control *chan, int8 action, int8 index);
void hshf_set_hf_indicator_enabled(bool enable);
bool hshf_report_enhanced_safety(struct hshf_control *chan, uint8_t value);
bool hshf_report_battery_level(struct hshf_control* chan, uint8_t value);
bool hshf_update_indicators_value(struct hshf_control *chan, uint8_t assigned_num, uint8_t level);
bool hshf_hf_indicators_1(struct hshf_control *chan);
bool hshf_hf_indicators_2(struct hshf_control *chan);
bool hshf_hf_indicators_3(struct hshf_control *chan);
bool hshf_codec_conncetion(struct hshf_control *chan);
bool hshf_list_current_calls(struct hshf_control *chan);
bool hshf_enable_voice_recognition(struct hshf_control *chan, uint8_t en);
bool hshf_is_voice_recognition_active(struct hshf_control *chan);
uint32 hfp_get_rfcomm_handle(struct hshf_control *hfp_ctl);
uint32 hfp_save_ctx(struct hshf_control *hfp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 hfp_restore_ctx(struct hfp_ctx_input *input, struct hfp_ctx_output *output);
void hshf_register_tws_current_ibrt_slave_role_callback(bool (*cb)(void* addr));
typedef void (*hshf_command_result_callback)(char *data, uint8_t device_id, uint8_t result);
void hfp_command_response_result_callback(hshf_command_result_callback result_cb);
bool hshf_hsp_send_key_pressed(struct hshf_control *chan);
void hshf_ibrt_role_switch_handle(struct hshf_control *chan, bt_bdaddr_t *remote);

void ag_process_input(struct hshf_control *hfp, const char *data, size_t len);
void hf_register_peer_sco_codec_receive_handler(void (*cb)(uint8_t device_id,void * chan,uint8_t codec));
void hf_receive_peer_sco_codec_info(const void* remote, uint8_t codec);
bool hfp_hf_send_command(struct hshf_control *hfp, hfp_response_func_t resp_cb, const char *data, unsigned int len);
bool hfp_hf_send_command_do(struct hshf_control *hfp, hfp_response_func_t resp_cb, const char *data, uint16_t len, bool is_cust_cmd ,uint8 param);
#define hfp_hf_send_command(h,rc,d,dl) call_wrapper(btprf, hfp_hf_send_command_do,h,rc,d,(uint16_t)dl,false,0xFF)
#define hfp_hf_send_command_private(h,rc,d,dl,param) call_wrapper(btprf, hfp_hf_send_command_do,h,rc,d,(uint16_t)dl,false,param)
bool hfp_context_get_string(struct hfp_response *context, char *buf, uint8_t len);
void hfp_context_skip_field(struct hfp_response *context);
void skip_whitespace(struct hfp_response *context);
bool hfp_context_open_container(struct hfp_response *context);
bool hfp_context_close_container(struct hfp_response *context);
bool hfp_context_get_unquoted_string(struct hfp_response *context, char *buf, uint8_t len);
bool hfp_context_has_next(struct hfp_response *context);
bool hfp_context_get_range(struct hfp_response *context, unsigned int *min, unsigned int *max);
bool hfp_context_get_number(struct hfp_response *context, unsigned int *val);
void hf_set_sco_path_status(hf_set_sco_state_param_t *param);

struct hshf_control *hfp_search_address(struct bdaddr_t *bdaddr);
bool hfp_msbc_is_enable(uint8_t device_id, struct bdaddr_t *bdaddr);
bool hf_indicator_feat_supported(struct hshf_control *chan);

struct hfp_ag_call_info
{
    uint8_t direction; // 0 outgoing, 1 incoming
    uint8_t state; // 0 active, 1 held, 2 outgoing dialing, 3 outgoing alerting, 4 incoming, 5 waiting, 6 held by Response and Hold
    uint8_t mode; // 0 voice, 1 data, 2 fax
    uint8_t multiparty; // 0 is not one of multiparty call parties, 1 is one of.
    const char* number; // calling number, optional
};

typedef int (*hfp_ag_handler)(void* hfp_chan);
typedef int (*hfp_ag_handler_int)(void* hfp_chan, int n);
typedef int (*hfp_ag_handler_str)(void* hfp_chan, const char* s);
typedef int (*hfp_ag_iterate_call_handler)(void* hfp_chan, struct hfp_ag_call_info* out);
typedef const char* (*hfp_ag_query_operator_handler)(void* hfp_chan);

struct hfp_ag_module_handler
{
    hfp_ag_handler answer_call;
    hfp_ag_handler hungup_call;
    hfp_ag_handler dialing_last_number;
    hfp_ag_handler release_held_calls;
    hfp_ag_handler release_active_and_accept_calls;
    hfp_ag_handler hold_active_and_accept_calls;
    hfp_ag_handler add_held_call_to_conversation;
    hfp_ag_handler connect_remote_two_calls;
    hfp_ag_handler disable_mobile_nrec;
    hfp_ag_handler_int release_specified_active_call;
    hfp_ag_handler_int hold_all_calls_except_specified_one;
    hfp_ag_handler_int hf_battery_change; /* battery level 0 ~ 100 */
    hfp_ag_handler_int hf_spk_gain_change; /* speaker gain 0 ~ 15 */
    hfp_ag_handler_int hf_mic_gain_change; /* mic gain 0 ~ 15 */
    hfp_ag_handler_int transmit_dtmf_code;
    hfp_ag_handler_int memory_dialing_call;
    hfp_ag_handler_str dialing_call;
    hfp_ag_handler_str handle_at_command;
    hfp_ag_query_operator_handler query_current_operator;
    hfp_ag_iterate_call_handler iterate_current_call;
};

struct hfp_ag_module_handler;
void hfp_ag_send_call_active_status(struct hshf_control *hfp, bool active);
void hfp_ag_send_callsetup_status(struct hshf_control *hfp, uint8 status);
void hfp_ag_send_callheld_status(struct hshf_control *hfp, uint8 status);
void hfp_ag_send_calling_ring(struct hshf_control *hfp, const char* number);
bool hfp_ag_set_speaker_gain(struct hshf_control *hfp, uint8 volume);
bool hfp_ag_set_microphone_gain(struct hshf_control *hfp, uint8 volume);
bool hfp_ag_set_inband_ring_tone(struct hshf_control *hfp, bool enabled);
void hfp_ag_send_call_waiting_notification(struct hshf_control *hfp, const char* number);
bool hfp_ag_send_result_code(struct hshf_control *hfp, const char *data, int len);
void hfp_ag_register_module_handler(struct hshf_control* hfp, struct hfp_ag_module_handler* handler);
void hfp_ag_send_result_ok(struct hshf_control *hfp);
void hfp_ag_send_result_error(struct hshf_control *hfp);
void hfp_ag_send_service_status(struct hshf_control *hfp, bool enabled);
void hfp_ag_send_mobile_signal_level(struct hshf_control *hfp, uint8 level);
void hfp_ag_send_mobile_roam_status(struct hshf_control *hfp, bool enabled);
bool hfp_ag_send_mobile_battery_level(struct hshf_control *hfp, uint8 level);

#if defined(__cplusplus)
}
#endif
#endif /* BT_HFP_SUPPORT */
#endif /*__HFP_I_H__*/
