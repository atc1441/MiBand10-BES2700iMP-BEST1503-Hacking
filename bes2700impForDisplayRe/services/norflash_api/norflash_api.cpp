#include "stdio.h"
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#else
#include "hal_timer.h"
#endif
#include "pmu.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "hal_cache.h"
#include "hal_dma.h"
#if 0
#define NORFLASH_API_TRACE TRACE
#else
#define NORFLASH_API_TRACE(level,...)
#endif

#if defined(FLASH_API_SIMPLE)
#define NORFLASH_API_SYS_BUFFER_NUM         (0)
#endif
#if defined(FLASH_API_HIGHPERFORMANCE)
#define NORFLASH_API_SYS_BUFFER_NUM         (4)
#endif
#if defined(FLASH_API_NORMAL)
#define NORFLASH_API_SYS_BUFFER_NUM         (1)
#endif
#define NORFLASH_API_SYS_OPRA_NUM           ((NORFLASH_API_SYS_BUFFER_NUM + 1) * 3)

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
#define NORFLASH_API_DATA_BUFFER_NUM        (2)
#define NORFLASH_API_DATA_OPRA_NUM          ((NORFLASH_API_DATA_BUFFER_NUM + 1) * 3)
#endif

#define NORFLASH_API_REMAP_FLASH_ID         HAL_FLASH_ID_0
#define NORFLASH_API_NORMAL_REMAP_ID        HAL_NORFLASH_REMAP_ID_0
#define IS_ALIGN(v,size)                    (((v/size)*size) == v)
#define IN_ONE_SECTOR(dev_id,addr,len,sec_size) \
                                            (((addr & (flash_total_size[dev_id]-1)) / sec_size) ==\
                                             (((addr & (flash_total_size[dev_id]-1)) + len-1) / sec_size))

#define ADDR_IS_VALID(dev_id,mod_base_addr,mod_len,start_addr,len) \
           ((start_addr & (flash_total_size[dev_id]-1)) >= (mod_base_addr & (flash_total_size[dev_id]-1)) && \
            (start_addr & (flash_total_size[dev_id]-1)) + len <= (mod_base_addr & (flash_total_size[dev_id]-1)) + mod_len)

typedef struct _opera_info
{
    enum NORFLASH_API_OPRATION_TYPE type;
    uint32_t addr;
    uint32_t len;
    uint32_t w_offs;
    uint32_t w_len;
    uint8_t *buff;
    bool lock;
    struct _opera_info *next;
}OPRA_INFO;

typedef struct
{
    bool is_used;
    bool is_registered;
    enum HAL_FLASH_ID_T dev_id;
    enum NORFLASH_API_MODULE_ID_T mod_id;
    uint32_t mod_base_addr;
    uint32_t mod_len;
    uint32_t mod_block_len;
    uint32_t mod_sector_len;
    uint32_t mod_page_len;
    uint32_t buff_len;
    NORFLASH_API_OPERA_CB cb_func;
    OPRA_INFO *opera_info;
    OPRA_INFO *cur_opera_info;
    enum NORFLASH_API_STATE state;
}MODULE_INFO;

typedef struct
{
    bool is_inited;
    MODULE_INFO mod_info[NORFLASH_API_MODULE_ID_COUNT];
    enum NORFLASH_API_MODULE_ID_T cur_mod_id;
    MODULE_INFO* cur_mod;
    NOFLASH_API_FLUSH_ALLOWED_CB allowed_cb[NORFLASH_API_USER_COUNTS];
}NORFLASH_API_INFO;

typedef struct
{
    bool is_used;
    OPRA_INFO opera_info;
}NORFLASH_OPERA;

typedef struct
{
    bool is_used;
    uint8_t buffer[NORFLASH_API_SECTOR_MAX_SIZE];
}NORFLASH_BUFFER;

typedef struct
{
    NORFLASH_API_HOOK_HANDLE hook_handle;
}NORFLASH_API_HOOK_T;

typedef struct
{
    bool is_used;
    uint32_t addr;
    uint32_t len;
    uint32_t remap_offset;
} FLASH_REMAP_CFG_T;

static NORFLASH_API_INFO norflash_api_info = {false,};
static NORFLASH_API_HOOK_T _norflash_api_hook[NORFLASH_API_HOOK_USER_QTY];

static NORFLASH_OPERA _sys_opera_list[NORFLASH_API_SYS_OPRA_NUM];
static NORFLASH_BUFFER _sys_buffer_list[NORFLASH_API_SYS_BUFFER_NUM];
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
static NORFLASH_OPERA _data_opera_list[NORFLASH_API_DATA_OPRA_NUM];
static NORFLASH_BUFFER _data_buffer_list[NORFLASH_API_DATA_BUFFER_NUM];
#endif

#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
static const osMutexAttr_t MutexAttr_norflash = {
    .name = "NORFLASH_API_MUTEX",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = NULL,
    .cb_size = 0U,
};
static osMutexId_t norflash_api_mutex_id = NULL;
#endif
static uint32_t _norflash_api_lock = 0;
static int suspend_number = 0;

static uint32_t flash_total_size[HAL_FLASH_ID_NUM];
static uint32_t flash_block_size[HAL_FLASH_ID_NUM];
static uint32_t flash_sector_size[HAL_FLASH_ID_NUM];
static uint32_t flash_page_size[HAL_FLASH_ID_NUM];
static uint8_t flash_dev_opened[HAL_FLASH_ID_NUM];
static int flash_chip_dual_enable[HAL_FLASH_ID_NUM];
#ifdef FLASH_REMAP
#define REMAP_ADDR_CHECK_IN                 1
#define REMAP_ADDR_CHECK_OUT                2
#define REMAP_ADDR_CHECK_INVALID            3
static FLASH_REMAP_CFG_T flash_remap_cfg[HAL_NORFLASH_REMAP_ID_QTY];
static bool _remap_status;
static enum HAL_NORFLASH_REMAP_ID_T _remap_id;
#endif

static int _norflash_api_exec_flush_hook(void);

static void* norflash_memset(void* s, uint8_t c, size_t n)
{
	uint32_t i;
	char *ss = (char*)s;

	for (i=0;i<n;i++) ss[i] = c;
	return s;
}

static void* norflash_memcpy(void* __dest, __const void* __src,
			    size_t __n)
{
	uint32_t i;
	char *d = (char *)__dest, *s = (char *)__src;

	for (i=0;i<__n;i++) d[i] = s[i];
	return __dest;
}

#ifdef FLASH_REMAP
//when remap enable, must keep function can't locate in the flash
#define PMU_FLASH_WRITE_CONFIG_FUNC()
#define PMU_FLASH_READ_CONFIG_FUNC()
#else
#define PMU_FLASH_WRITE_CONFIG_FUNC()       pmu_flash_write_config()
#define PMU_FLASH_READ_CONFIG_FUNC()        pmu_flash_read_config()
#endif

static void _cache_invalid(enum HAL_FLASH_ID_T dev_id, uint32_t start_addr, uint32_t len)
{
    bool is_cache_addr = false;
    uint32_t mark = ~(flash_total_size[dev_id]-1);

    //TRACE(1, "%s: dev_id = %d, start_addr = 0x%x, mark = 0x%x", __func__, dev_id, start_addr, mark);
    if(dev_id == HAL_FLASH_ID_0)
    {
        if((mark & start_addr) == (mark & FLASH_BASE))
        {
            //TRACE(1, "%s: is cache address, start_addr = 0x%x, len = 0x%x", __func__, start_addr, len);
            is_cache_addr = true;
        }
    }
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_1)
    {
        if((mark & start_addr) == (mark & FLASH1_BASE))
        {
            //TRACE(1, "%s: is cache address, start_addr = 0x%x, len = 0x%x", __func__, start_addr, len);
            is_cache_addr = true;
        }
    }
#endif
#if defined(FLASH2_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_2)
    {
        if((mark & start_addr) == (mark & FLASH2_BASE))
        {
            //TRACE(1, "%s: is cache address, start_addr = 0x%x, len = 0x%x", __func__, start_addr, len);
            is_cache_addr = true;
        }
    }
#endif
    else
    {
        ASSERT(0, "%s: ERROR ID: %d", __func__, dev_id);
    }

    if(is_cache_addr)
    {
        hal_cache_invalidate(HAL_CACHE_ID_FLASH_DATA, ADDR_ALIGN_TO_CACHE(start_addr), LEN_ALIGN_TO_CACHE(len, start_addr));
    }
}

static void* _norflash_api_malloc(enum HAL_FLASH_ID_T dev_id, uint32_t size)
{
    uint32_t i;
    NORFLASH_OPERA *opera;
    NORFLASH_BUFFER *buff;
    uint32_t opera_num;
    uint32_t buff_num;

    if(dev_id == HAL_FLASH_ID_0)
    {
        opera = (NORFLASH_OPERA*)_sys_opera_list;
        buff = (NORFLASH_BUFFER*)_sys_buffer_list;
        opera_num = ARRAY_SIZE(_sys_opera_list);
        buff_num = ARRAY_SIZE(_sys_buffer_list);
    }
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_1)
    {
        opera = (NORFLASH_OPERA*)_data_opera_list;
        buff = (NORFLASH_BUFFER*)_data_buffer_list;
        opera_num = ARRAY_SIZE(_data_opera_list);
        buff_num = ARRAY_SIZE(_data_buffer_list);
    }
#endif
#if defined(FLASH2_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_2)
    {
        opera = (NORFLASH_OPERA*)_data_opera_list;
        buff = (NORFLASH_BUFFER*)_data_buffer_list;
        opera_num = ARRAY_SIZE(_data_opera_list);
        buff_num = ARRAY_SIZE(_data_buffer_list);
    }
#endif
    else
    {
        ASSERT(0, "%s: dev_id(%d) ERROR!", __func__, dev_id);
    }

    if(size == sizeof(OPRA_INFO))
    {
        for(i = 0; i < opera_num; i++)
        {
            if(opera[i].is_used == false)
            {
                opera[i].is_used = true;
                return (void*)&opera[i].opera_info;
            }
        }
        return NULL;
    }
    else if(size == flash_sector_size[dev_id])
    {
        for(i = 0; i < buff_num; i++)
        {
            if(buff[i].is_used == false)
            {
                buff[i].is_used = true;
                return (void*)buff[i].buffer;
            }
        }
        return NULL;
    }
    else
    {
        ASSERT(0,"%s: size(0x%x) error!", __func__, size);
    }
}

static void _norflash_api_free(enum HAL_FLASH_ID_T dev_id, void *p)
{
    uint32_t i;
    NORFLASH_OPERA *opera;
    NORFLASH_BUFFER *buff;
    uint32_t opera_num;
    uint32_t buff_num;

    if(dev_id == HAL_FLASH_ID_0)
    {
        opera = (NORFLASH_OPERA*)_sys_opera_list;
        buff = (NORFLASH_BUFFER*)_sys_buffer_list;
        opera_num = ARRAY_SIZE(_sys_opera_list);
        buff_num = ARRAY_SIZE(_sys_buffer_list);
    }
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_1)
    {
        opera = (NORFLASH_OPERA*)_data_opera_list;
        buff = (NORFLASH_BUFFER*)_data_buffer_list;
        opera_num = ARRAY_SIZE(_data_opera_list);
        buff_num = ARRAY_SIZE(_data_buffer_list);
    }
#endif
#if defined(FLASH2_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_2)
    {
        opera = (NORFLASH_OPERA*)_data_opera_list;
        buff = (NORFLASH_BUFFER*)_data_buffer_list;
        opera_num = ARRAY_SIZE(_data_opera_list);
        buff_num = ARRAY_SIZE(_data_buffer_list);
    }
#endif
    else
    {
        ASSERT(0, "%s: dev_id(%d) ERROR!", __func__, dev_id);
    }

    for(i = 0; i < opera_num; i++)
    {
        if((uint8_t*)&opera[i].opera_info == p)
        {
            opera[i].is_used = false;
            return;
        }
    }

    for(i = 0; i < buff_num; i++)
    {
        if(buff[i].buffer == p)
        {
            buff[i].is_used = false;
            return;
        }
    }

    ASSERT(0,"%s: p(%p) error!", __func__, p);
}

static uint32_t _norflash_api_opera_num(enum HAL_FLASH_ID_T dev_id)
{
    uint32_t opera_num = 0;

    if(dev_id == HAL_FLASH_ID_0)
    {
        opera_num = ARRAY_SIZE(_sys_opera_list);
    }
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_1)
    {
        opera_num = ARRAY_SIZE(_data_opera_list);
    }
#endif
    else
    {
        ASSERT(0, "%s: dev_id(%d) error!", __func__, dev_id);
    }

    return opera_num;
}

static uint32_t _norflash_api_buffer_num(enum HAL_FLASH_ID_T dev_id)
{
    uint32_t buff_num = 0;
    if(dev_id == HAL_FLASH_ID_0)
    {
        buff_num = ARRAY_SIZE(_sys_buffer_list);
    }
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_1)
    {
        buff_num = ARRAY_SIZE(_data_buffer_list);
    }
#endif
    else
    {
        ASSERT(0, "%s: dev_id(%d) error!", __func__, dev_id);
    }

    return buff_num;
}

