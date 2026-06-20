#ifndef __APP_TENCENT_SMARTVOICE_H_
#define __APP_TENCENT_SMARTVOICE_H_

#define TENCENT_DEVICE_NAME     "TENCENT_EARPHONE"
#define TENCENT_DEVICE_NAME_NUM 16

#define IS_ENABLE_START_DATA_XFER_STEP          0

#define IS_USE_ENCODED_DATA_SEQN_PROTECTION  0     
//#define IS_USE_ENCODED_DATA_SEQN_PROTECTION     1       //origal code 
#define IS_TENCENT_SMARTVOICE_DEBUG_ON                  0
#define IS_FORCE_STOP_VOICE_STREAM              0

#define APP_TENCENT_PATH_TYPE_HFP    (1 << 2)

#define TENCENT_VOICE_ONESHOT_PROCESS_LEN    (640)


#ifdef __cplusplus
extern "C" {
#endif

enum {
    APP_TENCENT_SMARTVOICE_STATE_IDLE,
    APP_TENCENT_SMARTVOICE_STATE_MIC_STARTING,
    APP_TENCENT_SMARTVOICE_STATE_MIC_STARTED,
    APP_TENCENT_SMARTVOICE_STATE_SPEAKER_STARTING,
    APP_TENCENT_SMARTVOICE_STATE_SPEAKER_STARTED,
    APP_TENCENT_SMARTVOICE_STATE_MAX
};

enum {
    APP_TENCENT_SMARTVOICE_MESSAGE_ID_CUSTOMER_FUNC,
    APP_TENCENT_SMARTVOICE_MESSAGE_ID_MAX
};

//#define APP_TENCENT_PID_DATA   "\xCF\x75\x2B\x7D" 
#define  APP_TENCENT_PID_DATA     "\xF5\x0A\x00\x00"
#define APP_TENCENT_PID_DATA_LEN  (4)

typedef struct
{
    uint8_t    bit;
    uint8_t    operating_sys;  
    uint8_t    flag_value[32];
}__attribute__ ((__packed__))tencent_config_t;
typedef enum {
    xw_headphone_extra_config_key_unknown = 0,

    xw_headphone_extra_config_key_keyFunction = 100,
    xw_headphone_extra_config_key_skuid = 101,

    xw_headphone_extra_config_key_customizedName = 200,
} xw_headphone_extra_config_key;

typedef enum {
    xw_headphone_extra_config_value_keyFunction_unknown = 0,
    xw_headphone_extra_config_value_keyFunction_default = 1,
    xw_headphone_extra_config_value_keyFunction_xiaowei = 2,
} xw_headphone_extra_config_value_keyFunction;

typedef struct _xw_headphone_extra_config_item {
    xw_headphone_extra_config_key key;
    xw_headphone_extra_config_value_keyFunction value;
} xw_headphone_extra_config_item;

/*******************************114 command*****************************/

typedef struct
{
    uint16_t     key;
    uint8_t      length;
    int8_t       payload[6];
} __attribute__((packed)) xw_headphone_general_config_item;

// group 0 reserved for general use cases
#define xw_headphone_general_config_key_unknown                 0x00

//group 1 for noise cancellation
#define xw_headphone_general_config_key_nc_group                0x01

#define xw_headphone_general_config_key_nc_report               0x01
#define xw_headphone_general_config_key_nc_increase             0x02

#define xw_headphone_general_config_key_nc_decrease             0x03
#define xw_headphone_general_config_key_nc_set                  0x04

#define xw_headphone_general_config_key_nc_on                   0x05
#define xw_headphone_general_config_key_nc_off                  0x06

#define xw_headphone_general_config_key_nc_max                  0x07
#define xw_headphone_general_config_key_nc_min                  0x08
#define xw_headphone_general_config_key_conversation_mode_on    0x09
#define xw_headphone_general_config_key_conversation_mode_off   0x0a

// group 2 for equalizer
#define xw_headphone_general_config_key_group_eq                0x02

#define xw_headphone_general_config_key_eq_inc_treble           0x01
#define xw_headphone_general_config_key_eq_inc_mid              0x02
#define xw_headphone_general_config_key_eq_inc_bass             0x03
#define xw_headphone_general_config_key_eq_dec_treble           0x04
#define xw_headphone_general_config_key_eq_dec_mid              0x05
#define xw_headphone_general_config_key_eq_dec_bass             0x06
#define xw_headphone_general_config_key_eq_max_treble           0x07
#define xw_headphone_general_config_key_eq_max_mid              0x08
#define xw_headphone_general_config_key_eq_max_bass             0x09
#define xw_headphone_general_config_key_eq_min_treble           0x0a
#define xw_headphone_general_config_key_eq_min_mid              0x0b
#define xw_headphone_general_config_key_eq_min_bass             0x0c
#define xw_headphone_general_config_key_eq_reset_treble         0x0d
#define xw_headphone_general_config_key_eq_reset_mid            0x0e
#define xw_headphone_general_config_key_eq_reset_bass           0x0f
#define xw_headphone_general_config_key_eq_set_treble           0x10
#define xw_headphone_general_config_key_eq_set_mid              0x11
#define xw_headphone_general_config_key_eq_set_bass             0x12
#define xw_headphone_general_config_key_eq_set_eq               0x13

/*******************115 command**************************/
typedef struct {
    uint8_t skill_name_length;
    uint8_t skill_name[12];
    uint8_t intent_name_length;
    uint8_t intent_name[12];
    uint8_t slot_count;

} __attribute__((packed)) xw_headphone_payload_custom_skills_request;
typedef struct {
    uint8_t slot_name_length;
    uint8_t slot_name[12];
    uint8_t slot_value_length;
    uint8_t slot_value[12];
} __attribute__((packed)) xw_headphone_payload_custom_skills_rsp;
typedef struct {
    uint8_t     transport_mode;     //spp/ble;
    uint8_t     encode_format;      
    uint8_t     compression_ration; 
	uint8_t     sample_rate;        //sample rate；
	uint8_t     mic_channel_count;  //recording channel；
    uint8_t     bit_number;         //bit depth；
    uint8_t     record_method;      //recording mode-- wakeup method
    uint32_t    firmware_version;   //software version;
    uint8_t     param_length;       //earphone name length；
	uint8_t     param[TENCENT_DEVICE_NAME_NUM];          // earphone name
	uint8_t		protocol_version;
	xw_headphone_extra_config_item extra_data;
	uint8_t		extra_data_length;
} __attribute__((packed)) EARPHONE_CONFIG_INFO_T;


extern uint8_t  conn_tag;
extern const struct ai_handler_func_table tencent_handler_function_table;

typedef void (*APP_TENCENT_SMARTVOICE_MOD_HANDLE_T)(void);


void app_tencent_sv_start_voice_stream_via_hfp(uint8_t device_id);
int app_tencent_sv_start_voice_stream_via_ble(uint8_t device_id);
int app_tencent_sv_start_voice_stream_via_spp(uint8_t device_id);

int app_tencent_smartvoice_start_mic_stream(void);
int app_tencent_smartvoice_stop_mic_stream(void);
void app_tencent_smartvoice_switch_hfp_stream_status(void);
void app_tencent_smartvoice_switch_ble_stream_status(void);
void app_tencent_smartvoice_switch_spp_stream_status(void);
void tencent_wakeup_app(void);
void  tencent_double_click_to_stop_app_playing(void);
uint32_t tencent_loose_button_to_request_for_stop_record_voice_stream(void *param1, uint32_t param2,uint8_t param3);

void app_tencent_sv_throughput_test_init(void);
void app_tencent_sv_stop_throughput_test(void);
void app_tencent_sv_set_throughput_test_conn_parameter(uint8_t conidx, uint16_t minConnIntervalInMs, uint16_t maxConnIntervalInMs);
uint32_t app_tencent_sv_throughput_test_transmission_handler(void *param1, uint32_t param2,uint8_t device_id);
void app_tencent_manual_disconnect(uint8_t deviceID);


#ifdef __cplusplus
}
#endif

#endif

