#include "heap_api.h"
#include "hal_trace.h"
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_overlay.h"

#define SYS_MEM_POOL_RESERVED_SIZE          512

extern uint8_t __mem_pool_start__[];
extern uint8_t __mem_pool_end__[];

static uint32_t syspool_size = 0;
static uint32_t syspool_used = 0;
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
extern uint8_t __cp_bss_sram_end[];
extern uint8_t __cp_mem_pool_start__[];
extern uint8_t __cp_mem_pool_end__[];
#endif

uint32_t syspool_original_size(void)
{
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(UNIFY_HEAP_ENABLED)
    if(hal_overlay_is_used()){
        uint8_t *overlay_addr = (uint8_t *)hal_overlay_get_data_free_address();
        if(overlay_addr == NULL){
            ASSERT((__cp_mem_pool_start__ + SYS_MEM_POOL_RESERVED_SIZE) < __cp_mem_pool_end__,
                "%s: cp mem pool size too small: start=%p end=%p reserved_size=%u",
                __func__, __cp_mem_pool_start__, __cp_mem_pool_end__, SYS_MEM_POOL_RESERVED_SIZE);
            return __cp_mem_pool_end__ - __cp_mem_pool_start__ - SYS_MEM_POOL_RESERVED_SIZE;
        }else{
            ASSERT((overlay_addr + SYS_MEM_POOL_RESERVED_SIZE) < __cp_mem_pool_end__,
                "%s: overlay mem pool size too small: start=%p end=%p reserved_size=%u",
                __func__, overlay_addr, __cp_mem_pool_end__, SYS_MEM_POOL_RESERVED_SIZE);
            return __cp_mem_pool_end__ - overlay_addr - SYS_MEM_POOL_RESERVED_SIZE;
        }
    }else{
        ASSERT((__cp_bss_sram_end + SYS_MEM_POOL_RESERVED_SIZE) < __cp_mem_pool_end__,
            "%s: cp mem pool size too small: start=%p end=%p reserved_size=%u",
            __func__, __cp_bss_sram_end, __cp_mem_pool_end__, SYS_MEM_POOL_RESERVED_SIZE);
        return __cp_mem_pool_end__ - __cp_bss_sram_end - SYS_MEM_POOL_RESERVED_SIZE;
    }
#else
    ASSERT((__mem_pool_start__ + SYS_MEM_POOL_RESERVED_SIZE) < __mem_pool_end__,
        "%s: mem pool size too small: start=%p end=%p reserved_size=%u",
        __func__, __mem_pool_start__, __mem_pool_end__, SYS_MEM_POOL_RESERVED_SIZE);
    return __mem_pool_end__ - __mem_pool_start__ - SYS_MEM_POOL_RESERVED_SIZE;
#endif
}

void syspool_init_specific_size(uint32_t size)
{
    syspool_size = syspool_original_size();
    if(size<syspool_size)
        syspool_size = size;
    syspool_used = 0;
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(UNIFY_HEAP_ENABLED)
    if(hal_overlay_is_used()){
        uint8_t *overlay_addr = (uint8_t *)hal_overlay_get_data_free_address();
        if(overlay_addr == NULL){
            memset(__cp_mem_pool_start__,0,syspool_size);
            TRACE(2,"syspool_init: %p,0x%x",__cp_mem_pool_start__,syspool_size);
        }else{
            memset(overlay_addr,0,syspool_size);
            TRACE(2,"syspool_init: %p,0x%x",overlay_addr,syspool_size);
        }
    }else{
        memset(__cp_bss_sram_end,0,syspool_size);
        TRACE(2,"syspool_init: %p,0x%x",__cp_bss_sram_end,syspool_size);
    }
#else
    memset(__mem_pool_start__,0,syspool_size);
    TRACE(2,"syspool_init: %p,0x%x", __mem_pool_start__,syspool_size);
#endif
}

void syspool_init(void)
{
    syspool_init_specific_size(syspool_original_size());
}

uint8_t* syspool_start_addr(void)
{
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(UNIFY_HEAP_ENABLED)
    if(hal_overlay_is_used()){
        uint8_t *overlay_addr = (uint8_t *)hal_overlay_get_data_free_address();
        if(overlay_addr == NULL){
            return __cp_mem_pool_start__;
        }else{
            return overlay_addr;
        }
    }else{
        return __cp_bss_sram_end;
    }
#else
    return __mem_pool_start__;
#endif
}

uint32_t syspool_total_size(void)
{
    return syspool_size;
}

int syspool_free_size()
{
    return syspool_size - syspool_used;
}

int syspool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    buff_size_free = syspool_free_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }
    ASSERT (size <= buff_size_free, "System pool in shortage! To allocate size %d but free size %d.",
        size, buff_size_free);
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(UNIFY_HEAP_ENABLED)
    if(hal_overlay_is_used()){
        uint8_t *overlay_addr = (uint8_t *)hal_overlay_get_data_free_address();
        if(overlay_addr == NULL){
            *buff = __cp_mem_pool_start__ + syspool_used;
        }else{
            *buff = overlay_addr + syspool_used;
        }
    }else{
        *buff = __cp_bss_sram_end + syspool_used;
    }
