
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "tz_flash.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_dma.h"
#include "user_secure_boot.h"

#if 0
#define SEC_FLASH_TRACE  TRACE
#else
#define SEC_FLASH_TRACE(...)
#endif

static uint32_t _flash_total_size;
static uint32_t _flash_block_size;
static uint32_t _flash_sector_size;
//static uint32_t _flash_page_size;

static int _dual_enable;

#ifdef FLASH_REMAP
#define REMAP_ADDR_CHECK_IN                 1
#define REMAP_ADDR_CHECK_OUT                2
#define REMAP_ADDR_CHECK_INVALID            3
#define NORFLASH_API_ERR_REMAP              9

#define HAL_NORFLASH_REMAP_ID_TZ            HAL_NORFLASH_REMAP_ID_1

typedef struct
{
    bool is_used;
    uint32_t addr;
    uint32_t len;
    uint32_t remap_offset;
} FLASH_REMAP_CFG_T;
static FLASH_REMAP_CFG_T flash_remap_cfg[HAL_NORFLASH_REMAP_ID_QTY];
static bool _remap_status;
static enum HAL_NORFLASH_REMAP_ID_T _remap_id;

SRAM_TEXT_LOC static int _flash_remap_addr_check(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len, enum HAL_NORFLASH_REMAP_ID_T *premap_id)
{
    uint32_t remap_id;
    uint32_t remap_addr;
    uint32_t remap_len;
    uint32_t remap_offset;
    uint32_t start_addr;
    int ret = REMAP_ADDR_CHECK_OUT;

    start_addr = addr & (_flash_total_size-1);

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
            *premap_id = (enum HAL_NORFLASH_REMAP_ID_T)remap_id;
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

SRAM_TEXT_LOC static void _flash_remap_pre(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len)
{
    enum HAL_NORFLASH_RET_T ret;
    int check_result;

    if(id != HAL_FLASH_ID_0)
    {
        return;
    }

    _remap_id = HAL_NORFLASH_REMAP_ID_QTY;
    check_result = _flash_remap_addr_check(id, addr, len, &_remap_id);
    if(check_result == REMAP_ADDR_CHECK_IN)
    {
        SEC_FLASH_TRACE(3,"%s: INSIDE, _remap_id = %d.",__func__, _remap_id);
        _remap_status = hal_norflash_get_remap_status(id, _remap_id);
        if(!_remap_status)
        {
            SEC_FLASH_TRACE(3,"%s: remap_status not enabled.",__func__);
            ret = hal_norflash_enable_remap(id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable remap, ret = %d",
                __func__, ret);
        }
        else
        {
            SEC_FLASH_TRACE(3,"%s: remap_status enabled.",__func__);
            ret = hal_norflash_disable_remap(id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to disable remap, ret = %d",
                __func__, ret);
        }
        _remap_status = !_remap_status;
        return;
    }
    else if(check_result == REMAP_ADDR_CHECK_OUT)
    {
        _remap_id = HAL_NORFLASH_REMAP_ID_QTY;
        SEC_FLASH_TRACE(3,"%s: OUTSIDE.",__func__);
    }
    else
    {
        ASSERT(0,"%s: Address ranges bad!addr = 0x%x, len=0x%x",
            __func__, addr, len);
    }
}

SRAM_TEXT_LOC static void _flash_remap_post(enum HAL_FLASH_ID_T id)
{
    enum HAL_NORFLASH_RET_T ret;

    if(id != HAL_FLASH_ID_0)
    {
        return;
    }
    SEC_FLASH_TRACE(3,"%s: _remap_id = %d.", __func__, _remap_id);
    if(_remap_id < HAL_NORFLASH_REMAP_ID_QTY)
    {
        if(!_remap_status)
        {
            SEC_FLASH_TRACE(3,"%s: remap_status not enabled.", __func__);
            ret = hal_norflash_enable_remap(id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable remap, ret = %d",
                __func__, ret);
        }
        else
        {
            SEC_FLASH_TRACE(3,"%s: remap_status enabled.",__func__);
            ret = hal_norflash_disable_remap(id, _remap_id);
            ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to disable remap, ret = %d",
                __func__, ret);
        }
        _remap_status = !_remap_status;
        return;
    }
    _remap_id = HAL_NORFLASH_REMAP_ID_QTY;
}

