/***************************************************************************
*
*Copyright 2015-2021 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/


#ifndef __NVRECORD_BIXBY_MODEL_FILE_H__
#define __NVRECORD_BIXBY_MODEL_FILE_H__

#ifdef __cplusplus
extern "C"{
#endif

#define MAXIMUM_BIXBY_MODEL_FILES   16

typedef enum
{
    BIXBY_MODEL_DEFAULT = 0,
    BIXBY_MODEL_DE_DE   = 1,
    BIXBY_MODEL_EN_GB   = 2,
    BIXBY_MODEL_EN_US   = 3,
    BIXBY_MODEL_ES_ES   = 4,
    BIXBY_MODEL_FR_FR   = 5,
    BIXBY_MODEL_IT_IT   = 6,
    BIXBY_MODEL_KO_KR   = 7,
    BIXBY_MODEL_PT_BR   = 8,
    BIXBY_MODEL_ZH_CN   = 9,

    // TODO: add more model files
    // we reserve 6 more model files space here
    
    BIXBY_MODEL_FILE_NUM   = MAXIMUM_BIXBY_MODEL_FILES,
    
} BIXBY_MODEL_LANGUAGE_E;


typedef struct {

    BIXBY_MODEL_LANGUAGE_E modelLanguage;

    // start logic address of the model
    uint32_t startAddr;

    // length of model file
    uint32_t len;
    
} BIXBY_MODEL_FILE_INFO_ENTRY_T;

typedef struct
{
    bool isModelFilePendingForUpdate;
    BIXBY_MODEL_LANGUAGE_E currentLanguate;
    BIXBY_MODEL_FILE_INFO_ENTRY_T entry[MAXIMUM_BIXBY_MODEL_FILES];
    
} BIXBY_MODEL_FILE_INFO_T;


/******************************macro defination*****************************/
#define BIXBY_MODEL_FILE_MAX_SIZE (32 * 1024) //!< should adjust this value if
//!< hotword model size changed and should adjust the flash section size at
//!< the sametime., @see BIXBY_MODEL_FILES_SECTION_SIZE in common.mk

#define BIXBY_FLASH_LOGIC_ADDR        (FLASH_NC_BASE)

/******************************type defination******************************/

void nv_record_bixby_model_file_init(void);

uint8_t* nv_record_bixby_model_file_get_content(BIXBY_MODEL_LANGUAGE_E language, uint32_t* modelFileLen);

BIXBY_MODEL_LANGUAGE_E nv_record_bixby_model_file_get_current_language(void);

bool nv_record_bixby_model_file_update_language(BIXBY_MODEL_LANGUAGE_E language);

bool nv_record_bixby_model_file_is_pending_for_update(void);

void nv_record_bixby_model_file_clear_pending_for_update_flag(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __NVRECORD_BIXBY_MODEL_FILE_H__ */
