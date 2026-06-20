#ifndef  _NORFLASH_ASYNC_API_H_
#define _NORFLASH_ASYNC_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_norflash.h"
#ifdef FLASH_DUAL_CHIP
#define NORFLASH_API_BLOCK_MAX_SIZE     0x10000
#define NORFLASH_API_SECTOR_MAX_SIZE    0x2000
#define NORFLASH_API_PAGE_MAX_SIZE      0x200
#else
#define NORFLASH_API_BLOCK_MAX_SIZE     0x8000
#define NORFLASH_API_SECTOR_MAX_SIZE    0x1000
#define NORFLASH_API_PAGE_MAX_SIZE      0x100
#endif

#define GET_FLASH_DEV_ID(addr)          norflash_api_get_dev_id_by_addr(addr)

enum NORFLASH_API_MODULE_ID_T
{
#ifdef OS_WITH_FILESYSTEM
    NORFLASH_API_MODULE_ID_OS_FS,
#else
    NORFLASH_API_MODULE_ID_LOG_DUMP,
    NORFLASH_API_MODULE_ID_USERDATA,
    NORFLASH_API_MODULE_ID_BOOTUP_INFO,
    NORFLASH_API_MODULE_ID_OTA,
    NORFLASH_API_MODULE_ID_UPGRADE_LOG,
    NORFLASH_API_MODULE_ID_CUSTOM_PARAMETER,
#if defined(_AUTO_TEST_)
    NORFLASH_API_MODULE_ID_FREE,
    NORFLASH_API_MODULE_ID_RESERVED,
    NORFLASH_API_MODULE_ID_AUD,
#endif
    NORFLASH_API_MODULE_ID_CRASH_DUMP,
    NORFLASH_API_MODULE_ID_COREDUMP,
    NORFLASH_API_MODULE_ID_RAMDUMP,
    NORFLASH_API_MODULE_ID_FACTORY,
    NORFLASH_API_MODULE_ID_FACTORY_BACKUP,
    NORFLASH_API_MODULE_ID_HOTWORD_MODEL,
    NORFLASH_API_MODULE_ID_INTERACTION_OTA,
    NORFLASH_API_MODULE_ID_GMA_OTA,
    NORFLASH_API_MODULE_ID_AUDIO,
    NORFLASH_API_MODULE_ID_PROMPT,
    NORFLASH_API_MODULE_ID_ANC,
    NORFLASH_API_MODULE_ID_TEST1,
    NORFLASH_API_MODULE_ID_TEST2,
    NORFLASH_API_MODULE_ID_BOOT_INFO,
    NORFLASH_API_MODULE_ID_LITTLEFS,
    NORFLASH_API_MODULE_ID_NORFS,
    NORFLASH_API_MODULE_ID_KV,
    NORFLASH_API_MODULE_ID_MULTI_BIN,
    NORFLASH_API_MODULE_ID_MTD_FS,
    NORFLASH_API_MODULE_ID_WIFI,
#endif
    NORFLASH_API_MODULE_ID_MAX = 24,
    // 24 ~ 32 is used for app alloc
    NORFLASH_API_MODULE_ID_COUNT = 32,
};

enum NORFLASH_API_RET_T
{
    NORFLASH_API_OK,
    NORFLASH_API_BUFFER_FULL,
    NORFLASH_API_BAD_DEV_ID,
    NORFLASH_API_BAD_MOD_ID,
    NORFLASH_API_BAD_BUFF_LEN,
    NORFLASH_API_BAD_ADDR,
    NORFLASH_API_BAD_LEN,
    NORFLASH_API_ERR_UNINIT,
    NORFLASH_API_ERR_HASINIT,
    NORFLASH_API_ERR_REMAP,
    NORFLASH_API_ERR_PROTECTION,
    NORFLASH_API_ERR_REGISTRATION,
    NORFLASH_API_ERR,
};

enum NORFLASH_API_OPRATION_TYPE
{
    NORFLASH_API_WRITTING = 0x01,
    NORFLASH_API_ERASING = 0x02,
    NORFLASH_API_ALL    = 0x03
};