static inline void _norflash_api_mutex_init(void)
{
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    if(!norflash_api_mutex_id)
    {
        norflash_api_mutex_id = osMutexNew(&MutexAttr_norflash);
        ASSERT(norflash_api_mutex_id, "cannot create norflash api mutex");
    }
#endif
}

static inline void _norflash_api_mutex_wait(void)
{
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    if(norflash_api_mutex_id && !in_int_locked())
    {
        osMutexAcquire(norflash_api_mutex_id, osWaitForever);
    }
#endif
}

static inline void _norflash_api_mutex_release(void)
{
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    if(norflash_api_mutex_id && !in_int_locked())
    {
        osMutexRelease(norflash_api_mutex_id);
    }
#endif
}

static inline void _norflash_api_int_lock(enum HAL_FLASH_ID_T dev_id)
{
    if(dev_id == HAL_FLASH_ID_0)
    {
        _norflash_api_lock = int_lock_global();
    }
}

static inline void _norflash_api_int_unlock(enum HAL_FLASH_ID_T dev_id)
{
    if(dev_id == HAL_FLASH_ID_0)
    {
        int_unlock_global(_norflash_api_lock);
    }
}

static MODULE_INFO* _get_module_info(enum NORFLASH_API_MODULE_ID_T mod_id)
{
    return &norflash_api_info.mod_info[mod_id];
}

static OPRA_INFO* _get_tail(MODULE_INFO *mod_info,bool is_remove)
{
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *pre_node = NULL;
    OPRA_INFO *tmp;

    pre_node = mod_info->opera_info;
    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        tmp = opera_node->next;
        if(tmp)
        {
            pre_node = opera_node;
        }
    }
    if(is_remove)
    {
        if(pre_node)
        {
            pre_node->next = NULL;
        }
    }
    if(opera_node)
    {
        opera_node->lock = true;
    }
    return opera_node;
}

static void _opera_del(MODULE_INFO *mod_info,OPRA_INFO *node)
{
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *pre_node = NULL;
    OPRA_INFO *tmp;

    pre_node = mod_info->opera_info;
    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        if(opera_node == node)
        {
            if(mod_info->opera_info == opera_node)
            {
                mod_info->opera_info = NULL;
            }
            else
            {
                pre_node->next = NULL;
            }
            if(node->buff)
            {
                _norflash_api_free(mod_info->dev_id, node->buff);
            }
            _norflash_api_free(mod_info->dev_id, node);
            break;
        }
        tmp = opera_node->next;
        if(tmp)
        {
            pre_node = opera_node;
        }
    }
}

static uint32_t _get_ew_count(MODULE_INFO *mod_info)
{
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *tmp;
    uint32_t count = 0;

    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        count ++;
        tmp = opera_node->next;
    }
    return count;
}

static uint32_t _get_w_count(MODULE_INFO *mod_info)
{
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *tmp;
    uint32_t count = 0;

    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        if(opera_node->type == NORFLASH_API_WRITTING)
        {
            count ++;
        }
        tmp = opera_node->next;
    }
    return count;
}

static uint32_t _get_e_count(MODULE_INFO *mod_info)
{
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *tmp;
    uint32_t count = 0;

    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        if(opera_node->type == NORFLASH_API_ERASING)
        {
            count ++;
        }
        tmp = opera_node->next;
    }
    return count;
}

static MODULE_INFO* _get_cur_mod(void)
{
    uint32_t i;
    MODULE_INFO *mod_info;
    uint32_t tmp_mod_id = NORFLASH_API_MODULE_ID_COUNT;

    if(norflash_api_info.cur_mod)
    {
        return norflash_api_info.cur_mod;
    }

    tmp_mod_id = norflash_api_info.cur_mod_id;
    for(i = 0; i < NORFLASH_API_MODULE_ID_COUNT; i++)
    {
        tmp_mod_id =  tmp_mod_id + 1 >= NORFLASH_API_MODULE_ID_COUNT ? 0 : tmp_mod_id + 1;
        mod_info = _get_module_info((enum NORFLASH_API_MODULE_ID_T)tmp_mod_id);
        if(mod_info->is_registered)
        {
            if(_get_ew_count(mod_info) > 0)
            {
                return mod_info;
            }
        }
    }
    return NULL;
}

static enum NORFLASH_API_MODULE_ID_T _get_mod_id(MODULE_INFO *mod_info)
{
    uint32_t i;
    enum NORFLASH_API_MODULE_ID_T mod_id = NORFLASH_API_MODULE_ID_COUNT;
    MODULE_INFO *tmp_mod_info;

    for(i = 0; i < NORFLASH_API_MODULE_ID_COUNT; i++)
    {
        tmp_mod_info = _get_module_info((enum NORFLASH_API_MODULE_ID_T)i);
        if(tmp_mod_info == mod_info)
        {
            mod_id = (enum NORFLASH_API_MODULE_ID_T)i;
            break;
        }
    }
    return mod_id;
}

#ifdef FLASH_REMAP
static int _flash_remap_addr_check(enum HAL_FLASH_ID_T dev_id, uint32_t addr, uint32_t len, enum HAL_NORFLASH_REMAP_ID_T *premap_id)
{
    uint32_t remap_id;
    uint32_t remap_addr;
    uint32_t remap_len;
    uint32_t remap_offset;
    uint32_t start_addr;
    int ret = REMAP_ADDR_CHECK_OUT;

    start_addr = addr & (flash_total_size[dev_id]-1);

    for(remap_id = HAL_NORFLASH_REMAP_ID_0; remap_id < HAL_NORFLASH_REMAP_ID_QTY; remap_id ++)
    {
        if(!flash_remap_cfg[remap_id].is_used)
        {
            continue;
        }

        remap_addr = flash_remap_cfg[remap_id].addr;
        remap_len = flash_remap_cfg[remap_id].len;
        remap_offset = flash_remap_cfg[remap_id].remap_offset;

        if(start_addr >= remap_addr && start_addr + len <= remap_addr + remap_len)
        {
            *premap_id = (HAL_NORFLASH_REMAP_ID_T)remap_id;
            ret = REMAP_ADDR_CHECK_IN;
            break;
        }

        if((start_addr < remap_addr && start_addr + len > remap_addr)   ||
           (start_addr < remap_addr + remap_len && start_addr + len > remap_addr + remap_len) ||
           ((start_addr  < remap_addr + remap_offset + remap_len) && (start_addr + len > remap_addr + remap_offset)))
        {
            ret = REMAP_ADDR_CHECK_INVALID;
            break;
        }
    }

    return ret;
}

static void _flash_remap_pre(enum HAL_FLASH_ID_T dev_id, uint32_t addr, uint32_t len)
{
    enum HAL_NORFLASH_RET_T ret;
    int check_result;

    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return;
    }

    _remap_id = HAL_NORFLASH_REMAP_ID_QTY;
    check_result = _flash_remap_addr_check(dev_id, addr, len, &_remap_id);
    if(check_result == REMAP_ADDR_CHECK_IN)
    {
        NORFLASH_API_TRACE(3,"%s: INSIDE, _remap_id = %d.",__func__, _remap_id);
        _remap_status = hal_norflash_get_remap_status(dev_id, _remap_id);
        if(!_remap_status)
        {
            NORFLASH_API_TRACE(3,"%s: remap_status not enabled.",__func__);
            ret = hal_norflash_enable_remap(dev_id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable remap, ret = %d",
                __func__, ret);
        }
        else
        {
            NORFLASH_API_TRACE(3,"%s: remap_status enabled.",__func__);
            ret = hal_norflash_disable_remap(dev_id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to disable remap, ret = %d",
                __func__, ret);
        }
        _remap_status = !_remap_status;
        return;
    }
    else if(check_result == REMAP_ADDR_CHECK_OUT)
    {
        _remap_id = HAL_NORFLASH_REMAP_ID_QTY;
        NORFLASH_API_TRACE(3,"%s: OUTSIDE.",__func__);
    }
    else
    {
        ASSERT(0,"%s: Address ranges bad!addr = 0x%x, len=0x%x",
            __func__, addr, len);
    }
}

static void _flash_remap_post(enum HAL_FLASH_ID_T dev_id)
{
    enum HAL_NORFLASH_RET_T ret;

    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return;
    }
    NORFLASH_API_TRACE(3,"%s: _remap_id = %d.", __func__, _remap_id);
    if(_remap_id < HAL_NORFLASH_REMAP_ID_QTY)
    {
        if(!_remap_status)
        {
            NORFLASH_API_TRACE(3,"%s: remap_status not enabled.", __func__);
            ret = hal_norflash_enable_remap(dev_id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable remap, ret = %d",
                __func__, ret);
        }
        else
        {
            NORFLASH_API_TRACE(3,"%s: remap_status enabled.",__func__);
            ret = hal_norflash_disable_remap(dev_id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to disable remap, ret = %d",
                __func__, ret);
        }
        _remap_status = !_remap_status;
        return;
    }
    _remap_id = HAL_NORFLASH_REMAP_ID_QTY;
}

static int _flash_remap_config(HAL_FLASH_ID_T dev_id, enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t remap_offset)
{
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_OK;

    NORFLASH_API_TRACE(3,"%s: remap_id = %d, addr = 0x%x, len = 0x%x, remap_offset = 0x%x.",
                       __func__, remap_id, addr, len, remap_offset);
    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    flash_remap_cfg[remap_id].addr = addr & (flash_total_size[dev_id]-1);
    flash_remap_cfg[remap_id].len = len;
    flash_remap_cfg[remap_id].remap_offset= remap_offset;
    flash_remap_cfg[remap_id].is_used = true;

    if(!hal_norflash_get_remap_status(dev_id, remap_id))
    {
        NORFLASH_API_TRACE(3,"%s: remap status is disable.",__func__);
        ret = hal_norflash_config_remap(dev_id, remap_id, addr, len, remap_offset);
        if(ret)
        {
            NORFLASH_API_TRACE(0, "%s: Failed to config remap(%d, 0x%x,0x%x,0x%x) total_size = 0x%x, ret = %d",
                              __func__, remap_id, addr, len, remap_offset,
                              hal_norflash_get_flash_total_size(dev_id),
                              ret);
        }
        else
        {
            NORFLASH_API_TRACE(3,"%s: remap config done", __func__);
        }
    }
    else
    {
        NORFLASH_API_TRACE(3,"%s: remap status is enable", __func__);
    }

    return (int)ret;
}

POSSIBLY_UNUSED
static uint32_t _flash_remap_get_real_addr(enum HAL_FLASH_ID_T dev_id, uint32_t addr, uint32_t len)
{
    enum HAL_NORFLASH_REMAP_ID_T remap_id = HAL_NORFLASH_REMAP_ID_0;
    int check_result;

    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return addr;
    }
    check_result = _flash_remap_addr_check(dev_id, addr, len, &remap_id);
    if(check_result == REMAP_ADDR_CHECK_IN)
    {
        NORFLASH_API_TRACE(3,"%s: INSIDE, remap_id = %d.",__func__, remap_id);
        if(!hal_norflash_get_remap_status(dev_id, remap_id))
        {
            NORFLASH_API_TRACE(3,"%s: remap_status not enabled.",__func__);
            return addr + flash_remap_cfg[remap_id].remap_offset;
        }
        else
        {
            NORFLASH_API_TRACE(3,"%s: remap_status enabled.",__func__);
            return addr;
        }

    }
    else if(check_result == REMAP_ADDR_CHECK_OUT)
    {
        NORFLASH_API_TRACE(3,"%s: OUTSIDE, remap_id = %d.",__func__, remap_id);
        return addr;
    }
    else
    {
        ASSERT(0,"%s: Address ranges bad!addr = 0x%x, len=0x%x",
            __func__, addr, len);
    }
    return addr;
}

#define FLASH_REMAP_PRE                     _flash_remap_pre
#define FLASH_REMAP_POST                    _flash_remap_post
#define FLASH_REMAP_CONFIG                  _flash_remap_config
#define FLASH_REMAP_GET_REAL_ADDR           _flash_remap_get_real_addr
#else // !FLASH_REMAP
#define FLASH_REMAP_PRE(...)
#define FLASH_REMAP_POST(...)
#define FLASH_REMAP_CONFIG(...)
#endif // FLASH_REMAP

#ifdef FLASH_PROTECTION

#define FLASH_BP_NONE                       0x0000
#define FLASH_BP_ALL                        0x007C

#define FLASH_1KBYTE                        1024

static FLASH_BP_MAP_T flash_bp_map[FLASH_BP_MAP_LEN_MAX];
static uint32_t flash_bp_numer = 0;
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
static FLASH_BP_MAP_T flash1_bp_map[FLASH_BP_MAP_LEN_MAX];
static uint32_t flash1_bp_numer = 0;
#endif

static bool _flash_chip_dual_enable(enum HAL_FLASH_ID_T dev_id)
{
    return flash_chip_dual_enable[dev_id] == 0 ? false : true;
}

