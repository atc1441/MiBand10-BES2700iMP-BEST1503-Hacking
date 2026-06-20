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
#ifdef NEW_NV_RECORD_ENABLED

#ifndef __NVRECORD_EXTENSION_H__
#define __NVRECORD_EXTENSION_H__
#include "plat_types.h"
#include "bluetooth_bt_api.h"
#include "me_api.h"
#include "ble_device_info.h"
#ifdef CUSTOMER_APP_BOAT
#include "app_key.h"
#endif

// increase by 1 if the nvrecord's whole data structure is changed and the content needs to be rebuilt
#define NV_EXTENSION_MAJOR_VERSION 18
// increase by 1 if the new items are appended to the tail of the former nvrecord's data structure
#define NV_EXTENSION_MINOR_VERSION 1

#define NV_EXTENSION_SIZE NORFLASH_API_SECTOR_MAX_SIZE       // one flash sector size
#define NV_EXTENSION_PAGE_SIZE NORFLASH_API_PAGE_MAX_SIZE    // one flash page size
#define NV_EXTENSION_HEADER_SIZE sizeof(NVRECORD_HEADER_T)   // magic number and valid length
#define NV_EXTENSION_MAGIC_NUMBER 0x4E455854
#define NV_EXTENSION_VALID_LEN (sizeof(NV_EXTENSION_RECORD_T) - sizeof(NVRECORD_HEADER_T))

/* unused, just for backwards compatible */
#define section_name_ddbrec "ddbrec"

/* BT paired device info */
#define MAX_BT_PAIRED_DEVICE_COUNT 8

/* BLE paired device information */
#define BLE_RECORD_NUM 5
#define BLE_GATT_CACHE_NUM 1

#define BLE_ADDR_SIZE 6
#define BLE_ENC_RANDOM_SIZE 8
#define BLE_LTK_SIZE 16
#define BLE_IRK_SIZE 16

#define BLE_STATIC_ADDR 0
#define BLE_RANDOM_ADDR 1

#ifdef GFPS_ENABLED
/* fast pair account key */
#define FP_ACCOUNT_KEY_RECORD_NUM 5
#define FP_ACCOUNT_KEY_SIZE 16
#define FP_SAME_KEY_COUNT 3
#define FP_MAX_NAME_LEN 64
#define FP_EPH_IDENTITY_KEY_LEN 32
#define FP_SPOT_ADV_DATA_LEN 20
#endif

#ifdef AI_OTA
#define MAX_VERSION_STRING_LEN 16
#define OTA_DEVICE_CNT 2 //!< should be equal to OTA_DEVICE_NUM in @see OTA_DEVICE_E
#endif

#ifdef BISTO_ENABLED
#ifdef GSOUND_HOTWORD_ENABLED
#define MODEL_FILE_EMBEDED 1 //!< define this macro when model file is emneded in our bin file

#if MODEL_FILE_EMBEDED
#define HOTWORD_MODLE_MAX_NUM 11//!< need to adjust this value if support more model
#else
#define HOTWORD_MODLE_MAX_NUM 10 //!< need to adjust this value if support more model
#endif

#endif // #ifdef GSOUND_HOTWORD_ENABLED
#endif // #ifdef BISTO_ENABLED

#ifdef PROMPT_IN_FLASH
#define LOCAL_PACKAGE_MAX 1
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
#define COMBO_INFO_NUM_MAX   4 // current :soc firmware + mcu firmware
#endif

/* tws name max length */
#define TWS_NAME_MAX_LENGTH   32

// TODO: should be increased if NV_EXTENSION_MIRROR_RAM_SIZE exceeds this value
#ifndef NV_EXTENSION_MIRROR_RAM_SIZE
#define NV_EXTENSION_MIRROR_RAM_SIZE 0x1000
#endif

#define set_bit(x, y)         ((x)|=(1<<(y)))
#define clr_bit(x, y)         ((x)&=~(1<<(y)))
#define reverse_bit(x, y)     ((x)^=(1<<(y)))
#define get_bit(x, y)         ((x)>>(y)&1)