typedef struct
{
    enum NORFLASH_API_OPRATION_TYPE type;
    uint32_t addr;
    uint32_t len;
    uint32_t remain_num;
    enum NORFLASH_API_RET_T result;
    uint32_t suspend_num;
}NORFLASH_API_OPERA_RESULT;

enum NORFLASH_API_STATE
{
    NORFLASH_API_STATE_UNINITED,
    NORFLASH_API_STATE_IDLE,
    NORFLASH_API_STATE_WRITTING,
    NORFLASH_API_STATE_WRITTING_SUSPEND,
    NORFLASH_API_STATE_WRITTING_RESUME,
    NORFLASH_API_STATE_ERASE,
    NORFLASH_API_STATE_ERASE_SUSPEND,
    NORFLASH_API_STATE_ERASE_RESUME,
};

enum NORFLASH_API_USER
{
    NORFLASH_API_USER_CP,
    NORFLASH_API_USER_TRUST_ZONE,
    NORFLASH_API_USER_COUNTS,
};

typedef void (* NORFLASH_API_OPERA_CB)(void* opera_result);
typedef bool (*NOFLASH_API_FLUSH_ALLOWED_CB)(void);
typedef int (*NORFLASH_API_HOOK_HANDLE)(void);

#define FLASH_BP_MAP_LEN_MAX   64

typedef struct{
    uint32_t bp;
    uint32_t start_addr;
    uint32_t end_addr;
}FLASH_BP_MAP_T;


enum FLASH_BP_PORTION_T{
    FLASH_PORTION_UPPER,
    FLASH_PORTION_LOWER,
    FLASH_PORTION_TOP,
    FLASH_PORTION_BOTTOM,
};

typedef struct{
    uint32_t bp;
    enum FLASH_BP_PORTION_T portion;
    uint32_t numerator;
    uint32_t denominator;
}FLASH_BP_MAP_T1;

enum NORFLASH_API_HOOK_USER_T
{
    NORFLASH_API_HOOK_USER_0,
    NORFLASH_API_HOOK_USER_USERDATA,
#if defined(DUMP_LOG_ENABLE)
    NORFLASH_API_HOOK_USER_LOG_DUMP,
#endif
    NORFLASH_API_HOOK_USER_WIFI,
    NORFLASH_API_HOOK_USER_QTY,
};

enum NORFLASH_API_RET_T norflash_api_init(void);

int norflash_api_alloc_mod(void);

enum NORFLASH_API_RET_T norflash_api_register(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                enum HAL_FLASH_ID_T dev_id,
                uint32_t mod_base_addr,
                uint32_t mod_len,
                uint32_t mod_block_len,
                uint32_t mod_sector_len,
                uint32_t mod_page_len,
                uint32_t buffer_len,
                NORFLASH_API_OPERA_CB cb_func
                );
enum HAL_FLASH_ID_T norflash_api_get_dev_id_by_addr(
                uint32_t addr
                );
// read flash buffer or flash,priority read flash buffer.
enum NORFLASH_API_RET_T norflash_api_read(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                uint8_t *buffer,
                uint32_t len
                );

// read flash.
enum NORFLASH_API_RET_T norflash_sync_read(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                uint8_t *buffer,
                uint32_t len
                );

enum NORFLASH_API_RET_T norflash_api_erase(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                uint32_t len,
                bool async
                );

enum NORFLASH_API_RET_T norflash_api_write(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                const uint8_t *buffer,
                uint32_t len,
                bool async
                );

int norflash_api_flush(void);

bool norflash_api_buffer_is_free(
                enum NORFLASH_API_MODULE_ID_T mod_id);

uint32_t norflash_api_get_used_buffer_count(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                enum NORFLASH_API_OPRATION_TYPE type
                );

uint32_t norflash_api_get_free_buffer_count(
                enum HAL_FLASH_ID_T dev_id,
                enum NORFLASH_API_OPRATION_TYPE type
                );
void norflash_api_flush_all(bool force);
void norflash_api_flush_disable(enum NORFLASH_API_USER user_id,
                                uint32_t cb,
                                bool wait_idle_status);
