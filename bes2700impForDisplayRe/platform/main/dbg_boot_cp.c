/* Earliest-possible boot checkpoint over a raw func-2 HW-UART.
 *
 * Target UART/pin follows DEBUG_PORT (same selector as dbg_boot_init in main.cpp
 * and the SDK trace): DEBUG_PORT==2 -> UART1 TXD on P2_1 = GPIO17, otherwise
 * UART0 TXD on P2_3 = GPIO19.
 *
 * Called from startup_main.S BEFORE the C runtime (data copy / bss clear), so it
 * must use only flash .rodata + registers — NO RAM globals/statics. dbg_init()
 * is idempotent (re-asserts clocks + IOMUX func-2 mux + TX each call), so every
 * checkpoint is self-contained. Pure PL011 poll-TX; baud is inherited from the
 * boot ROM (UART0) — for the UART1 case UART0's bootrom clock + baud regs are
 * cloned onto UART1. Used to find exactly which pre-main startup stage hangs when
 * the best1306-substituted firmware runs on best1503 silicon. */
#define DBG_IOMUX_BASE 0x40086000u
#define DBG_UART0_BASE 0x4000B000u
#define DBG_UART1_BASE 0x4000C000u
#define DBG_CMU_BASE   0x40000000u
#if (DEBUG_PORT == 2)
#define DBG_UART_BASE  DBG_UART1_BASE   /* P2_1 = GPIO17 */
#else
#define DBG_UART_BASE  DBG_UART0_BASE   /* P2_3 = GPIO19 */
#endif

static void dbg_init(void)
{
    /* hal_cmu_module_init_state() puts the UART into reset + may gate its clock;
     * undo that so logging survives the clock bring-up. CMU writes are lock-gated. */
    volatile unsigned int *cmu = (volatile unsigned int *)DBG_CMU_BASE;
    volatile unsigned int *io  = (volatile unsigned int *)DBG_IOMUX_BASE;
    cmu[0x080/4] = 0xCAFE0001u;          /* WRITE_UNLOCK */
#if (DEBUG_PORT == 2)
    /* UART1 / P2_1 (GPIO17). UART0_TX is silicon-fixed to P2_3, so GPIO17 needs
     * UART1; the boot ROM only sets up UART0, so clone its clock + baud here. */
    volatile unsigned int *u0 = (volatile unsigned int *)DBG_UART0_BASE;
    volatile unsigned int *u1 = (volatile unsigned int *)DBG_UART1_BASE;
    { unsigned int uc = cmu[0x070/4]; cmu[0x070/4] = (uc & ~0x3E0u) | ((uc & 0x1Fu) << 5); }
    cmu[0x040/4] = (1u<<11);             /* PRESET_CLR : release UART1 pclk reset */
    cmu[0x048/4] = (1u<<13);             /* ORESET_CLR : release UART1 oclk reset */
    cmu[0x008/4] = (1u<<11);             /* PCLK_ENABLE: UART1 */
    cmu[0x010/4] = (1u<<13);             /* OCLK_ENABLE: UART1 */
    io[0x00C/4] = (io[0x00C/4] & ~0x00F0u) | 0x0020u; /* P2_1 -> func 2 (UART1_TX) */
    io[0x02C/4] &= ~(1u<<17);            /* no pull on TXD (GPIO17) */
    io[0x030/4] &= ~(1u<<17);            /* clear pull-down         */
    u1[0x024/4] = u0[0x024/4];           /* IBRD  (clone bootrom baud) */
    u1[0x028/4] = u0[0x028/4];           /* FBRD  */
    u1[0x04C/4] = u0[0x04C/4];           /* OVSAMP */
    u1[0x02C/4] = u0[0x02C/4];           /* LCR_H (8N1 + FIFO) */
    u1[0x030/4] |= (1u<<0)|(1u<<8);      /* UARTCR: UARTEN | TXE */
#else
    /* UART0 / P2_3 (GPIO19): boot ROM left it configured, just re-assert. */
    cmu[0x040/4] = (1u<<10);             /* PRESET_CLR : release UART0 pclk reset */
    cmu[0x048/4] = (1u<<12);             /* ORESET_CLR : release UART0 oclk reset */
    cmu[0x008/4] = (1u<<10);             /* PCLK_ENABLE: UART0 */
    cmu[0x010/4] = (1u<<12);             /* OCLK_ENABLE: UART0 */
    io[0x050/4] |= 3u;
    io[0x00C/4] = (io[0x00C/4] & ~0xFF00u) | 0x2200u; /* P2_2/P2_3 -> func 2 */
    io[0x02C/4] |= (1u<<18); io[0x02C/4] &= ~(1u<<19);
    io[0x030/4] &= ~((1u<<18)|(1u<<19));
    volatile unsigned int *u = (volatile unsigned int *)DBG_UART0_BASE;
    u[0x030/4] |= (1u<<0)|(1u<<8);                    /* UARTCR: UARTEN | TXE */
#endif
}
static void dbg_putc(char c)
{
    volatile unsigned int *u = (volatile unsigned int *)DBG_UART_BASE;
    unsigned int g = 0;
    while ((u[0x018/4] & (1u<<5)) && ++g < 0x100000u) {}
    u[0] = (unsigned int)(unsigned char)c;
}
static void dbg_puts(const char *s) { while (*s) { if (*s=='\n') dbg_putc('\r'); dbg_putc(*s++); } }