#define TILE_INFO_SIZE 428
#define BT_FREQENCY_RANGE_NUM   3
#define BT_IQ_INVALID_MAGIC_NUM 0xFFFFFFFF
#define BT_IQ_VALID_MAGIC_NUM   0x5a5a5a5a
typedef struct
{
    uint32_t validityMagicNum;
    uint16_t gain_cal_val[BT_FREQENCY_RANGE_NUM];
    uint16_t phase_cal_val[BT_FREQENCY_RANGE_NUM];
} BT_IQ_CALIBRATION_CONFIG_T;

typedef struct
{
    uint32_t validityMagicNum;
    int dc_i_val;
    int dc_q_val;
} BT_DC_CALIBRATION_CONFIG_T;

typedef struct
{
    uint32_t validityMagicNum;
    uint16_t rx_gain_cal_val[BT_FREQENCY_RANGE_NUM];
    uint16_t rx_phase_cal_val[BT_FREQENCY_RANGE_NUM];
} BT_IQ_RX_CALIBRATION_CONFIG_T;

typedef enum
{
    NV_REBUILD_ALL = 0,
    NV_REBUILD_SDK_ONLY = 1,
    NV_REBUILD_CUSTOMER_ONLY = 2,
} NV_REBUILD_TYPE_E;

/* nv record header data structure */
typedef struct
{
    uint32_t magicNumber;
    uint16_t majorVersion;  // should be NV_EXTENSION_MAJOR_VERSION
    uint16_t minorVersion;  // should be NV_EXTENSION_MINOR_VERSION
    uint32_t validLen;      // should be the valid content in this nv record version
    uint32_t crc32;         // crc32 of following valid values in the nv extention section
} NVRECORD_HEADER_T;

/* system information */
typedef struct {
    int8_t language;
} media_language_t;

#if defined(BT_SOURCE)
typedef struct {
    int8_t src_snk_mode;
} src_snk_t;
#endif

typedef struct {
    uint32_t mode;
    btif_device_record_t record;
    bool tws_connect_success;
    bool tws_enable;
    uint8_t tws_side;
    uint8_t conn_mode;
    uint8_t reserved;  /* for future use */
    uint8_t tws_name_len;
    uint8_t tws_name[TWS_NAME_MAX_LENGTH];
} ibrt_mode_t;

enum NV_FACOTRY_T{
   NV_BT_NONSIGNALING_MODE          = 0x0,
   NV_LE_NONSIGNALING_TX_MODE       = 0x1,
   NV_LE_NONSIGNALING_RX_MODE       = 0x2,
   NV_LE_NONSIGNALING_CONT_TX_MODE  = 0x3,
   NV_LE_NONSIGNALING_CONT_RX_MODE  = 0x4,
   NV_READ_LE_TEST_RESULT           = 0x5,
   NV_BT_DUT_MODE                   = 0x6,
   NV_BT_NONSIGNALING_TX_MODE       = 0x7,
   NV_BT_NONSIGNALING_RX_MODE       = 0x8,
   NV_READ_BT_RX_TEST_RESULT        = 0x9,
   NV_BT_SINGLETONE_TX_MODE         = 0xa,
   NV_BT_ERROR_TEST_MODE            = 0xff,
};

typedef struct {
    uint8_t rx_channel;
    uint8_t phy;
    uint8_t mod_idx;
    bool test_done;
    uint16_t test_result;
}le_rx_nonsignaling_test_t;

typedef struct {
    uint8_t tx_channel;
    uint8_t data_len;
    uint8_t pkt_payload;
    uint8_t phy;
}le_tx_nonsignaling_test_t;

typedef struct {
    uint8_t hopping_mode;
    uint8_t whitening_mode;
    uint8_t tx_freq;
    uint8_t rx_freq;
    uint8_t power_level;
    uint8_t lt_addr;
    uint8_t edr_enabled;
    uint8_t packet_type;
    uint8_t payload_pattern;
    bool test_done;
    uint16_t payload_length;
    uint16_t test_result;
    uint16_t hec_nb;
    uint16_t crc_nb;
    uint32_t tx_packet_num;
}bt_nonsignaling_test_t;

typedef struct {
    uint8_t test_type;
    uint32_t test_end_timeout;
    uint16_t tx_packet_nb;
}test_scenarios_t;

