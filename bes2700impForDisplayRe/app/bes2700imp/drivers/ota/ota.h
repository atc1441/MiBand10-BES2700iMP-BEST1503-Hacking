/* ota.h — in-system firmware update (OTA core). [OK] verified working on silicon.
 *
 * The running FW can erase+program its own NOR flash from RAM (hal_norflash runs from
 * SRAM and gates XIP). Use this to stage a new image into a spare/2nd flash partition,
 * then reboot the boot-ROM into it. NOTE: you cannot erase the *running* image's own
 * sectors in place — write to a second partition (or fall back to the boot-ROM download
 * path that the dev flasher uses). */
#ifndef BES2700IMP_OTA_H
#define BES2700IMP_OTA_H
#include <stdint.h>
#include <stdbool.h>

typedef enum { OTA_OK = 0, OTA_ERR_ERASE, OTA_ERR_WRITE, OTA_ERR_VERIFY } ota_ret_t;

/* one-shot: erase the region [off,off+len), program `data`, verify via the non-cached
 * flash alias. off/len are flash OFFSETS (0 = 0x2C000000). len need not be sector-aligned
 * for the write; the erase rounds to whole 4 KB sectors. */
ota_ret_t ota_program(uint32_t off, const uint8_t *data, uint32_t len);

/* streaming OTA: begin a region, feed chunks, finalize. */
ota_ret_t ota_begin(uint32_t off, uint32_t total_len);
ota_ret_t ota_write_chunk(const uint8_t *data, uint32_t len);
ota_ret_t ota_finish(void);   /* flush + verify */

/* read back flash (offset based), via the non-cached alias so it's never the stale XIP cache */
void      ota_read(uint32_t off, uint8_t *buf, uint32_t len);

#endif /* BES2700IMP_OTA_H */