SRAM_TEXT_LOC static int _flash_remap_config(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t remap_offset)
{
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_OK;

    SEC_FLASH_TRACE(3,"%s: remap_id = %d, addr = 0x%x, len = 0x%x, remap_offset = 0x%x.",
                       __func__, remap_id, addr, len, remap_offset);
    if(id != HAL_FLASH_ID_0)
    {
        return NORFLASH_API_ERR_REMAP;
    }
    flash_remap_cfg[remap_id].addr = addr & (_flash_total_size-1);
    flash_remap_cfg[remap_id].len = len;
    flash_remap_cfg[remap_id].remap_offset= remap_offset;
    flash_remap_cfg[remap_id].is_used = true;

    if(!hal_norflash_get_remap_status(id, remap_id))
    {
        SEC_FLASH_TRACE(3,"%s: remap status is disable.",__func__);
        ret = hal_norflash_config_remap(id, remap_id, addr, len, remap_offset);
        if(ret)
        {
            SEC_FLASH_TRACE(0, "%s: Failed to config remap(%d, 0x%x,0x%x,0x%x) total_size = 0x%x, ret = %d",
                              __func__, remap_id, addr, len, remap_offset,
                              hal_norflash_get_flash_total_size(id),
                              ret);
        }
        else
        {
            SEC_FLASH_TRACE(3,"%s: remap config done", __func__);
        }
    }
    else
    {
        SEC_FLASH_TRACE(3,"%s: remap status is enable", __func__);
    }

    return (int)ret;
}

POSSIBLY_UNUSED
SRAM_TEXT_LOC static uint32_t _flash_remap_get_real_addr(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len)
{
    enum HAL_NORFLASH_REMAP_ID_T remap_id = HAL_NORFLASH_REMAP_ID_TZ;
    int check_result;

    if(id != HAL_FLASH_ID_0)
    {
        return addr;
    }
    check_result = _flash_remap_addr_check(id, addr, len, &remap_id);
    if(check_result == REMAP_ADDR_CHECK_IN)
    {
        SEC_FLASH_TRACE(3,"%s: INSIDE, remap_id = %d.",__func__, remap_id);
        if(!hal_norflash_get_remap_status(id, remap_id))
        {
            SEC_FLASH_TRACE(3,"%s: remap_status not enabled.",__func__);
            return addr + flash_remap_cfg[remap_id].remap_offset;
        }
        else
        {
            SEC_FLASH_TRACE(3,"%s: remap_status enabled.",__func__);
            return addr;
        }

    }
    else if(check_result == REMAP_ADDR_CHECK_OUT)
    {
        SEC_FLASH_TRACE(3,"%s: OUTSIDE, remap_id = %d.",__func__, remap_id);
        return addr;
    }
    else
    {
        ASSERT(0,"%s: Address ranges bad!addr = 0x%x, len=0x%x",
            __func__, addr, len);
    }
    return addr;
}

#define _FLASH_REMAP_PRE                     _flash_remap_pre
#define _FLASH_REMAP_POST                    _flash_remap_post
#define _FLASH_REMAP_CONFIG                  _flash_remap_config
#define _FLASH_REMAP_GET_REAL_ADDR           _flash_remap_get_real_addr
#else // !FLASH_REMAP
#define _FLASH_REMAP_PRE(...)
#define _FLASH_REMAP_POST(...)
#define _FLASH_REMAP_CONFIG(...)
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

SRAM_TEXT_LOC static bool _flash_chip_dual_enable(enum HAL_FLASH_ID_T id)
{
    return _dual_enable == 0 ? false : true;
}