typedef struct {
    uint32_t status;
} factory_tester_status_t;

typedef struct {
    bool    voice_key_enable;
    uint8_t setedCurrentAi; //if false, set ai default mode
    uint8_t currentAiSpec;  //
    uint8_t aiStatusDisableFlag;        //all ai disable flag
    uint8_t amaAssistantEnableStatus;   //ama enable flag
} AI_MANAGER_INFO_T;

#ifdef CUSTOMER_APP_BOAT
typedef struct {
    APP_KEY_STATUS status;
    uint8_t lr;
    uint8_t button_set_event;
}BUTTON_SET_EVENT_INFO_T;
#endif

struct nvrecord_env_t {
    // if len of nvrecord_env_t in flash is the same as current firmware and the CRC check
    // of the nv extension info in flash gets pass, copy the information of
    // nvrecord_env_t from flash to mirror ram
    uint32_t nvrecord_env_len;
    media_language_t media_language;
#if defined(BT_SOURCE)
    src_snk_t src_snk_flag;
#endif
    ibrt_mode_t ibrt_mode;
    factory_tester_status_t factory_tester_status;
#if defined(__TENCENT_VOICE__)
    uint8_t flag_value[8];
#endif
#ifdef CUSTOMER_APP_BOAT
    BUTTON_SET_EVENT_INFO_T buttonInfo_left[2];
    BUTTON_SET_EVENT_INFO_T buttonInfo_right[2];
#endif

    AI_MANAGER_INFO_T aiManagerInfo;
    test_scenarios_t factory_test_env;
    le_rx_nonsignaling_test_t le_rx_env;
    le_tx_nonsignaling_test_t le_tx_env;
    bt_nonsignaling_test_t bt_nonsignaling_env;
};

typedef struct btdevice_volume {
    uint8_t a2dp_vol;
    uint8_t hfp_vol;
} btdevice_volume;

typedef struct btdevice_profile {
    bool hfp_act;
    uint8_t a2dp_abs_vol;
    bool a2dp_act;
    uint8_t a2dp_codectype;
} btdevice_profile;

#ifdef CUSTOM_BITRATE
typedef struct
{
    uint32_t aac_bitrate;
    uint32_t sbc_bitpool;
    uint32_t audio_latentcy;
} nv_codec_user_info_t;
#endif

typedef struct {
    btif_device_record_t record;
    btdevice_volume device_vol;
    btdevice_profile device_plf;
#ifdef BT_DIP_SUPPORT
    bt_dip_pnp_info_t pnp_info;
#endif
} nvrec_btdevicerecord;

typedef struct {
    uint32_t pairedDevNum;
    nvrec_btdevicerecord pairedBtDevInfo[MAX_BT_PAIRED_DEVICE_COUNT];
} NV_RECORD_PAIRED_BT_DEV_INFO_T;

typedef enum {
    section_usrdata_ddbrecord,
    section_none
} SECTIONS_ADP_ENUM;

typedef struct {
    uint32_t saved_list_num;
    BLE_BASIC_INFO_T self_info;
    uint8_t local_database_hash[16];
    BleDeviceinfo ble_nv[BLE_RECORD_NUM];
    gattc_nv_cache_t gatt_cache[BLE_GATT_CACHE_NUM];
} __attribute__ ((packed)) NV_RECORD_PAIRED_BLE_DEV_INFO_T;

typedef struct {
    uint8_t     set_member;
    uint8_t     sirk[16];
} __attribute__ ((packed)) NV_RECORD_BLE_AUDIO_DEV_INFO_T;

typedef enum {
    NV_APP_MONE_MIN = 0,
    /// earbud
    NV_APP_EARBUDS = NV_APP_MONE_MIN,
    NV_APP_EARBUDS_MONO = NV_APP_MONE_MIN,
    NV_APP_EARBUDS_STEREO_ONE_CIS,
    NV_APP_EARBUDS_STEREO_TWO_CIS,
    /// headset
    NV_APP_HEADSET_STEREO_ONE_CIS,
    NV_APP_HEADSET_STEREO_TWO_CIS,
    /// walkie talkie
    NV_APP_WALKIE_TALKIE,
    /// dongle
    NV_APP_DONGLE,
    /// watch
    NV_APP_WATCH,
    /// sound
    NV_APP_SOUND,
    /// glasses
    NV_APP_GLASSES,
    NV_APP_MONE_MAX,
} nvrec_appmode_e;

