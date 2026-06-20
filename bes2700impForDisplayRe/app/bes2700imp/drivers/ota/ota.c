/* ota.c — in-system flash update. [OK] verified: erase+write+verify a spare sector PASS.
 * The one non-obvious requirement: NOR sectors are write-protected by default, so
 * hal_norflash_disable_protection() MUST be called first (else erase/write return OK but
 * do nothing — NOR can only clear 1->0, so an un-erased sector then fails verify). */
#include "ota.h"
#include "board_config.h"
#include "hal_norflash.h"
#include "cmsis.h"          /* int_lock / int_unlock */

#define SECTOR          4096u
#define FNC(off)        ((volatile uint8_t *)(BRD_FLASH_NC_BASE + (off)))   /* non-cached read */

static uint32_t s_off, s_total, s_done;

void ota_read(uint32_t off, uint8_t *buf, uint32_t len)
{
    volatile uint8_t *f = FNC(off);
    for (uint32_t i = 0; i < len; i++) buf[i] = f[i];
}

static ota_ret_t erase_range(uint32_t off, uint32_t len)
{
    uint32_t a = off & ~(SECTOR - 1);
    uint32_t end = (off + len + SECTOR - 1) & ~(SECTOR - 1);
    uint32_t lk = int_lock();
    hal_norflash_disable_protection(HAL_FLASH_ID_0);          /* <-- the key step */
    enum HAL_NORFLASH_RET_T r = hal_norflash_erase(HAL_FLASH_ID_0, a, end - a);
    int_unlock(lk);
    return r == HAL_NORFLASH_OK ? OTA_OK : OTA_ERR_ERASE;
}

static ota_ret_t write_verify(uint32_t off, const uint8_t *data, uint32_t len)
{
    uint32_t lk = int_lock();
    hal_norflash_disable_protection(HAL_FLASH_ID_0);
    enum HAL_NORFLASH_RET_T r = hal_norflash_write(HAL_FLASH_ID_0, off, data, len);
    int_unlock(lk);
    if (r != HAL_NORFLASH_OK) return OTA_ERR_WRITE;
    volatile uint8_t *f = FNC(off);
    for (uint32_t i = 0; i < len; i++) if (f[i] != data[i]) return OTA_ERR_VERIFY;
    return OTA_OK;
}

ota_ret_t ota_program(uint32_t off, const uint8_t *data, uint32_t len)
{
    ota_ret_t r = erase_range(off, len);
    if (r) return r;
    return write_verify(off, data, len);
}

ota_ret_t ota_begin(uint32_t off, uint32_t total_len)
{
    s_off = off; s_total = total_len; s_done = 0;
    return erase_range(off, total_len);
}
ota_ret_t ota_write_chunk(const uint8_t *data, uint32_t len)
{
    ota_ret_t r = write_verify(s_off + s_done, data, len);
    if (!r) s_done += len;
    return r;
}
ota_ret_t ota_finish(void)
{
    return (s_done == s_total) ? OTA_OK : OTA_ERR_VERIFY;
}
