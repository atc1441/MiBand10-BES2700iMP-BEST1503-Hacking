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
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "cmsis.h"
#include "nvrecord_extension.h"
#include "hal_trace.h"
#include "crc32_c.h"
#include "hal_sleep.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_ble.h"
#include "nvrecord_dma_config.h"
#include "nvrecord_fp_account_key.h"
#include "customparam_section.h"
#include "mpu.h"
#include "bluetooth_bt_api.h"
#include "besbt.h"
#include "hal_timer.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "factory_section.h"

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#endif

#ifdef CUSTOM_BITRATE
#include "app_ibrt_customif_cmd.h"
#endif

#ifndef RAM_NV_RECORD
extern uint32_t __userdata_start[];
extern uint32_t __userdata_end[];
#else
uint32_t __userdata_start[1024*2];
#endif
extern void nvrecord_rebuild_system_env(struct nvrecord_env_t* pSystemEnv, bool isRebuildAll);
extern void nvrecord_rebuild_paired_bt_dev_info(NV_RECORD_PAIRED_BT_DEV_INFO_T* pPairedBtInfo);
#ifdef GFPS_ENABLED
extern void nvrecord_rebuild_fp_account_key(NV_FP_ACCOUNT_KEY_RECORD_T* pFpAccountKey);
#endif
#ifdef NVREC_BAIDU_DATA_SECTION
extern void nvrecord_rebuild_dma_configuration(NV_DMA_CONFIGURATION_T* pDmaConfig);
#endif

typedef enum
{
    NV_STATE_IDLE,
    NV_STATE_MAIN_ERASING,
    NV_STATE_MAIN_ERASED,
    NV_STATE_MAIN_WRITTING,
    NV_STATE_MAIN_WRITE_PENDING,
    NV_STATE_MAIN_WRITTEN,
    NV_STATE_MAIN_DONE,
    NV_STATE_BAK_ERASING,
    NV_STATE_BAK_ERASED,
    NV_STATE_BAK_WRITTING,
    NV_STATE_BAK_WRITE_PENDING,
    NV_STATE_BAK_WRITTEN,
    NV_STATE_BAK_DONE,
}NV_STATE;

typedef struct
{
    bool is_update;
    NV_STATE state;
    uint32_t written_size;
}NV_FLUSH_STATE;

static NV_FLUSH_STATE nv_flush_state;
static bool nvrec_init = false;
static uint32_t _user_data_main_start;
static uint32_t _user_data_bak_start;
#ifndef RAM_NV_RECORD
static NV_EXTENSION_RECORD_T _nv_burn_buf;
#endif

NV_EXTENSION_RECORD_T *nvrecord_extension_p  = NULL;

#ifdef RTOS
osMutexDef(NV_RECORD_MUTEX);
static osMutexId nv_record_mutex_id = NULL;
#endif

/*
 *Note: the NV_EXTENSION_MIRROR_RAM_SIZE must be power of 2
 */
#if defined(__ARM_ARCH_8M_MAIN__)
#define __NV_BUF_MPU_ALIGNED __ALIGNED(0x20)
#else
/*
 * armv7 mpu require the address must be aligned to the section size and
 * the section size must be algined to power of 2
 */
#define __NV_BUF_MPU_ALIGNED __ALIGNED(NV_EXTENSION_MIRROR_RAM_SIZE)
#endif

#ifndef RAM_NV_RECORD
static NV_MIRROR_BUF_T local_extension_data __NV_BUF_MPU_ALIGNED
                            __attribute__((section(".sram_data"))) =
#else
static NV_MIRROR_BUF_T local_extension_data =
#endif
{
    .nv_record = {
        {  // header
            NV_EXTENSION_MAGIC_NUMBER,
            NV_EXTENSION_MAJOR_VERSION,
            NV_EXTENSION_MINOR_VERSION,
            NV_EXTENSION_VALID_LEN,
            0,
        },

        {  // system info
            sizeof(struct nvrecord_env_t)-sizeof(uint32_t),
        },

        {  // bt_pair_info
            0
        },
#ifdef APP_SOUND_ENABLE
        {  // bt_pair_info_cache
            0
        },
#endif
        {  // ble_pair_info

        },

        {  // app mode

        },

#ifdef TWS_SYSTEM_ENABLED
        {  // tws_info

        },
#endif

#ifdef GFPS_ENABLED
        {  // fp_account_key_rec
            0
        },
#endif

#ifdef NVREC_BAIDU_DATA_SECTION
        {  // dma_config
            BAIDU_DATA_DEF_FM_FREQ,
        },
#endif

#ifdef TILE_DATAPATH
        {
            {0}
        },
#endif

#ifdef AI_OTA
        {
            {0,},
        },
#endif

#if defined(BISTO_ENABLED)
        {
            true,
        },
#endif

#if 1//def TX_IQ_CAL
        {
            BT_IQ_INVALID_MAGIC_NUM,
            {0},
            {0},
        },
#endif

#ifdef PROMPT_IN_FLASH
        {
            0
        },
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
        {
            0
        },
#endif

#ifdef CODEC_DAC_DC_NV_DATA
        {
            {0,},
        },
#endif

#if 1//def RX_IQ_CAL
        {
            BT_IQ_INVALID_MAGIC_NUM,
            {0},
            {0},
        },
#endif

#if 1//def BT_DC_CAL
        {
            BT_IQ_INVALID_MAGIC_NUM,
            0,
            0,
        },
#endif

#if BLE_AUDIO_ENABLED || defined(KEEP_BLE_AUDIO_IN_NV_RECORD)
        {
            0
        },
        {
            0
        },
#endif
#ifdef CODEC_ADC_DC_NV_DATA
        {
            {0,},
        },
#endif

#ifdef CUSTOM_BITRATE
        {
            USER_CONFIG_DEFAULT_AAC_BITRATE,
            USER_CONFIG_DEFAULT_SBC_BITPOOL,
            USER_CONFIG_DEFAULT_AUDIO_LATENCY,
        },
#endif
        // TODO:
        // If want to extend the nvrecord while keeping the history information,
        // append the new items to the tail of NV_EXTENSION_RECORD_T and
        // set their intial content here
    },
};

STATIC_ASSERT(sizeof(local_extension_data) <= NV_EXTENSION_MIRROR_RAM_SIZE, "NV local buffer too small");

static int nv_record_extension_flush(bool is_async);

#ifdef TILE_DATAPATH
static void nvrecord_rebuild_tileconfig(NV_TILE_INFO_CONFIG_T *tileConfig)
{
    memset((uint8_t*)tileConfig,0,sizeof(NV_TILE_INFO_CONFIG_T));
}
#endif

#ifdef AI_OTA
static void nvrecord_rebuild_ota_info(NV_OTA_INFO_T *ota_info)
{
    memset((uint8_t *)ota_info, 0, OTA_DEVICE_CNT * sizeof(NV_OTA_INFO_T));
}
#endif

#if defined(BISTO_ENABLED)
static void nvrecord_rebuild_gsound_info(NV_GSOUND_INFO_T *gsound_info)
{
    memset((uint8_t*)gsound_info,0,sizeof(NV_GSOUND_INFO_T));
}
#endif