typedef struct {
    nvrec_appmode_e app_mode;
} __attribute__ ((packed)) NV_RECORD_APP_MODE_INFO_T;

#ifdef TWS_SYSTEM_ENABLED
typedef struct {
    BLE_BASIC_INFO_T ble_info;
} TWS_INFO_T;
#endif  // #ifdef TWS_SYSTEM_ENABLED

#ifdef GFPS_ENABLED
typedef struct {
    uint8_t count;
    uint8_t addr[FP_SAME_KEY_COUNT][BTIF_BD_ADDR_SIZE];
    uint8_t key[FP_ACCOUNT_KEY_SIZE];

} NV_FP_ACCOUNT_KEY_ENTRY_T;

typedef struct {
    uint32_t key_count;
    NV_FP_ACCOUNT_KEY_ENTRY_T accountKey[FP_ACCOUNT_KEY_RECORD_NUM];
    uint16_t nameLen;
    uint8_t name[FP_MAX_NAME_LEN];
    uint8_t eph_identity_key[FP_EPH_IDENTITY_KEY_LEN];
    uint8_t spot_adv_data[FP_SPOT_ADV_DATA_LEN];
    bool spot_adv_enabled;
    uint32_t beacon_time;
    uint8_t hashed_value;
    uint32_t poweroff_time;
    uint32_t has_poweredoff_time;
} NV_FP_ACCOUNT_KEY_RECORD_T;
#endif  // #ifdef GFPS_ENABLED

#ifdef NVREC_BAIDU_DATA_SECTION
/*  DMA owned configuration information */
typedef struct {
    int32_t fmfreq;
    char rand[BAIDU_DATA_RAND_LEN + 1];

} NV_DMA_CONFIGURATION_T;
#endif  // #ifdef NVREC_BAIDU_DATA_SECTION

#ifdef TILE_DATAPATH
typedef struct {
    uint8_t tileInfo[TILE_INFO_SIZE];
} NV_TILE_INFO_CONFIG_T;
#endif

#if defined(AI_OTA)
typedef struct {
    // hotword model ID, arry size should equal to GSOUND_HOTWORD_MODEL_ID_BYTES
    char modelId[5];

    // start addr of the model
    uint32_t startAddr;

    // length of model file
    uint32_t len;
}HOTWORD_MODEL_INFO_T;

typedef struct {
    uint32_t imageSize; //!< image size of ongoing upgrade
    uint32_t breakPoint; //!< break point of ongoing upgrade
    char version[MAX_VERSION_STRING_LEN]; //!< version string of ongoing upgrade
    uint8_t versionLen; //!< version string length of ongoing upgrade
    uint8_t otaStatus; //!< status of ongoing upgrade
    uint8_t user; //!< user of ongoing upgrade
} NV_OTA_INFO_T;
#endif

#ifdef BISTO_ENABLED
typedef struct {
    uint8_t isGsoundEnabled;

    // this is used for hotword model
    // for non-hotword version, this should always be 0
    uint8_t supportedModelCnt;

#ifdef GSOUND_HOTWORD_ENABLED
    // store the hotword model info
    HOTWORD_MODEL_INFO_T modelInfo[HOTWORD_MODLE_MAX_NUM];
#endif
} NV_GSOUND_INFO_T;
#endif

#ifdef PROMPT_IN_FLASH
typedef struct
{
    /// ID of package(corresponding language)
    uint8_t id;

    // start addr of the package
    uint32_t startAddr;

    // length of package
    uint32_t len;
} PACKAGE_NODE_T;

/**
 * @brief Prompt language information
 *
 * NOTE: prompt rely on language saved in @see nvrecord_env_t
 *
 */
typedef struct
{
    uint8_t newFlag;
    uint8_t num;
    PACKAGE_NODE_T packageInfo[LOCAL_PACKAGE_MAX];
} NV_PROMPT_INFO_T;
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
typedef struct
{
    uint32_t id;
    uint32_t offset;
    uint32_t length;
    uint32_t crc32;
} COMBO_CONTENT_INFO_T;