static void _bp_init_2m_128m(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    uint32_t prot_size, unprot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    FLASH_BP_MAP_T *bp_map;

    multi = 0x1 << ((total_size/(0x800000)) % 0x2000000);
    if(_flash_chip_dual_enable(dev_id))
    {
        multi = multi >> (total_size/0x2000000);
    }
    // NORFLASH_API_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
#endif
    // bp 0: none.
    bp_map[ind].bp = FLASH_BP_NONE;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = 0;
    ind ++;

    // bp 0: All.
    bp_map[ind].bp = FLASH_BP_ALL;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = total_size-1;
    ind ++;

    // CMP = 0
    // Upper
    i = 0;
    prot_size = 0;
    while(1)
    {
        n = (64<<i);
        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+1)<<2); // max bp = 0x6
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr =  total_size - 1;
        ind ++;
        i ++;
    }

    // Lower
    prot_size = 0;
    i = 0;
    while(1)
    {
        n = (64<<i);
        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+9)<<2); // max bp = 0xe
        bp_map[ind].start_addr = 0x0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

    // Top
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        prot_size = (n*FLASH_1KBYTE);
        if(prot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | (((0x1<<4)|(i+1))<<2); // max bp = 0x14
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    };

    // Botton
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        prot_size = (n*FLASH_1KBYTE);
        if(prot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | (((0x1<<4)|(i+9))<<2); // max bp = 0x1c
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

    // CMP = 1
    // Lower
    unprot_size = 0;
    i = 0;
    while(1)
    {
        n = (64<<i);
        unprot_size = (n*multi*FLASH_1KBYTE);
        if(unprot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | ((i+1)<<2); // max bp = 0x6
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr =  total_size - unprot_size - 1;
        ind ++;
        i ++;
    }

    // Upper
    unprot_size = 0;
    i = 0;
    while(1)
    {
        n = (64<<i);
        unprot_size = (n*multi*FLASH_1KBYTE);
        if(unprot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | ((i+9)<<2); // max bp = 0xe
        bp_map[ind].start_addr = unprot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    }

    // L-
    unprot_size = 0;
    i = 0;

    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        unprot_size = (n*FLASH_1KBYTE);
        if(unprot_size > 32*FLASH_1KBYTE)
        {
           break;
        }
        bp_map[ind].bp = 0x4000 | (((0x1<<4)|(i+1))<<2); // max bp = 0x1c
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr = total_size - unprot_size - 1;
        ind ++;
        i ++;
    }

    // U -
    unprot_size = total_size;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        unprot_size = (n*FLASH_1KBYTE);
        if(unprot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | (((0x1<<4)|(i+9))<<2); // max bp = 0x1c
        bp_map[ind].start_addr = unprot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    }

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    flash_bp_numer = ind;
#endif
}

// GD25LE255E BP without CMP.
static void _bp_init_256m(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    uint32_t prot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    uint32_t base_shift;
    FLASH_BP_MAP_T *bp_map;
    //uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];
    base_shift = 0x2;

    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // NORFLASH_API_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
#endif
    // bp 0: none.
    bp_map[ind].bp = FLASH_BP_NONE;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = 0;
    ind ++;

    // bp 0: All.
    bp_map[ind].bp = FLASH_BP_ALL;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = total_size-1;
    ind ++;

    // CMP = 0
    // Upper
    i = 0;
    prot_size = 0;
    while(1)
    {
       if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }

        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+1)<<2);
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr =  total_size - 1;
        ind ++;
        i ++;
    }

    // Lower
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }

        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+0x11)<<2);
        bp_map[ind].start_addr = 0x0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

    //hal_norflash_get_id(id, flash_id, ARRAY_SIZE(flash_id));
    // GD25LE255 without CMP bit.
    //if(!(flash_id[0] == 0xC8 && flash_id[1] == 0x60 && flash_id[2] == 0x19))

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    flash_bp_numer = ind;
#endif
}

// P25Q256L BP
static void _bp_init_puya256m(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    uint32_t prot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    uint32_t base_shift;
    FLASH_BP_MAP_T *bp_map;
    uint32_t unprot_size;

    base_shift = 0x02;
    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // NORFLASH_API_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
#endif
    // bp 0: none.
    bp_map[ind].bp = FLASH_BP_NONE;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = 0;
    ind ++;

    // bp 0: All.
    bp_map[ind].bp = FLASH_BP_ALL;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = total_size-1;
    ind ++;

    // CMP = 0
    // Upper
    i = 0;
    prot_size = 0;
    while(1)
    {
       if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }

        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+1)<<2);
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr =  total_size - 1;
        ind ++;
        i ++;
    }

    // Lower
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }

        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+0x11)<<2);
        bp_map[ind].start_addr = 0x0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

    // CMP = 1
    // Lower
    unprot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }
        unprot_size = (n*multi*FLASH_1KBYTE);
        if(unprot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | ((i+1)<<2);
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr =  total_size - unprot_size - 1;
        ind ++;
        i ++;
    }

    // Upper
    unprot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }
        unprot_size = (n*multi*FLASH_1KBYTE);
        if(unprot_size*2 > total_size)
        {
            break;
        }

        bp_map[ind].bp = 0x4000 | ((i+0x11)<<2);

        bp_map[ind].start_addr = unprot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    }

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    flash_bp_numer = ind;
#endif
}

// GD25LQ255E BP with CMP
static void _bp_init_gd256m_with_cmp(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    uint32_t prot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    uint32_t base_shift;
    FLASH_BP_MAP_T *bp_map;
    uint32_t unprot_size;

    base_shift = 0x10;
    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // NORFLASH_API_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
#endif
    // bp 0: none.
    bp_map[ind].bp = FLASH_BP_NONE;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = 0;
    ind ++;

    // bp 0: All.
    bp_map[ind].bp = FLASH_BP_ALL;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = total_size-1;
    ind ++;

    // CMP = 0
    // Upper
    i = 0;
    prot_size = 0;
    while(1)
    {
       if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }

        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+1)<<2);
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr =  total_size - 1;
        ind ++;
        i ++;
    }

    // Lower
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }

        prot_size = (n*multi*FLASH_1KBYTE);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+9)<<2);
        bp_map[ind].start_addr = 0x0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

    // Top
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        prot_size = (n*FLASH_1KBYTE);
        if(prot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | (((0x1<<4)|(i+1))<<2);
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    };

    // Botton
    prot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        prot_size = (n*FLASH_1KBYTE);
        if(prot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | (((0x1<<4)|(i+9))<<2);
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

    // CMP = 1
    // Lower
    unprot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }
        unprot_size = (n*multi*FLASH_1KBYTE);
        if(unprot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | ((i+1)<<2);
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr =  total_size - unprot_size - 1;
        ind ++;
        i ++;
    }

    // Upper
    unprot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (base_shift<<i);
        }
        else
        {
            n = ((base_shift*2)<<i);
        }
        unprot_size = (n*multi*FLASH_1KBYTE);
        if(unprot_size*2 > total_size)
        {
            break;
        }

        bp_map[ind].bp = 0x4000 | ((i+9)<<2);

        bp_map[ind].start_addr = unprot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    }


    unprot_size = 0;
    i = 0;

    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (8<<i);
        }
        else
        {
            n = (4<<i);
        }

        // L-
        unprot_size = (n*FLASH_1KBYTE);
        if(unprot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | (((0x1<<4)|(i+1))<<2); // max bp = 0x1c
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr = total_size - unprot_size - 1;
        ind ++;
        i ++;
        }

        // U -
        unprot_size = total_size;
        i = 0;
        while(1)
        {
            if(_flash_chip_dual_enable(dev_id))
            {
                n = (8<<i);
            }
            else
            {
                n = (4<<i);
            }

        unprot_size = (n*FLASH_1KBYTE);
        if(unprot_size > 32*FLASH_1KBYTE)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | (((0x1<<4)|(i+9))<<2); // max bp = 0x1c
        bp_map[ind].start_addr = unprot_size;
        bp_map[ind].end_addr = total_size - 1;
        ind ++;
        i ++;
    }

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    flash_bp_numer = ind;
#endif
}


