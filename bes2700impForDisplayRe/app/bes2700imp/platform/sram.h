/* sram.h — full 1.4 MB on-chip SRAM. [OK] verified (march-test PASS on the FW-free span).
 * The stock SDK linker only uses ~296 KB / declares 512 KB; this BES2700IMP part actually
 * has ~1.4 MB (0x20000000..0x20160000, datasheet-confirmed). Use these to claim the extra
 * ~1.1 MB for heap / display framebuffer / sensor buffers. */
#ifndef BES2700IMP_SRAM_H
#define BES2700IMP_SRAM_H
#include <stdint.h>
#include <stdbool.h>

#define SRAM_FREE_BASE   0x2004C000u                 /* first byte past the running FW image */
#define SRAM_FREE_TOP    0x20160000u                 /* end of real SRAM (~1.4 MB total)     */
#define SRAM_FREE_SIZE   (SRAM_FREE_TOP - SRAM_FREE_BASE)

/* march-test the whole FW-free span (drops + restores the MPU). Returns 0 = PASS. */
int      sram_full_test(void);
/* bump allocator over the extra SRAM (use for framebuffer / big buffers) */
void     sram_pool_reset(void);
void    *sram_pool_alloc(uint32_t bytes, uint32_t align);

#endif /* BES2700IMP_SRAM_H */