typedef struct
{
    uint32_t mainInfo;
    uint32_t version;
    uint32_t contentNum;
    COMBO_CONTENT_INFO_T info[COMBO_INFO_NUM_MAX];
    uint32_t crc32;
} NV_COMBO_IMAGE_INFO_T;
#endif

#ifdef CODEC_DAC_DC_NV_DATA
#define DAC_DC_CALIB_DATA_NV_NUM 2
struct HAL_CODEC_DAC_DRE_CALIB_CFG_NV_T {
    uint32_t valid;
    uint32_t dc_l;
    uint32_t dc_r;
    float    gain_l;
    float    gain_r;
    uint16_t ana_dc_l;
    uint16_t ana_dc_r;
    uint8_t ana_gain;
    uint8_t ini_ana_gain;
    uint8_t gain_offset;
    uint8_t step_mode;
    uint8_t top_gain;
    uint8_t rsv[3];
};
#endif

#if BLE_AUDIO_ENABLED || defined(KEEP_BLE_AUDIO_IN_NV_RECORD)
#define BLE_GATT_CACHE_REC_NUM    (2)
#define ACC_MCS_DFT_MEDIA_NUM     (2)
#define ACC_TBS_DFT_BEARER_NUM    (2)
#define ACC_MCS_CHAR_TYPE_NV_MAX  (22)
#define ACC_MCS_DESC_TYPE_NV_MAX  (17)
// @ACC_TB_CHAR_TYPE_MAX
#define ACC_TBS_CHAR_TYPE_NV_MAX  (16)
// @ACC_TB_DESC_TYPE_MAX
#define ACC_TBS_DESC_TYPE_NV_MAX  (12)
#define GATT_DB_HASH_NV_LEN       (16)

typedef struct
{
    /// start handle
    uint16_t shdl;
    /// end handle
    uint16_t ehdl;
} prf_svc_nv_t;

/// Characteristic information structure
typedef struct
{
    /// Value handle
    uint16_t val_hdl;
    /// Characteristic properties
    uint8_t prop;
} prf_char_nv_t;

typedef struct
{
    /// Descriptor handle
    uint16_t desc_hdl;
} prf_desc_nv_t;

typedef struct
{
    /// Service description
    prf_svc_nv_t svc_info;
    /// UUID
    uint16_t uuid;
    /// Characteristics description
    prf_char_nv_t char_info[ACC_MCS_CHAR_TYPE_NV_MAX];
    /// Descriptors description
    prf_desc_nv_t desc_info[ACC_MCS_DESC_TYPE_NV_MAX];
} nv_acc_mcc_mcs_info_t;

typedef struct
{
    /// Service description
    prf_svc_nv_t svc_info;
    /// UUID
    uint16_t uuid;
    /// Characteristics description
    prf_char_nv_t char_info[ACC_TBS_CHAR_TYPE_NV_MAX];
    /// Descriptors description
    prf_desc_nv_t desc_info[ACC_TBS_DESC_TYPE_NV_MAX];
} nv_acc_tbc_tbs_info_t;

typedef struct
{
    /// Number of (Generic) Media Control service
    uint8_t nb_media;
    /// MCS Cache
    nv_acc_mcc_mcs_info_t mcs_info[ACC_MCS_DFT_MEDIA_NUM];
} acc_mcc_nv_bond_data_t;

typedef struct
{
    /// Number of (Generic) Telephone Bearer service
    uint8_t nb_bearer;
    /// TBS Cache
    nv_acc_tbc_tbs_info_t tbs_info[ACC_TBS_DFT_BEARER_NUM];
} acc_tbc_nv_bond_data_t;

typedef struct
{
    /// Peer GATT Service Start handle
    uint16_t           gatt_start_hdl;
    /// Peer GATT Service End Handle
    uint16_t           gatt_end_hdl;
    /// Peer Service Change value handle
    uint16_t           svc_chg_hdl;
    /// Server supported features    (see enum #gapc_srv_feat)
    uint8_t            srv_feat;
    /// Client bond data information (see enum #gapc_cli_info)
    uint8_t            cli_info;
    /// The number of server(Local device)
    uint8_t            srv_nb;
} gapc_nv_bond_data_t;