static void _bp_init_gd25d20_40(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    uint32_t unprot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    FLASH_BP_MAP_T *bp_map;

    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // NORFLASH_API_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
#endif
    // bp 0: none.
    bp_map[ind].bp = FLASH_BP_NONE;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = 0;
    ind ++;

    // bp 0: All.
    bp_map[ind].bp = FLASH_BP_ALL;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = total_size-1;
    ind ++;

    unprot_size = 0;
    i = 0;
    while(1)
    {
        if(_flash_chip_dual_enable(dev_id))
        {
            n = (16<<i);
        }
        else
        {
            n = (8<<i);
        }
        unprot_size = (n*multi*1024);
        if(unprot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = ((i+1)<<2);
        bp_map[ind].start_addr = 0;
        bp_map[ind].end_addr = total_size - unprot_size - 1;
        ind ++;
        i ++;
    }


#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    flash_bp_numer = ind;
#endif
}

static void _bp_init_xt25q08b(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    uint32_t prot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    FLASH_BP_MAP_T *bp_map;

    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // NORFLASH_API_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
#endif
    // bp 0: none.
    bp_map[ind].bp = FLASH_BP_NONE;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = 0;
    ind ++;

    // bp 0: All.
    bp_map[ind].bp = FLASH_BP_ALL;
    bp_map[ind].start_addr = 0;
    bp_map[ind].end_addr = total_size-1;
    ind ++;

    // CMP = 0
    // Upper
    i = 0;
    prot_size = 0;
    while(1)
    {
        n = (64<<i);
        prot_size = (n*multi*1024);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x0000 | ((i+1)<<2); // max bp = 0x6
        bp_map[ind].start_addr = total_size - prot_size;
        bp_map[ind].end_addr =  total_size - 1;
        ind ++;
        i ++;
    }

    // CMP = 1
    // Lower
    prot_size = 0;
    i = 0;
    while(1)
    {
        n = (64<<i);
        prot_size = (n*multi*1024);
        if(prot_size*2 > total_size)
        {
            break;
        }
        bp_map[ind].bp = 0x4000 | ((i+1)<<2); // max bp = 0x6
        bp_map[ind].start_addr = 0x0;
        bp_map[ind].end_addr = prot_size - 1;
        ind ++;
        i ++;
    }

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    flash_bp_numer = ind;
#endif
}

void norflash_api_protection_bp_init(enum HAL_FLASH_ID_T dev_id, uint32_t total_size)
{
    bool protection_enable = false;
    uint8_t flash_id[3];
#ifdef FLASH_GD256M_SR_WHIT_CMP
    bool with_cmp = true;
#else
    bool with_cmp = false;
#endif

    if(_flash_chip_dual_enable(dev_id))
    {
        if(total_size >= 0x20000 && total_size <= 0x2000000)
        {
            protection_enable = true;
            _bp_init_2m_128m(dev_id, total_size);
        }
    }
    else
    {
        hal_norflash_get_id(dev_id, flash_id, ARRAY_SIZE(flash_id));
        if(flash_id[0] == 0xC8 && flash_id[1] == 0x40 && (flash_id[2] == 0x12 || flash_id[2] == 0x13))
        {
            protection_enable = true;
            _bp_init_gd25d20_40(dev_id, total_size);
        }
        if(flash_id[0] == 0x0B && flash_id[1] == 0x60 && flash_id[2] == 0x14)
        {
            protection_enable = true;
            _bp_init_xt25q08b(dev_id, total_size);
        }
        else
        {
            if(total_size >= 0x10000 && total_size < 0x2000000)
            {
                protection_enable = true;
                _bp_init_2m_128m(dev_id, total_size);
            }
            else if(total_size == 0x2000000)
            {
                protection_enable = true;
                if(flash_id[0] == 0x85)
                {
                    _bp_init_puya256m(dev_id, total_size);
                }
                else
                {
                    if(with_cmp)
                    {
                        _bp_init_gd256m_with_cmp(dev_id, total_size);
                    }
                    else
                    {
                        _bp_init_256m(dev_id, total_size);
                    }
                }
            }
        }
    }

    ASSERT(protection_enable,"%s: Unkown protection BP defining! total_size = 0x%x", __func__, total_size);

}

#ifdef FLASH_PROTECTION_BOOT_SECTION_FIRST
static bool _with_boot_section(enum HAL_FLASH_ID_T dev_id, uint32_t start_addr, uint32_t end_addr)
{
#ifdef OTA_CODE_OFFSET
    uint32_t boot_start;
    uint32_t boot_end;

    boot_start = 0x0;
    boot_end = OTA_CODE_OFFSET & (flash_total_size[dev_id]-1);

    if(dev_id == HAL_FLASH_ID_0)
    {
        if(start_addr <= boot_start && end_addr >= boot_end)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
#endif
    {
        return true;
    }
}
#endif

static void _protection_part(enum HAL_FLASH_ID_T dev_id, uint32_t start_addr, uint32_t size)
{
    enum HAL_NORFLASH_RET_T result;
    uint32_t i;
    uint32_t bp = FLASH_BP_ALL;
    uint32_t real_addr;
    uint32_t addr;
    uint32_t tmp_size;
    uint32_t max_size = 0;
    uint32_t bp_i = 0;
    FLASH_BP_MAP_T *bp_map;
    uint32_t bp_map_number;

#ifdef FLASH_REMAP
    real_addr = FLASH_REMAP_GET_REAL_ADDR(dev_id, start_addr, size);
    addr = (real_addr & (flash_total_size[dev_id]-1));
#else
    real_addr = start_addr;
    addr = (real_addr & (flash_total_size[dev_id]-1));
#endif

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
        bp_map_number = flash_bp_numer;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
        bp_map_number = flash1_bp_numer;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    bp_map_number = flash_bp_numer;
#endif

    for(i = 0; i < bp_map_number; i++)
    {
        if((addr >= bp_map[i].end_addr
            || addr + size <= bp_map[i].start_addr)
#ifdef FLASH_PROTECTION_BOOT_SECTION_FIRST
            && _with_boot_section(dev_id, bp_map[i].start_addr,bp_map[i].end_addr)
#endif
            )
        {
            tmp_size = bp_map[i].end_addr - bp_map[i].start_addr;
            if(tmp_size > max_size)
            {
                max_size = tmp_size;
                bp = bp_map[i].bp;
                bp_i = i;
                bp_i = bp_i;
            }
        }
    }

    NORFLASH_API_TRACE(2,"%s: w: (0x%x -- 0x%x) bp: 0x%x(0x%x -- 0x%x).",
           __func__,
           addr, addr + size,
           bp,
           bp_map[bp_i].start_addr, bp_map[bp_i].end_addr);
    ASSERT(bp != FLASH_BP_ALL, "%s: protection area undefined! addr:0x%x -- 0x%x",
           __func__, addr, addr + size );

    result = hal_norflash_set_protection(dev_id, bp);
    ASSERT(result == HAL_NORFLASH_OK,
            "%s: set protection fail! ret = %d",
            __func__, result);

}

static void _protection_all(enum HAL_FLASH_ID_T dev_id)
{
    enum HAL_NORFLASH_RET_T result;
    uint32_t bp = FLASH_BP_ALL;

    result = hal_norflash_set_protection(dev_id, bp);
    ASSERT(result == HAL_NORFLASH_OK,
            "%s: set protection fail! ret = %d",
            __func__, result);
}
void norflash_api_get_pb_map(enum HAL_FLASH_ID_T dev_id,
                        FLASH_BP_MAP_T **pb_map,
                        uint32_t       *pb_count)
{

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(dev_id == HAL_FLASH_ID_0)
    {
        *pb_map = (FLASH_BP_MAP_T*)flash_bp_map;
        *pb_count = flash_bp_numer;
    }
    else
    {
        *pb_map = (FLASH_BP_MAP_T*)flash1_bp_map;
        *pb_count = flash1_bp_numer;
    }
#else
    ASSERT(dev_id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, dev_id);
    *pb_map = (FLASH_BP_MAP_T*)flash_bp_map;
    *pb_count = flash_bp_numer;
#endif
}

#else // FLASH_PROTECTION

static void _protection_part(enum HAL_FLASH_ID_T dev_id, uint32_t start_addr, uint32_t size){}
static void _protection_all(enum HAL_FLASH_ID_T dev_id){}

#endif // FLASH_PROTECTION

#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)

static void _norflash_api_thread(void const *argument);
static osThreadId_t _norflash_api_tid = NULL;

static uint64_t os_thread_def_stack [ROUND_UP(4096, 8) / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_norflash = {
    .name = (char *)"norflash_api_thread",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack,
    .stack_size = ROUND_UP(4096, 8),
    .priority = osPriorityLow,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};

static void _norflash_api_thread(void const *argument)
{
    while(1){
        osThreadFlagsWait(CMSIS_V1_SIGNAL_ANY, osFlagsWaitAny, osWaitForever);
        _norflash_api_exec_flush_hook();
    }
}

static int norflash_api_thread_init(void)
{
    if(!_norflash_api_tid)
    {
        _norflash_api_tid = osThreadNew((osThreadFunc_t)_norflash_api_thread, NULL, &ThreadAttr_norflash);
        if(_norflash_api_tid == NULL) {
            TRACE(1,"Failed to Create _flush_thread\n");
            return -2;
        }
    }
    return 0;
}

static void _norflash_api_hook_activate(void)
{
    osThreadFlagsSet(_norflash_api_tid, 0x1);
}

#endif

int norflash_api_hook_activate(void)
{
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    _norflash_api_hook_activate();
#endif
    return 0;
}

void norflash_api_set_hook(enum NORFLASH_API_HOOK_USER_T user_id , NORFLASH_API_HOOK_HANDLE hook_handle)
{
    ASSERT(user_id < NORFLASH_API_HOOK_USER_QTY, "%s: user_id %d invalid.", __func__, user_id);
    _norflash_api_hook[user_id].hook_handle = hook_handle;
}

uint32_t norflash_api_to_nc_addr(enum HAL_FLASH_ID_T dev_id, uint32_t addr)
{
    uint32_t nc_addr = 0;

    if(dev_id == HAL_FLASH_ID_0)
    {
        nc_addr = (((addr) & (flash_total_size[dev_id]-1)) | FLASH_NC_BASE);
    }
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_1)
    {
        nc_addr = (((addr) & (flash_total_size[dev_id]-1)) | FLASH1_NC_BASE);
    }
#endif
#if defined(FLASH2_CTRL_BASE) && defined(USE_MULTI_FLASH)
    else if(dev_id == HAL_FLASH_ID_2)
    {
        nc_addr = (((addr) & (flash_total_size[dev_id]-1)) | FLASH2_NC_BASE);
    }
#endif
    else
    {
        ASSERT(0, "%s: id(%d) invalid!!!", __func__, dev_id);
    }
    return nc_addr;
}

static int32_t _opera_read(MODULE_INFO *mod_info,
               uint32_t addr,
               uint8_t *buff,
               uint32_t len)
{
    OPRA_INFO *opera_node;
    OPRA_INFO *e_node = NULL;
    OPRA_INFO *w_node = NULL;
    OPRA_INFO *tmp;
    uint32_t r_offs;
    uint32_t sec_start;
    uint32_t sec_len;

    sec_len = mod_info->mod_sector_len;
    sec_start = (addr/sec_len)*sec_len;
    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        tmp = opera_node->next;
        if(opera_node->addr == sec_start)
        {
            if(opera_node->type == NORFLASH_API_WRITTING)
            {
                w_node = opera_node;
                break;
            }
            else
            {
                e_node = opera_node;
                break;
            }
        }
    }

    if(w_node)
    {
        r_offs = addr - sec_start;
        norflash_memcpy(buff,w_node->buff + r_offs,len);
    }
    else
    {
        if(e_node)
        {
            norflash_memset(buff,0xff,len);
        }
        else
        {
            FLASH_REMAP_PRE(mod_info->dev_id,addr,len);
#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
            if(mod_info->dev_id > HAL_FLASH_ID_0)
            {
                hal_norflash_read(mod_info->dev_id, addr, buff, len);
            }
            else
#endif
            {
                norflash_memcpy(buff,(uint8_t*)addr,len);
            }
            FLASH_REMAP_POST(mod_info->dev_id);
            /*
            HAL_NORFLASH_RET_T result;
            result = hal_norflash_read(mod_info->dev_id,addr,buff,len);
            if(result != HAL_NORFLASH_OK)
            {
                NORFLASH_API_TRACE(2,"%s: hal_norflash_read failed,result = %d.",
                        __func__,result);
                return result;
            }
            */
        }
    }
    return 0;
}

static int32_t _e_opera_add(MODULE_INFO *mod_info,
               uint32_t addr,
               uint32_t len
               )
{
    OPRA_INFO *cur_node = NULL;
    OPRA_INFO *pre_node = NULL;
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *tmp;
    int32_t ret = 0;

    pre_node = mod_info->opera_info;
    tmp = mod_info->opera_info;
    while(tmp)
    {
        cur_node = tmp;
        tmp = cur_node->next;
        // new_node(addr,len) == cur_node(addr,len)
        if((cur_node->addr == addr) && ((cur_node->addr + cur_node->len) == (addr + len)))
        {
            if(cur_node->lock == false)
            {
                NORFLASH_API_TRACE(3,"%s:new_node merge cur_node. type = %d",__func__, cur_node->type);
                if(cur_node == mod_info->opera_info)
                {
                    mod_info->opera_info = tmp;
                }
                else
                {
                    pre_node->next = tmp;
                }
                if(cur_node->type == NORFLASH_API_WRITTING)
                {
                    if(cur_node->buff)
                    {
                        _norflash_api_free(mod_info->dev_id, cur_node->buff);
                    }
                }
                _norflash_api_free(mod_info->dev_id, cur_node);
            }
            else
            {
                if(cur_node->type == NORFLASH_API_ERASING)
                {
                    NORFLASH_API_TRACE(3,"%s: new_node is being merged by tmp_node.",
                        __func__);
                    ret = 0;
                    goto _func_end;
                }
                else
                {
                    pre_node = cur_node;
                }
            }
        }
        // cur_node(addr,len) in new_node(addr,len)
        else if((cur_node->addr >= addr) && ((cur_node->addr + cur_node->len) <= (addr + len)))
        {
            if(cur_node->lock == false)
            {
                NORFLASH_API_TRACE(3,"%s:new_node merge cur_node",__func__);
                if(cur_node == mod_info->opera_info)
                {
                    mod_info->opera_info = tmp;
                }
                else
                {
                    pre_node->next = tmp;
                }
                if(cur_node->type == NORFLASH_API_WRITTING)
                {
                    if(cur_node->buff)
                    {
                        _norflash_api_free(mod_info->dev_id, cur_node->buff);
                    }
                }
                _norflash_api_free(mod_info->dev_id, cur_node);
            }
            else
            {
                pre_node = cur_node;
            }
        }
        else
        {
            pre_node = cur_node;
        }
    }

    // add new node to header.
    opera_node = (OPRA_INFO*)_norflash_api_malloc(mod_info->dev_id, sizeof(OPRA_INFO));
    if(opera_node == NULL)
    {
        NORFLASH_API_TRACE(3,"%s:%d,_norflash_api_malloc failed! size = %d.",
                __func__,__LINE__,sizeof(OPRA_INFO));
        ret = 1;
        goto _func_end;
    }
    opera_node->type = NORFLASH_API_ERASING;
    opera_node->addr = addr;
    opera_node->len = len;
    opera_node->w_offs = 0;
    opera_node->w_len = 0;
    opera_node->buff = NULL;
    opera_node->lock = false;
    opera_node->next = mod_info->opera_info;
    mod_info->opera_info = opera_node;
    ret = 0;
_func_end:
    return ret;
}

static int32_t _w_opera_add(MODULE_INFO *mod_info,
               uint32_t addr,
               uint32_t len,
               uint8_t *buff)
{
    OPRA_INFO *opera_node = NULL;
    OPRA_INFO *e_node = NULL;
    OPRA_INFO *w_node = NULL;
    OPRA_INFO *tmp;
    uint32_t w_offs;
    uint32_t w_len;
    uint32_t sec_start;
    uint32_t sec_len;
    uint32_t w_end1;
    uint32_t w_end2;
    uint32_t w_start;
    uint32_t w_end;
    uint32_t w_len_new;
    int32_t ret = 0;

    sec_len = mod_info->mod_sector_len;
    sec_start = (addr/sec_len)*sec_len;
    w_offs = addr - sec_start;
    w_len = len;
    tmp = mod_info->opera_info;
    while(tmp)
    {
        opera_node = tmp;
        tmp = opera_node->next;

        if(opera_node->addr == sec_start)
        {
            if(opera_node->type == NORFLASH_API_WRITTING)
            {
                if(!opera_node->lock)
                {
                    // select the first w_node in the list.
                    w_node = opera_node;
                    break;
                }
            }
            else
            {
                e_node = opera_node;
                break;
            }
        }
    }

    if(w_node)
    {
        norflash_memcpy(w_node->buff + w_offs,buff,w_len);
        w_start = w_node->w_offs <= w_offs ? w_node->w_offs:w_offs;
        w_end1 = w_node->w_offs + w_node->w_len;
        w_end2 = w_offs + w_len;
        w_end = w_end1 >= w_end2 ? w_end1 : w_end2;
        w_len_new = w_end - w_start;
        w_node->w_offs = w_start;
        w_node->w_len = w_len_new;
        opera_node = w_node;
        ret = 0;
    }
    else
    {
        opera_node = (OPRA_INFO*)_norflash_api_malloc(mod_info->dev_id, sizeof(OPRA_INFO));
        if(opera_node == NULL)
        {
            NORFLASH_API_TRACE(3,"%s:%d,_norflash_api_malloc failed! size = %d.",
                    __func__,__LINE__,sizeof(OPRA_INFO));
             ret = 1;
             goto _func_end;
        }
        opera_node->type = NORFLASH_API_WRITTING;
        opera_node->addr = sec_start;
        opera_node->len = sec_len;
        opera_node->w_offs = w_offs;
        opera_node->w_len = w_len;
        opera_node->buff = (uint8_t*)_norflash_api_malloc(mod_info->dev_id, opera_node->len);
        if(opera_node->buff == NULL)
        {
            _norflash_api_free(mod_info->dev_id, opera_node);
            NORFLASH_API_TRACE(3,"%s:%d,_norflash_api_malloc failed! size = %d.",
                    __func__,__LINE__,opera_node->len);
             ret = 1;
             goto _func_end;
        }
        if(e_node)
        {
            norflash_memset(opera_node->buff,0xff,opera_node->len);
        }
        else
        {
            FLASH_REMAP_PRE(mod_info->dev_id,opera_node->addr,opera_node->len);
#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
            if(mod_info->dev_id > HAL_FLASH_ID_0)
            {
                hal_norflash_read(mod_info->dev_id, opera_node->addr, opera_node->buff, opera_node->len);
            }
            else
#endif
            {
                norflash_memcpy(opera_node->buff,(uint8_t*)opera_node->addr,opera_node->len);
            }
            FLASH_REMAP_POST(mod_info->dev_id);
        }
        norflash_memcpy(opera_node->buff + w_offs,buff,w_len);
        opera_node->lock = false;
        opera_node->next = mod_info->opera_info;
        mod_info->opera_info = opera_node;
        ret = 0;
    }

_func_end:
    return ret;
}

static bool _opera_flush(MODULE_INFO *mod_info,bool nosuspend)
{
    OPRA_INFO *cur_opera_info;
    enum HAL_NORFLASH_RET_T result;
    bool opera_is_completed = false;
    NORFLASH_API_OPERA_RESULT opera_result;
    bool ret = false;
    bool suspend;

#if defined(FLASH_SUSPEND)
    suspend = true;
#else
    suspend = false;
#endif
    suspend = nosuspend == true ? false: suspend;

    if(!mod_info->cur_opera_info)
    {
        mod_info->cur_opera_info = _get_tail(mod_info, false);
    }

    if(!mod_info->cur_opera_info)
    {
        return false;
    }

    ret = true;
    cur_opera_info = mod_info->cur_opera_info;
    if(cur_opera_info->type == NORFLASH_API_WRITTING)
    {
        if(mod_info->state == NORFLASH_API_STATE_IDLE)
        {
            suspend_number = 0;
            if(cur_opera_info->w_len > 0)
            {
                NORFLASH_API_TRACE(5,"%s: %d,hal_norflash_write_suspend,addr = 0x%x,len = 0x%x,suspend = %d.",
                                __func__,__LINE__,
                                cur_opera_info->addr + cur_opera_info->w_offs,
                                cur_opera_info->w_len,
                                suspend);

                _protection_part(mod_info->dev_id,cur_opera_info->addr + cur_opera_info->w_offs,cur_opera_info->w_len);
                FLASH_REMAP_PRE(mod_info->dev_id,cur_opera_info->addr + cur_opera_info->w_offs,cur_opera_info->w_len);
                PMU_FLASH_WRITE_CONFIG_FUNC();
                result = hal_norflash_write_suspend(mod_info->dev_id,
                             cur_opera_info->addr + cur_opera_info->w_offs,
                             cur_opera_info->buff + cur_opera_info->w_offs,
                             cur_opera_info->w_len,
                             suspend);
                PMU_FLASH_READ_CONFIG_FUNC();
                FLASH_REMAP_POST(mod_info->dev_id);
            }
            else
            {
                result = HAL_NORFLASH_OK;
            }

            if(result == HAL_NORFLASH_OK)
            {
                _protection_all(mod_info->dev_id);
                opera_is_completed = true;
                goto __opera_is_completed;
            }
            else if(result == HAL_NORFLASH_SUSPENDED)
            {
                mod_info->state = NORFLASH_API_STATE_WRITTING_SUSPEND;
            }
            else
            {
                ASSERT(0, "%s: %d, hal_norflash_write_suspend failed,result = %d",__func__,__LINE__,result);
            }
        }
        else if(mod_info->state == NORFLASH_API_STATE_WRITTING_SUSPEND)
        {
            suspend_number ++;
            FLASH_REMAP_PRE(mod_info->dev_id,cur_opera_info->addr + cur_opera_info->w_offs,cur_opera_info->w_len);
            PMU_FLASH_WRITE_CONFIG_FUNC();
            result = hal_norflash_write_resume(mod_info->dev_id, suspend);
            PMU_FLASH_READ_CONFIG_FUNC();
            FLASH_REMAP_POST(mod_info->dev_id);
            if(result == HAL_NORFLASH_OK)
            {
                _protection_all(mod_info->dev_id);
                opera_is_completed = true;
                goto __opera_is_completed;
            }
            else if(result == HAL_NORFLASH_SUSPENDED)
            {
                mod_info->state = NORFLASH_API_STATE_WRITTING_SUSPEND;
            }
            else
            {
                ASSERT(0, "%s: %d, write resume failed! r = %d, addr: 0x%x, len: 0x%x",
                       __func__,__LINE__, result,
                       cur_opera_info->addr + cur_opera_info->w_offs, cur_opera_info->w_len);
            }
        }
        else
        {
            ASSERT(0, "%s: %d, mod_info->state error,state = %d",__func__,__LINE__,mod_info->state);
        }
    }
    else
    {
        if(mod_info->state == NORFLASH_API_STATE_IDLE)
        {
            suspend_number = 0;
            NORFLASH_API_TRACE(5,"%s: %d,hal_norflash_erase_suspend,addr = 0x%x,len = 0x%x,suspend = %d.",
                                __func__,__LINE__,
                                cur_opera_info->addr,
                                cur_opera_info->len,
                                suspend);
            _protection_part(mod_info->dev_id, cur_opera_info->addr, cur_opera_info->len);
            FLASH_REMAP_PRE(mod_info->dev_id, cur_opera_info->addr, cur_opera_info->len);
            PMU_FLASH_WRITE_CONFIG_FUNC();
            result = hal_norflash_erase_suspend(mod_info->dev_id,
                         cur_opera_info->addr,
                         cur_opera_info->len,
                         suspend);
            PMU_FLASH_READ_CONFIG_FUNC();
            FLASH_REMAP_POST(mod_info->dev_id);
            if(result == HAL_NORFLASH_OK)
            {
                _protection_all(mod_info->dev_id);
                opera_is_completed = true;
                goto __opera_is_completed;
            }
            else if(result == HAL_NORFLASH_SUSPENDED)
            {
                mod_info->state = NORFLASH_API_STATE_ERASE_SUSPEND;
            }
            else
            {
                ASSERT(0, "%s: %d, hal_norflash_erase_suspend failed,result = %d",__func__,__LINE__,result);
            }
        }
        else if(mod_info->state == NORFLASH_API_STATE_ERASE_SUSPEND)
        {
            suspend_number ++;
            FLASH_REMAP_PRE(mod_info->dev_id, cur_opera_info->addr, cur_opera_info->len);
            PMU_FLASH_WRITE_CONFIG_FUNC();
            result = hal_norflash_erase_resume(mod_info->dev_id,
                         suspend);
            PMU_FLASH_READ_CONFIG_FUNC();
            FLASH_REMAP_POST(mod_info->dev_id);
            if(result == HAL_NORFLASH_OK)
            {
                _protection_all(mod_info->dev_id);
                opera_is_completed = true;
                goto __opera_is_completed;
            }
            else if(result == HAL_NORFLASH_SUSPENDED)
            {
                mod_info->state = NORFLASH_API_STATE_ERASE_SUSPEND;
            }
            else
            {
                 ASSERT(0, "%s: %d, erase resume failed! r = %d, addr: 0x%x, len: 0x%x",
                       __func__,__LINE__,result,cur_opera_info->addr,cur_opera_info->len);
            }
        }
        else
        {
            ASSERT(0, "%s: %d, mod_info->state error,state = %d",
                __func__,__LINE__,mod_info->state);
        }
    }

__opera_is_completed:

    if(opera_is_completed)
    {
        mod_info->state = NORFLASH_API_STATE_IDLE;
        if(!nosuspend
           && mod_info->cb_func
           && ((cur_opera_info->w_len > 0 && cur_opera_info->type == NORFLASH_API_WRITTING)
               || (cur_opera_info->len > 0 && cur_opera_info->type == NORFLASH_API_ERASING))
           )
        {
            NORFLASH_API_TRACE(6,"%s: w/e done.type:%d,addr:0x%x,w_len:0x%x,len:0x%x,suspend_num:%d.",
                __func__,
                cur_opera_info->type,
                cur_opera_info->addr + cur_opera_info->w_offs,
                cur_opera_info->w_len,
                cur_opera_info->len,
                suspend_number);
            if(cur_opera_info->type == NORFLASH_API_WRITTING)
            {
                opera_result.addr = cur_opera_info->addr + cur_opera_info->w_offs;
                opera_result.len = cur_opera_info->w_len;
            }
            else
            {
                opera_result.addr = cur_opera_info->addr;
                opera_result.len = cur_opera_info->len;
            }
            _cache_invalid(mod_info->dev_id, opera_result.addr, opera_result.len);
            opera_result.type = cur_opera_info->type;
            opera_result.result = NORFLASH_API_OK;
            opera_result.remain_num = _get_ew_count(mod_info) - 1;
            opera_result.suspend_num = suspend_number;
            mod_info->cb_func(&opera_result);
        }
        _opera_del(mod_info,cur_opera_info);
        mod_info->cur_opera_info = NULL;
    }

    return ret;
}

static bool _flush_allowed_cb(void)
{
    return false;
}

static void _flush_disable(enum NORFLASH_API_USER user_id,uint32_t cb)
{
    norflash_api_info.allowed_cb[user_id] = (NOFLASH_API_FLUSH_ALLOWED_CB)cb;
}

static void _flush_enable(enum NORFLASH_API_USER user_id)
{
    if(norflash_api_info.allowed_cb[user_id])
    {
        norflash_api_info.allowed_cb[user_id] = NULL;
    }
}

static bool _flush_is_allowed(void)
{
    bool ret = true;
    uint32_t user_id;

    for(user_id = NORFLASH_API_USER_CP; user_id < NORFLASH_API_USER_COUNTS; user_id ++)
    {
        if(norflash_api_info.allowed_cb[user_id])
        {
            if(!norflash_api_info.allowed_cb[user_id]())
            {
                ret = false;
                break;
            }
            else
            {
                norflash_api_info.allowed_cb[user_id] = NULL;
            }
        }
    }
    return ret;
}

static int _flush_one_pending_opera(MODULE_INFO *mod_info, bool nosuspend)
{
    bool bresult;

    while(mod_info->state != NORFLASH_API_STATE_IDLE)
    {
        if(!_flush_is_allowed())
        {
            continue;
        }
        bresult = _opera_flush(mod_info,nosuspend);
        if(!bresult)
        {
            norflash_api_info.cur_mod = NULL;
        }
    }

    return 0;
}

static int _norflash_api_flush(void)
{
    enum NORFLASH_API_MODULE_ID_T mod_id = NORFLASH_API_MODULE_ID_COUNT;
    MODULE_INFO *mod_info;
    int ret;

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return 0;
    }
#if defined(FLASH_API_SIMPLE)
    return 0;
#endif

    if(!_flush_is_allowed())
    {
        return 0;
    }

    mod_info = _get_cur_mod();
    if(!mod_info)
    {
        return 0;
    }
    _norflash_api_int_lock(mod_info->dev_id);
    mod_id = _get_mod_id(mod_info);

    norflash_api_info.cur_mod_id = mod_id;
    norflash_api_info.cur_mod = mod_info;
    if(!_opera_flush(mod_info,false))
    {
        norflash_api_info.cur_mod = NULL;
    }

    if(!_get_cur_mod())
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    _norflash_api_int_unlock(mod_info->dev_id);

    return ret;
}