#else
    *buff = __mem_pool_start__ + syspool_used;
#endif
    syspool_used += size;
    TRACE(0, "[%s] ptr=%p size=%u free=%u user=%p", __func__, *buff, size, buff_size_free, __builtin_return_address(0));
    return buff_size_free;
}

int syspool_get_available(uint8_t **buff)
{
    uint32_t buff_size_free;
    buff_size_free = syspool_free_size();

    TRACE(2, "[%s] free=%d", __func__, buff_size_free);
    if (buff_size_free < 8)
        return -1;
    if (buff != NULL)
    {
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(UNIFY_HEAP_ENABLED)
        if(hal_overlay_is_used()){
            uint8_t *overlay_addr = (uint8_t *)hal_overlay_get_data_free_address();
            if(overlay_addr == NULL){
                *buff = __cp_mem_pool_start__ + syspool_used;
            }else{
                *buff = overlay_addr + syspool_used;
            }
        }else{
            *buff = __cp_bss_sram_end + syspool_used;
        }
#else
        *buff = __mem_pool_start__ + syspool_used;
#endif
        syspool_used += buff_size_free;
    }
    return buff_size_free;
}

void *syspool_malloc(uint32_t size)
{
    uint8_t *pBuf = NULL;

    syspool_get_buff(&pBuf, size);

    return (void *)pBuf;
}

void *syspool_calloc(uint32_t num, uint32_t size)
{
    uint8_t *pBuf = NULL;

    syspool_get_buff(&pBuf, num * size);
    memset(pBuf, 0, num * size);

    return (void *)pBuf;
}

static void *pool_malloc(size_t size)
{
    void *ptr = NULL;
    syspool_get_buff((uint8_t **)&ptr, size);
    ASSERT(ptr, "[%s] no memory", __FUNCTION__);
    return ptr;
}

static void *pool_calloc(size_t nmemb, size_t size)
{
    if (size == 0)
        return NULL;

    void *ptr = pool_malloc(nmemb * size);

    if (ptr)
    {
        memset(ptr, 0 , nmemb * size);
    }

    return ptr;
}

static void pool_free(void *p)
{
}

static custom_allocator allocator = {
    .malloc = pool_malloc,
    .calloc = pool_calloc,
    .free = pool_free,
};

custom_allocator *pool_allocator(void)
{
    return &allocator;
}

#if defined(A2DP_LDAC_ON)
int syspool_force_used_size(uint32_t size)
{
    return syspool_used  = size;
}
#endif
#if !defined(UNIFY_HEAP_ENABLED)
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)

static uint32_t cp_pool_size = 0;
static uint32_t cp_pool_used = 0;

void cp_pool_init(void)
{
    cp_pool_size = __cp_mem_pool_end__ - __cp_mem_pool_start__;
    cp_pool_used = 0;
    TRACE(5,"[CP] %s start %p end %p size 0x%x %d",__FUNCTION__,
            __cp_mem_pool_start__,
            __cp_mem_pool_end__,cp_pool_size,cp_pool_size);
    if (cp_pool_size) {
        memset(__cp_mem_pool_start__, 0, cp_pool_size);
    }
}

int cp_pool_free_size(void)
{
    return cp_pool_size - cp_pool_used;
}

int cp_pool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    size = (size + 3) & ~3;
    buff_size_free = cp_pool_free_size();

    if (size <= buff_size_free) {
        *buff = __cp_mem_pool_start__ + cp_pool_used;
        cp_pool_used += size;
    } else {
        *buff = NULL;
    }
    TRACE(5,"[CP] %s get %d used %d free 0x%x %d",__FUNCTION__,
            size,cp_pool_used,buff_size_free,buff_size_free);
    return buff_size_free;
}

uint8_t* cp_pool_start_addr(void)
{
    return __cp_mem_pool_start__;
}

uint32_t cp_pool_total_size(void)
{
    return cp_pool_size;
}
#endif
#endif

#if defined(A2DP_DECODER_CROSS_CORE) || defined(GAF_CODEC_CROSS_CORE) || defined(APP_MCPP_SRV)
#define OFF_BTH_SYS_MEM_POOL_RESERVED_SIZE          512

extern uint8_t __HeapLimit[];
extern uint8_t __StackLimit[];

static uint32_t off_bth_syspool_size = 0;
static uint32_t off_bth_syspool_used = 0;
static uint8_t off_bth_syspool_user_bitmap = 0;
static off_bth_user_callback off_bth_user_cb[SYSPOOL_USER_MAX];

uint32_t off_bth_syspool_original_size(void)
{
    ASSERT((__HeapLimit + OFF_BTH_SYS_MEM_POOL_RESERVED_SIZE) < __StackLimit,
        "%s: off bth mem pool size too small: start=%p end=%p reserved_size=%u",
        __func__, __HeapLimit, __StackLimit, OFF_BTH_SYS_MEM_POOL_RESERVED_SIZE);
    return __StackLimit - __HeapLimit - OFF_BTH_SYS_MEM_POOL_RESERVED_SIZE;
}