SRAM_TEXT_LOC static void _bp_init_2m_128m(enum HAL_FLASH_ID_T id, uint32_t total_size)
{
    uint32_t prot_size, unprot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    FLASH_BP_MAP_T *bp_map;

    multi = 0x1 << ((total_size/(0x800000)) % 0x2000000);
    if(_flash_chip_dual_enable(id))
    {
        multi = multi >> (total_size/0x2000000);
    }
    // SEC_FLASH_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
    if(id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    flash_bp_numer = ind;
#endif
}

// GD25LE255E BP without CMP.
SRAM_TEXT_LOC static void _bp_init_256m(enum HAL_FLASH_ID_T id, uint32_t total_size)
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

    // SEC_FLASH_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
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
       if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
    if(id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    flash_bp_numer = ind;
#endif
}

// P25Q256L BP
SRAM_TEXT_LOC static void _bp_init_puya256m(enum HAL_FLASH_ID_T id, uint32_t total_size)
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

    // SEC_FLASH_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
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
       if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
    if(id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    flash_bp_numer = ind;
#endif
}

// GD25LQ255E BP with CMP
SRAM_TEXT_LOC static void _bp_init_gd256m_with_cmp(enum HAL_FLASH_ID_T id, uint32_t total_size)
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

    // SEC_FLASH_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
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
       if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
        if(_flash_chip_dual_enable(id))
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
            if(_flash_chip_dual_enable(id))
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
    if(id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    flash_bp_numer = ind;
#endif
}


SRAM_TEXT_LOC static void _bp_init_gd25d20_40(enum HAL_FLASH_ID_T id, uint32_t total_size)
{
    uint32_t unprot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    FLASH_BP_MAP_T *bp_map;

    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // SEC_FLASH_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
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
        if(_flash_chip_dual_enable(id))
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
    if(id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    flash_bp_numer = ind;
#endif
}

SRAM_TEXT_LOC static void _bp_init_xt25q08b(enum HAL_FLASH_ID_T id, uint32_t total_size)
{
    uint32_t prot_size;
    uint32_t ind = 0;
    uint32_t multi;
    uint32_t n,i;
    FLASH_BP_MAP_T *bp_map;

    multi = 0x1 << ((total_size/(0x800000)) % 2000000);

    // SEC_FLASH_TRACE(1, "%s: total_size = 0x%x, mults = 0x%x", __func__, total_size, mults);
#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
    {
        bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    }
    else
    {
        bp_map = (FLASH_BP_MAP_T*)flash1_bp_map;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
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
    if(id == HAL_FLASH_ID_0)
    {
        flash_bp_numer = ind;
    }
    else
    {
        flash1_bp_numer = ind;
    }
#else
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    flash_bp_numer = ind;
#endif
}

SRAM_TEXT_LOC static void _norflash_api_protection_bp_init(enum HAL_FLASH_ID_T id, uint32_t total_size)
{
    bool protection_enable = false;
    uint8_t flash_id[3];
#ifdef FLASH_GD256M_SR_WHIT_CMP
    bool with_cmp = true;
#else
    bool with_cmp = false;
#endif

    if(_flash_chip_dual_enable(id))
    {
        if(total_size >= 0x20000 && total_size <= 0x2000000)
        {
            protection_enable = true;
            _bp_init_2m_128m(id, total_size);
        }
    }
    else
    {
        hal_norflash_get_id(id, flash_id, ARRAY_SIZE(flash_id));
        if(flash_id[0] == 0xC8 && flash_id[1] == 0x40 && (flash_id[2] == 0x12 || flash_id[2] == 0x13))
        {
            protection_enable = true;
            _bp_init_gd25d20_40(id, total_size);
        }
        if(flash_id[0] == 0x0B && flash_id[1] == 0x60 && flash_id[2] == 0x14)
        {
            protection_enable = true;
            _bp_init_xt25q08b(id, total_size);
        }
        else
        {
            if(total_size >= 0x10000 && total_size < 0x2000000)
            {
                protection_enable = true;
                _bp_init_2m_128m(id, total_size);
            }
            else if(total_size == 0x2000000)
            {
                protection_enable = true;
                if(flash_id[0] == 0x85)
                {
                    _bp_init_puya256m(id, total_size);
                }
                else
                {
                    if(with_cmp)
                    {
                        _bp_init_gd256m_with_cmp(id, total_size);
                    }
                    else
                    {
                        _bp_init_256m(id, total_size);
                    }
                }
            }
        }
    }

    ASSERT(protection_enable,"%s: Unkown protection BP defining! total_size = 0x%x", __func__, total_size);

}

#ifdef FLASH_PROTECTION_BOOT_SECTION_FIRST
SRAM_TEXT_LOC static bool _with_boot_section(enum HAL_FLASH_ID_T id, uint32_t start_addr, uint32_t end_addr)
{
#ifdef OTA_CODE_OFFSET
    uint32_t boot_start;
    uint32_t boot_end;

    boot_start = 0x0;
    boot_end = OTA_CODE_OFFSET & (_flash_total_size - 1);

    if(id == HAL_FLASH_ID_0)
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

SRAM_TEXT_LOC static void _protection_part(enum HAL_FLASH_ID_T id, uint32_t start_addr, uint32_t size)
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
    real_addr = _FLASH_REMAP_GET_REAL_ADDR(id, start_addr, size);
    addr = (real_addr & (_flash_total_size-1));
#else
    real_addr = start_addr;
    addr = (real_addr & (_flash_total_size-1));
#endif

#if defined(FLASH1_CTRL_BASE) && defined(USE_MULTI_FLASH)
    if(id == HAL_FLASH_ID_0)
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
    ASSERT(id == HAL_FLASH_ID_0, "%s: Invalid id: 0x%x ", __func__, id);
    bp_map = (FLASH_BP_MAP_T*)flash_bp_map;
    bp_map_number = flash_bp_numer;
#endif

    for(i = 0; i < bp_map_number; i++)
    {
        if((addr >= bp_map[i].end_addr
            || addr + size <= bp_map[i].start_addr)
#ifdef FLASH_PROTECTION_BOOT_SECTION_FIRST
            && _with_boot_section(id, bp_map[i].start_addr,bp_map[i].end_addr)
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

    SEC_FLASH_TRACE(2,"%s: w: (0x%x -- 0x%x) bp: 0x%x(0x%x -- 0x%x).",
           __func__,
           addr, addr + size,
           bp,
           bp_map[bp_i].start_addr, bp_map[bp_i].end_addr);
    ASSERT(bp != FLASH_BP_ALL, "%s: protection area undefined! addr:0x%x -- 0x%x",
           __func__, addr, addr + size );

    result = hal_norflash_set_protection(id, bp);
    ASSERT(result == HAL_NORFLASH_OK,
            "%s: set protection fail! ret = %d",
            __func__, result);

}

SRAM_TEXT_LOC static void _protection_all(enum HAL_FLASH_ID_T id)
{
    enum HAL_NORFLASH_RET_T result;
    uint32_t bp = FLASH_BP_ALL;

    result = hal_norflash_set_protection(id, bp);
    ASSERT(result == HAL_NORFLASH_OK,
            "%s: set protection fail! id = %d ret = %d",
            __func__, id, result);
}

#else // FLASH_PROTECTION

SRAM_TEXT_LOC static void _protection_part(enum HAL_FLASH_ID_T id, uint32_t start_addr, uint32_t size){}
SRAM_TEXT_LOC static void _protection_all(enum HAL_FLASH_ID_T id){}

#endif // FLASH_PROTECTION


SRAM_TEXT_LOC static uint32_t _get_opera_size(uint32_t start_addr, uint32_t total_size, uint32_t done_size, uint32_t align_size)
{
    uint32_t opera_size;

    if(((start_addr + done_size) & (align_size - 1)) > 0)
    {
        if((total_size - done_size) > align_size - ((start_addr + done_size) & (align_size - 1)))
        {
            opera_size = align_size - ((start_addr + done_size) & (align_size - 1));
        }
        else
        {
            opera_size = (total_size - done_size);
        }

    }
    else
    {
        if((total_size - done_size) > align_size)
        {
            opera_size = align_size;
        }
        else
        {
            opera_size = (total_size - done_size) ;
        }
    }
    return opera_size;
}

///------------------------------------------------------------------------------------------------
///------------------------ API---------------------------------------------------------------------

SRAM_TEXT_LOC int sec_flash_init(enum HAL_FLASH_ID_T id)
{
    int ret;
#ifdef FLASH_SUSPEND
    const int16_t *irq = NULL;
    uint32_t irq_cnt = 0;
#endif

    ret = hal_norflash_get_size(id, &_flash_total_size, &_flash_block_size, &_flash_sector_size, NULL);
    if(ret != HAL_NORFLASH_OK)
    {
        SEC_FLASH_TRACE(1, "%s: get size fail!, ret = %d", __func__, (int)ret);
        return 1;
    }

    ret = hal_norflash_get_dual_chip_mode(id, &_dual_enable, NULL);
    if(ret != HAL_NORFLASH_OK)
    {
        SEC_FLASH_TRACE(1, "%s: get dual chip mode fail!, ret = %d", __func__, (int)ret);
        return 2;
    }

#ifdef FLASH_SUSPEND
    hal_dma_get_irq_list(&irq, &irq_cnt);
    for(uint32_t i = 0; i < irq_cnt; i++)
    {
        hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, irq[i], true);
    }
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA_IRQn, true);
    hal_norflash_suspend_check_irq(HAL_FLASH_ID_0, ISDATA1_IRQn, true);
#endif

#ifdef FLASH_PROTECTION
    _norflash_api_protection_bp_init(id, _flash_total_size);
#endif

#ifdef FLASH_REMAP
    SEC_FLASH_TRACE(1, "%s OTA_CODE_OFFSET = 0x%x, OTA_REMAP_OFFSET = 0x%x", __func__, OTA_CODE_OFFSET, OTA_REMAP_OFFSET);
    _flash_remap_config(HAL_FLASH_ID_0, HAL_NORFLASH_REMAP_ID_TZ, OTA_CODE_OFFSET, OTA_REMAP_OFFSET, OTA_REMAP_OFFSET);
    // TODO: set mults-flash-remap.
#endif

    return 0;
}

SRAM_TEXT_LOC int sec_flash_erase(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len, uint32_t suspend)
{
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_OK;
    int s_ret = 0;
    uint32_t s_len;
    uint32_t b_len;
    uint32_t i;
    // POSSIBLY_UNUSED uint32_t start_time = hal_sys_timer_get();
    uint32_t start_addr = addr;
    uint32_t offs;
    uint32_t lock;

    start_addr = addr;
    offs = (addr & (_flash_total_size - 1));
    if(((addr & (_flash_sector_size - 1)) == 0 && (len & (_flash_sector_size - 1)) != 0))
    {
        SEC_FLASH_TRACE(0,
            "%s: No sec size alignment! addr = 0x%x, len = 0x%x",
            __func__,
            addr,
            len);
        s_ret = 1;
        goto _func_fail;
    }
    SEC_FLASH_TRACE(1, "%s: id = %d addr = 0x%x, len = 0x%x",
          __func__, id, addr, len);

    // erase pre-sectors.
    if(len > ((_flash_block_size-1) & offs))
    {
        s_len = ((_flash_block_size-1) & offs);
    }
    else
    {
        s_len = len;
    }

    SEC_FLASH_TRACE(1, "%s: pre sectors erase: start_addr = 0x%x, s_len = 0x%x",
          __func__, start_addr, s_len);

    for(i = 0; i < s_len/_flash_sector_size; i++)
    {
         SEC_FLASH_TRACE(1,"%s: pre sector(0x%x) erase", __func__, start_addr);
         lock = int_lock_global();
        _protection_part(id, start_addr, _flash_sector_size);
        _FLASH_REMAP_PRE(id, start_addr,_flash_sector_size);
        ret = hal_norflash_erase_suspend(id,
                                 start_addr,
                                 _flash_sector_size,
                                 suspend);
        _FLASH_REMAP_POST(id);
        int_unlock_global(lock);
        do{
            if (HAL_NORFLASH_OK == ret)
            {
                SEC_FLASH_TRACE(1,"%s: ok!", __func__);
                _protection_all(id);
                break;
            }
            else if (HAL_NORFLASH_SUSPENDED == ret)
            {
                SEC_FLASH_TRACE(1,"Flash suspend.");
                lock = int_lock_global();
                _FLASH_REMAP_PRE(id, start_addr,_flash_sector_size);
                ret = hal_norflash_erase_resume(id, suspend);
                _FLASH_REMAP_POST(id);
                int_unlock_global(lock);
            }
            else
            {
                SEC_FLASH_TRACE(1, "pre sector erase failed. ret=%d, start_addr:0x%x, addr:0x%x, s_size: 0x%x",
                        ret, start_addr, addr, _flash_sector_size);
                s_ret = 2;
                goto _func_fail;
            }
        }while(1);
        start_addr += _flash_sector_size;
    }

    // erase blocks.
    b_len = ((len - s_len)/_flash_block_size)*_flash_block_size ;
    SEC_FLASH_TRACE(1, "%s: blocks erase: start_addr = 0x%x, b_len = 0x%x",
          __func__, start_addr, b_len);
    for(i = 0; i < b_len/_flash_block_size; i++)
    {
        SEC_FLASH_TRACE(1,"%s: sector(0x%x) erase", __func__, start_addr);
        lock = int_lock_global();
         _protection_part(id, start_addr, _flash_block_size);
        _FLASH_REMAP_PRE(id, start_addr,_flash_block_size);
        ret = hal_norflash_erase_suspend(id,
                                 start_addr,
                                 _flash_block_size,
                                 suspend);
        _FLASH_REMAP_POST(id);
        int_unlock_global(lock);

        do {
            if (HAL_NORFLASH_OK == ret)
            {
                SEC_FLASH_TRACE(1,"%s: block(0x%x) erase ok!", __func__, start_addr);
                _protection_all(id);
                break;
            }
            else if (HAL_NORFLASH_SUSPENDED == ret)
            {
                SEC_FLASH_TRACE(1,"Flash suspend.");
                lock = int_lock_global();
                _FLASH_REMAP_PRE(id, start_addr,_flash_block_size);
                ret = hal_norflash_erase_resume(id, suspend);
                _FLASH_REMAP_POST(id);
                int_unlock_global(lock);
            }
            else
            {
                SEC_FLASH_TRACE(0, "block erase failed. ret=%d, start_addr:0x%x, addr:0x%x, b_size: 0x%x",
                       ret, start_addr, addr, _flash_block_size);
                s_ret = 3;
                goto _func_fail;
            }
        }while(1);
        start_addr += _flash_block_size;
    }

    // erase post sectors.
    s_len = len - s_len - b_len;
    SEC_FLASH_TRACE(1, "%s: post sectors erase: start_addr = 0x%x, s_len = 0x%x",
          __func__, start_addr, s_len);
    for(i = 0; i < s_len/_flash_sector_size; i++)
    {
        SEC_FLASH_TRACE(1,"%s: sector(0x%x) erase", __func__, start_addr);
        lock = int_lock_global();
        _protection_part(id, start_addr, _flash_sector_size);
        _FLASH_REMAP_PRE(id, start_addr,_flash_sector_size);
        ret = hal_norflash_erase_suspend(id,
                                     start_addr,
                                     _flash_sector_size,
                                     suspend);
        _FLASH_REMAP_POST(id);
        int_unlock_global(lock);

        do {
            if (HAL_NORFLASH_OK == ret)
            {
                SEC_FLASH_TRACE(1,"%s: sector(0x%x) erase ok!", __func__, start_addr);
                 _protection_all(id);
                break;
            }
            else if (HAL_NORFLASH_SUSPENDED == ret)
            {
                SEC_FLASH_TRACE(1,"Flash suspend.");
                lock = int_lock_global();
                _FLASH_REMAP_PRE(id, start_addr,_flash_sector_size);
                ret = hal_norflash_erase_resume(id, suspend);
                _FLASH_REMAP_POST(id);
                int_unlock_global(lock);

            }
            else
            {
                SEC_FLASH_TRACE(0, "post sector erase failed. ret=%d, start_addr:0x%x, addr:0x%x, s_size: 0x%x",
                       ret, start_addr, addr, _flash_sector_size);
                s_ret = 4;
                goto _func_fail;
            }
        }while(1);
        start_addr += _flash_sector_size;
    }

    // SEC_FLASH_TRACE(1,"%s: erase: 0x%x,0x%x, time= %d(ms)done",
    //     __func__, addr, len, TICKS_TO_MS(hal_sys_timer_get() - start_time));
    return 0;

_func_fail:
    SEC_FLASH_TRACE(1,"%s: erase: 0x%x,0x%x failed, s_ret = %d",
        __func__, addr, len, s_ret);
    return s_ret;
}

SRAM_TEXT_LOC int sec_flash_program(enum HAL_FLASH_ID_T id,
                       uint32_t addr,
                       uint8_t *buffer,
                       uint32_t len,
                       uint32_t suspend)
{
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_OK;
    uint32_t start_addr;
    uint32_t write_len;
    uint32_t written_len = 0;
    uint32_t lock;

    start_addr = addr;

    SEC_FLASH_TRACE(1, "%s id = %d, addr = 0x%x, len = %d, suspend = %d", __func__, id, addr, len, suspend);

    while(written_len < len)
    {
        write_len = _get_opera_size(start_addr, len, written_len, _flash_sector_size);
        lock = int_lock_global();
        _protection_part(id, start_addr + written_len,  write_len);
        _FLASH_REMAP_PRE(id, start_addr + written_len,  write_len);
        ret = hal_norflash_write_suspend(id,
                                         start_addr + written_len,
                                         buffer + written_len,
                                         write_len,
                                         suspend);
        _FLASH_REMAP_POST(id);
        int_unlock_global(lock);

        do
        {

            if (HAL_NORFLASH_OK == ret)
            {
                SEC_FLASH_TRACE(1,"%s: write(0x%x, 0x%x) ok!", __func__, start_addr + written_len, write_len);
                 _protection_all(id);
                written_len += write_len;
                break;
            }
            else if (HAL_NORFLASH_SUSPENDED == ret)
            {
                SEC_FLASH_TRACE(1,"%s:buffer full! To flush it.", __func__);
                lock = int_lock_global();
                _FLASH_REMAP_PRE(id, start_addr + written_len,  write_len);
                ret = hal_norflash_write_resume(id, suspend);
                _FLASH_REMAP_POST(id);
                int_unlock_global(lock);
            }
            else
            {
                SEC_FLASH_TRACE(1, "%s: write failed. ret = %d", __func__, ret);
                goto _func_fail;
            }
        } while (1);
    }

    SEC_FLASH_TRACE(1,"%s: write: 0x%x,0x%x done.", __func__, start_addr, len);
    return 0;

_func_fail:
    return 1;
}

SRAM_TEXT_LOC int sec_flash_read(enum HAL_FLASH_ID_T id,
                    uint32_t addr,
                    uint8_t *buffer,
                    uint32_t len)
{
    uint32_t start_addr;
    uint32_t read_len;
    uint32_t has_read_len = 0;
    uint32_t lock;

    start_addr = addr;
    SEC_FLASH_TRACE(1, "%s id = %d, addr = 0x%x, len = %d", __func__, id, addr, len);

    while(has_read_len < len)
    {
        read_len = _get_opera_size(start_addr, len, has_read_len, _flash_sector_size);
        SEC_FLASH_TRACE(1, "%s read addr = 0x%x, len = 0x%x", __func__, start_addr + has_read_len, read_len);

        lock = int_lock_global();
        _FLASH_REMAP_PRE(id, start_addr + has_read_len,  read_len);
        hal_norflash_read(id, start_addr + has_read_len, &buffer[has_read_len], read_len);
        _FLASH_REMAP_POST(id);
        int_unlock_global(lock);
        has_read_len += read_len;
    }

    return 0;
}

SRAM_TEXT_LOC int sec_flash_remap_config(enum HAL_FLASH_ID_T id,
                          enum HAL_NORFLASH_REMAP_ID_T remap_id,
                          uint32_t addr,
                          uint32_t len,
                          uint32_t remap_offset)
{
#ifdef FLASH_REMAP
    return _flash_remap_config(id, remap_id, addr, len, remap_offset);
#else
    return 0;
#endif
}

bool sec_flash_is_remap_enabled(void)
{
#ifdef FLASH_REMAP
    return true;
#else
    return false;
#endif  
}

bool sec_flash_check_signature(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len)
{
    uint32_t real_addr = 0;
    uint32_t start_addr = 0;
    int ret = 0;

#ifdef FLASH_REMAP
    real_addr = _FLASH_REMAP_GET_REAL_ADDR(id, addr, len);
    start_addr = (real_addr & (_flash_total_size-1));
#else
    real_addr = start_addr;
    start_addr = (real_addr & (_flash_total_size-1));
#endif

    SEC_FLASH_TRACE(1, "%s id = %d, addr = 0x%x, len = %d", __func__, id, start_addr, len);
    ret = user_secure_boot_ota_check(start_addr);

    return ret == 0;
}