void norflash_api_flush_enable(enum NORFLASH_API_USER user_id);
void norflash_api_flush_enable_all(void);
enum NORFLASH_API_STATE norflash_api_get_state(enum NORFLASH_API_MODULE_ID_T mod_id);

enum NORFLASH_API_RET_T norflash_api_get_base_addr(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t *addr);
enum NORFLASH_API_RET_T norflash_api_get_dev_id(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                enum HAL_FLASH_ID_T *dev_id);

uint32_t norflash_api_get_total_size(enum HAL_FLASH_ID_T dev_id);
uint32_t norflash_api_get_block_size(enum HAL_FLASH_ID_T dev_id);
uint32_t norflash_api_get_sector_size(enum HAL_FLASH_ID_T dev_id);
uint32_t norflash_api_get_page_size(enum HAL_FLASH_ID_T dev_id);
bool norflash_api_dev_opened(enum HAL_FLASH_ID_T dev_id);

enum NORFLASH_API_RET_T norflash_api_remap_config(
                          enum HAL_FLASH_ID_T dev_id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id,
                          uint32_t addr,
                          uint32_t len,
                          uint32_t remap_offset);
enum NORFLASH_API_RET_T norflash_api_remap_enable(
                          enum HAL_FLASH_ID_T dev_id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id);
enum NORFLASH_API_RET_T norflash_api_remap_disable(
                          enum HAL_FLASH_ID_T dev_id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id);
enum NORFLASH_API_RET_T norflash_api_remap_pre(
                         enum HAL_FLASH_ID_T dev_id,
                         uint32_t start_addr,
                         uint32_t len);
enum NORFLASH_API_RET_T norflash_api_remap_post(enum HAL_FLASH_ID_T dev_id);
enum NORFLASH_API_RET_T norflash_api_get_remap_config(
                         enum HAL_FLASH_ID_T dev_id,
                         enum HAL_NORFLASH_REMAP_ID_T remap_id,
                         uint8_t *is_used,
                         uint32_t *addr,
                         uint32_t *len,
                         uint32_t *offset);
void norflash_api_protection_bp_init(
                         enum HAL_FLASH_ID_T dev_id,
                         uint32_t total_size);
void norflash_api_get_pb_map(
                         enum HAL_FLASH_ID_T dev_id,
                         FLASH_BP_MAP_T **pb_map,
                         uint32_t       *pb_count);
uint32_t norflash_api_to_nc_addr(enum HAL_FLASH_ID_T dev_id,
                                        uint32_t addr);
bool norflash_api_flush_is_in_task(void);
enum NORFLASH_API_RET_T norflash_api_get_dual_chip_mode(enum NORFLASH_API_MODULE_ID_T mod_id, int *dual_flash, int *dual_sec_reg);
enum NORFLASH_API_RET_T norflash_api_set_dual_chip_mode(enum NORFLASH_API_MODULE_ID_T mod_id, int dual_flash, int dual_sec_reg);

void norflash_api_flash_operation_start(uint32_t addr);
void norflash_api_flash_operation_end(uint32_t addr);

enum NORFLASH_API_RET_T norflash_api_security_register_lock(enum HAL_FLASH_ID_T dev_id, uint32_t start_address, uint32_t len);
enum NORFLASH_API_RET_T norflash_api_security_register_erase(enum HAL_FLASH_ID_T dev_id, uint32_t start_address, uint32_t len);
enum NORFLASH_API_RET_T norflash_api_security_register_write(enum HAL_FLASH_ID_T dev_id, uint32_t start_address, const uint8_t *buffer, uint32_t len);
enum NORFLASH_API_RET_T norflash_api_security_register_read(enum HAL_FLASH_ID_T dev_id, uint32_t start_address, uint8_t *buffer, uint32_t len);

void norflash_api_set_hook(enum NORFLASH_API_HOOK_USER_T user_id, NORFLASH_API_HOOK_HANDLE hook_handle);
int norflash_api_hook_activate(void);


#ifdef __cplusplus
}
#endif

#endif //_NORFLASH_ASYNC_API_H_