#if 1//def TX_IQ_CAL
static void nvrecord_rebuild_btiqcalconfig(BT_IQ_CALIBRATION_CONFIG_T *btIqCalConfig)
{
    memset((uint8_t*)btIqCalConfig,0,sizeof(BT_IQ_CALIBRATION_CONFIG_T));
    btIqCalConfig->validityMagicNum = BT_IQ_INVALID_MAGIC_NUM;
}
#endif

#if 1//def RX_IQ_CAL
static void nvrecord_rebuild_btiqrxcalconfig(BT_IQ_RX_CALIBRATION_CONFIG_T *btIqRxCalConfig)
{
    memset((uint8_t*)btIqRxCalConfig,0,sizeof(BT_IQ_RX_CALIBRATION_CONFIG_T));
    btIqRxCalConfig->validityMagicNum = BT_IQ_INVALID_MAGIC_NUM;
}
#endif

#if 1//def BT_DC_CAL
static void nvrecord_rebuild_btDcCalibConfig(BT_DC_CALIBRATION_CONFIG_T *btDcCalibConfig)
{
    memset((uint8_t*)btDcCalibConfig,0,sizeof(BT_DC_CALIBRATION_CONFIG_T));
    btDcCalibConfig->validityMagicNum = BT_IQ_INVALID_MAGIC_NUM;
}
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
static void nvrecord_rebuild_combo_bin_info(NV_COMBO_IMAGE_INFO_T* combo_info)
{
     memset((uint8_t*)combo_info,0,sizeof(NV_COMBO_IMAGE_INFO_T));
}
#endif
#ifdef CODEC_ADC_DC_NV_DATA
static void nvrecord_rebuild_adc_dc_calib_info(HAL_CODEC_ADC_DC_CALIB_CFG_NV_T* adc_dc_calib_info)
{
    memset((uint8_t*)adc_dc_calib_info, 0,
                sizeof(HAL_CODEC_ADC_DC_CALIB_CFG_NV_T) * ADC_DC_CALIB_DATA_NV_NUM);
}

int nvrecord_adc_dc_calib_info_set(uint32_t channel_index,
                                                    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T *adc_dc_calib_info)
{
    if ((NULL == nvrecord_extension_p) || (NULL == adc_dc_calib_info))
    {
        return -1;
    }
    if (channel_index >= ADC_DC_CALIB_DATA_NV_NUM)
    {
        TRACE(0, "adc dc calib info input channel_index error!!!");
        return -1;
    }

    uint32_t lock = nv_record_pre_write_operation();

    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T* info = nvrecord_extension_p->adc_dc_calib_cfg_info;
    memcpy((uint8_t*)&info[channel_index],
                (uint8_t*)adc_dc_calib_info, sizeof(HAL_CODEC_ADC_DC_CALIB_CFG_NV_T));

    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    //nv_record_flash_flush();
    //nv_record_execute_async_flush();
    return 0;
}

int nvrecord_adc_dc_calib_info_get(uint32_t channel_index,
                                                    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T *adc_dc_calib_info)
{
    if ((NULL == nvrecord_extension_p) || (NULL == adc_dc_calib_info))
    {
        return -1;
    }
    if (channel_index >= ADC_DC_CALIB_DATA_NV_NUM)
    {
        TRACE(0, "adc dc calib info input channel_index error!!!");
        return -1;
    }

    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T* info = nvrecord_extension_p->adc_dc_calib_cfg_info;
    memcpy((uint8_t*)adc_dc_calib_info, (uint8_t*)&info[channel_index], sizeof(HAL_CODEC_ADC_DC_CALIB_CFG_NV_T));

    return 0;
}

int nvrecord_adc_dc_calib_info_dump(uint32_t channel_index)
{
    if (NULL == nvrecord_extension_p)
    {
        return -1;
    }
    if (channel_index >= ADC_DC_CALIB_DATA_NV_NUM)
    {
        TRACE(0, "adc dc calib info input channel_index error!!!");
        return -1;
    }

    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T* info = nvrecord_extension_p->adc_dc_calib_cfg_info;
    TRACE(0, "===========Dump adc dc calibrate result===========");
    TRACE(1, "channel_index:                %d",        channel_index);
    TRACE(1, "calib_valid:                  %d",        info[channel_index].valid);
    TRACE(1, "channel_map:                  0x%04x",    info[channel_index].adc_channel_map);
    TRACE(1, "dc_origin_value:              %d",        info[channel_index].adc_dc_original);
    TRACE(1, "dc_calib_result:              %d",        info[channel_index].adc_dc_calib_result);
    TRACE(1, "dc_calib_reg_offset:          0x%04x",    info[channel_index].adc_dc_calib_offset);
    TRACE(1, "dc_calib_reg_step:            0x%04x",    info[channel_index].adc_dc_calib_step);

    return 0;
}
#endif

#if BLE_AUDIO_ENABLED || defined(KEEP_BLE_AUDIO_IN_NV_RECORD)
static void nvrecord_rebuild_ble_audio_dev_info(NV_RECORD_BLE_AUDIO_DEV_INFO_T* leaudio_info)
{
    leaudio_info->set_member = 0;
    memset(leaudio_info->sirk, 0, 16);
}
#endif

static void nvrecord_rebuild_sdk_info(uint8_t* sdk_reserve)
{
     NV_SDK_RESERVE_SPACE_T *uni_sdk = (NV_SDK_RESERVE_SPACE_T*)sdk_reserve;
     TOTA_ADDA_TEST_T  *adda_test;

     memset((uint8_t*)uni_sdk,0,sizeof(NV_SDK_RESERVE_SPACE_T));
     adda_test = &(uni_sdk->info.adda_test);

     adda_test->ff_gain = 512;
     adda_test->fb_gain = 512;
     adda_test->calib_value = 1.0;
     adda_test->golden_flag = false;
     adda_test->calib_gain[0] = 1234.825f;
     adda_test->calib_gain[1] = 1096.515f;
     adda_test->calib_gain[2] = 1032.47f;
}