//-------------------------------------------------------------------
// APIS Function.
//-------------------------------------------------------------------
enum NORFLASH_API_RET_T norflash_api_init(void)
{
    uint32_t i;
    uint32_t total_size;
    bool protection_enable = false;
    enum HAL_NORFLASH_RET_T ret;
#ifdef FLASH_SUSPEND
    const int16_t *irq = NULL;
    uint32_t irq_cnt = 0;
#endif
#if defined(CHIP_BEST2003) && defined(USE_MULTI_FLASH)
    uint32_t flash0_size = 0;
#endif

    if (norflash_api_info.is_inited) {
        return NORFLASH_API_OK;
    }

    norflash_memset((uint8_t*)flash_dev_opened, 0, sizeof(flash_dev_opened));
    norflash_memset((void*)&norflash_api_info, 0, sizeof(NORFLASH_API_INFO));
    norflash_api_info.cur_mod_id = NORFLASH_API_MODULE_ID_COUNT;
    norflash_api_info.is_inited = true;
    norflash_api_info.cur_mod = NULL;
    for(i = 0; i < NORFLASH_API_MODULE_ID_COUNT; i++)
    {
        norflash_api_info.mod_info[i].state = NORFLASH_API_STATE_UNINITED;
    }

    _norflash_api_mutex_init();
    norflash_api_set_hook(NORFLASH_API_HOOK_USER_0, norflash_api_flush);
#if !defined(FLASH_API_SIMPLE)
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    norflash_api_thread_init();
#else
#ifdef FLASH_SUSPEND
    hal_sleep_set_sleep_hook(HAL_SLEEP_HOOK_NORFLASH_API, _norflash_api_exec_flush_hook);
#else //FLASH_SUSPEND
    hal_sleep_set_deep_sleep_hook(HAL_DEEP_SLEEP_HOOK_NORFLASH_API, _norflash_api_exec_flush_hook);
#endif // FLASH_SUSPEND
#endif
#endif // FLASH_API_SIMPLE

#ifdef FLASH_SUSPEND
    hal_dma_get_irq_list(&irq, &irq_cnt);
    for(i = 0; i < irq_cnt; i++)
    {
        hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, irq[i], true);
    }
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA_IRQn, true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA1_IRQn, true);
#ifdef BLE_USB_AUDIO_SUPPORT
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, USB_IRQn, true);
#endif
#endif // FLASH_SUSPEND

