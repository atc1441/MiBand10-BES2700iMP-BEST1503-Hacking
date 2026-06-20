/* sram.c — full SRAM test + a simple pool over the extra ~1.1 MB. [OK] */
#include "sram.h"
#include "cmsis.h"
#include "mpu_cfg.h"

static uint32_t s_pool = SRAM_FREE_BASE;

int sram_full_test(void)
{
    int err = 0;
    ARM_MPU_Disable();                              /* reach past the FW's 0x2004a000 MPU bound */
    for (uint32_t a = SRAM_FREE_BASE; a < SRAM_FREE_TOP; a += 4) *(volatile uint32_t*)a = a;
    for (uint32_t a = SRAM_FREE_BASE; a < SRAM_FREE_TOP; a += 4) if (*(volatile uint32_t*)a != a) err++;
    static const uint32_t pat[] = {0x00000000u,0xFFFFFFFFu,0xAAAAAAAAu,0x55555555u};
    for (unsigned k = 0; k < 4; k++) {
        for (uint32_t a = SRAM_FREE_BASE; a < SRAM_FREE_TOP; a += 4) *(volatile uint32_t*)a = pat[k];
        for (uint32_t a = SRAM_FREE_BASE; a < SRAM_FREE_TOP; a += 4) if (*(volatile uint32_t*)a != pat[k]) err++;
    }
    mpu_cfg();                                      /* restore the firmware MPU */
    return err;
}

void  sram_pool_reset(void) { s_pool = SRAM_FREE_BASE; }
void *sram_pool_alloc(uint32_t bytes, uint32_t align)
{
    if (align < 4) align = 4;
    s_pool = (s_pool + align - 1) & ~(align - 1);
    if (s_pool + bytes > SRAM_FREE_TOP) return 0;   /* out of extra SRAM */
    void *p = (void *)(uintptr_t)s_pool;
    s_pool += bytes;
    return p;
}
