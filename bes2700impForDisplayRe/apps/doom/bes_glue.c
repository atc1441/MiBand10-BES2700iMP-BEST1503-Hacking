/* bes_glue.c - BES2700/best1503 implementations of the Apollo SDK / board
 * helpers that the copied GBADoom engine still references, plus the launcher.
 *
 * Keeping these in one file makes the engine independent of the AmbiqSuite tree:
 * the only external symbols doom now needs from the firmware are fb_refresh()
 * (display push, lcd.c) and hal_sys_timer_get() (timing, r_hotpath.iwram.c).
 */
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "bes_compat.h"

/* --- dedicated DOOM heap --------------------------------------------------- *
 * The SDK's newlib malloc is capped at a 4KB _sbrk heap, and its big BT
 * mem-pool overlaps the fixed framebuffer at 0x20080000. So doom gets its own
 * arena in the RAM ABOVE the framebuffer (the lds keeps the system stack/pool
 * below 0x20080000 when DOOM is defined). doom_malloc/free/calloc/realloc back
 * doom's z_zone and direct allocations (routed via -Dmalloc=doom_malloc ... in
 * apps/doom/Makefile). First-fit free list with split + forward coalescing. */
/* TWO full-panel 212x520 RGB565 ping-pong buffers (220 KB each) now occupy 0x20080000..0x200EBD80
 * (lcd.c), so the arena moves above them. It runs up to 0x2012A000 (just below the linker's
 * .sram_text at the top of the main-core RAM region); the unused RAMCP area above that is the
 * coprocessor's. 248 KB is enough for DOOM's zone (capped to match in z_zone.c) + the 38 KB
 * render fb + WAD lump cache. */
#define DOOM_ARENA_BASE  0x200EC000u           /* above FB-B (0x200B6000..0x200EBD80)           */
#define DOOM_ARENA_SIZE  0x0003E000u           /* 248KB: 0x200EC000 .. 0x2012A000 (< sram_text) */

typedef struct doom_blk { uint32_t size; struct doom_blk *next; int free; } doom_blk_t;
static doom_blk_t *doom_freelist;
#define DOOM_ALIGN8(n) (((n) + 7u) & ~7u)

static void doom_arena_init(void)
{
    doom_freelist = (doom_blk_t *)DOOM_ARENA_BASE;
    doom_freelist->size = DOOM_ARENA_SIZE - sizeof(doom_blk_t);
    doom_freelist->next = NULL;
    doom_freelist->free = 1;
}

void *doom_malloc(size_t want)
{
    if (!doom_freelist) doom_arena_init();
    if (!want) return NULL;
    size_t need = DOOM_ALIGN8(want);
    for (doom_blk_t *b = doom_freelist; b; b = b->next) {
        if (b->free && b->size >= need) {
            if (b->size >= need + sizeof(doom_blk_t) + 8u) {   /* split */
                doom_blk_t *n = (doom_blk_t *)((uint8_t *)b + sizeof(doom_blk_t) + need);
                n->size = b->size - need - sizeof(doom_blk_t);
                n->free = 1;
                n->next = b->next;
                b->size = need;
                b->next = n;
            }
            b->free = 0;
            return (uint8_t *)b + sizeof(doom_blk_t);
        }
    }
    return NULL;   /* out of doom arena */
}

void doom_free(void *p)
{
    if (!p) return;
    doom_blk_t *b = (doom_blk_t *)((uint8_t *)p - sizeof(doom_blk_t));
    b->free = 1;
    /* list stays address-ordered, so a single forward pass coalesces neighbours */
    for (doom_blk_t *c = doom_freelist; c && c->next; c = c->next) {
        if (c->free && c->next->free &&
            (uint8_t *)c->next == (uint8_t *)c + sizeof(doom_blk_t) + c->size) {
            c->size += sizeof(doom_blk_t) + c->next->size;
            c->next = c->next->next;
        }
    }
}

void *doom_calloc(size_t n, size_t s)
{
    size_t t = n * s;
    void *p = doom_malloc(t);
    if (p) memset(p, 0, t);
    return p;
}