static bool nv_record_data_is_valid(NV_EXTENSION_RECORD_T *nv_record)
{
    bool is_valid = false;

#ifndef RAM_NV_RECORD
    NVRECORD_HEADER_T* pExtRecInFlash = &nv_record->header;
    uint8_t* pData = (uint8_t*)nv_record + NV_EXTENSION_HEADER_SIZE;

    TRACE(0,"nv ext magic 0x%x valid len %d", pExtRecInFlash->magicNumber,
        pExtRecInFlash->validLen);
    TRACE(0,"Former major ver %d current major ver %d", pExtRecInFlash->majorVersion,
        NV_EXTENSION_MAJOR_VERSION);

    if ((NV_EXTENSION_MAJOR_VERSION == pExtRecInFlash->majorVersion) &&
        (NV_EXTENSION_MAGIC_NUMBER == pExtRecInFlash->magicNumber))
    {
        TRACE(2,"Former nv ext valid len %d", pExtRecInFlash->validLen);
        TRACE(2,"Current FW version nv ext valid len %d", NV_EXTENSION_VALID_LEN);

        // check whether the data length is valid
        if (pExtRecInFlash->validLen <= NV_EXTENSION_SIZE-NV_EXTENSION_HEADER_SIZE)
        {
            // check crc32
            uint32_t crc = crc32_c(0, pData, pExtRecInFlash->validLen);
            TRACE(1,"generated crc32: 0x%x, header crc: 0x%x",crc, pExtRecInFlash->crc32);
            if (crc == pExtRecInFlash->crc32)
            {
                // correct
                TRACE(2,"Nv extension is valid.");

                if (NV_EXTENSION_VALID_LEN < pExtRecInFlash->validLen)
                {
                    TRACE(0,"Valid length of extension must be increased,"
                    "use the default value.");
                }
                else
                {
                    if(NV_EXTENSION_VALID_LEN > pExtRecInFlash->validLen)
                    {
                        TRACE(2, "NV extension is extended! (0x%x) -> (0x%x)",
                              pExtRecInFlash->validLen,NV_EXTENSION_VALID_LEN);
                    }
                    is_valid = true;
                }
            }
        }
    }
#endif

    return is_valid;
}

static bool nv_record_items_is_valid(NV_EXTENSION_RECORD_T *nv_record)
{
    bool is_valid = false;
    nv_record = nv_record;

#ifndef RAM_NV_RECORD
#if 0
    NV_RECORD_PAIRED_BT_DEV_INFO_T *pbt_pair_info;
    pbt_pair_info = &nv_record->bt_pair_info;
    TRACE(1,"%s: pairedDevNum: %d ", __func__,pbt_pair_info->pairedDevNum);
    for(uint8_t i = 0; i < pbt_pair_info->pairedDevNum; i++)
    {
        DUMP8("0x%x,", (uint8_t*)pbt_pair_info->pairedBtDevInfo[i].record.bdAddr.address,6);
    }
#endif
    // TODO:  add cheking for nv_record items.

    is_valid = true;
#endif

    return is_valid;
}

#ifdef CUSTOM_BITRATE
POSSIBLY_UNUSED static void nvrecord_rebuild_codec_user_config(nv_codec_user_info_t *codec_user_config)
{
    memset((uint8_t*)codec_user_config, 0, sizeof(nv_codec_user_info_t));
    codec_user_config->aac_bitrate = USER_CONFIG_DEFAULT_AAC_BITRATE;
    codec_user_config->sbc_bitpool = USER_CONFIG_DEFAULT_SBC_BITPOOL;
    codec_user_config->audio_latentcy = USER_CONFIG_DEFAULT_AUDIO_LATENCY;
}
#endif

static void nv_record_reset_sdk_info(NV_EXTENSION_RECORD_T *nv_record, NV_REBUILD_TYPE_E rebuild_type)
{
    switch (rebuild_type)
    {
        case NV_REBUILD_CUSTOMER_ONLY:
            memset((uint8_t*)nv_record + (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->customer_owned_start), 0,
                sizeof(NV_EXTENSION_RECORD_T) - (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->customer_owned_start));
            break;
        default:
            break;
    }

    nvrecord_rebuild_system_env(&(nv_record->system_info), false);
    nvrecord_rebuild_paired_bt_dev_info(&(nv_record->bt_pair_info));
    nvrecord_rebuild_paired_ble_dev_info(&(nv_record->ble_pair_info));

#ifdef TWS_SYSTEM_ENABLED
    memset(&(nv_record->tws_info), 0, sizeof(nv_record->tws_info));
#endif

#ifdef GFPS_ENABLED
    nvrecord_rebuild_fp_account_key(&(nv_record->fp_account_key_rec));
#endif

#ifdef NVREC_BAIDU_DATA_SECTION
    nvrecord_rebuild_dma_configuration(&(nv_record->dma_config));
#endif

#ifdef TILE_DATAPATH
    nvrecord_rebuild_tileconfig(&nv_record->tileConfig);
#endif

#if defined(BISTO_ENABLED)
    nvrecord_rebuild_gsound_info(&nv_record->gsound_info);
#endif

#ifdef AI_OTA
    nvrecord_rebuild_ota_info((NV_OTA_INFO_T *)&nv_record->ota_info);
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
    nvrecord_rebuild_combo_bin_info((NV_COMBO_IMAGE_INFO_T*)&nv_record->combo_bin_info);
#endif

#ifdef CUSTOM_BITRATE
    nvrecord_rebuild_codec_user_config(&nv_record->codec_user_info);
#endif

    nv_record->header.magicNumber = NV_EXTENSION_MAGIC_NUMBER;
    nv_record->header.majorVersion = NV_EXTENSION_MAJOR_VERSION;
    nv_record->header.minorVersion = NV_EXTENSION_MINOR_VERSION;
    nv_record->header.validLen = NV_EXTENSION_VALID_LEN;
    nv_record->header.crc32 =
        crc32_c(0, ((uint8_t *)nv_record + NV_EXTENSION_HEADER_SIZE),
                   NV_EXTENSION_VALID_LEN);
}