typedef struct
{
    /// Use Public Addr to recognize a dev
    uint8_t peer_addr[6];
    /// Client characteristic configuration
    // VCS
    uint8_t vcs_cli_cfg_bf;
    // ASCS
    uint8_t ascs_cli_cfg_bf;
    uint8_t ascs_ase_cli_cfg_bf;
    // PACS
    uint8_t pacs_cli_cfg_bf;
    uint8_t pacs_pac_cli_cfg_bf;
    // CSIS
    uint8_t csis_cli_cfg_bf;
    uint8_t csis_set_lid;

#ifdef BLE_BATT_ENABLE
    uint8_t batt_ntf_cfg_bf;
#endif
    uint8_t bass_cfg_bf;

    /// Add other services here
} GATTC_SRV_ATTR_t;

typedef struct
{
    /// idx start at 0 to BLE_GATT_CACHE_REC_NUM wrap to 0
    uint8_t wrap_idx;

    GATTC_SRV_ATTR_t gatt_nv_arv_attr[BLE_GATT_CACHE_REC_NUM];

} GATTC_NV_SRV_ATTR_t;


#endif

#ifdef BT_DIP_SUPPORT
    // 94 bytes are from MAX_BT_PAIRED_DEVICE_COUNT times of bt_dip_pnp_info_t
    #define NV_SDK_RESERVED_LEN  (368-106)
#else
    #define NV_SDK_RESERVED_LEN  368
#endif

#ifdef CODEC_ADC_DC_NV_DATA
#define ADC_DC_CALIB_DATA_NV_NUM        (8)
typedef struct {
    uint32_t valid;
    uint32_t adc_channel_map;
    uint32_t adc_dc_calib_offset;
    uint32_t adc_dc_calib_step;
    uint32_t adc_ana_gain;
    int32_t  adc_dc_original;
    int32_t  adc_dc_calib_result;
    uint32_t rsvd0;
} HAL_CODEC_ADC_DC_CALIB_CFG_NV_T;
#endif

typedef struct {
    int ff_gain;
    int fb_gain;
    float calib_value;
    bool golden_flag;
    float calib_gain[3];
}TOTA_ADDA_TEST_T;

typedef struct{
    TOTA_ADDA_TEST_T adda_test;
    // TODO: add other items for SDK.
}NV_SDK_INFO_T;

typedef union{
    NV_SDK_INFO_T info;
    uint8_t reserve[NV_SDK_RESERVED_LEN];
} NV_SDK_RESERVE_SPACE_T;

STATIC_ASSERT(sizeof(NV_SDK_INFO_T) < NV_SDK_RESERVED_LEN, "nv sdk info too big");

