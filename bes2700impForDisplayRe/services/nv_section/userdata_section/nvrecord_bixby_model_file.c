#include "cmsis.h"
#include "hal_trace.h"
#include "nvrecord_bixby_model_file.h"

extern uint32_t embedded_bixby_model_file_default_start[];
extern uint32_t embedded_bixby_model_file_default_end[];

extern uint32_t embedded_bixby_model_file_de_de_start[];
extern uint32_t embedded_bixby_model_file_de_de_end[];

extern uint32_t embedded_bixby_model_file_en_gb_start[];
extern uint32_t embedded_bixby_model_file_en_gb_end[];

extern uint32_t embedded_bixby_model_file_en_us_start[];
extern uint32_t embedded_bixby_model_file_en_us_end[];

extern uint32_t embedded_bixby_model_file_es_es_start[];
extern uint32_t embedded_bixby_model_file_es_es_end[];

extern uint32_t embedded_bixby_model_file_fr_fr_start[];
extern uint32_t embedded_bixby_model_file_fr_fr_end[];

extern uint32_t embedded_bixby_model_file_it_it_start[];
extern uint32_t embedded_bixby_model_file_it_it_end[];

extern uint32_t embedded_bixby_model_file_ko_kr_start[];
extern uint32_t embedded_bixby_model_file_ko_kr_end[];

extern uint32_t embedded_bixby_model_file_pt_br_start[];
extern uint32_t embedded_bixby_model_file_pt_br_end[];

extern uint32_t embedded_bixby_model_file_zh_cn_start[];
extern uint32_t embedded_bixby_model_file_zh_cn_end[];

static BIXBY_MODEL_FILE_INFO_ENTRY_T defaultBixbyModel[] = {
    {BIXBY_MODEL_DEFAULT, (uint32_t)embedded_bixby_model_file_default_start,  BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_DE_DE, (uint32_t)embedded_bixby_model_file_de_de_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_EN_GB, (uint32_t)embedded_bixby_model_file_en_gb_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_EN_US, (uint32_t)embedded_bixby_model_file_en_us_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_ES_ES, (uint32_t)embedded_bixby_model_file_es_es_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_FR_FR, (uint32_t)embedded_bixby_model_file_fr_fr_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_IT_IT, (uint32_t)embedded_bixby_model_file_it_it_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_KO_KR, (uint32_t)embedded_bixby_model_file_ko_kr_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_PT_BR, (uint32_t)embedded_bixby_model_file_pt_br_start,      BIXBY_MODEL_FILE_MAX_SIZE},
    {BIXBY_MODEL_ZH_CN, (uint32_t)embedded_bixby_model_file_zh_cn_start,      BIXBY_MODEL_FILE_MAX_SIZE},
};

static BIXBY_MODEL_FILE_INFO_T bixby_model_file_info;


static bool nv_record_bixby_model_file_refresh_default_model(BIXBY_MODEL_FILE_INFO_ENTRY_T* pEntry)
{
    if (pEntry)
    {
        if ((pEntry->modelLanguage >= BIXBY_MODEL_DEFAULT) &&
            pEntry->modelLanguage < BIXBY_MODEL_FILE_NUM)
        {
            pEntry->startAddr |= BIXBY_FLASH_LOGIC_ADDR;
            bixby_model_file_info.entry[pEntry->modelLanguage] = *pEntry;
            return true;
        }        
    }

    return false;
}

void nv_record_bixby_model_file_init(void)
{
    // TODO: shall be replaced with Samsung implementation of model file management
    for (uint32_t index = 0;index < sizeof(defaultBixbyModel)/sizeof(defaultBixbyModel[0]);index++)
    {
        nv_record_bixby_model_file_refresh_default_model(&defaultBixbyModel[index]);
    }

    bixby_model_file_info.isModelFilePendingForUpdate = false;

    bixby_model_file_info.currentLanguate = BIXBY_MODEL_DEFAULT;
}

uint8_t* nv_record_bixby_model_file_get_content(BIXBY_MODEL_LANGUAGE_E language, uint32_t* modelFileLen)
{
    BIXBY_MODEL_FILE_INFO_ENTRY_T* pEntry = &(bixby_model_file_info.entry[language]);
    *modelFileLen = pEntry->len;
    return (uint8_t *)pEntry->startAddr;
}

BIXBY_MODEL_LANGUAGE_E nv_record_bixby_model_file_get_current_language(void)
{
    return bixby_model_file_info.currentLanguate;
}

bool nv_record_bixby_model_file_update_language(BIXBY_MODEL_LANGUAGE_E language)
{
    if (language >= BIXBY_MODEL_FILE_NUM)
    {
        TRACE(0, "nv_record_bixby_model_file_update_language parameter error!");
        return false;
    }
    else if (language > BIXBY_MODEL_ZH_CN)
    {
        language = BIXBY_MODEL_DEFAULT;
    }

    if (bixby_model_file_info.currentLanguate != language)
    {
        TRACE(0, "Language from %d to %d", bixby_model_file_info.currentLanguate, language);
        bixby_model_file_info.currentLanguate = language;
        bixby_model_file_info.isModelFilePendingForUpdate = true;
        return true;
    }
    else
    {
        return false;
    }
}

bool nv_record_bixby_model_file_is_pending_for_update(void)
{
    return bixby_model_file_info.isModelFilePendingForUpdate;
}

void nv_record_bixby_model_file_clear_pending_for_update_flag(void)
{
    bixby_model_file_info.isModelFilePendingForUpdate = false;
}