static void nv_record_set_default(NV_EXTENSION_RECORD_T *nv_record, NV_REBUILD_TYPE_E rebuild_type)
{
    switch (rebuild_type)
    {
        case NV_REBUILD_SDK_ONLY:
            memset((uint8_t*)nv_record, 0, (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->sdk_owned_end));
            break;
        case NV_REBUILD_CUSTOMER_ONLY:
            memset((uint8_t*)nv_record + (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->customer_owned_start), 0,
                sizeof(NV_EXTENSION_RECORD_T) - (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->customer_owned_start));
            break;
        default:
            memset((uint8_t*)nv_record, 0, sizeof(NV_EXTENSION_RECORD_T));
            break;
    }

    nvrecord_rebuild_system_env(&(nv_record->system_info), true);

    uint8_t* pDefaultPeerBtAddr = factory_section_get_default_peer_bt_address();
    if (pDefaultPeerBtAddr)
    {
        memcpy(nv_record->system_info.ibrt_mode.record.bdAddr.address,
            pDefaultPeerBtAddr, 6);
    }

    nv_record->system_info.ibrt_mode.mode = factory_section_get_default_bt_nv_role();

    nvrecord_rebuild_paired_bt_dev_info(&(nv_record->bt_pair_info));
    nvrecord_rebuild_paired_ble_dev_info(&(nv_record->ble_pair_info));
#ifdef GFPS_ENABLED
    nvrecord_rebuild_fp_account_key(&(nv_record->fp_account_key_rec));
#endif

#ifdef NVREC_BAIDU_DATA_SECTION
    nvrecord_rebuild_dma_configuration(&(nv_record->dma_config));
#endif

#ifdef TILE_DATAPATH
    nvrecord_rebuild_tileconfig(&nv_record->tileConfig);
#endif

#if defined(BISTO_ENABLED)
    nvrecord_rebuild_gsound_info(&nv_record->gsound_info);
#endif

#ifdef AI_OTA
    nvrecord_rebuild_ota_info((NV_OTA_INFO_T *)&nv_record->ota_info);
#endif

#if 1//def TX_IQ_CAL
    nvrecord_rebuild_btiqcalconfig(&nv_record->btIqCalConfig);
#endif

#if 1//def RX_IQ_CAL
    nvrecord_rebuild_btiqrxcalconfig(&nv_record->btIqRxCalConfig);
#endif

#if 1//def BT_DC_CAL
    nvrecord_rebuild_btDcCalibConfig(&nv_record->btDcCalibConfig);
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
    nvrecord_rebuild_combo_bin_info((NV_COMBO_IMAGE_INFO_T*)&nv_record->combo_bin_info);
#endif

#if BLE_AUDIO_ENABLED || defined(KEEP_BLE_AUDIO_IN_NV_RECORD)
    nvrecord_rebuild_ble_audio_dev_info((NV_RECORD_BLE_AUDIO_DEV_INFO_T*)&nv_record->ble_audio_dev_info);
#if BLE_AUDIO_ENABLED
    bes_ble_aob_gattc_rebuild_cache((GATTC_NV_SRV_ATTR_t*)&nv_record->srv_cache_attr);
#endif
#endif
#ifdef CODEC_ADC_DC_NV_DATA
    nvrecord_rebuild_adc_dc_calib_info((HAL_CODEC_ADC_DC_CALIB_CFG_NV_T*)&nv_record->adc_dc_calib_cfg_info);
#endif
#ifdef CUSTOM_BITRATE
    nvrecord_rebuild_codec_user_config(&nv_record->codec_user_info);
#endif
    nvrecord_rebuild_sdk_info((uint8_t*)&nv_record->sdk_reserve);

    nv_record->header.magicNumber = NV_EXTENSION_MAGIC_NUMBER;
    nv_record->header.majorVersion = NV_EXTENSION_MAJOR_VERSION;
    nv_record->header.minorVersion = NV_EXTENSION_MINOR_VERSION;
    nv_record->header.validLen = NV_EXTENSION_VALID_LEN;
    nv_record->header.crc32 =
        crc32_c(0, ((uint8_t *)nv_record + NV_EXTENSION_HEADER_SIZE),
                   NV_EXTENSION_VALID_LEN);
}

#ifdef RTOS
static void _nv_mutex_init(void)
{
    nv_record_mutex_id = osMutexCreate(osMutex(NV_RECORD_MUTEX));
    ASSERT(nv_record_mutex_id, "cannot create nv record mutex");
}

static void _nv_mutex_wait(void)
{
    if(nv_record_mutex_id)
    {
        osMutexWait(nv_record_mutex_id, osWaitForever);
    }
}

static void _nv_mutex_release(void)
{
    if(nv_record_mutex_id)
    {
        osMutexRelease(nv_record_mutex_id);
    }
}

#if !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
static void async_flush_confirm_timer_callback(void const *param);
osTimerDef(NORFLASH_ASYNC_CONFIRM_TIMER, async_flush_confirm_timer_callback);
static osTimerId debouce_timer;

static void _nv_async_flush_timer_init(void)
{
    debouce_timer = osTimerCreate(osTimer(NORFLASH_ASYNC_CONFIRM_TIMER), osTimerOnce, NULL);
    if (debouce_timer == NULL) {
        TRACE(1,"Failed to Create async flush timer\n");
    }
}
#endif

#endif

static void _nv_record_extension_init(void)
{
#ifndef RAM_NV_RECORD
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr((uint32_t)__userdata_start);
    hal_norflash_get_size(flash_id,
               NULL,
               &block_size,
               &sector_size,
               &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_USERDATA,
                    flash_id,
                    ((uint32_t)__userdata_start),
                    ((uint32_t)__userdata_end - (uint32_t)__userdata_start),
                    block_size,
                    sector_size,
                    page_size,
                    NV_EXTENSION_SIZE*2,
                    nv_extension_callback
                    );
    ASSERT(result == NORFLASH_API_OK,"_nv_record_extension_init: module register failed! result = %d.",result);
#endif
}

uint32_t nv_record_pre_write_operation(void)
{
    int ret = 0;
    uint32_t nv_start = (uint32_t)&local_extension_data.nv_record;
    uint32_t len = NV_EXTENSION_MIRROR_RAM_SIZE;

    uint32_t lock = int_lock_global();
    ret = mpu_set(nv_start, len, MPU_ATTR_READ_WRITE, MEM_ATTR_INT_SRAM);
    TRACE(2,"set nv ram RW: 0x%x len %d result %d", nv_start, len, ret);
    return lock;
}