void off_bth_syspool_init(off_bth_syspool_user user, off_bth_user_callback cb)
{
    ASSERT(user < SYSPOOL_USER_MAX, "%s, user:%d" , __func__, user);
    uint32_t lock = int_lock();
    if (0 == off_bth_syspool_user_bitmap)
    {
        off_bth_syspool_size = off_bth_syspool_original_size();
        off_bth_syspool_used = 0;
        // memset(__HeapLimit, 0, off_bth_syspool_size);
    }
    off_bth_syspool_user_bitmap |= (1 << user);
    off_bth_user_cb[user] = cb;
    int_unlock(lock);
    TRACE(2,"%s,%p,size:0x%x,user:%d,map:0x%x", __func__, __HeapLimit,
        off_bth_syspool_size, user, off_bth_syspool_user_bitmap);
}

void off_bth_syspool_deinit(off_bth_syspool_user user)
{
    uint32_t lock = int_lock();
    off_bth_syspool_user_bitmap &= ~(1 << user);
    if (0 == off_bth_syspool_user_bitmap) {
        for (uint8_t i = 0; i < SYSPOOL_USER_MAX; i++) {
            if (off_bth_user_cb[i]) {
                off_bth_user_cb[i]();
            }
        }
    }
    int_unlock(lock);

    TRACE(0, "%s,user:%d, map:0x%x", __func__, user, off_bth_syspool_user_bitmap);
}

void off_bth_syspool_init_specific_size(uint32_t size)
{
    off_bth_syspool_size = off_bth_syspool_original_size();
    off_bth_syspool_used = 0;
    TRACE(2, "off bth syspool_init_specific_size: %d/%d",size, off_bth_syspool_size);
    if(size<off_bth_syspool_size)
        off_bth_syspool_size = size;
    memset(__HeapLimit,0,off_bth_syspool_size);
    TRACE(2, "off bth syspool_init_specific_size: %p,0x%x", __HeapLimit, size);
}

int off_bth_syspool_free_size(void)
{
    return off_bth_syspool_size - off_bth_syspool_used;
}

int off_bth_syspool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    buff_size_free = off_bth_syspool_free_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }
    ASSERT (size <= buff_size_free, "off bth system pool in shortage! To allocate size %d but free size %d.",
        size, buff_size_free);
    *buff = __HeapLimit + off_bth_syspool_used;
    off_bth_syspool_used += size;
    TRACE(0, "[%s] ptr=%p size=%u free=%u user=%p", __func__, *buff, size, buff_size_free, __builtin_return_address(0));
    return buff_size_free;
}

void *off_bth_syspool_malloc(uint32_t size)
{
    uint8_t *pBuf = NULL;

    off_bth_syspool_get_buff(&pBuf, size);

    return (void *)pBuf;
}

void *off_bth_syspool_calloc(uint32_t num, uint32_t size)
{
    uint8_t *pBuf = NULL;

    off_bth_syspool_get_buff(&pBuf, num * size);
    memset(pBuf, 0, num * size);

    return (void *)pBuf;
}
#endif

#if BLE_BIS_TRANSPORT
extern uint8_t __ble_bis_tran_mem_pool_start__[];
extern uint8_t __ble_bis_tran_mem_pool_end__[];

static uint32_t ble_bis_tran_syspool_size = 0;
static uint32_t ble_bis_tran_syspool_used = 0;

uint32_t ble_bis_tran_syspool_get_original_size(void)
{
    ASSERT(__ble_bis_tran_mem_pool_start__ < __ble_bis_tran_mem_pool_end__,
        "%s: mem pool size too small: start=%p end=%p ",
        __func__, __ble_bis_tran_mem_pool_start__, __ble_bis_tran_mem_pool_end__);
    return __ble_bis_tran_mem_pool_end__ - __ble_bis_tran_mem_pool_start__;
}

void ble_bis_tran_syspool_init(void)
{
    ble_bis_tran_syspool_size = ble_bis_tran_syspool_get_original_size();
    ble_bis_tran_syspool_used = 0;
    memset(__ble_bis_tran_mem_pool_start__,0,ble_bis_tran_syspool_size);
    TRACE(2,"syspool_init: %p,0x%x",__ble_bis_tran_mem_pool_start__,ble_bis_tran_syspool_size);
}

uint32_t ble_bis_tran_syspool_get_free_size(void)
{
    return ble_bis_tran_syspool_size - ble_bis_tran_syspool_used;
}

uint32_t ble_bis_tran_syspool_get_buff(uint8_t **buff, uint32_t size)
{
    uint32_t buff_size_free;

    buff_size_free = ble_bis_tran_syspool_get_free_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }
    ASSERT (size <= buff_size_free, "System pool in shortage! To allocate size %d but free size %d.",
        size, buff_size_free);
    *buff = __ble_bis_tran_mem_pool_start__ + ble_bis_tran_syspool_used;
    ble_bis_tran_syspool_used += size;
    TRACE(0, "[%s] ptr=%p size=%u free=%u user=%p", __func__, *buff, size, buff_size_free, __builtin_return_address(0));
    return buff_size_free;
}
#endif
