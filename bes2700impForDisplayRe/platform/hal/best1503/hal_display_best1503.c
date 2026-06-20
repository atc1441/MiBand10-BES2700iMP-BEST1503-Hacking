/***************************************************************************
 * best1503 display (MIPI-DSI + LCDC) chip-glue  —  SKELETON / bring-up
 *
 * The best1306-based SDK ships platform/hal/hal_dsi.c and hal_lcdc.c, but they
 * are gated behind DSI_BASE/LCDC_BASE and call a handful of chip-specific CMU /
 * DSI-PHY / IOMUX helpers that were never implemented for best1503. This file
 * provides those helpers so the RM690B0 AMOLED on the MiBand 9 can be driven.
 *
 * STATUS: every register write below marked with TODO(vela) is a PLACEHOLDER.
 * The real CMU clock bits, DSI-PHY config and DSI TE pin must be lifted from the
 * MiBand 9 Vela FW (vela_ap.bin @0x2C150000) once Ghidra's full auto-analysis has
 * resolved its PIC/GOT references, OR by probing on the running firmware. Until
 * then this compiles but must NOT be trusted to bring the panel up.
 *
 * Reference flow (platform/drivers/graphic/lcd/rm690b0_lcd.c, modelled on the
 * stock dumy_lcd.c rm69330_init):
 *   power/RST gpio -> hal_dsi_init(WIDTH) -> rm690b0_init() -> hal_dsi_start()
 *   -> lcdc_initialize(fb)
 ****************************************************************************/
#include "plat_types.h"
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_trace.h"
#include "hal_timer.h"

/* DSI_BASE/LCDC_BASE come from plat_addr_map (via cmsis.h) under
 * CONFIG_DISPLAY_DSI_BEST1503, so the guard MUST sit after the includes. */
#if defined(DSI_BASE) && defined(LCDC_BASE)

/* CMU register banks (confirmed during our CMU bring-up). */
#define CMU_REG_BASE        0x40000000u
#define AON_CMU_REG_BASE    0x40080000u
#define CMU_WRITE_UNLOCK    0xCAFE0001u   /* CMU[0x080] write-unlock (used by dbg) */

/* ---------------------------------------------------------------------------
 * TODO(vela): the exact enable bits for the LCDC and DSI module clocks, the DSI
 * pixel/byte clock divider, and the DSI-PHY PLL settings. Lift from vela_ap.bin
 * hal_cmu_lcdc_clock_enable / hal_cmu_dsi_clock_enable. Placeholders below only
 * unlock the CMU and touch best-guess enable registers so the call chain links.
 * ------------------------------------------------------------------------- */
#define LCDC_CMU_CLK_ENABLE_BIT   (1u << 0)   /* TODO(vela): real bit/register   */
#define DSI_CMU_CLK_ENABLE_BIT    (1u << 0)   /* TODO(vela): real bit/register   */
#define DSI_PHY_RESET_BIT         (1u << 0)   /* TODO(vela): real reset bit       */

void hal_cmu_lcdc_clock_enable(uint32_t pixmhz_speed)
{
    (void)pixmhz_speed;
    volatile uint32_t *cmu_rst = (volatile uint32_t *)0x40000044u;
    volatile uint32_t *cmu_en  = (volatile uint32_t *)0x40000048u;
    *cmu_rst = (1u << 27);
    hal_sys_timer_delay_us(2);
    *cmu_en = (1u << 27);
    *cmu_rst = (1u << 28);
    hal_sys_timer_delay_us(2);
    *cmu_en = (1u << 28);
}

void hal_cmu_dsi_clock_enable(void)
{
    volatile uint32_t *cmu = (volatile uint32_t *)CMU_REG_BASE;
    cmu[0x080/4] = CMU_WRITE_UNLOCK;
    /* TODO(vela): cmu[<DSI clk-enable reg>/4] |= DSI_CMU_CLK_ENABLE_BIT; */
    TR_INFO(TR_MOD(TEST), "//best1503 hal_cmu_dsi_clock_enable STUB");
}

void hal_cmu_dsi_clock_disable(void) { /* TODO(vela) */ }
void hal_cmu_dsi_sleep(void)  { /* TODO(vela): DSI low-power entry */ }
void hal_cmu_dsi_wakeup(void) { /* TODO(vela): DSI low-power exit  */ }
void hal_cmu_lcdc_sleep(void)  { /* TODO(vela): LCDC low-power entry */ }
void hal_cmu_lcdc_wakeup(void) { /* TODO(vela): LCDC low-power exit  */ }

void hal_cmu_dsi_phy_reset_set(void)
{
    /* TODO(vela): assert DSI-PHY reset in the (AON?)CMU. */
}
void hal_cmu_dsi_phy_reset_clear(void)
{
    /* TODO(vela): de-assert DSI-PHY reset. */
}

/* ---------------------------------------------------------------------------
 * DSI D-PHY open. hal_dsi.c uses the CHIP_HAS_MIPIPHY path -> extern dsiphy_open().
 * TODO(vela): the PHY PLL/lane config (HS clock for the panel pixel clock). The
 * MiBand 9 RM690B0 runs 1-lane command mode, so the PHY only needs the escape/LP
 * clock + a modest HS bitclock. Lift the PLL words from vela_ap.bin dsiphy_open.
 * ------------------------------------------------------------------------- */
void dsiphy_open(int unused)
{
    (void)unused;
    /* TODO(vela): program MIPI D-PHY PLL + lane enables. */
    TR_INFO(TR_MOD(TEST), "//best1503 dsiphy_open STUB");
}

/* ---------------------------------------------------------------------------
 * DSI Tearing-Effect (TE) input pin mux. The RM690B0 drives TE back to the SoC so
 * the LCDC can sync frame pushes. TODO(vela): the exact P?_? pad + func. Lift from
 * the MiBand 9 IOMUX setup (boot ROM / vela), or leave unmuxed for a first
 * no-TE (free-run) bring-up.
 * ------------------------------------------------------------------------- */
void hal_iomux_set_dsi_te(void)
{
    /* TODO(vela): mux the DSI TE pad. No-op = free-run (no tear-sync) for first light. */
}

enum HAL_IOMUX_PIN_T hal_iomux_get_dsi_te_pin(void)
{
    return (enum HAL_IOMUX_PIN_T)0; /* TODO(vela): real TE pad */
}

#endif /* DSI_BASE && LCDC_BASE */