typedef struct {
    NVRECORD_HEADER_T header;
    // this data structure shall never be changed since Day 1
    struct nvrecord_env_t system_info;

    NV_RECORD_PAIRED_BT_DEV_INFO_T bt_pair_info;
#ifdef APP_SOUND_ENABLE
    NV_RECORD_PAIRED_BT_DEV_INFO_T bt_pair_info_cache;
#endif
    NV_RECORD_PAIRED_BLE_DEV_INFO_T ble_pair_info;
    NV_RECORD_APP_MODE_INFO_T app_mode_info;

#ifdef TWS_SYSTEM_ENABLED
    TWS_INFO_T tws_info;
#endif

#ifdef GFPS_ENABLED
    NV_FP_ACCOUNT_KEY_RECORD_T fp_account_key_rec;
#endif

#ifdef NVREC_BAIDU_DATA_SECTION
    NV_DMA_CONFIGURATION_T dma_config;
#endif

#ifdef TILE_DATAPATH
    NV_TILE_INFO_CONFIG_T tileConfig;
#endif

#ifdef AI_OTA
    NV_OTA_INFO_T ota_info[OTA_DEVICE_CNT];
#endif

#if defined(BISTO_ENABLED)
    NV_GSOUND_INFO_T gsound_info;
#endif

#if 1//def TX_IQ_CAL
    BT_IQ_CALIBRATION_CONFIG_T btIqCalConfig;
#endif

#ifdef PROMPT_IN_FLASH
    NV_PROMPT_INFO_T prompt_info;
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
    NV_COMBO_IMAGE_INFO_T combo_bin_info;
#endif

#ifdef CODEC_DAC_DC_NV_DATA
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_NV_T dac_dre_calib_cfg_nv[DAC_DC_CALIB_DATA_NV_NUM];
#endif

#if 1//def RX_IQ_CAL
    BT_IQ_RX_CALIBRATION_CONFIG_T btIqRxCalConfig;
#endif

#if 1//def BT_DC_CAL
    BT_DC_CALIBRATION_CONFIG_T btDcCalibConfig;
#endif

#if BLE_AUDIO_ENABLED || defined(KEEP_BLE_AUDIO_IN_NV_RECORD)
    NV_RECORD_BLE_AUDIO_DEV_INFO_T  ble_audio_dev_info;

    GATTC_NV_SRV_ATTR_t             srv_cache_attr;
#endif
#ifdef CODEC_ADC_DC_NV_DATA
    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T adc_dc_calib_cfg_info[ADC_DC_CALIB_DATA_NV_NUM];
#endif
#ifdef CUSTOM_BITRATE
    nv_codec_user_info_t codec_user_info;
#endif
    // reserve space for sdk info, if need to add new data structure
    // grab space from the reserve space, but must assure that the whole space
    // size is not changed.
   NV_SDK_RESERVE_SPACE_T sdk_reserve;

    uint8_t sdk_owned_end[0];
    uint8_t customer_owned_start[0];
    // TODO: If wanna OTA to update the nv record, two choices:
    // 1. Change above data structures and increase NV_EXTENSION_MAJOR_VERSION.
    //     Then the nv record will be rebuilt and the whole history information will be cleared
    // 2. Don't touch above data structures but just add new items here and increase NV_EXTENSION_MINOR_VERSION.
    //     Then the nv record will keep all the whole hisotry.

} NV_EXTENSION_RECORD_T;

typedef union {
    NV_EXTENSION_RECORD_T nv_record;
    /*
     * dummy data, just make sure the mirror buffer's size is
     * "NV_EXTENSION_MIRROR_RAM_SIZE"
     */
    uint8_t dummy_data[NV_EXTENSION_MIRROR_RAM_SIZE];
} NV_MIRROR_BUF_T;

#ifdef __cplusplus
extern "C" {
#endif

extern NV_EXTENSION_RECORD_T *nvrecord_extension_p;

int nv_record_env_init(void);

NV_EXTENSION_RECORD_T *nv_record_get_extension_entry_ptr(void);

void nv_record_extension_update(void);

void nv_extension_callback(void *param);

int nv_record_touch_cause_flush(void);

void nv_record_sector_clear(void);

void nv_record_flash_flush(void);

int nv_record_flash_flush_in_sleep(void);

void nv_record_execute_async_flush(void);

void nv_record_update_runtime_userdata(void);

void nv_record_rebuild(NV_REBUILD_TYPE_E rebuild_type);

uint32_t nv_record_pre_write_operation(void);

void nv_record_post_write_operation(uint32_t lock);

bt_status_t nv_record_open(SECTIONS_ADP_ENUM section_id);

void nv_record_init(void);

uint32_t nv_record_get_ibrt_mode(void);

uint8_t* nv_record_get_ibrt_peer_addr(void);

#ifdef CODEC_ADC_DC_NV_DATA
int nvrecord_adc_dc_calib_info_set(uint32_t channel_index, HAL_CODEC_ADC_DC_CALIB_CFG_NV_T *adc_dc_calib_info);
int nvrecord_adc_dc_calib_info_get(uint32_t channel_index, HAL_CODEC_ADC_DC_CALIB_CFG_NV_T *adc_dc_calib_info);
int nvrecord_adc_dc_calib_info_dump(uint32_t channel_index);
#endif
#ifdef __cplusplus
}
#endif
#endif
#endif  //#if defined(NEW_NV_RECORD_ENABLED)