void nv_record_post_write_operation(uint32_t lock)
{
    int ret = 0;
    uint32_t nv_start = (uint32_t)&local_extension_data.nv_record;
    uint32_t len = NV_EXTENSION_MIRROR_RAM_SIZE;

    ret = mpu_set(nv_start, len, MPU_ATTR_READ, MEM_ATTR_INT_SRAM);
    int_unlock_global(lock);
    TRACE(2,"set nv ram RO: 0x%x len %d result %d", nv_start, len, ret);
}
static POSSIBLY_UNUSED bool nv_record_is_crc_check_pass(NV_EXTENSION_RECORD_T *nv_record)
{
    NVRECORD_HEADER_T* pExtRecInFlash = &nv_record->header;
    if (NV_EXTENSION_MAGIC_NUMBER != pExtRecInFlash->magicNumber)
    {
        return false;
    }

    if (pExtRecInFlash->validLen >= (NV_EXTENSION_SIZE - NV_EXTENSION_HEADER_SIZE))
    {
        return false;
    }

    uint8_t* pData = (uint8_t*)nv_record + NV_EXTENSION_HEADER_SIZE;
    uint32_t crc = crc32_c(0, pData, pExtRecInFlash->validLen);
    TRACE(1,"generated crc32: 0x%x, header crc: 0x%x",crc, pExtRecInFlash->crc32);
    if (crc == pExtRecInFlash->crc32)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void nv_record_extension_init(void)
{
    uint32_t lock;
    bool main_is_valid = false;
    bool bak_is_valid = false;
    bool data_is_valid = false;

    if(nvrec_init)
    {
        return;
    }

    _user_data_main_start = (uint32_t)__userdata_start;
    _user_data_bak_start = (uint32_t)__userdata_start + NV_EXTENSION_SIZE;


    lock = nv_record_pre_write_operation();
    _nv_record_extension_init();

    nv_flush_state.is_update = false;
    nv_flush_state.written_size = 0;
    nv_flush_state.state = NV_STATE_IDLE;

    nvrecord_extension_p = &local_extension_data.nv_record;

    if(nvrecord_extension_p->header.magicNumber != NV_EXTENSION_MAGIC_NUMBER ||
       nvrecord_extension_p->header.majorVersion != NV_EXTENSION_MAJOR_VERSION ||
       nvrecord_extension_p->header.minorVersion != NV_EXTENSION_MINOR_VERSION ||
       nvrecord_extension_p->header.validLen >= (NV_EXTENSION_SIZE - NV_EXTENSION_HEADER_SIZE))
    {
        ASSERT(0, "%s: local_extension_data error!(0x%x,0x%x,0x%x,0x%x)",
               __func__,
               nvrecord_extension_p->header.magicNumber,
               nvrecord_extension_p->header.majorVersion,
               nvrecord_extension_p->header.minorVersion,
               nvrecord_extension_p->header.validLen);
    }

    // Check main sector.
    if(nv_record_data_is_valid((NV_EXTENSION_RECORD_T*)_user_data_main_start)
        && nv_record_items_is_valid((NV_EXTENSION_RECORD_T*)_user_data_main_start))
    {
        //TRACE(2,"%s,main sector is valid.",__func__);
        main_is_valid = true;
    }
    else
    {
        TRACE(1,"%s,main sector is invalid!",__func__);
        main_is_valid = false;
    }

    // Check bak secotr.
    if(nv_record_data_is_valid((NV_EXTENSION_RECORD_T*)_user_data_bak_start)
       && nv_record_items_is_valid((NV_EXTENSION_RECORD_T*)_user_data_bak_start))
    {
        //TRACE(2,"%s,bak sector is valid.",__func__);
        bak_is_valid = true;
    }
    else
    {
        TRACE(1,"%s,bak sector is invalid!",__func__);
        bak_is_valid = false;
    }

    if(main_is_valid)
    {
        data_is_valid = true;
        if(!bak_is_valid)
        {
            nv_flush_state.is_update = true;
            nv_flush_state.state = NV_STATE_MAIN_DONE;
        }
    }
    else
    {
        if(bak_is_valid)
        {
            data_is_valid = true;
            nv_flush_state.is_update = true;
            nv_flush_state.state = NV_STATE_IDLE;
        }
        else
        {
            data_is_valid = false;
        }
    }

    if(data_is_valid)
    {
        TRACE(2,"%s,data is valid.", __func__);
        if(main_is_valid)
        {
            memcpy((uint8_t *)nvrecord_extension_p + NV_EXTENSION_HEADER_SIZE,
                       (uint8_t *)_user_data_main_start + NV_EXTENSION_HEADER_SIZE,
                        NV_EXTENSION_VALID_LEN);
        }
        else
        {
            memcpy((uint8_t *)nvrecord_extension_p + NV_EXTENSION_HEADER_SIZE,
                       (uint8_t *)_user_data_bak_start + NV_EXTENSION_HEADER_SIZE,
                        NV_EXTENSION_VALID_LEN);
        }
        nvrecord_extension_p->header.crc32 =
                    crc32_c(0,
                          ((uint8_t *)nvrecord_extension_p + NV_EXTENSION_HEADER_SIZE),
                          nvrecord_extension_p->header.validLen);
    }
    else
    {
        TRACE(1,"%s,data invalid, rebuild... ",__func__);
        nv_record_set_default(nvrecord_extension_p, NV_REBUILD_ALL);
        TRACE(2,"%s,rebuild crc = 0x%x. ",__func__,nvrecord_extension_p->header.crc32);
        // need to update the content in the flash
        nv_record_extension_update();
    }

    TRACE(0, "peer addr:");
    DUMP8("%02x ", nv_record_get_ibrt_peer_addr(), BT_ADDR_OUTPUT_PRINT_NUM);

    nvrec_init = true;

    nv_record_post_write_operation(lock);

    if(nv_flush_state.is_update)
    {
        nv_record_extension_flush(false);
    }
    TRACE(2,"%s,done.", __func__);

    TRACE(0, "sdk_owned_end: %d customer_owned_start: %d",
        (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->sdk_owned_end),
        (uint32_t)&(((NV_EXTENSION_RECORD_T *)0)->customer_owned_start));
}

NV_EXTENSION_RECORD_T* nv_record_get_extension_entry_ptr(void)
{
    return nvrecord_extension_p;
}

void nv_record_extension_update(void)
{
    nv_flush_state.is_update = true;
#ifndef RAM_NV_RECORD
    norflash_api_hook_activate();
#endif
}

#ifndef RAM_NV_RECORD
static int nv_record_extension_flush_main(bool is_async)
{
    enum NORFLASH_API_RET_T ret = NORFLASH_API_OK;
    uint32_t crc;
    enum HAL_FLASH_ID_T dev_id;

    ret = norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_USERDATA, &dev_id);
    if(ret)
    {
        TRACE(1,"%s,norflash_api_get_dev_id fail ret = %d.", __func__, ret);
        goto _func_end;
    }

    if(NULL == nvrecord_extension_p)
    {
        TRACE(1,"%s,nvrecord_extension_p is null.", __func__);
        goto _func_end;
    }

    if(is_async)
    {
        if(nv_flush_state.state == NV_STATE_IDLE
           && nv_flush_state.is_update == true)
        {
            TRACE(3,"%s: async flush begin!", __func__);
        }

        if(nv_flush_state.state == NV_STATE_IDLE
           || nv_flush_state.state == NV_STATE_MAIN_ERASING)
        {
            if(nv_flush_state.state == NV_STATE_IDLE)
            {
                nv_flush_state.state = NV_STATE_MAIN_ERASING;
                // TRACE(4,"%s: NV_STATE_MAIN_ERASING", __func__);
            }

            ret = norflash_api_erase(NORFLASH_API_MODULE_ID_USERDATA,
                        (uint32_t)(__userdata_start),
                        NV_EXTENSION_SIZE,
                        true);
            if(ret == NORFLASH_API_OK)
            {
                nv_flush_state.state = NV_STATE_MAIN_ERASED;
                // TRACE(4,"%s: NV_STATE_MAIN_ERASED", __func__);
                // TRACE(4,"%s: norflash_api_erase ok,addr = 0x%x.",__func__,_user_data_main_start);
            }
            else if(ret == NORFLASH_API_BUFFER_FULL)
            {
                norflash_api_flush();
            }
            else
            {
                ASSERT(0,"%s: norflash_api_erase err,ret = %d,addr = 0x%x.",__func__,ret,_user_data_main_start);
            }
        }
        else if(nv_flush_state.state == NV_STATE_MAIN_ERASED)
        {
            uint32_t tmpLock = nv_record_pre_write_operation();

            crc = crc32_c(0,(uint8_t *)nvrecord_extension_p + NV_EXTENSION_HEADER_SIZE,
                 NV_EXTENSION_VALID_LEN);
            nvrecord_extension_p->header.crc32 = crc;
            ASSERT(nv_record_data_is_valid(nvrecord_extension_p)
                   && nv_record_items_is_valid(nvrecord_extension_p),
                  "%s nv_record is invalid!",__func__);
            memcpy((uint8_t *)&_nv_burn_buf, (uint8_t *)nvrecord_extension_p, sizeof(NV_EXTENSION_RECORD_T));
            nv_flush_state.state = NV_STATE_MAIN_WRITE_PENDING;
            nv_flush_state.is_update = false;
            nv_record_post_write_operation(tmpLock);
            // TRACE(4,"%s: NV_STATE_MAIN_WRITE_PENDING", __func__);
        }
        else if(nv_flush_state.state == NV_STATE_MAIN_WRITE_PENDING)
        {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_USERDATA,
                        (uint32_t)(__userdata_start),
                        (uint8_t *)&_nv_burn_buf,
                        sizeof(NV_EXTENSION_RECORD_T),
                        true);

            if(ret == NORFLASH_API_OK)
            {
                nv_flush_state.state = NV_STATE_MAIN_WRITTEN;
                // TRACE(4,"%s: NV_STATE_MAIN_WRITTEN", __func__);
                // TRACE(4,"%s: norflash_api_write ok,addr = 0x%x.",__func__,_user_data_main_start);
            }
            else if(ret == NORFLASH_API_BUFFER_FULL)
            {
                norflash_api_flush();
            }
            else
            {
                ASSERT(0,"%s: norflash_api_write err,ret = %d,addr = 0x%x.",__func__,ret,_user_data_main_start);
            }
        }
        else if(nv_flush_state.state == NV_STATE_MAIN_WRITTEN)
        {
            if(norflash_api_get_used_buffer_count(NORFLASH_API_MODULE_ID_USERDATA,NORFLASH_API_ALL) == 0)
            {
                nv_flush_state.state = NV_STATE_MAIN_DONE;
                // TRACE(3,"%s: async flush done.", __func__);
            }
            else
            {
                norflash_api_flush();
            }
        }
        else
        {
            ASSERT(0,"%s: nv_flush_state.state ERROR stat = %d.",
                    __func__, nv_flush_state.state);
        }
    }
    else
    {
        // TRACE(3,"%s: sync flush begin!", __func__);
        if(nv_flush_state.state == NV_STATE_IDLE
           || nv_flush_state.state == NV_STATE_MAIN_ERASING)
        {
            do
            {
                ret = norflash_api_erase(NORFLASH_API_MODULE_ID_USERDATA,
                        (uint32_t)(__userdata_start),
                        NV_EXTENSION_SIZE,
                        true);
                if(ret == NORFLASH_API_OK)
                {
                    nv_flush_state.state = NV_STATE_MAIN_ERASED;
                    // TRACE(4,"%s: norflash_api_erase ok,addr = 0x%x.",__func__, _user_data_main_start);
                }
                else if(ret == NORFLASH_API_BUFFER_FULL)
                {
                    do
                    {
                        norflash_api_flush();
                    }while(norflash_api_get_free_buffer_count(dev_id, NORFLASH_API_ERASING) == 0);
                }
                else
                {
                    ASSERT(0,"%s: norflash_api_erase err,ret = %d,addr = 0x%x.",__func__,ret,_user_data_main_start);
                }

            }while(ret == NORFLASH_API_BUFFER_FULL);
        }

        if(nv_flush_state.state == NV_STATE_MAIN_ERASED)
        {
            uint32_t tmpLock = nv_record_pre_write_operation();
            crc = crc32_c(0,(uint8_t *)nvrecord_extension_p + NV_EXTENSION_HEADER_SIZE,
                 NV_EXTENSION_VALID_LEN);
            nvrecord_extension_p->header.crc32 = crc;
            ASSERT(nv_record_data_is_valid(nvrecord_extension_p)
                   && nv_record_items_is_valid(nvrecord_extension_p),
                    "%s nv_record is invalid!",__func__);
            memcpy((uint8_t *)&_nv_burn_buf, (uint8_t *)nvrecord_extension_p, sizeof(NV_EXTENSION_RECORD_T));
            nv_flush_state.state = NV_STATE_MAIN_WRITE_PENDING;
            nv_flush_state.is_update = false;
            nv_record_post_write_operation(tmpLock);
        }

        if(nv_flush_state.state == NV_STATE_MAIN_WRITE_PENDING)
        {
            do
            {
                ret = norflash_api_write(NORFLASH_API_MODULE_ID_USERDATA,
                        (uint32_t)(__userdata_start),
                        (uint8_t *)&_nv_burn_buf,
                        sizeof(NV_EXTENSION_RECORD_T),
                        true);
                if(ret == NORFLASH_API_OK)
                {
                    nv_flush_state.state = NV_STATE_MAIN_WRITTEN;
                    // TRACE(4,"%s: norflash_api_write ok,addr = 0x%x.",__func__,_user_data_main_start);
                }
                else if(ret == NORFLASH_API_BUFFER_FULL)
                {
                    do
                    {
                        norflash_api_flush();
                    }while(norflash_api_get_free_buffer_count(dev_id, NORFLASH_API_WRITTING) == 0);
                }
                else
                {
                    ASSERT(0,"%s: norflash_api_write err,ret = %d,addr = 0x%x.",__func__,ret,_user_data_main_start);
                }
            }while(ret == NORFLASH_API_BUFFER_FULL);
        }

        do
        {
            norflash_api_flush();
        }while(norflash_api_get_used_buffer_count(NORFLASH_API_MODULE_ID_USERDATA,NORFLASH_API_ALL) > 0);

        nv_flush_state.state = NV_STATE_MAIN_DONE;

        // TRACE(3,"%s: sync flush done.", __func__);
    }