/* Override newlib's __libc_init_array to log each C++ static constructor as it
 * runs, so we can find which global ctor hangs on best1503 (the last index +
 * address printed before the UART goes silent is the culprit). */
extern void (*__preinit_array_start[])(void) __attribute__((weak));
extern void (*__preinit_array_end[])(void) __attribute__((weak));
extern void (*__init_array_start[])(void) __attribute__((weak));
extern void (*__init_array_end[])(void) __attribute__((weak));
void dbg_hex16(unsigned int v);

void __libc_init_array(void)
{
    int count, i;
    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++) __preinit_array_start[i]();
    count = __init_array_end - __init_array_start;
    dbg_puts("[CTORS n="); dbg_hex16((unsigned)count); dbg_puts("]\n");
    for (i = 0; i < count; i++) {
        dbg_hex16((unsigned)i);                                   /* ctor index */
        dbg_hex16(((unsigned)(unsigned long)__init_array_start[i]) & 0xFFFFu); /* ctor addr low16 */
        __init_array_start[i]();
    }
    dbg_puts("[CTORS done]\n");
}

/* print a 16-bit value as hex: "[Hxxxx]" — for inspecting register reads */
void dbg_hex16(unsigned int v)
{
    volatile unsigned int *u = (volatile unsigned int *)DBG_UART_BASE;
    unsigned int g; int i;
    dbg_init();
    dbg_puts("[H");
    for (i = 12; i >= 0; i -= 4) {
        unsigned int d = (v >> i) & 0xF;
        dbg_putc((char)(d < 10 ? '0'+d : 'A'+d-10));
    }
    dbg_puts("]\n");
    g = 0; while ((u[0x018/4] & (1u<<3)) && ++g < 0x200000u) {}
}

/* best1503 bring-up: catch faults so a silent stall shows up as [FAULT pc=...] */
void dbg_init(void);

/* Memory-map probe support: when g_probe_active!=0 a data-abort is non-fatal —
 * the handler records it and resumes just past the faulting load/store so the
 * self-test can poke unmapped/candidate windows (PSRAM etc.) without hanging. */