#ifdef FLASH_REMAP
    _flash_remap_config(HAL_FLASH_ID_0, NORFLASH_API_NORMAL_REMAP_ID, OTA_CODE_OFFSET, OTA_REMAP_OFFSET, OTA_REMAP_OFFSET);
#endif

    for(i = HAL_FLASH_ID_0; i < HAL_FLASH_ID_NUM; i++)
    {
        if(i > HAL_FLASH_ID_0)
        {
            if(i == 1)
            {
#if !(defined(FLASH1_CTRL_BASE) && (defined(FLASH1_CALIB_SECTION_SIZE) || defined(FLASH1_CALIB_SEQ_ADDR))  && defined(USE_MULTI_FLASH))

                continue;
#else
                hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH1);
                hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH1);
                hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH1);
                hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH1);
#if defined(CHIP_BEST2003) && defined(USE_MULTI_FLASH)
                TRACE(1, "%s flash0_size = 0x%x", __func__, flash0_size);
                hal_cmu_set_flash0_size(flash0_size);
#endif
#endif
            }
            if(i == 2)
            {
#if !(defined(FLASH2_CTRL_BASE) && (defined(FLASH2_CALIB_SECTION_SIZE) || defined(USE_MULTI_FLASH)))
                continue;
#else
                hal_cmu_clock_enable(HAL_CMU_MOD_H_FLASH2);
                hal_cmu_clock_enable(HAL_CMU_MOD_O_FLASH2);
                hal_cmu_reset_clear(HAL_CMU_MOD_H_FLASH2);
                hal_cmu_reset_clear(HAL_CMU_MOD_O_FLASH2);
#endif
            }

            TRACE(1,"%s: norflash (%d) init", __func__, (int)i);
            ret = hal_norflash_init((enum HAL_FLASH_ID_T)i);
            if(ret)
            {
                TRACE(1,"%s: norflash(%d) init faile! ret = %d", __func__, (int)i, (int)ret);
                continue;
            }
            else
            {
                TRACE(1,"%s: norflash(%d) init success", __func__, (int)i);
            }
        }
#if defined(CHIP_BEST2003) && defined(USE_MULTI_FLASH)
        if(i == HAL_FLASH_ID_0)
        {
            if(hal_norflash_opened(HAL_FLASH_ID_0))
            {
#ifdef FLASH0_MAX_SIZE_MB
                flash0_size = FLASH0_MAX_SIZE_MB*0x100000;
#else
                flash0_size = hal_norflash_get_flash_total_size(HAL_FLASH_ID_0);
#endif
            }
        }
#endif
        flash_dev_opened[i] = 1;
        ret = hal_norflash_get_size((enum HAL_FLASH_ID_T)i, &flash_total_size[i], &flash_block_size[i], &flash_sector_size[i], &flash_page_size[i]);
        ASSERT(ret == HAL_NORFLASH_OK, "%s: get size fail!, ret = %d", __func__, (int)ret);

        ret = hal_norflash_get_dual_chip_mode((enum HAL_FLASH_ID_T)i, &flash_chip_dual_enable[i], NULL);
        ASSERT(ret == HAL_NORFLASH_OK, "%s: get dual chip mode fail!, ret = %d", __func__, (int)ret);
#ifdef FLASH_PROTECTION
        total_size = hal_norflash_get_flash_total_size((enum HAL_FLASH_ID_T)i);
        if(_flash_chip_dual_enable((enum HAL_FLASH_ID_T)i))
        {
            if(total_size >= 0x40000 && total_size <= 0x2000000)
            {
                protection_enable = true;
            }
        }
        else
        {
            if(total_size >= 0x20000 && total_size <= 0x2000000)
            {
                protection_enable = true;
            }
        }

        ASSERT(protection_enable, "PROTECTION NOT SUPPORT AT THIS SIZE! flash id = %d, total size: 0x%x",i, total_size);
        norflash_api_protection_bp_init((enum HAL_FLASH_ID_T)i, total_size);
#else
        total_size = 0;
        total_size = total_size;
        protection_enable = false;
        protection_enable = protection_enable;
#endif

    }
    return NORFLASH_API_OK;
}

int norflash_api_alloc_mod(void)
{
    // alloc from the tail -> head
    int cur_mod_id = NORFLASH_API_MODULE_ID_COUNT;
    // find free mod
    while (--cur_mod_id >= NORFLASH_API_MODULE_ID_MAX) {
        if (norflash_api_info.mod_info[cur_mod_id].is_used == 0) {
            norflash_api_info.mod_info[cur_mod_id].is_used = 1;
            break;
        }
    }
    return (cur_mod_id >= NORFLASH_API_MODULE_ID_MAX) ? cur_mod_id : -1;
}

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
                )
{
    MODULE_INFO *mod_info;

    NORFLASH_API_TRACE(5,"mod_id = %d,dev_id = %d,base_addr = 0x%x,mod_len = 0x%x",
            mod_id,dev_id,mod_base_addr,mod_len);
    NORFLASH_API_TRACE(4,"mod_block_len = 0x%x,mod_sector_len = 0x%x,mod_page_len = 0x%x,buffer_len = 0x%x.",
            mod_block_len,mod_sector_len,mod_page_len,buffer_len);
    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(2,"%s: %d, norflash_api uninit!",__func__,__LINE__);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    if(dev_id >= HAL_FLASH_ID_NUM)
    {
        NORFLASH_API_TRACE(2,"%s : dev_id error! mod_id = %d.",__func__,dev_id);
        return NORFLASH_API_BAD_DEV_ID;
    }

    if(buffer_len < mod_sector_len || !IS_ALIGN(buffer_len,mod_sector_len))
    {
        NORFLASH_API_TRACE(2,"%s : buffer_len error buffer_len = %d.",__func__, buffer_len);
        return NORFLASH_API_BAD_BUFF_LEN;
    }

    if (norflash_api_info.mod_info[mod_id].is_used == 0) {
        norflash_api_info.mod_info[mod_id].is_used = 1;
    }

    mod_info = _get_module_info(mod_id);
    if(mod_info->is_registered)
    {
        NORFLASH_API_TRACE(1,"%s: %d, norflash_async[%d] has registered!",__func__,__LINE__,mod_id);
        return NORFLASH_API_ERR_REGISTRATION;
    }
    _norflash_api_mutex_wait();
    mod_info->dev_id = dev_id;
    mod_info->mod_id = mod_id;
    mod_info->mod_base_addr = mod_base_addr;
    mod_info->mod_len = mod_len;
    mod_info->mod_block_len = mod_block_len;
    mod_info->mod_sector_len = mod_sector_len;
    mod_info->mod_page_len = mod_page_len;
    mod_info->buff_len = buffer_len;
    mod_info->cb_func = cb_func;
    mod_info->opera_info = NULL;
    mod_info->cur_opera_info = NULL;
    mod_info->state = NORFLASH_API_STATE_IDLE;
    mod_info->is_registered = true;
    _norflash_api_mutex_release();
    return NORFLASH_API_OK;
}

enum HAL_FLASH_ID_T norflash_api_get_dev_id_by_addr(uint32_t addr)
{
    return hal_norflash_addr_to_id(addr);
}

enum NORFLASH_API_RET_T norflash_api_read(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                uint8_t *buffer,
                uint32_t len
                )
{
    MODULE_INFO *mod_info;
    int32_t result;
    enum NORFLASH_API_RET_T ret;

    NORFLASH_API_TRACE(4,"%s:mod_id = %d,start_addr = 0x%x,len = 0x%x",
                __func__,mod_id, start_addr, len);
    ASSERT(buffer,"%s:buffer is null! ",
                __func__);

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }
    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        NORFLASH_API_TRACE(2,"%s : module unregistered! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(!ADDR_IS_VALID(mod_info->dev_id, mod_info->mod_base_addr, mod_info->mod_len, start_addr, len))
    {
        NORFLASH_API_TRACE(3,"%s : reading out of range! start_address = 0x%x,len = 0x%x.",
                    __func__, start_addr, len);
        return NORFLASH_API_BAD_ADDR;
    }

    if((len == 0)
       || !IN_ONE_SECTOR(mod_info->dev_id, start_addr, len, flash_sector_size[mod_info->dev_id]))
    {
        NORFLASH_API_TRACE(2,"%s : len error! start_addr = 0x%x, len = %d.",
                   __func__, start_addr, len);
        return NORFLASH_API_BAD_LEN;
    }

    if(flash_chip_dual_enable[mod_info->dev_id])
    {
        if(!IS_ALIGN(start_addr,2))
        {
             NORFLASH_API_TRACE(2,"%s : in DUAL_CHIP_FLASH mode, addr error! start_addr = 0x%x",
                    __func__, start_addr);
            return NORFLASH_API_BAD_ADDR;
        }
        if(!IS_ALIGN(len,2))
        {
             NORFLASH_API_TRACE(2,"%s : in DUAL_CHIP_FLASH mode, len error! len = 0x%x",
                    __func__, len);
            return NORFLASH_API_BAD_LEN;
        }
    }

    _norflash_api_mutex_wait();
    _norflash_api_int_lock(mod_info->dev_id);

    result = _opera_read(mod_info,start_addr,(uint8_t*)buffer,len);

    if(result)
    {
        ret =  NORFLASH_API_ERR;
    }
    else
    {
        ret = NORFLASH_API_OK;
    }
    _norflash_api_int_unlock(mod_info->dev_id);
    _norflash_api_mutex_release();
    NORFLASH_API_TRACE(2,"%s: done. ret = %d.",__func__,ret);
    return ret;
}

enum NORFLASH_API_RET_T norflash_sync_read(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                uint8_t *buffer,
                uint32_t len
                )
{
    MODULE_INFO *mod_info;

    NORFLASH_API_TRACE(4,"%s:mod_id = %d,start_addr = 0x%x,len = 0x%x",
                __func__,mod_id,start_addr,len);
    ASSERT(buffer,"%s:%d,buffer is null! ",
                __func__,__LINE__);

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!", __func__);
        return NORFLASH_API_ERR_UNINIT;
    }
    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.", __func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        NORFLASH_API_TRACE(2,"%s : module unregistered! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(!ADDR_IS_VALID(mod_info->dev_id, mod_info->mod_base_addr, mod_info->mod_len, start_addr, len))
    {
        NORFLASH_API_TRACE(3,"%s : reading out of range! start_address = 0x%x,len = 0x%x.",
                    __func__, start_addr, len);
        return NORFLASH_API_BAD_ADDR;
    }

    if((len == 0)
       || !IN_ONE_SECTOR(mod_info->dev_id, start_addr, len, flash_sector_size[mod_info->dev_id]))
    {
        NORFLASH_API_TRACE(2,"%s : len error! start_addr = 0x%x, len = %d.",
                   __func__, start_addr, len);
        return NORFLASH_API_BAD_LEN;
    }

    if(flash_chip_dual_enable[mod_info->dev_id])
    {
        if(!IS_ALIGN(start_addr,2))
        {
             NORFLASH_API_TRACE(2,"%s : in DUAL_CHIP_FLASH mode, addr error! start_addr = 0x%x",
                    __func__, start_addr);
            return NORFLASH_API_BAD_ADDR;
        }
        if(!IS_ALIGN(len,2))
        {
             NORFLASH_API_TRACE(2,"%s : in DUAL_CHIP_FLASH mode, len error! len = 0x%x",
                    __func__, len);
            return NORFLASH_API_BAD_LEN;
        }
    }

    _norflash_api_mutex_wait();
    _norflash_api_int_lock(mod_info->dev_id);
    FLASH_REMAP_PRE(mod_info->dev_id,start_addr, len);
#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
    if(mod_info->dev_id > HAL_FLASH_ID_0)
    {
        hal_norflash_read(mod_info->dev_id, start_addr, buffer, len);
    }
    else
#endif
    {
        norflash_memcpy(buffer,(uint8_t*)start_addr,len);
    }
    FLASH_REMAP_POST(mod_info->dev_id);
    _norflash_api_int_unlock(mod_info->dev_id);
    _norflash_api_mutex_release();
    NORFLASH_API_TRACE(1,"%s: done.",__func__);
    return NORFLASH_API_OK;
}