_func_end:
    return (ret == NORFLASH_API_OK) ? 0:1;
}

static int nv_record_extension_flush_bak(bool is_async)
{
    enum NORFLASH_API_RET_T ret = NORFLASH_API_OK;
    uint8_t *burn_buf = (uint8_t*)&_nv_burn_buf;
    enum HAL_FLASH_ID_T dev_id;

    ret = norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_USERDATA, &dev_id);
    if(ret)
    {
        TRACE(1,"%s,norflash_api_get_dev_id fail ret = %d.", __func__, ret);
        goto _func_end;
    }

    if(is_async)
    {
        if(nv_flush_state.state == NV_STATE_MAIN_DONE
           || nv_flush_state.state == NV_STATE_BAK_ERASING)
        {
            if(nv_flush_state.state == NV_STATE_MAIN_DONE)
            {
                nv_flush_state.state = NV_STATE_BAK_ERASING;
                // TRACE(3,"%s: async flush begin", __func__);
            }

            ret = norflash_api_erase(NORFLASH_API_MODULE_ID_USERDATA,
                    _user_data_bak_start,
                    NV_EXTENSION_SIZE,
                    true);
            if(ret == NORFLASH_API_OK)
            {
                nv_flush_state.state = NV_STATE_BAK_ERASED;
                // TRACE(4,"%s: norflash_api_erase ok,addr = 0x%x.",
                //        __func__,_user_data_bak_start);
            }
            else if(ret == NORFLASH_API_BUFFER_FULL)
            {
                norflash_api_flush();
            }
            else
            {
                ASSERT(0,"%s: norflash_api_erase err,ret = %d,addr = 0x%x.",
                        __func__,ret,_user_data_bak_start);
            }
        }
        else if(nv_flush_state.state == NV_STATE_BAK_ERASED ||
                nv_flush_state.state == NV_STATE_BAK_WRITTING)
        {
            nv_flush_state.state = NV_STATE_BAK_WRITTING;
            // TRACE(4,"%s: NV_STATE_BAK_WRITTING", __func__);

            ret = norflash_api_read(NORFLASH_API_MODULE_ID_USERDATA,
                    _user_data_main_start + nv_flush_state.written_size,
                    burn_buf,
                    sizeof(NV_EXTENSION_RECORD_T)
                    );
            ASSERT(ret == NORFLASH_API_OK,"norflash_api_read failed! ret = %d, addr = 0x%x.",
                               (int32_t)ret,_user_data_main_start + nv_flush_state.written_size);
            nv_flush_state.state = NV_STATE_BAK_WRITE_PENDING;
         }
         else if(nv_flush_state.state == NV_STATE_BAK_WRITE_PENDING)
         {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_USERDATA,
                    _user_data_bak_start + nv_flush_state.written_size,
                    burn_buf,
                    sizeof(NV_EXTENSION_RECORD_T),
                    true
                    );

            if(ret == NORFLASH_API_OK)
            {
                nv_flush_state.state = NV_STATE_BAK_WRITTEN;
            }
            else if(ret == NORFLASH_API_BUFFER_FULL)
            {
                norflash_api_flush();
            }
            else
            {
                ASSERT(0,"%s: norflash_api_write err,ret = %d,addr = 0x%x.",
                    __func__,ret,_user_data_bak_start + nv_flush_state.written_size);
            }
        }
        else if(nv_flush_state.state == NV_STATE_BAK_WRITTEN)
        {
            if(norflash_api_get_used_buffer_count(NORFLASH_API_MODULE_ID_USERDATA,NORFLASH_API_ALL) == 0)
            {
                nv_flush_state.state = NV_STATE_BAK_DONE;
                //TRACE(3,"%s: async flush done.", __func__);
            }
            else
            {
                norflash_api_flush();
            }
        }
        else
        {
            ASSERT(0,"%s: nv_flush_state.state ERROR stat = %d.",
                    __func__, nv_flush_state.state);
        }
    }
    else
    {
        //TRACE(3,"%s: sync flush begin.", __func__);
        if(nv_flush_state.state == NV_STATE_MAIN_DONE
           || nv_flush_state.state == NV_STATE_BAK_ERASING)
        {
            do
            {
                ret = norflash_api_erase(NORFLASH_API_MODULE_ID_USERDATA,
                            _user_data_bak_start,
                            NV_EXTENSION_SIZE,
                            true);
                if(ret == NORFLASH_API_OK)
                {
                    nv_flush_state.state = NV_STATE_BAK_ERASED;
                    //TRACE(4,"%s: norflash_api_erase ok,addr = 0x%x.",
                    //    __func__, _user_data_bak_start);
                    break;
                }
                else if(ret == NORFLASH_API_BUFFER_FULL)
                {
                    do
                    {
                        norflash_api_flush();
                    }while(norflash_api_get_free_buffer_count(dev_id, NORFLASH_API_ERASING) == 0);
                }
                else
                {
                    ASSERT(0,"%s: norflash_api_erase err,ret = %d,addr = 0x%x.",__func__,ret,_user_data_bak_start);
                }
            }while(ret == NORFLASH_API_BUFFER_FULL);
        }

        if(nv_flush_state.state == NV_STATE_BAK_ERASED ||
           nv_flush_state.state == NV_STATE_BAK_WRITTEN)
        {
            nv_flush_state.state = NV_STATE_BAK_WRITTEN;
            ret = norflash_api_read(NORFLASH_API_MODULE_ID_USERDATA,
                    _user_data_main_start + nv_flush_state.written_size,
                    burn_buf,
                    sizeof(NV_EXTENSION_RECORD_T)
                    );
            ASSERT(ret == NORFLASH_API_OK,"norflash_api_read failed! ret = %d, addr = 0x%x.",
                    (int32_t)ret, _user_data_main_start + nv_flush_state.written_size);
            nv_flush_state.state = NV_STATE_BAK_WRITE_PENDING;
        }

        if(nv_flush_state.state == NV_STATE_BAK_WRITE_PENDING)
        {
            do
            {
                ret = norflash_api_write(NORFLASH_API_MODULE_ID_USERDATA,
                        _user_data_bak_start + nv_flush_state.written_size,
                        burn_buf,
                        sizeof(NV_EXTENSION_RECORD_T),
                        true
                        );
                if(ret == NORFLASH_API_OK)
                {
                    nv_flush_state.state = NV_STATE_BAK_WRITTEN;
                    break;
                }
                else if(ret == NORFLASH_API_BUFFER_FULL)
                {
                    do
                    {
                        norflash_api_flush();
                    }while(norflash_api_get_free_buffer_count(dev_id, NORFLASH_API_WRITTING) == 0);
                }
                else
                {
                    ASSERT(0,"%s: norflash_api_write err,ret = %d,addr = 0x%x.",
                        __func__,ret,_user_data_bak_start + nv_flush_state.written_size);
                }
            }while(1);
        }

        do
        {
            norflash_api_flush();
        }while(norflash_api_get_used_buffer_count(NORFLASH_API_MODULE_ID_USERDATA,NORFLASH_API_ALL) > 0);

        // TRACE(3,"%s: sync flush done.", __func__);
        nv_flush_state.state = NV_STATE_BAK_DONE;
    }

    if(nv_flush_state.state == NV_STATE_BAK_DONE)
    {
       // TRACE(3,"%s: NV_STATE_IDLE", __func__);
       nv_flush_state.state = NV_STATE_IDLE;
    }