volatile int g_probe_active = 0;
volatile int g_probe_faulted = 0;
void dbg_hardfault_c(unsigned int *frame)
{
    if (g_probe_active) {
        unsigned int pc = frame[6];
        unsigned short hw = *(volatile unsigned short *)pc;
        unsigned int sz = ((hw >> 11) >= 0x1D) ? 4u : 2u;   /* Thumb 32- vs 16-bit */
        g_probe_faulted = 1;
        frame[6] = pc + sz;                                  /* resume past the access */
        *(volatile unsigned int *)0xE000ED28u = *(volatile unsigned int *)0xE000ED28u; /* clr CFSR */
        return;
    }
    unsigned int pc  = frame[6];     /* stacked PC (r0,r1,r2,r3,r12,lr,PC,xPSR) */
    unsigned int lr  = frame[5];     /* stacked LR = caller (who branched here) */
    unsigned int r0v = frame[0];
    unsigned int r2v = frame[2];
    unsigned int r3v = frame[3];
    unsigned int cfsr = *(volatile unsigned int *)0xE000ED28u;
    unsigned int mmfar = *(volatile unsigned int *)0xE000ED34u; /* MemManage fault addr */
    unsigned int bfar  = *(volatile unsigned int *)0xE000ED38u; /* BusFault fault addr */
    dbg_init();
    dbg_puts("\n[FAULT pc=");  dbg_hex16((pc>>16)&0xFFFFu);  dbg_hex16(pc&0xFFFFu);
    dbg_puts(" lr=");          dbg_hex16((lr>>16)&0xFFFFu);  dbg_hex16(lr&0xFFFFu);
    dbg_puts(" r0=");          dbg_hex16((r0v>>16)&0xFFFFu); dbg_hex16(r0v&0xFFFFu);
    dbg_puts(" r2=");          dbg_hex16((r2v>>16)&0xFFFFu); dbg_hex16(r2v&0xFFFFu);
    dbg_puts(" r3=");          dbg_hex16((r3v>>16)&0xFFFFu); dbg_hex16(r3v&0xFFFFu);
    dbg_puts(" sp=");          dbg_hex16(((unsigned)(unsigned long)frame>>16)&0xFFFFu); dbg_hex16((unsigned)(unsigned long)frame&0xFFFFu);
    dbg_puts(" cfsr=");        dbg_hex16((cfsr>>16)&0xFFFFu); dbg_hex16(cfsr&0xFFFFu);
    dbg_puts(" mmfar=");       dbg_hex16((mmfar>>16)&0xFFFFu); dbg_hex16(mmfar&0xFFFFu);
    dbg_puts(" bfar=");        dbg_hex16((bfar>>16)&0xFFFFu); dbg_hex16(bfar&0xFFFFu);
    dbg_puts("]\n");
    while (1) {}
}
__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile ("tst lr,#4\nite eq\nmrseq r0,msp\nmrsne r0,psp\nb dbg_hardfault_c\n");
}
__attribute__((naked)) void BusFault_Handler(void)
{ __asm volatile ("tst lr,#4\nite eq\nmrseq r0,msp\nmrsne r0,psp\nb dbg_hardfault_c\n"); }
__attribute__((naked)) void MemManage_Handler(void)
{ __asm volatile ("tst lr,#4\nite eq\nmrseq r0,msp\nmrsne r0,psp\nb dbg_hardfault_c\n"); }
__attribute__((naked)) void UsageFault_Handler(void)
{ __asm volatile ("tst lr,#4\nite eq\nmrseq r0,msp\nmrsne r0,psp\nb dbg_hardfault_c\n"); }

/* called as: mov r0,#N ; bl dbg_cp_n   — prints "[CPn]" */
void dbg_cp_n(int n)
{
    volatile unsigned int *u = (volatile unsigned int *)DBG_UART_BASE;
    unsigned int g;
    dbg_init();
    dbg_puts("[CP");
    dbg_putc((char)(n < 10 ? '0'+n : 'A'+n-10));
    dbg_puts("]\n");
    /* flush: wait for TX to fully drain (UARTFR.BUSY bit3) so the bytes are out
     * before the next line possibly kills the UART clock */
    g = 0; while ((u[0x018/4] & (1u<<3)) && ++g < 0x200000u) {}
}