void *doom_realloc(void *p, size_t s)
{
    if (!p) return doom_malloc(s);
    if (!s) { doom_free(p); return NULL; }
    doom_blk_t *b = (doom_blk_t *)((uint8_t *)p - sizeof(doom_blk_t));
    if (b->size >= DOOM_ALIGN8(s)) return p;
    void *np = doom_malloc(s);
    if (np) { memcpy(np, p, b->size); doom_free(p); }
    return np;
}

/* --- logging -------------------------------------------------------------- */
/* DOOM logs (and the FPS line) -> written DIRECTLY to the UART hardware, polling the PL011
 * TX-FIFO-full flag. The hal_trace DMA/thread path never flushes once DOOM owns the CPU (and an
 * explicit flush hangs), so we bypass it. Writes to every ENABLED UART (UART0 0x4000B000 /
 * UART1 0x4000C000 / UART2 0x4000D000) so the line shows up on whichever COM is the trace port,
 * regardless of DEBUG_PORT. Disabled UARTs are skipped (no stall). PL011: DR +0x00, FR +0x18
 * (TXFF bit5), CR +0x30 (UARTEN bit0, TXE bit8). */
#include <stdio.h>
static void raw_uart_puts_all(const char *s, int n)
{
    static const unsigned int bases[3] = { 0x4000B000u, 0x4000C000u, 0x4000D000u };
    for (int u = 0; u < 3; u++) {
        volatile unsigned int *CR = (volatile unsigned int *)(bases[u] + 0x30u);
        if ((*CR & ((1u << 0) | (1u << 8))) != ((1u << 0) | (1u << 8))) continue;  /* not enabled+TXE */
        volatile unsigned int *DR = (volatile unsigned int *)(bases[u] + 0x00u);
        volatile unsigned int *FR = (volatile unsigned int *)(bases[u] + 0x18u);
        for (int i = 0; i < n; i++) {
            unsigned int g = 0;
            while ((*FR & (1u << 5)) && ++g < 50000u) { }   /* TX FIFO not full (bounded) */
            *DR = (unsigned char)s[i];
        }
        /* wait for the TX FIFO to fully drain (TXFE = bit 7) so the whole line is physically out
         * before any later code gates this UART's clock (else the FIFO tail is lost). */
        unsigned int d = 0;
        while (!(*FR & (1u << 7)) && ++d < 300000u) { }
    }
}
void am_util_stdio_printf(const char *fmt, ...)
{
    char msg[160];
    va_list v; va_start(v, fmt);
    int n = vsnprintf(msg, sizeof(msg), fmt, v);
    va_end(v);
    if (n <= 0) return;
    if (n > (int)sizeof(msg)) n = (int)sizeof(msg);
    raw_uart_puts_all(msg, n);
}

/* --- input (stub) --------------------------------------------------------- */
/* i_system.c's read_touchscreen() polls digitalRead(24) (touch IRQ, active-low)
 * then touch_transceive(). Until the best1503 touch controller is wired in,
 * report "no touch" (IRQ idle-high) so the engine runs without input. */
int digitalRead(int pin)
{
    (void)pin;
    return 1;
}

void touch_transceive(unsigned int reg, unsigned int *buf, int len)
{
    (void)reg; (void)buf; (void)len;
}

/* --- machine init --------------------------------------------------------- */
/* D_DoomMainSetup() calls I_Init() to set up timers/machine state. In the
 * Apollo build this lived in the (uncopied) board main; on BES the DOOM tick is
 * read straight from the free-running system timer, so there is nothing to do. */
void I_Init(void)
{
}

/* --- launcher ------------------------------------------------------------- */
/* D_DoomMain runs the game loop and never returns. Call doom_launch() from the
 * firmware once the panel is up (e.g. from hw_selftest when DOOM=1). It should
 * run on its own stack/thread so it does not starve the BES system tasks. */
extern void InitGlobals(void);        /* sets _g = dataForG and zero-inits it */
extern void I_PreInitGraphics(void);  /* allocates the render framebuffer (fb) + lcd_init() */
extern void D_DoomMain(void);

void doom_launch(void)
{
    InitGlobals();         /* MUST run before any _g-> access (was done by the Apollo main) */
    I_PreInitGraphics();   /* allocate fb; D_DoomMain's renderer writes to it (else NULL deref) */
    D_DoomMain();
}
