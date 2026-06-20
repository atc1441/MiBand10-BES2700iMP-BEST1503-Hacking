#include "main_entry.h"
#include "hal_cache.h"
#include "reg_uart.h"      /* struct UART_T / PL011 register layout            */
#include "plat_addr_map.h" /* UART0_BASE (0x4000B000), AON_IOMUX_BASE (0x40086000) */

/* ====================================================================== *
 *  best1503 (MiBand9 / BES2700iMP) hardware UART0 - polled, no DMA/IRQ
 * ---------------------------------------------------------------------- *
 *  The boot-ROM CMD-UART (the pad your flash adapter is on, COM @921600) is
 *  plain ARM PL011 UART0 on P2_2 (RXD/GPIO18) / P2_3 (TXD/GPIO19).
 *
 *  >>> KEY best1503 fact: the IOMUX function for UART0 on these pads is 2,
 *      NOT 1.  The best1306 SDK's hal_iomux_set_uart0() uses func 1, which on
 *      best1503 does NOT connect TXD to GPIO19 - nothing comes out.  Func 2 is
 *      what the real boot ROM uses (decrypted boot ROM @0x27744).  <<<
 *
 *  The boot ROM already left UART0 enabled and configured for 8N1 @921600
 *  (IBRD=1, FBRD=40 at the 24 MHz crystal), so we only route the pads and reuse
 *  that config - no clock/PMU/baud setup needed.  Set your adapter to 921600
 *  8N1, no flow control, RX on GPIO19.
 * ====================================================================== */
#define HWUART_BAUD  921600
#define HWU          ((volatile struct UART_T *)UART0_BASE)              /* 0x4000B000 */
#define IOMUX_R(off) (((volatile uint32_t *)AON_IOMUX_BASE)[(off) / 4])  /* 0x40086000 */

/* Route P2_2/P2_3 to UART0 (func 2) and make sure TX is enabled.  Exact replica
 * of the best1503 boot ROM's CMD-UART pad setup. */
static void hwuart_init(void)
{
    IOMUX_R(0x050) |= (1u << 0) | (1u << 1);                /* analog-I2C pads -> GPIO   */
    IOMUX_R(0x00C) = (IOMUX_R(0x00C) & ~0xFF00u) | 0x2200u; /* P2_2/P2_3 sel = func2=UART0 */
    IOMUX_R(0x02C) |=  (1u << 18);                          /* pull-up on RXD  (P2_2)    */
    IOMUX_R(0x02C) &= ~(1u << 19);                          /* no pull   on TXD (P2_3)   */
    IOMUX_R(0x030) &= ~((1u << 18) | (1u << 19));           /* clear pull-downs          */

    HWU->UARTCR |= UARTCR_UARTEN | UARTCR_TXE;              /* keep boot-ROM cfg, ensure TX */
}

/* Push one byte (block while the TX FIFO is full). */
static void hwuart_putc(uint8_t c)
{
    while (HWU->UARTFR & UARTFR_TXFF)
    {
        /* TX FIFO full - spin */
    }
    HWU->UARTDR = c;
}

static void hwuart_puts(const char *s)
{
    while (*s)
    {
        hwuart_putc((uint8_t)*s++);
    }
}

static void hwuart_puthex(uint32_t v)
{
    static const char hexd[] = "0123456789ABCDEF";
    int i;
    for (i = 28; i >= 0; i -= 4)
    {
        hwuart_putc((uint8_t)hexd[(v >> i) & 0xFu]);
    }
}

/* Called early from startup_main.S - enable I/D cache + write buffer. */
void boot_loader_pre_init_hook(void)
{
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
}

/* MAIN_ENTRY expands to _start (NOSTD=1) and overrides the weak _start in
 * libprogrammer_inflash.a. */
int MAIN_ENTRY(void)
{
    uint32_t n = 0;

    hwuart_init();

    for (;;)
    {
        hwuart_puts("best1503 HW-UART0 (PL011 poll, GPIO19, func2) #");
        hwuart_puthex(n++);
        hwuart_puts("\r\n");

        /* simple busy delay (no sys timer dependency in this early context) */
        {
            volatile uint32_t d = 2000000u;
            while (d--)
            {
                __asm volatile("nop");
            }
        }
    }

    return 0;
}

void programmer_inflash_main(void)
{
    MAIN_ENTRY();
}