enum NORFLASH_API_RET_T norflash_api_erase(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                uint32_t len,
                bool async
                )
{
    MODULE_INFO *mod_info;
    MODULE_INFO *cur_mod_info;
    int32_t result;
    bool bresult = 0;
    enum NORFLASH_API_RET_T ret;

    NORFLASH_API_TRACE(5,"%s: mod_id = %d,start_addr = 0x%x,len = 0x%x,async = %d.",
                __func__,mod_id,start_addr,len,async);

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }
    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : invalid mod_id! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        NORFLASH_API_TRACE(2,"%s : module unregistered! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(!ADDR_IS_VALID(mod_info->dev_id, mod_info->mod_base_addr, mod_info->mod_len, start_addr, len))
    {
        NORFLASH_API_TRACE(3,"%s : erase out of range! start_address = 0x%x,len = 0x%x.",
                    __func__,start_addr,len);
        return NORFLASH_API_BAD_ADDR;
    }

    if(
#ifdef PUYA_FLASH_ERASE_PAGE_ENABLE
       (start_addr & (mod_info->mod_page_len - 1)) != 0 &&
#endif
       (start_addr & (mod_info->mod_sector_len - 1)) != 0)
    {
        NORFLASH_API_TRACE(2,"%s : start_address no alignment! start_address = %d.",
                   __func__, start_addr);
        return NORFLASH_API_BAD_ADDR;
    }

    if(
#ifdef PUYA_FLASH_ERASE_PAGE_ENABLE
       (len & (mod_info->mod_page_len - 1)) != 0 &&
#endif
       (len & (mod_info->mod_sector_len - 1)) != 0)
    {
        NORFLASH_API_TRACE(2,"%s : len error. len = %d!",
                   __func__, len);
        return NORFLASH_API_BAD_LEN;
    }
    _norflash_api_mutex_wait();
#if defined(FLASH_API_SIMPLE)
    async = false;
#endif
    if(async)
    {
        _norflash_api_int_lock(mod_info->dev_id);
        // add to opera_info chain header.
        result = _e_opera_add(mod_info,start_addr,len);
        if(result == 0)
        {
            ret = NORFLASH_API_OK;
        }
        else
        {
            ret = NORFLASH_API_BUFFER_FULL;
        }
        _norflash_api_int_unlock(mod_info->dev_id);
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
        _norflash_api_hook_activate();
#endif
        NORFLASH_API_TRACE(4,"%s: _e_opera_add done. start_addr = 0x%x,len = 0x%x,ret = %d.",
                __func__,start_addr,len,ret);
    }
    else
    {
        _norflash_api_int_lock(mod_info->dev_id);
        // Handle the suspend operation.
        if(norflash_api_info.cur_mod != NULL
           && mod_info != norflash_api_info.cur_mod)
        {
            cur_mod_info = norflash_api_info.cur_mod;
            _flush_one_pending_opera(cur_mod_info, true);
        }

        // flush all of cur module opera.
        // norflash_api_info.cur_mod_id = mod_id;
        do{
            if(!_flush_is_allowed())
            {
                continue;
            }
            bresult = _opera_flush(mod_info,true);
        }while(bresult);

        _protection_part(mod_info->dev_id,start_addr,len);
        FLASH_REMAP_PRE(mod_info->dev_id,start_addr,len);
        PMU_FLASH_WRITE_CONFIG_FUNC();
        result = hal_norflash_erase(mod_info->dev_id,start_addr, len);
        PMU_FLASH_READ_CONFIG_FUNC();
        FLASH_REMAP_POST(mod_info->dev_id);
        _protection_all(mod_info->dev_id);
        if(result == HAL_NORFLASH_OK)
        {
            ret = NORFLASH_API_OK;
        }
        else if(result == HAL_NORFLASH_BAD_ADDR)
        {
            ret = NORFLASH_API_BAD_ADDR;
        }
        else if(result == HAL_NORFLASH_BAD_LEN)
        {
            ret = NORFLASH_API_BAD_LEN;
        }
        else
        {
            ret = NORFLASH_API_ERR;
        }
        _cache_invalid(mod_info->dev_id, start_addr, len);
        _norflash_api_int_unlock(mod_info->dev_id);
        NORFLASH_API_TRACE(4,"%s: hal_norflash_erase done. start_addr = 0x%x,len = 0x%x,ret = %d.",
                __func__,start_addr,len,ret);
    }
    _norflash_api_mutex_release();
    //NORFLASH_API_TRACE(2,"%s: done.ret = %d.",__func__, ret);
    return ret;
}

enum NORFLASH_API_RET_T norflash_api_write(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t start_addr,
                const uint8_t *buffer,
                uint32_t len,
                bool async
                )
{
    MODULE_INFO *mod_info;
    MODULE_INFO *cur_mod_info;
    int32_t result;
    bool bresult = 0;
    enum NORFLASH_API_RET_T ret;

    NORFLASH_API_TRACE(4,"%s: mod_id = %d,start_addr = 0x%x,len = 0x%x.",
                __func__,mod_id,start_addr,len);

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        NORFLASH_API_TRACE(2,"%s :module unregistered! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(!ADDR_IS_VALID(mod_info->dev_id, mod_info->mod_base_addr, mod_info->mod_len, start_addr, len))
    {
        NORFLASH_API_TRACE(2,"%s : writting out of range! start_address = 0x%x,len = 0x%x.",
                    __func__,mod_info->mod_base_addr,mod_info->mod_len);
        return NORFLASH_API_BAD_ADDR;
    }

    if((len == 0)
       || !IN_ONE_SECTOR(mod_info->dev_id, start_addr, len, flash_sector_size[mod_info->dev_id]))
    {
        NORFLASH_API_TRACE(2,"%s : len error! start_addr = 0x%x, len = %d.",
                   __func__, start_addr, len);
        return NORFLASH_API_BAD_LEN;
    }

    if(flash_chip_dual_enable[mod_info->dev_id])
    {
        if(!IS_ALIGN(start_addr,2))
        {
             NORFLASH_API_TRACE(2,"%s : in DUAL_CHIP_FLASH mode, addr error! start_addr = 0x%x",
                    __func__, start_addr);
            return NORFLASH_API_BAD_ADDR;
        }
        if(!IS_ALIGN(len,2))
        {
             NORFLASH_API_TRACE(2,"%s : in DUAL_CHIP_FLASH mode, len error! len = 0x%x",
                    __func__, len);
            return NORFLASH_API_BAD_LEN;
        }
    }

    _norflash_api_mutex_wait();
#if defined(FLASH_API_SIMPLE)
    async = false;
#endif
    if(async)
    {
        // add to opera_info chain header.
        _norflash_api_int_lock(mod_info->dev_id);
        result = _w_opera_add(mod_info,start_addr,len,(uint8_t*)buffer);
        if(result == 0)
        {
            ret = NORFLASH_API_OK;
        }
        else
        {
            ret = NORFLASH_API_BUFFER_FULL;
        }
        _norflash_api_int_unlock(mod_info->dev_id);
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
        _norflash_api_hook_activate();
#endif
        NORFLASH_API_TRACE(4,"%s: _w_opera_add done. start_addr = 0x%x,len = 0x%x,ret = %d.",
                __func__,start_addr,len,ret);
    }
    else
    {
        _norflash_api_int_lock(mod_info->dev_id);

        // flush the opera of currently being processed.
        if(norflash_api_info.cur_mod != NULL
            && mod_info != norflash_api_info.cur_mod)
        {
            cur_mod_info = norflash_api_info.cur_mod;
            _flush_one_pending_opera(cur_mod_info, true);
        }

        // flush all of cur module opera.
        do{
            if(!_flush_is_allowed())
            {
                continue;
            }
            bresult = _opera_flush(mod_info,true);
        }while(bresult);
        _protection_part(mod_info->dev_id,start_addr,len);
        FLASH_REMAP_PRE(mod_info->dev_id,start_addr,len);
        PMU_FLASH_WRITE_CONFIG_FUNC();
        result = hal_norflash_write(mod_info->dev_id,start_addr, buffer, len);
        PMU_FLASH_READ_CONFIG_FUNC();
        FLASH_REMAP_POST(mod_info->dev_id);
        _protection_all(mod_info->dev_id);
        if(result == HAL_NORFLASH_OK)
        {
            ret = NORFLASH_API_OK;
        }
        else if(result == HAL_NORFLASH_BAD_ADDR)
        {
            ret = NORFLASH_API_BAD_ADDR;
        }
        else if(result == HAL_NORFLASH_BAD_LEN)
        {
            ret = NORFLASH_API_BAD_LEN;
        }
        else
        {
            ret = NORFLASH_API_ERR;
        }
        _cache_invalid(mod_info->dev_id, start_addr, len);
        _norflash_api_int_unlock(mod_info->dev_id);
        NORFLASH_API_TRACE(4,"%s: hal_norflash_write done. start_addr = 0x%x,len = 0x%x,ret = %d.",
                __func__,start_addr,len,ret);
    }
    _norflash_api_mutex_release();
    return ret;
}

// 0:all pending flash op flushed, 1:still pending flash op to be flushed
int norflash_api_flush(void)
{
    int ret;

    _norflash_api_mutex_wait();
    ret = _norflash_api_flush();
    _norflash_api_mutex_release();
    return ret;
}

// 0:all pending flash op flushed, 1:still pending flash op to be flushed
static int _norflash_api_exec_flush_hook(void)
{
    uint32_t i;
    int ret;
    bool hook_done;

    do
    {
        hook_done = true;
        for(i = 0; i < NORFLASH_API_HOOK_USER_QTY; i++)
        {
            if(_norflash_api_hook[i].hook_handle)
            {
                ret = _norflash_api_hook[i].hook_handle();
                if(ret)
                {
#if !(defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK))
                    break;
#endif
                    hook_done = false;
                }
            }
        }
    }while(!hook_done);

    return hook_done ? 0 : 1;
}

bool norflash_api_buffer_is_free(
                enum NORFLASH_API_MODULE_ID_T mod_id)
{
    MODULE_INFO *mod_info;
    uint32_t count;

    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        ASSERT(0,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        ASSERT(0,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
    }
    _norflash_api_mutex_wait();
    count = _get_ew_count(mod_info);
    _norflash_api_mutex_release();
    if(count > 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

uint32_t norflash_api_get_used_buffer_count(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                enum NORFLASH_API_OPRATION_TYPE type
                )
{
    MODULE_INFO *mod_info;
    uint32_t count = 0;

    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        ASSERT(0,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
    }
    _norflash_api_mutex_wait();
    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        ASSERT(0,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
    }
    if(type & NORFLASH_API_WRITTING)
    {
        count = _get_w_count(mod_info);
    }

    if(type & NORFLASH_API_ERASING)
    {
        count += _get_e_count(mod_info);
    }
    _norflash_api_mutex_release();
    return count;
}

uint32_t norflash_api_get_free_buffer_count(
                enum HAL_FLASH_ID_T dev_id,
                enum NORFLASH_API_OPRATION_TYPE type
                )
{
    MODULE_INFO *mod_info;
    uint32_t i;
    uint32_t used_count = 0;
    uint32_t free_count = 0;
    uint32_t total_count;

    _norflash_api_mutex_wait();
    if(type & NORFLASH_API_WRITTING)
    {
        for(i = NORFLASH_API_MODULE_ID_LOG_DUMP; i < NORFLASH_API_MODULE_ID_COUNT; i ++)
        {
            mod_info = _get_module_info((enum NORFLASH_API_MODULE_ID_T)i);
            if(mod_info->is_registered && mod_info->dev_id == dev_id)
            {
                used_count += _get_w_count(mod_info);
            }
        }
        total_count = _norflash_api_opera_num(dev_id);
        ASSERT(used_count <= total_count,"writting opra count error!");
        free_count += (total_count - used_count);
    }

    if(type & NORFLASH_API_ERASING)
    {
        for(i = NORFLASH_API_MODULE_ID_LOG_DUMP; i < NORFLASH_API_MODULE_ID_COUNT; i ++)
        {
            mod_info = _get_module_info((enum NORFLASH_API_MODULE_ID_T)i);
            if(mod_info->is_registered && mod_info->dev_id == dev_id)
            {
                used_count += _get_e_count(mod_info);
            }
        }
        total_count = _norflash_api_buffer_num(dev_id);
        ASSERT(used_count <= total_count,"erase opra count error!");
        free_count += (total_count - used_count);
    }
    _norflash_api_mutex_release();
    return free_count;
}


void norflash_api_flush_all(bool force)
{
    int ret;
    int cnt = 0;

    if(force)
    {
        norflash_api_flush_enable_all();
    }

    do
    {
        ret = norflash_api_flush();
        if(ret == 1)
        {
            cnt ++;
        }
    } while (1 == ret);

    NORFLASH_API_TRACE(2,"%s: done. cnt = %d.",__func__,cnt);
}

void norflash_api_flush_disable(enum NORFLASH_API_USER user_id, uint32_t cb, bool wait_idle_status)
{
    MODULE_INFO *cur_mod_info;
    bool bresult;

    if(!norflash_api_info.is_inited)
    {
        return;
    }

    ASSERT(user_id < NORFLASH_API_USER_COUNTS, "%s: user_id(%d) error!", __func__, user_id);

    if(wait_idle_status)
    {
        cur_mod_info = norflash_api_info.cur_mod;
        if(cur_mod_info)
        {
            if(cur_mod_info->dev_id == HAL_FLASH_ID_0)
            {
                while(cur_mod_info->state != NORFLASH_API_STATE_IDLE)
                {
                    if(!_flush_is_allowed())
                    {
                        continue;
                    }
                    _norflash_api_int_lock(cur_mod_info->dev_id);
                    bresult = _opera_flush(cur_mod_info,false);
                    _norflash_api_int_unlock(cur_mod_info->dev_id);
                    if(!bresult)
                    {
                        norflash_api_info.cur_mod = NULL;
                    }
                }
            }
        }
    }

    if(cb){
        _flush_disable(user_id, cb);
    }
    else
    {
        _flush_disable(user_id, (uint32_t)_flush_allowed_cb);
    }
}

void norflash_api_flush_enable(enum NORFLASH_API_USER user_id)
{
    if(!norflash_api_info.is_inited)
    {
        return;
    }
    ASSERT(user_id < NORFLASH_API_USER_COUNTS, "%s: user_id(%d) too large!", __func__, user_id);
    _flush_enable(user_id);
}

void norflash_api_flush_enable_all(void)
{
    uint32_t user_id;

    if(!norflash_api_info.is_inited)
    {
        return;
    }
    for(user_id = NORFLASH_API_USER_CP; user_id < NORFLASH_API_USER_COUNTS; user_id ++)
    {
        _flush_enable((enum NORFLASH_API_USER)user_id);
    }
}


enum NORFLASH_API_STATE norflash_api_get_state(enum NORFLASH_API_MODULE_ID_T mod_id)
{
    ASSERT(mod_id < NORFLASH_API_MODULE_ID_COUNT,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
    return norflash_api_info.mod_info[mod_id].state;
}

enum NORFLASH_API_RET_T norflash_api_get_base_addr(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                uint32_t *addr)
{
    ASSERT(addr, "null pointer received in %s", __func__);
    *addr = 0;

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(2,"%s: %d, norflash_api uninit!",__func__,__LINE__);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    MODULE_INFO *mod_info =_get_module_info(mod_id);
    if(mod_info->is_registered)
    {
        *addr = mod_info->mod_base_addr;
        return NORFLASH_API_OK;
    }
    else
    {
        NORFLASH_API_TRACE(2,"%s: %d, norflash_api uninit!",__func__,__LINE__);
        return NORFLASH_API_ERR_UNINIT;
    }
}

enum NORFLASH_API_RET_T norflash_api_get_dev_id(
                enum NORFLASH_API_MODULE_ID_T mod_id,
                enum HAL_FLASH_ID_T *dev_id)
{
    ASSERT(dev_id, "null pointer received in %s", __func__);

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(2,"%s: %d, norflash_api uninit!",__func__,__LINE__);
        return NORFLASH_API_ERR_UNINIT;
    }

    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    MODULE_INFO *mod_info =_get_module_info(mod_id);
    if(mod_info->is_registered)
    {
        *dev_id = mod_info->dev_id;
        return NORFLASH_API_OK;
    }
    else
    {
        NORFLASH_API_TRACE(2,"%s: %d, norflash_api uninit!",__func__,__LINE__);
        return NORFLASH_API_ERR_UNINIT;
    }
}

uint32_t norflash_api_get_total_size(enum HAL_FLASH_ID_T dev_id)
{
    return flash_total_size[dev_id];
}

uint32_t norflash_api_get_block_size(enum HAL_FLASH_ID_T dev_id)
{
    return flash_block_size[dev_id];
}

uint32_t norflash_api_get_sector_size(enum HAL_FLASH_ID_T dev_id)
{
    return flash_sector_size[dev_id];
}

uint32_t norflash_api_get_page_size(enum HAL_FLASH_ID_T dev_id)
{
    return flash_page_size[dev_id];
}

bool norflash_api_dev_opened(enum HAL_FLASH_ID_T dev_id)
{
    return !!(flash_dev_opened[dev_id]);
}

enum NORFLASH_API_RET_T norflash_api_remap_pre(
                         enum HAL_FLASH_ID_T dev_id,
                         uint32_t start_addr,
                         uint32_t len)
{
    NORFLASH_API_TRACE(4,"%s:dev_id = %d,start_addr = 0x%x,len = 0x%x",
                __func__, dev_id, start_addr, len);

    FLASH_REMAP_PRE(dev_id, start_addr, len);
    return NORFLASH_API_OK;
}

enum NORFLASH_API_RET_T norflash_api_remap_post(HAL_FLASH_ID_T dev_id)
{
    NORFLASH_API_TRACE(4,"%s:dev_id = %d",
                __func__, dev_id);

    FLASH_REMAP_POST(dev_id);
    return NORFLASH_API_OK;
}

enum NORFLASH_API_RET_T norflash_api_remap_config(
                          HAL_FLASH_ID_T dev_id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id,
                          uint32_t addr,
                          uint32_t len,
                          uint32_t remap_offset)
{
#ifdef FLASH_REMAP
    int ret;

    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    ret = _flash_remap_config(dev_id, remap_id, addr, len, remap_offset);
    if(ret)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    else
    {
        return NORFLASH_API_OK;
    }
#else
    return NORFLASH_API_OK;
#endif
}

enum NORFLASH_API_RET_T norflash_api_remap_enable(
                          enum HAL_FLASH_ID_T dev_id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id
                          )
{
#ifdef FLASH_REMAP
    int ret;

    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    ret = hal_norflash_enable_remap(dev_id, remap_id);
    if(ret)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    else
    {
        return NORFLASH_API_OK;
    }
#else
    return NORFLASH_API_OK;
#endif
}

enum NORFLASH_API_RET_T norflash_api_remap_disable(
                          enum HAL_FLASH_ID_T dev_id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id
                          )
{
#ifdef FLASH_REMAP
    int ret;

    if(dev_id != NORFLASH_API_REMAP_FLASH_ID)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    ret = hal_norflash_disable_remap(dev_id, remap_id);
    if(ret)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    else
    {
        return NORFLASH_API_OK;
    }
#else
    return NORFLASH_API_OK;
#endif
}

enum NORFLASH_API_RET_T norflash_api_get_remap_config(
                         enum HAL_FLASH_ID_T dev_id,
                         enum HAL_NORFLASH_REMAP_ID_T remap_id,
                         uint8_t *is_used,
                         uint32_t *addr,
                         uint32_t *len,
                         uint32_t *offset)
{
    if(dev_id != HAL_FLASH_ID_0)
    {
        return NORFLASH_API_ERR_REMAP;
    }

#ifdef FLASH_REMAP
    if(remap_id > HAL_NORFLASH_REMAP_ID_QTY)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    *is_used = flash_remap_cfg[remap_id].is_used;
    *addr = flash_remap_cfg[remap_id].addr;
    *len = flash_remap_cfg[remap_id].len;
    *offset = flash_remap_cfg[remap_id].remap_offset;
    return NORFLASH_API_OK;
#else
    return NORFLASH_API_ERR_REMAP;
#endif
}

bool norflash_api_flush_is_in_task(void)
{
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    return true;
#else
    return false;
#endif
}

enum NORFLASH_API_RET_T norflash_api_get_dual_chip_mode(enum NORFLASH_API_MODULE_ID_T mod_id, int *dual_flash, int *dual_sec_reg)
{
    MODULE_INFO *mod_info;

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }
    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"%s : mod_id error! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        NORFLASH_API_TRACE(2,"%s : module unregistered! mod_id = %d.",__func__, mod_id);
        return NORFLASH_API_ERR_UNINIT;
    }

    hal_norflash_get_dual_chip_mode(mod_info->dev_id, dual_flash, dual_sec_reg);

    return NORFLASH_API_OK;
}

enum NORFLASH_API_RET_T norflash_api_set_dual_chip_mode(enum NORFLASH_API_MODULE_ID_T mod_id, int dual_flash, int dual_sec_reg)
{
    int ret;
    MODULE_INFO *mod_info;

    NORFLASH_API_TRACE(2,"norflash_api_set_dual_chip_mode");
    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"norflash_api uninit!");
        return NORFLASH_API_ERR_UNINIT;
    }
    if(mod_id >= NORFLASH_API_MODULE_ID_COUNT)
    {
        NORFLASH_API_TRACE(2,"mod_id error! mod_id = %d.", mod_id);
        return NORFLASH_API_BAD_MOD_ID;
    }

    mod_info = _get_module_info(mod_id);
    if(!mod_info->is_registered)
    {
        NORFLASH_API_TRACE(2,"module unregistered! mod_id = %d.", mod_id);
        return NORFLASH_API_ERR_UNINIT;
    }

    // flash all pending operations.
    norflash_api_flush_enable_all();
    do
    {
        ret = norflash_api_flush();
    } while (1 == ret);

    NORFLASH_API_TRACE(2,"flush done.");

    ret = hal_norflash_set_dual_chip_mode(mod_info->dev_id, dual_flash, dual_sec_reg);
    if(ret)
    {
        NORFLASH_API_TRACE(2,"set dual chip mode fail! mod_id = %d, ret = %d", mod_id, ret);
        return NORFLASH_API_ERR;
    }

    ret = norflash_api_init();
    if(ret)
    {
        NORFLASH_API_TRACE(2,"norflash api init fail! mod_id = %d, ret = %d", mod_id, ret);
        return NORFLASH_API_ERR;
    }

    NORFLASH_API_TRACE(2,"norflash_api_set_dual_chip_mode done.");
    return NORFLASH_API_OK;
}