_func_end:
    return (ret == NORFLASH_API_OK) ? 0:1;
}
#endif


static int nv_record_extension_flush(bool is_async)
{
    int ret = 0;

#ifdef RTOS
    _nv_mutex_wait();
#endif
#ifndef RAM_NV_RECORD
    do{
        if(nv_flush_state.state == NV_STATE_IDLE
           && nv_flush_state.is_update == FALSE)
        {
            break;
        }

        if((nv_flush_state.state == NV_STATE_IDLE
               && nv_flush_state.is_update == TRUE)
            || nv_flush_state.state == NV_STATE_MAIN_ERASING
            || nv_flush_state.state == NV_STATE_MAIN_ERASED
            || nv_flush_state.state == NV_STATE_MAIN_WRITTING
            || nv_flush_state.state == NV_STATE_MAIN_WRITE_PENDING
            || nv_flush_state.state == NV_STATE_MAIN_WRITTEN
        )
        {
            ret = nv_record_extension_flush_main(is_async);
            if(is_async)
            {
                break;
            }
        }

        if(nv_flush_state.state == NV_STATE_MAIN_DONE
            || nv_flush_state.state == NV_STATE_BAK_ERASING
            || nv_flush_state.state == NV_STATE_BAK_ERASED
            || nv_flush_state.state == NV_STATE_BAK_WRITTING
            || nv_flush_state.state == NV_STATE_BAK_WRITE_PENDING
            || nv_flush_state.state == NV_STATE_BAK_WRITTEN
            || nv_flush_state.state == NV_STATE_BAK_DONE
        )
        {
            ret = nv_record_extension_flush_bak(is_async);
        }
    }while(!is_async);

#endif
#ifdef RTOS
    _nv_mutex_release();
#endif
    return ret;
}