void norflash_api_flash_operation_start(uint32_t addr)
{
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    if(norflash_api_get_dev_id_by_addr(addr) == HAL_FLASH_ID_1)
    {
        _norflash_api_mutex_wait();
    }
#endif
    // _norflash_api_int_lock(norflash_api_get_dev_id_by_addr(addr));
}

void norflash_api_flash_operation_end(uint32_t addr)
{
    // _norflash_api_int_unlock(norflash_api_get_dev_id_by_addr(addr));
#if defined(RTOS) && !defined(NORFLASH_API_FLUSH_IN_SLEEP_HOOK)
    if(norflash_api_get_dev_id_by_addr(addr) == HAL_FLASH_ID_1)
    {
        _norflash_api_mutex_release();
    }
#endif
}

#ifdef FLASH_SECURITY_REGISTER
static inline enum NORFLASH_API_RET_T RET_VALUE(enum HAL_NORFLASH_RET_T err_code)
{
	enum NORFLASH_API_RET_T ret;

	switch(err_code)
	{
		case HAL_NORFLASH_OK:
			ret = NORFLASH_API_OK;
			break;
		case HAL_NORFLASH_BAD_OP:
			ret = NORFLASH_API_ERR_REGISTRATION;
			break;
		case HAL_NORFLASH_BAD_ADDR:
			ret = NORFLASH_API_BAD_ADDR;
			break;
		case NORFLASH_API_BAD_LEN:
			ret = NORFLASH_API_BAD_LEN;
			break;
		default:
			ret = NORFLASH_API_ERR;
			break;
	}
	return ret;
}

enum NORFLASH_API_RET_T norflash_api_security_register_lock(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len)
{
    enum HAL_NORFLASH_RET_T err_code;
    enum NORFLASH_API_RET_T ret;
    MODULE_INFO* cur_mod_info;

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }

    _norflash_api_int_lock(id);
    cur_mod_info = norflash_api_info.cur_mod;
    if(cur_mod_info)
    {
        if(cur_mod_info->dev_id == id)
        {
            _flush_one_pending_opera(cur_mod_info, true);
        }
    }
    err_code = hal_norflash_security_register_lock(id, start_address, len);

    _norflash_api_int_unlock(id);

    ret = RET_VALUE(err_code);
    return ret;
}

enum NORFLASH_API_RET_T norflash_api_security_register_erase(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len)
{
    enum HAL_NORFLASH_RET_T err_code;
    enum NORFLASH_API_RET_T ret;
    MODULE_INFO* cur_mod_info;

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }

    _norflash_api_int_lock(id);
    cur_mod_info = norflash_api_info.cur_mod;
    if(cur_mod_info)
    {
        if(cur_mod_info->dev_id == id)
        {
            _flush_one_pending_opera(cur_mod_info, true);
        }
    }

    err_code = hal_norflash_security_register_erase(id, start_address, len);
    _norflash_api_int_unlock(id);

    ret = RET_VALUE(err_code);
    return ret;
}

enum NORFLASH_API_RET_T norflash_api_security_register_write(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len)
{
    enum HAL_NORFLASH_RET_T err_code;
    enum NORFLASH_API_RET_T ret;
    MODULE_INFO* cur_mod_info;

    if(!norflash_api_info.is_inited)
    {
        NORFLASH_API_TRACE(1,"%s: norflash_api uninit!",__func__);
        return NORFLASH_API_ERR_UNINIT;
    }

    _norflash_api_int_lock(id);
    cur_mod_info = norflash_api_info.cur_mod;
    if(cur_mod_info)
    {
        if(cur_mod_info->dev_id == id)
        {
            _flush_one_pending_opera(cur_mod_info, true);
        }
    }

    err_code = hal_norflash_security_register_write(id, start_address, buffer, len);
    _norflash_api_int_unlock(id);

    ret = RET_VALUE(err_code);
    return ret;
}

enum NORFLASH_API_RET_T norflash_api_security_register_read(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len)
{
    enum HAL_NORFLASH_RET_T err_code;
    enum NORFLASH_API_RET_T ret;

    _norflash_api_int_lock(id);
    err_code = hal_norflash_security_register_read(id, start_address, buffer, len);
    _norflash_api_int_unlock(id);

    ret = RET_VALUE(err_code);
    return ret;
}
#endif