void nv_extension_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;
    opera_result = (NORFLASH_API_OPERA_RESULT*)param;
    TRACE(6, "%s:type=%d,addr=0x%x,len=0x%x,rem=%d,rst=%d,suspend=%d",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->remain_num,
                opera_result->result,
                opera_result->suspend_num);
}

void nv_record_init(void)
{
#ifdef RTOS
    _nv_mutex_init();
#if !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    _nv_async_flush_timer_init();
#endif
#endif
    nv_record_open(section_usrdata_ddbrecord);

    nv_custom_parameter_section_init();
}

bt_status_t nv_record_open(SECTIONS_ADP_ENUM section_id)
{
    nv_record_extension_init();
    nvrecord_bt_init();
#ifndef RAM_NV_RECORD
#ifdef FLASH_SUSPEND
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, AUDMA_IRQn, true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA_IRQn, true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA1_IRQn, true);
    norflash_api_set_hook(NORFLASH_API_HOOK_USER_USERDATA, nv_record_flash_flush_in_sleep);
#endif //FLASH_SUSPEND
#endif
    return BT_STS_SUCCESS;
}

void nv_record_update_runtime_userdata(void)
{
    nv_record_extension_update();
}

int nv_record_touch_cause_flush(void)
{
    nv_record_update_runtime_userdata();
    return 0;
}

void nv_record_sector_clear(void)
{
#ifndef RAM_NV_RECORD
    enum NORFLASH_API_RET_T ret;

    ret = norflash_api_erase(NORFLASH_API_MODULE_ID_USERDATA,
                          (uint32_t)__userdata_start,
                          NV_EXTENSION_SIZE,
                          false);
    ASSERT(ret == NORFLASH_API_OK,
        "%s: norflash_api_erase(0x%x) failed! ret = %d.",
        __func__, (uint32_t)__userdata_start, (int32_t)ret);

    ret = norflash_api_erase(NORFLASH_API_MODULE_ID_USERDATA,
                          (uint32_t)__userdata_start + NV_EXTENSION_SIZE,
                          NV_EXTENSION_SIZE,
                          false);
    ASSERT(ret == NORFLASH_API_OK,
        "%s: norflash_api_erase(0x%x) failed! ret = %d.",
        __func__, (uint32_t)__userdata_start + NV_EXTENSION_SIZE, (int32_t)ret);
#endif
}

void nv_record_rebuild(NV_REBUILD_TYPE_E rebuild_type)
{
    if (nvrecord_extension_p)
    {
        TRACE(1,"%s: begin.", __func__);
        uint32_t nv_start;
        nv_start = (uint32_t)&local_extension_data.nv_record;
        uint32_t lock = int_lock_global();
        mpu_set(nv_start, NV_EXTENSION_MIRROR_RAM_SIZE, MPU_ATTR_READ_WRITE, MEM_ATTR_INT_SRAM);
        nv_record_reset_sdk_info(nvrecord_extension_p, rebuild_type);
        mpu_set(nv_start, NV_EXTENSION_MIRROR_RAM_SIZE, MPU_ATTR_READ, MEM_ATTR_INT_SRAM);
        nv_record_extension_update();
        int_unlock_global(lock);
        nv_record_flash_flush();
        TRACE(1,"%s: done.", __func__);
    }
    else
    {
        TRACE(1,"%s: begin. nvrecord_extension_p = null.", __func__);
        uint32_t lock = int_lock_global();
        nv_record_sector_clear();
        nvrec_init = false;
        nv_record_extension_init();
        int_unlock_global(lock);
        TRACE(1,"%s: done.", __func__);
    }
}

static uint32_t lastFlushCheckPointInMs = 0;

static void nv_record_reset_flush_checkpoint(void)
{
    lastFlushCheckPointInMs = GET_CURRENT_MS();
}

#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
static void async_flush_confirm_timer_callback(void const *param)
{
    norflash_api_hook_activate();
}

static bool nv_norflash_async_flush_check()
{
    static bool flush_is_delay = false;
    bool ret = false;

    if (!flush_is_delay) {
        osStatus_t status = osTimerStart(debouce_timer, NV_RECORD_ASYNC_FLUSH_CONFIRM_TIME_MS);
        if (status == osOK) {
            flush_is_delay = true;
        } else {
            ret = true;
        }
    } else {
        if (osTimerIsRunning(debouce_timer)) {
            osTimerStop(debouce_timer);
            osStatus_t status = osTimerStart(debouce_timer, NV_RECORD_ASYNC_FLUSH_CONFIRM_TIME_MS);
            if (status != osOK) {
                ret = true;
                flush_is_delay = false;
            }
        } else {
            ret = true;
            flush_is_delay = false;
        }
    }
    return ret;
}
#endif

static bool nv_record_is_timer_expired_to_check(void)
{
    uint32_t passedTimerMs;
    uint32_t currentTimerMs = GET_CURRENT_MS();

#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    return nv_norflash_async_flush_check();
#endif

    if (0 == lastFlushCheckPointInMs)
    {
        passedTimerMs = currentTimerMs;
    }
    else
    {
        if (currentTimerMs >= lastFlushCheckPointInMs)
        {
            passedTimerMs = currentTimerMs - lastFlushCheckPointInMs;
        }
        else
        {
            passedTimerMs = TICKS_TO_MS(0xFFFFFFFF) - lastFlushCheckPointInMs + currentTimerMs + 1;
        }
    }

    if (passedTimerMs > NV_RECORD_FLUSH_EXECUTION_INTERVAL_MS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void nv_record_flash_flush(void)
{
#ifndef RAM_NV_RECORD
    nv_record_extension_flush(false);
    nv_record_reset_flush_checkpoint();
#endif
}

void nv_record_execute_async_flush(void)
{
    int ret = nv_record_extension_flush(true);

    if (0 == ret)
    {
        nv_record_reset_flush_checkpoint();
    }
}

int nv_record_flash_flush_in_sleep(void)
{
    if ((NV_STATE_IDLE == nv_flush_state.state) &&
        !nv_record_is_timer_expired_to_check())
    {
        return 0;
    }

    nv_record_execute_async_flush();
    if(nv_flush_state.state == NV_STATE_IDLE && nv_flush_state.is_update == FALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint32_t nv_record_get_ibrt_mode(void)
{
    if (nvrecord_extension_p)
    {
        return nvrecord_extension_p->system_info.ibrt_mode.mode;
    }
    else
    {
        return NVRAM_ENV_TWS_MODE_DEFAULT;
    }
}

uint8_t* nv_record_get_ibrt_peer_addr(void)
{
    if (nvrecord_extension_p)
    {
        return nvrecord_extension_p->system_info.ibrt_mode.record.bdAddr.address;
    }
    else
    {
        return NULL;
    }
}

#endif // #if defined(NEW_NV_RECORD_ENABLED)
