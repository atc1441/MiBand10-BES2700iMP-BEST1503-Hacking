/* best1503 / MiBand9 on-SoC hardware self-test (no display/peripherals attached).
 * Runs from main() after the trace UART is up; output goes to the SDK trace UART
 * (COM9 @ the calibrated 1.152 Mbps). Enable with -DHW_SELFTEST. */
#if defined(HW_SELFTEST)

#include <string.h>
#include "plat_types.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_i2c.h"
#include "hal_cmu.h"
#include "hal_cache.h"
#include "hal_sysfreq.h"
#include "hal_uart.h"

#define ST(...) TRACE(0, "[SELFTEST] " __VA_ARGS__)

/* ---- 1. on-chip RAM walking-pattern test (safe: a local static buffer) ------ */
static int st_ram_buffer(void)
{
    static uint32_t buf[512]; /* 2 KB in .bss */
    static const uint32_t pats[] = {0x00000000u, 0xFFFFFFFFu, 0xA5A5A5A5u, 0x5A5A5A5Au, 0xDEADBEEFu};
    int err = 0;
    for (unsigned p = 0; p < sizeof(pats) / sizeof(pats[0]); p++)
    {
        for (int i = 0; i < 512; i++)
            buf[i] = pats[p] ^ (uint32_t)(i * 0x01010101u);
        for (int i = 0; i < 512; i++)
            if (buf[i] != (pats[p] ^ (uint32_t)(i * 0x01010101u)))
                err++;
    }
    ST("RAM buffer (2KB x5 patterns): %s err=%d @%p", err ? "FAIL" : "PASS", err, (void *)buf);
    return err;
}

/* ---- 2. system timer monotonic test ----------------------------------------- */
static int st_timer(void)
{
    uint32_t t0 = hal_sys_timer_get();
    for (volatile int i = 0; i < 200000; i++)
    {
        __NOP();
    }
    uint32_t t1 = hal_sys_timer_get();
    int ok = (t1 != t0);
    ST("sys_timer: t0=0x%08x t1=0x%08x delta=%u : %s", t0, t1, (unsigned)(t1 - t0), ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

/* ---- 3. PSRAM probe ---------------------------------------------------------- *
 * best1503 has PSRAM clock controls in the CMU but this SDK has no PSRAM base/
 * controller config for it, so we can only report the clock state here, not do a
 * real memory r/w yet (accessing an unmapped PSRAM window would HardFault).
 * TODO(vela): once the PSRAM controller base + init are known, do a real buffer
 * write/read/verify across the PSRAM window. */
extern volatile int g_probe_active, g_probe_faulted;
extern void dbg_hex16(unsigned int v); /* immediate-flush hex over the dbg UART */

/* Fault-tolerant probe of one address. do_write!=0 => also classify writability
 * (writes a pattern then restores the original word). Reveals the real memory map
 * directly on silicon — sidesteps reversing the obfuscated Vela binary. */
static void st_probe1(uint32_t addr, int do_write)
{
    g_probe_faulted = 0;
    g_probe_active = 1;
    __DSB();
    __ISB();
    uint32_t v0 = *(volatile uint32_t *)addr;
    __DSB();
    __ISB();
    int rf = g_probe_faulted;
    g_probe_active = 0;
    if (rf)
    {
        ST("  %08x: FAULT (unmapped)", (unsigned)addr);
        return;
    }

    int w = -2;
    if (do_write)
    {
        g_probe_faulted = 0;
        g_probe_active = 1;
        __DSB();
        __ISB();
        *(volatile uint32_t *)addr = 0x5A5A5A5Au;
        __DSB();
        __ISB();
        uint32_t vb = *(volatile uint32_t *)addr;
        __DSB();
        __ISB();
        *(volatile uint32_t *)addr = v0; /* restore */
        __DSB();
        __ISB();
        int wf = g_probe_faulted;
        g_probe_active = 0;
        w = wf ? -1 : (vb == 0x5A5A5A5Au ? 1 : 0);
    }
    ST("  %08x: rd=0x%08x %s", (unsigned)addr, (unsigned)v0,
       w == 1 ? "WRITABLE(RAM/PSRAM)" : w == 0 ? "read-only"
                                    : w == -1  ? "write-FAULT"
                                               : "(read-only probe)");
}

/* Characterise a writable window: find its size (1 MB steps) + verify reliability
 * with a multi-pattern write/read across the span. This is how we confirm the
 * external PSRAM (several MB) without knowing the controller config. */
static uint32_t st_rd(uint32_t a)
{
    g_probe_faulted = 0;
    g_probe_active = 1;
    __DSB();
    __ISB();
    uint32_t v = *(volatile uint32_t *)a;
    __DSB();
    __ISB();
    g_probe_active = 0;
    return v;
}
static int st_wr(uint32_t a, uint32_t v)
{
    g_probe_faulted = 0;
    g_probe_active = 1;
    __DSB();
    __ISB();
    *(volatile uint32_t *)a = v;
    __DSB();
    __ISB();
    g_probe_active = 0;
    return g_probe_faulted;
}

static void st_window_characterize(uint32_t base)
{
    /* 1) distinct-marker test: are nearby words independent (real RAM) or
     *    aliased/flaky? Write unique markers, read all back. */
    st_wr(base + 0x00, 0x11112222u);
    st_wr(base + 0x04, 0x33334444u);
    st_wr(base + 0x40, 0x55556666u);
    st_wr(base + 0x1000, 0x77778888u);
    uint32_t a0 = st_rd(base + 0x00), a1 = st_rd(base + 0x04), a2 = st_rd(base + 0x40), a3 = st_rd(base + 0x1000);
    int distinct = (a0 == 0x11112222u) + (a1 == 0x33334444u) + (a2 == 0x55556666u) + (a3 == 0x77778888u);
    ST("window @0x%08x: distinct-marker %d/4 (rd %08x %08x %08x %08x)",
       (unsigned)base, distinct, (unsigned)a0, (unsigned)a1, (unsigned)a2, (unsigned)a3);

    /* 2) alias-based size: marker at base+0, see at which power-of-2 offset it
     *    reappears (= wrap = real size). */
    st_wr(base + 0, 0xA5A5A5A5u);
    uint32_t wrap = 0;
    for (uint32_t off = 0x100000u; off <= 0x4000000u; off <<= 1)
    {
        st_wr(base + off, 0x5A5A5A5Au); /* clobber the candidate */
        if (st_rd(base + 0) != 0xA5A5A5A5u)
        {
            wrap = off;
            break;
        } /* base+0 changed => alias */
    }
    if (wrap)
        ST("window @0x%08x: aliases at +0x%x => real size ~%u KB",
           (unsigned)base, (unsigned)wrap, (unsigned)(wrap / 1024));
    else
        ST("window @0x%08x: no alias within 64MB (large or non-wrapping)", (unsigned)base);

    /* 3) reliability across 0..min(size,2MB) */
    uint32_t span = wrap ? wrap : 0x200000u;
    static const uint32_t pats[] = {0xA5A5A5A5u, 0x5A5A5A5Au, 0xFFFFFFFFu, 0, 0xDEADBEEFu};
    int err = 0, tot = 0;
    for (uint32_t off = 0x40u; off < span; off += (span / 8u > 4 ? span / 8u : 0x40u))
    {
        for (unsigned k = 0; k < sizeof(pats) / sizeof(pats[0]); k++)
        {
            tot++;
            if (st_wr(base + off, pats[k]) || st_rd(base + off) != pats[k])
                err++;
        }
    }
    ST("window @0x%08x: r/w reliability %s (err=%d/%d)", (unsigned)base, err ? "FAIL" : "PASS", err, tot);
}

__attribute__((unused)) static void st_mem_probe(void)
{
    ST("---- memory map probe (rd, +wr where safe) ----");
    /* on-chip RAM extent (writable) */
    static const uint32_t ram[] = {0x20000000, 0x20040000, 0x2004a000, 0x20060000,
                                   0x20080000, 0x200a0000, 0x20100000};
    for (unsigned i = 0; i < sizeof(ram) / sizeof(ram[0]); i++)
        st_probe1(ram[i], 1);
    /* flash (read-only, no write) */
    st_probe1(0x2C000000, 0);
    st_probe1(0x28000000, 0);
    st_probe1(0x10000000, 0);
    st_probe1(0x14000000, 0);
    /* PSRAM-window candidates (try write — only succeeds if mapped+inited) */
    static const uint32_t ps[] = {0x18000000, 0x1C000000, 0x34000000, 0x38000000,
                                  0x24000000, 0x30000000, 0x42000000, 0x60000000};
    for (unsigned i = 0; i < sizeof(ps) / sizeof(ps[0]); i++)
        st_probe1(ps[i], 1);
    /* characterise the writable windows found above (size + reliability) */
    st_window_characterize(0x34000000u); /* external PSRAM candidate */
    ST("---- mem probe done ----");
}

/* ---- 4. I2C bus scan (all 6 controllers; hal_i2c_open auto-muxes the pads) --- */
__attribute__((unused)) static void st_i2c_scan(int id)
{
    struct HAL_I2C_CONFIG_T cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.mode = HAL_I2C_API_MODE_SIMPLE;
    cfg.use_sync = 1;
    cfg.as_master = 1;
    cfg.speed = 100000;

    uint32_t r = hal_i2c_open((enum HAL_I2C_ID_T)id, &cfg);
    if (r)
    {
        ST("i2c%d: open FAIL 0x%x", id, r);
        return;
    }

    /* Curated common smart-watch sensor addresses (accel/IMU/mag/HR-PPG/touch/baro/
     * charger). A floating/unpopulated bus NAK-times-out ~1ms each here (yield=1);
     * a populated MiBand bus ACKs its sensors fast. TODO(vela): replace with the
     * exact MiBand9 bus+addresses once the board I2C map is known. */
    static const uint8_t addrs[] = {
        0x0C,
        0x0D,
        0x0E,
        0x0F,
        0x18,
        0x19,
        0x1E,
        0x38,
        0x3B,
        0x44,
        0x45,
        0x57,
        0x5A,
        0x5D,
        0x68,
        0x69,
        0x6A,
        0x6B,
        0x76,
        0x77,
    };
    int found = 0;
    uint32_t t0 = hal_sys_timer_get();
    for (unsigned k = 0; k < sizeof(addrs); k++)
    {
        uint8_t a = addrs[k], b = 0;
        uint32_t act = 0;
        uint32_t ret = hal_i2c_mst_read((enum HAL_I2C_ID_T)id, a, &b, 1, &act, 0, 1, 1);
        if (ret == 0)
        {
            ST("i2c%d: *** ACK @0x%02x data=0x%02x ***", id, a, b);
            found++;
        }
    }
    uint32_t dt = hal_sys_timer_get() - t0;
    ST("i2c%d: scan done, %d dev, %u ticks", id, found, (unsigned)dt);
    hal_i2c_close((enum HAL_I2C_ID_T)id);
}

/* I2C pin brute-force: the MiBand9 sensors aren't on the stock best1503 I2C pads,
 * so route I2C controller 0 to every adjacent pad pair (both orderings, internal
 * pull-ups on) and scan common sensor addresses. A pair that ACKs = the real pins.
 * Skips the debug-UART pads P2_2/P2_3 (idx 18/19) so the trace survives. */
__attribute__((unused)) static void st_i2c_bruteforce(void)
{
    struct HAL_I2C_CONFIG_T cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.mode = HAL_I2C_API_MODE_SIMPLE;
    cfg.use_sync = 1;
    cfg.as_master = 1;
    cfg.speed = 100000;
    if (hal_i2c_open(HAL_I2C_ID_0, &cfg))
    {
        ST("bf: i2c0 open fail");
        return;
    }
    ST("---- I2C pin brute-force (ctrl0 over adjacent pad pairs, pullups on) ----");
    static const uint8_t addrs[] = {0x18, 0x19, 0x1E, 0x38, 0x44, 0x57, 0x68, 0x6A};
    /* NOTE: on the MiBand9 board almost every pad above P1_1 is owned by a peripheral
     * (QSPI flash, ISPI/PMU, clocks, …) and re-muxing it HardFaults the running FW;
     * best1503's hal_iomux_get_function() returns 0 for all pads so it can't gate this.
     * The genuinely-free pads (P0_0..P1_1) carry no sensors. So we only sweep the safe
     * range here — finding the real sensor bus needs the board I2C map + sensor power
     * rail (logic-analyzer on the stock FW), not a blind pad sweep. */
    int total = 0, prev_a = -1, prev_b = -1;
    for (int i = 0; i < 9; i++)
    { /* P0_0..P1_1 = the only safe-to-remux pads */
        ST("bf: pad idx %d ...", i);
        hal_trace_flush_buffer(); /* accurate progress */
        for (int sw = 0; sw < 2; sw++)
        {
            int scl = sw ? i + 1 : i, sda = sw ? i : i + 1;
            /* restore the previous pair to GPIO first */
            if (prev_a >= 0)
            {
                struct HAL_IOMUX_PIN_FUNCTION_MAP g[2] = {
                    {(enum HAL_IOMUX_PIN_T)prev_a, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
                    {(enum HAL_IOMUX_PIN_T)prev_b, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL}};
                hal_iomux_init(g, 2);
                prev_a = prev_b = -1;
            }
            /* hal_iomux_get_function() returns 0 for every pad on best1503 (doesn't
             * reflect peripheral ownership), so skip the firmware-critical pads by
             * index: UART P2_2/3 (18/19), QSPI-flash P3_0..5 (24..29), and P1_2/3
             * (10/11) which crash when re-muxed (ISPI/PMU or clock). */
            if (scl == 18 || scl == 19 || sda == 18 || sda == 19)
                continue;
            if ((scl >= 24 && scl <= 29) || (sda >= 24 && sda <= 29))
                continue;
            if (scl == 10 || scl == 11 || sda == 10 || sda == 11)
                continue;
            struct HAL_IOMUX_PIN_FUNCTION_MAP m[2] = {
                {(enum HAL_IOMUX_PIN_T)scl, HAL_IOMUX_FUNC_MCU_I2C_M0_SCL, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
                {(enum HAL_IOMUX_PIN_T)sda, HAL_IOMUX_FUNC_MCU_I2C_M0_SDA, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}};
            hal_iomux_init(m, 2);
            prev_a = scl;
            prev_b = sda;
            for (unsigned k = 0; k < sizeof(addrs); k++)
            {
                uint8_t b = 0;
                uint32_t act = 0;
                if (hal_i2c_mst_read(HAL_I2C_ID_0, addrs[k], &b, 1, &act, 0, 1, 1) == 0)
                {
                    ST("*** SCL=P%d_%d SDA=P%d_%d : ACK @0x%02x data=0x%02x ***",
                       scl / 8, scl % 8, sda / 8, sda % 8, addrs[k], b);
                    total++;
                }
            }
        }
    }
    ST("---- brute-force done: %d ACK(s) ----", total);
    hal_i2c_close(HAL_I2C_ID_0);
}

/* ============ software (bit-bang) I2C scanner with power-rail discovery ========
 * Uses raw GPIO so it works on ANY pad (incl. the extra GPIOs of the 170-ball BGA,
 * P4_4.. via the 2nd GPIO controller) and never touches the HW I2C block. For each
 * SCL/SDA pad pair it probes all 127 addresses; the whole sweep runs once with every
 * OTHER safe GPIO driven LOW and once HIGH, so if a sensor power-enable line is among
 * them, one pass powers the sensors. Critical pads (flash/UART/ISPI-PMU/clock) are
 * excluded so we don't crash the running FW. */
#include "hal_gpio.h"

/* known firmware-critical pad indices (re-muxing/driving these HardFaults XIP/PMU) */
static int st_pad_critical(int p)
{
    return (p == 18 || p == 19)    /* UART0 P2_2/P2_3 (trace)         */
           || (p >= 24 && p <= 29) /* QSPI flash P3_0..P3_5          */
           || (p == 10 || p == 11) /* P1_2/P1_3 (crash on drive)     */
           || (p >= 32 && p <= 35) /* P4_0..P4_3 (crash on drive)    */
           || (p >= 38);           /* P4_6+ crash (P4_4/P4_5 OK)     */
}
static void st_pad_gpio_out(int p, int level)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP m = {(enum HAL_IOMUX_PIN_T)p, HAL_IOMUX_FUNC_AS_GPIO,
                                           HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL};
    hal_iomux_init(&m, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)p, HAL_GPIO_DIR_OUT, level ? 1 : 0);
}
static int g_scl, g_sda;
static inline void swdly(void)
{
    for (volatile int i = 0; i < 40; i++)
        __NOP();
}
static inline void SCL(int hi) { hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)g_scl, HAL_GPIO_DIR_OUT, hi ? 1 : 0); }
static inline void SDA(int hi)
{
    if (hi)
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)g_sda, HAL_GPIO_DIR_IN, 0);
    else
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)g_sda, HAL_GPIO_DIR_OUT, 0);
}
static inline int SDR(void) { return hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)g_sda); }
static int sw_i2c_probe(uint8_t addr)
{
    SDA(1);
    SCL(1);
    swdly();
    SDA(0);
    swdly();
    SCL(0);
    swdly(); /* START */
    uint8_t b = (uint8_t)((addr << 1) | 0);
    for (int i = 0; i < 8; i++)
    {
        SDA(b & 0x80);
        b <<= 1;
        swdly();
        SCL(1);
        swdly();
        SCL(0);
        swdly();
    }
    SDA(1);
    swdly();
    SCL(1);
    swdly();
    int ack = SDR();
    SCL(0);
    swdly(); /* read ACK */
    SDA(0);
    SCL(1);
    swdly();
    SDA(1);
    swdly();         /* STOP */
    return ack == 0; /* 1 = device acked */
}
static void sw_i2c_scan(void)
{
    /* config the two I2C pads with internal pull-ups */
    struct HAL_IOMUX_PIN_FUNCTION_MAP m[2] = {
        {(enum HAL_IOMUX_PIN_T)g_scl, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {(enum HAL_IOMUX_PIN_T)g_sda, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}};
    hal_iomux_init(m, 2);
    /* idle-bus health: release both lines, both must read high (pull-ups). If SDA is
     * stuck low the "bus" is just two shorted/grounded GPIOs -> every address would
     * false-ACK; skip those. */
    SCL(1);
    SDA(1);
    swdly();
    swdly();
    if (SDR() == 0)
        return; /* SDA stuck low -> not a real bus */

    uint8_t hits[10];
    int n = 0;
    for (uint8_t a = 1; a <= 0x7F; a++)
        if (sw_i2c_probe(a))
        {
            if (n < 10)
                hits[n] = a;
            n++;
        }
    if (n == 0 || n >= 16)
        return; /* none, or all-ACK (stuck) -> ignore */
    for (int k = 0; k < n && k < 10; k++)
        ST("*** SW-I2C SCL=P%d_%d SDA=P%d_%d : %d dev, ACK @0x%02x ***",
           g_scl / 8, g_scl % 8, g_sda / 8, g_sda % 8, n, hits[k]);
}
/* find which pads are safe to drive as GPIO output (driving a flash/PMU/clock pad
 * HardFaults). Logs each pad with a flush so the last "driving" before silence = the
 * culprit; "OK" lines = the safe-to-drive set. Restores each pad to input after. */
static void st_drive_safety_scan(int start, int max_pad)
{
    ST("---- pad drive-safety scan %d..%d ----", start, max_pad);
    hal_trace_flush_buffer();
    for (int p = start; p < max_pad; p++)
    {
        if (st_pad_critical(p))
        {
            ST("  P%d_%d skip(known-crit)", p / 8, p % 8);
            hal_trace_flush_buffer();
            continue;
        }
        ST("  P%d_%d drive..", p / 8, p % 8);
        hal_trace_flush_buffer();
        st_pad_gpio_out(p, 0);
        for (volatile int i = 0; i < 2000; i++)
            __NOP();
        st_pad_gpio_out(p, 1);
        for (volatile int i = 0; i < 2000; i++)
            __NOP();
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)p, HAL_GPIO_DIR_IN, 0);
        ST("  P%d_%d OK", p / 8, p % 8);
        hal_trace_flush_buffer();
    }
    ST("---- drive-safety done ----");
    hal_trace_flush_buffer();
}

static void sw_i2c_power_sweep(int max_pad)
{
    for (int lvl = 0; lvl < 2; lvl++)
    {
        ST("---- SW-I2C sweep: all other GPIOs driven %s ----", lvl ? "HIGH" : "LOW");
        for (int i = 0; i < max_pad - 1; i++)
        {
            if (st_pad_critical(i) || st_pad_critical(i + 1))
                continue;
            /* drive every safe pad except the two I2C ones to the power level */
            for (int p = 0; p < max_pad; p++)
                if (p != i && p != i + 1 && !st_pad_critical(p))
                    st_pad_gpio_out(p, lvl);
            g_scl = i;
            g_sda = i + 1;
            sw_i2c_scan();
            g_scl = i + 1;
            g_sda = i;
            sw_i2c_scan();
            hal_trace_flush_buffer();
            if ((i & 7) == 0)
                ST("  sw sweep lvl=%d idx=%d", lvl, i);
        }
    }
    ST("---- SW-I2C power sweep done ----");
}

/* ===================== full-size SRAM test ===================================
 * The firmware links into 0x20000000..~0x2004a000 (296 KB) and an MPU (mpu_cfg())
 * faults accesses past it. best1503 actually has RAM0..RAM3 = 4x128 KB = 512 KB at
 * 0x20000000..0x20080000 (+ RAMCP cache RAM). To test the *whole* SRAM we drop the
 * MPU, map the real extent, then march-test the banks the running FW is NOT using
 * (RAM2-upper + RAM3, i.e. 0x2004c000..0x20080000), then restore the MPU. */
#include "mpu_cfg.h"
static int st_march(uint32_t a0, uint32_t a1) /* address-uniqueness + inverse + patterns */
{
    int err = 0;
    volatile uint32_t *p;
    for (uint32_t a = a0; a < a1; a += 4)
        *(volatile uint32_t *)a = a; /* write addr   */
    for (uint32_t a = a0; a < a1; a += 4)
        if (*(volatile uint32_t *)a != a)
            err++; /* verify uniq  */
    for (uint32_t a = a0; a < a1; a += 4)
        *(volatile uint32_t *)a = ~a; /* write ~addr  */
    for (uint32_t a = a0; a < a1; a += 4)
        if (*(volatile uint32_t *)a != ~a)
            err++;
    static const uint32_t pats[] = {0x00000000u, 0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u};
    for (unsigned k = 0; k < sizeof(pats) / sizeof(pats[0]); k++)
    {
        for (uint32_t a = a0; a < a1; a += 4)
            *(volatile uint32_t *)a = pats[k];
        for (uint32_t a = a0; a < a1; a += 4)
            if (*(volatile uint32_t *)a != pats[k])
                err++;
    }
    (void)p;
    return err;
}
static void st_sram_full_test(void)
{
    ST("---- full SRAM test (MPU off) ----");
    ARM_MPU_Disable(); /* remove the FW's RAM-bound protection */
    /* 1) map the real writable extent (4 KB steps, fault-tolerant) */
    uint32_t end = 0x20000000u;
    for (uint32_t a = 0x20000000u; a < 0x20200000u; a += 0x1000u)
    {
        g_probe_faulted = 0;
        g_probe_active = 1;
        __DSB();
        __ISB();
        volatile uint32_t *p = (volatile uint32_t *)a;
        uint32_t o = *p;
        *p = 0xA5A5A5A5u;
        __DSB();
        __ISB();
        uint32_t rb = *p;
        *p = o;
        __DSB();
        __ISB();
        int f = g_probe_faulted;
        g_probe_active = 0;
        if (f || rb != 0xA5A5A5A5u)
            break;
        end = a + 0x1000u;
    }
    ST("SRAM writable 0x20000000..0x%08x = %u KB (banks RAM0..RAM3=512KB expected)",
       (unsigned)end, (unsigned)((end - 0x20000000u) / 1024));
    hal_trace_flush_buffer();
    /* aliasing check: a marker in RAM3 (unused) must NOT reappear higher up; if it
     * does, the "extra" space is just the 512 KB SRAM mirrored, not real memory. */
    *(volatile uint32_t *)0x20060000u = 0xC0DE1234u;
    __DSB();
    for (uint32_t off = 0x20000u; off <= 0x100000u; off <<= 1)
    {
        uint32_t a = 0x20060000u + off;
        if (a < end)
            ST("  alias chk 0x%08x rd=0x%08x%s", (unsigned)a,
               (unsigned)*(volatile uint32_t *)a,
               (*(volatile uint32_t *)a == 0xC0DE1234u) ? "  <== MIRRORS RAM3!" : "");
    }
    hal_trace_flush_buffer();
    /* 2) march-test the FW-unused banks (don't touch 0x20000000..0x2004c000 = live FW) */
    uint32_t t0 = 0x2004c000u, t1 = end; /* march the WHOLE FW-free extent (full size) */
    if (t1 > t0)
    {
        int err = st_march(t0, t1);
        ST("SRAM march %uKB: %s err=%d", (unsigned)((t1 - t0) / 1024), err ? "FAIL" : "PASS", err);
        hal_trace_flush_buffer();
    }
    else
        ST("SRAM: no FW-free bank above 0x2004c000 to march-test");
    mpu_cfg(); /* restore the firmware MPU */
    ST("---- full SRAM test done (MPU restored) ----");
    hal_trace_flush_buffer();
}

/* ===================== in-system flash write (OTA core) =======================
 * Demonstrate the running FW can erase+program its own NOR flash from RAM (the
 * "RAM flash algo" needed for OTA). hal_norflash_* run from SRAM and gate XIP during
 * the op. Tests a spare sector (offset 2 MB, well past the ~1.08 MB image), verifies,
 * then restores the original content. int_lock around each op so XIP isn't fetched
 * mid-erase. */
#include "hal_norflash.h"
static void st_flash_test(void)
{
    ST("---- flash (RAM-flash-algo / OTA core) test ----");
    hal_trace_flush_buffer();
    uint8_t buf[8];
    hal_norflash_read(HAL_FLASH_ID_0, 0, buf, 8);
    ST("flash@off0 = %02x %02x %02x %02x (expect 1c ec 57 be magic)", buf[0], buf[1], buf[2], buf[3]);
    hal_trace_flush_buffer();

    uint32_t off = 0x200000u; /* 2 MB in = spare */
    static uint8_t orig[256], test[256], rb[256];
    for (int i = 0; i < 256; i++)
        test[i] = (uint8_t)(i ^ 0x5A);
    hal_norflash_read(HAL_FLASH_ID_0, off, orig, 256);

    /* read actual flash via the NON-cached alias so the XIP cache can't show stale data */
    volatile uint8_t *fnc = (volatile uint8_t *)(0x28000000u + off);
    dbg_hex16(0xF1F1);
    uint32_t lk = int_lock();
    hal_norflash_disable_protection(HAL_FLASH_ID_0); /* flash sectors are WP by default */
    enum HAL_NORFLASH_RET_T e = hal_norflash_erase(HAL_FLASH_ID_0, off, 4096);
    int_unlock(lk);
    dbg_hex16(0xF2F2);
    ST("after erase: nc=%02x %02x %02x %02x (expect ff ff ff ff)", fnc[0], fnc[1], fnc[2], fnc[3]);
    hal_trace_flush_buffer();
    lk = int_lock();
    enum HAL_NORFLASH_RET_T w = hal_norflash_write(HAL_FLASH_ID_0, off, test, 256);
    int_unlock(lk);
    dbg_hex16(0xF3F3);
    int err = 0;
    for (int i = 0; i < 256; i++)
    {
        rb[i] = fnc[i];
        if (rb[i] != test[i])
            err++;
    }
    ST("flash erase+write+verify @0x%x: erase=%d write=%d err=%d : %s",
       (unsigned)off, e, w, err, err ? "FAIL" : "PASS");
    hal_trace_flush_buffer();
    /* restore original content */
    lk = int_lock();
    hal_norflash_erase(HAL_FLASH_ID_0, off, 4096);
    hal_norflash_write(HAL_FLASH_ID_0, off, orig, 256);
    int_unlock(lk);
    ST("---- flash test done (sector restored) ----");
    hal_trace_flush_buffer();
}

/* ===================== sleep / low-power demo =================================
 * Enable chip deep power-down and sleep in ~3 s windows, waking like a BLE
 * advertiser would. Even if some peripheral isn't gated yet, the PPK2 current
 * should drop during each window and spike on wake. */
#include "hal_sleep.h"
#include "cmsis_os2.h"
static void st_sleep_loop(void)
{
    ST("---- SLEEP demo: power-down enable, wake ~every 3s (BLE-like) ----");
    hal_trace_flush_buffer();
    hal_sleep_power_down_enable(); /* allow chip deep sleep */
    /* BLE advertiser profile: a short wake (~200 ms "adv event") then sleep ~2.8 s.
     * On the PPK2 this is a low sleep floor with a brief current spike every ~3 s. */
    for (int i = 0; i < 9; i++)
    {
        ST("[ble] WAKE %d (adv event)", i);
        hal_trace_flush_buffer();
        uint32_t t0 = hal_sys_timer_get();
        while ((hal_sys_timer_get() - t0) < MS_TO_TICKS(200))
        {
            __NOP();
        } /* brief wake */
        for (int u = 0; u < HAL_SYS_WAKE_LOCK_USER_QTY; u++)
            hal_sys_wake_unlock((enum HAL_SYS_WAKE_LOCK_USER_T)u);
        osDelay(2800); /* sleep until next adv interval */
    }
    ST("---- SLEEP demo done ----");
    hal_trace_flush_buffer();
}

/* ===================== RM690C0/B0 LCDC smart-panel ID read =====================
 * Ported straight from the MiBand9 stock Vela FW (vela_ap.bin) reverse-engineering:
 *   - the display is driven by the LCDC "smart panel" (SMPN / "MCU SPILCD") interface
 *     at LCDC_BASE = 0x40200000 (matches the SDK reg_lcdc_v1.h: smpn_ctrl 0x188,
 *     pn_ctrl1 0x194, sclk_div 0x1A8, misc_ctrl 0x1F8, pn_gra_ctrl 0x264, ...).
 *   - panel ID via MIPI-DCS RDID1/2/3 (0xDA/0xDB/0xDC) -> 24-bit id (stock
 *     rm690c0_check_id). Known IDs: 0x260120 VXN, 0x260220 TM, 0x460120 O66 VXN,
 *     0x460220 O66 TM, 0x460420 O66 XL; else default rm690b0.
 *   - controller config = stock sub_2C564E0C(2,4); 1-byte read = stock
 *     screen_spi_transfer(); command-done poll = stock sub_2C564D7C (spu_irq_isr bit18).
 * This is the verified hardware path (the earlier DSI@0x40160000 guess was wrong;
 * 0x40160000 is a TIMER). Everything is wrapped in the fault-tolerant probe so a
 * wrong clock/base prints a clean FAULT line instead of hanging. */
#include "hal_cmu.h"
#include "hal_gpio.h"
#include "hal_psc.h"
extern void hal_iomux_set_spilcd(void);

#define LCDC_BASE_ADDR 0x40200000u
#define LCDC_R(off) (*(volatile uint32_t *)(LCDC_BASE_ADDR + (off)))
#define LCDC_SMPN_CTRL 0x188 /* lcdc_smpn_ctrl  */
#define LCDC_SCLK_DIV 0x1A8
#define LCDC_PN_CTRL1 0x194
#define LCDC_MISC_CTRL 0x1F8
#define LCDC_VDMA_BURST 0x22C
#define LCDC_PN_GRA_CTRL 0x264
#define LCDC_CFG_180 0x180   /* SMPN base config / read-mode select */
#define LCDC_SMPN_DATA 0x284 /* command/data FIFO (reg<<8 | 0x03000000) */
#define LCDC_SMPN_TRIG 0x280 /* =1 : start transfer                    */
#define LCDC_SMPN_MISC2A8 0x2A8
#define LCDC_RX_FIFO 0x140 /* read data FIFO                          */
#define LCDC_LEN_27C 0x27C /* (words-1)<<16                           */
#define LCDC_IRQ_ISR 0x1C4 /* spu_irq_isr, bit18 (0x40000)=cmd done   */
#define LCDC_LINE_RDY_230 0x230

/* ===== EXACT best1503 CMU display-clock bring-up (now readable: ex-ROM at 0x26xxxx
 * is RAM mirrored from 0x20000000).  Replicates sub_2C564E0C's clock prologue:
 *   sub_2C4DDA80(48) = sub_26F6B8(48)  -> display HCLK enable + reset toggles
 *   sub_2C4DD970(96) = sub_26F7BA(96)  -> 96MHz display sub-clock
 * CMU @ 0x40000000.  sub_275678(1) = ~1 AON-timer tick delay. ===== */
#define CMU_R(o) (*(volatile uint32_t *)(0x40000000u + (o)))
static void cmu_tick(void)
{
    for (volatile int i = 0; i < 60; i++)
        __NOP();
}
#define CMK(s) \
    do         \
    {          \
    } while (0) /* CMU sequence proven; markers silenced */
static void stock_cmu_display_clock(void)
{
    /* sub_26F468(48): secondary display clock-domain cfg (bits[12:8] of 0x104/0x100) */
    CMU_R(0x104) = (CMU_R(0x104) & 0xFFFFE0FFu) | 0x1B00u;
    CMU_R(0x100) = (CMU_R(0x100) & 0xFFFFE0FFu) | 0x400u;
    CMK("F468 src done");
    /* sub_26F6B8(48) body: display HCLK enable + reset sequence.
     * NOTE: 0x00/0x10/0x34 are written as direct stores in the stock, but to avoid
     * clobbering live HCLKs on this running system we OR the module bit in. */
    CMU_R(0x000) |= 0x200000u;
    CMK("000");
    CMU_R(0x140) = 64u;
    CMK("140");
    CMU_R(0x010) = 0x200000u;
    CMK("010");
    CMU_R(0x044) = 0x20000000u;
    cmu_tick();
    CMK("044=29");
    CMU_R(0x048) = 0x20000000u;
    CMK("048=29");
    CMU_R(0x150) = 64u;
    cmu_tick();
    CMU_R(0x154) = 64u;
    CMK("150/154=6");
    CMU_R(0x150) = 256u;
    cmu_tick();
    CMU_R(0x154) = 256u;
    CMK("150/154=8");
    CMU_R(0x034) = 0x200000u;
    cmu_tick();
    CMK("034"); /* direct write (read stalls) */
    CMU_R(0x038) = (1u << 21);
    CMK("038=21");
    CMU_R(0x150) = 128u;
    cmu_tick();
    CMU_R(0x154) = 128u;
    CMK("150/154=7");
    CMU_R(0x044) = 0x10000000u;
    cmu_tick();
    CMK("044=28");
    CMU_R(0x048) = 0x10000000u;
    CMK("048=28");
    /* sub_26F578(96): clock source/divider for the 96MHz display domain (bits[17:13]) */
    CMU_R(0x104) = (CMU_R(0x104) & 0xFFFC1FFFu) | 0x3A000u;
    CMU_R(0x100) = (CMU_R(0x100) & 0xFFFC1FFFu) | 0x4000u;
    CMK("F578 src done");
    /* sub_26F7BA(96) tail */
    CMU_R(0x168) = 1024u;
    CMU_R(0x048) = 0x200000u;
    CMU_R(0x178) = 1024u;
    CMK("F7BA done");
}

/* sub_26F578(12): the per-TRANSFER SMPN clock (dword_200903A0 = 12MHz).  screen_spi_
 * transfer switches to this slow clock around every panel transfer; without it the
 * SMPN engine has no SCLK and the command never completes (lcdc_wait_done times out).
 * Sets the 0x180 divider field [29:25]=15 and routes 0x100/0x104 bits[17:13]. */
/* Port of stock cmu_disp_clk_set_freq (sub_26F578): set the display/SMPN clock to
 * 'mhz' MHz.  Fixed PLL taps for 24/25, 48-50, 96-104 MHz; otherwise a DIVIDED clock
 * (0x180 divider = (mhz+199)/mhz - 2, valid ~12..199 MHz).  Higher MHz = faster SMPN/
 * FIFO transfers.  (PLL re-lock ROM calls omitted -- the divided path doesn't need them,
 * and the original hardcoded 12/96 MHz setters worked without them.) */
static int cmu_disp_clk_set_freq(unsigned int mhz)
{
    uint32_t r104 = CMU_R(0x104);
    uint32_t r100 = CMU_R(0x100) & 0xFFFC1FFFu;
    if (mhz - 24u <= 1u)
    { /* 24-25 MHz (PLL tap) */
        r104 |= 0x3E000u;
    }
    else if (mhz - 48u <= 2u)
    { /* 48-50 MHz (PLL tap) */
        r104 = (r104 & 0xFFFC1FFFu) | 0x36000u;
        r100 |= 0x8000u;
    }
    else if (mhz - 96u <= 8u)
    { /* 96-104 MHz (PLL tap, idle clock) */
        r104 = (r104 & 0xFFFC1FFFu) | 0x3A000u;
        r100 |= 0x4000u;
    }
    else if (mhz <= 199u)
    { /* divided clock (e.g. 12,16,30,40,60..) */
        if (mhz == 0u)
        {
            CMU_R(0x104) = 0x3E000u;
            return -1;
        }
        uint32_t div = (mhz + 199u) / mhz - 2u;
        if (div > 0xFu)
        {
            CMU_R(0x104) = 0x3E000u;
            return -1;
        }
        CMU_R(0x180) = (CMU_R(0x180) & 0xE1FFFFFFu) | (div << 25);
        r104 = (r104 & 0xFFFC1FFFu) | 0x2C000u;
        r100 |= 0x12000u;
    }
    else
    { /* >199 MHz */
        r104 = (r104 & 0xFFFC1FFFu) | 0xE000u;
        r100 |= 0x30000u;
    }
    CMU_R(0x104) = r104;
    CMU_R(0x100) = r100;
    return 0;
}
/* SMPN transfer clock -- stock uses 12 MHz; raise SMPN_XFER_MHZ for faster FIFO writes
 * (try 24/48 PLL taps or e.g. 30/40/60 divided). */
#ifndef SMPN_XFER_MHZ
#define SMPN_XFER_MHZ 12
#endif
static void cmu_smpn_xfer_clock(unsigned int mhz) { cmu_disp_clk_set_freq(mhz); }
/* sub_26F578(96): the IDLE display clock (stock restores this after each transfer). */
static void cmu_disp_clk_96(void) { cmu_disp_clk_set_freq(96u); }

/* Port of stock cmu_disp_clk2_set_freq (sub_26F468): the SECOND display clock (clk2),
 * via CMU 0x104 bits[12:8] + 0x100 bits[12:9] (DIFFERENT field than the main clk). */
static void cmu_disp_clk2_set_freq(unsigned int mhz)
{
    uint32_t r104 = CMU_R(0x104);
    uint32_t r100 = CMU_R(0x100) & 0xFFFFE0FFu;
    if (mhz - 24u <= 1u)
    {
        r104 |= 0x1F00u;
    }
    else if (mhz - 48u <= 2u)
    {
        r104 = (r104 & 0xFFFFE0FFu) | 0x1B00u;
        r100 |= 0x400u;
    }
    else if (mhz - 96u <= 8u)
    {
        r104 = (r104 & 0xFFFFE0FFu) | 0x1D00u;
        r100 |= 0x200u;
    }
    else
    {
        r104 = (r104 & 0xFFFFE0FFu) | 0x700u;
        r100 |= 0x1800u;
    }
    CMU_R(0x104) = r104;
    CMU_R(0x100) = r100;
}

/* *** FULL display clock-domain enable = stock cmu_disp_clk_enable_seq(48) + cmu_disp_clk_96_seq(96)
 *     (sub_26F6B8 + sub_26F7BA, transcribed from the disasm). TWO clocks (clk2@48 + main@96) plus
 *     ~20 CMU clock-gate enables across 0x00/0x10/0x34/0x38/0x44/0x48/0x140/0x150/0x154/0x168/0x178.
 *     My old lcdc_pixel_clock_enable did ONLY 0x44/0x48 -> the SMPN/DMA clock domains were never
 *     enabled, so the data transfer never clocked (no header, no data). cmu_clk_helper(2/3) = the
 *     PSC display-island power-up, already done in st_display_id, so omitted here. */
static void cmu_disp_clk_full_seq(void)
{
    /* === cmu_disp_clk_enable_seq(48) === */
    cmu_disp_clk2_set_freq(48u);
    CMU_R(0x10) = 0x80000000u;
    CMU_R(0x140) = 0x80u;
    CMU_R(0x00) = 0x200000u;
    CMU_R(0x140) = 0x40u;
    CMU_R(0x10) = 0x20000000u;
    CMU_R(0x10) = 0x8000000u;
    CMU_R(0x10) = 0x10000000u;
    CMU_R(0x10) = 0x200000u;
    CMU_R(0x44) = 0x20000000u;
    hal_sys_timer_delay_us(30);
    CMU_R(0x48) = 0x20000000u;
    CMU_R(0x150) = 0x40u;
    hal_sys_timer_delay_us(30);
    CMU_R(0x154) = 0x40u;
    CMU_R(0x150) = 0x100u;
    hal_sys_timer_delay_us(30);
    CMU_R(0x154) = 0x100u;
    CMU_R(0x34) = 0x200000u;
    hal_sys_timer_delay_us(30);
    CMU_R(0x38) = 0x200000u; /* cmu_set_clock_enable_bit(21): CMU[0x38] = 1<<21 */
    CMU_R(0x150) = 0x80u;
    hal_sys_timer_delay_us(30);
    CMU_R(0x154) = 0x80u;
    CMU_R(0x44) = 0x8000000u;
    CMU_R(0x44) = 0x10000000u;
    hal_sys_timer_delay_us(30);
    CMU_R(0x48) = 0x8000000u;
    CMU_R(0x48) = 0x10000000u;
    /* === cmu_disp_clk_96_seq(96) === */
    cmu_disp_clk_set_freq(96u);
    CMU_R(0x168) = 0x400u;
    CMU_R(0x48) = 0x200000u;
    CMU_R(0x178) = 0x400u;
}

/* stock sub_2C564E0C(a1=2, a2=4): configure the SMPN pixel/command pipeline */
static void lcdc_smpn_config(void)
{
    LCDC_R(LCDC_SCLK_DIV) = 1;
    LCDC_R(LCDC_SMPN_MISC2A8) = 0x10009F01;
    LCDC_R(LCDC_MISC_CTRL) = 67;                      /* a2=4 path */
    LCDC_R(LCDC_CFG_180) = 0x2000F89;                 /* a1=2 path */
    LCDC_R(LCDC_VDMA_BURST) = 0x80000000 | 0x6002C00; /* a1=2: 0x80000000|0x6002C00 */
    LCDC_R(LCDC_PN_CTRL1) = 0x81;
    LCDC_R(LCDC_SMPN_CTRL) = 0x2501; /* a1=2 */
    LCDC_R(LCDC_PN_GRA_CTRL) = 1;
}

static uint32_t g_last_rx = 0;
static int g_last_done = 0; /* 1 if the last SMPN transfer completed (bit18 set) */

/* stock sub_2C564D7C: wait for SMPN command-done (spu_irq_isr bit 0x40000) */
static int lcdc_wait_done(void)
{
    uint32_t t0 = hal_sys_timer_get();
    while ((LCDC_R(LCDC_IRQ_ISR) & 0x40000u) == 0)
    {
        if ((hal_sys_timer_get() - t0) > MS_TO_TICKS(5))
            return -1;
    }
    LCDC_R(LCDC_IRQ_ISR) &= ~0x40000u;
    return 0;
}

/* Panel chip-select = GPIO pin 41 (P5_1).  Decoded: the stock transfer "bracket"
 * sub_26FF90(1/0) is just hal_gpio_pin_clr/set(0x29=41) — CS asserted LOW around the
 * transfer.  Muxed to GPIO func 0; driven manually here (the LCDC does NOT auto-CS). */
static void gpio_set(int pin); /* defined below with the other SoC-GPIO helpers */
static void gpio_clr(int pin);
/* best1503 iomux writer — 4 bits/pin, 8 pins/reg, base 0x40086000: pin p -> reg
 * (base + 0x04 + (p/8)*4), shift (p%8)*4. Proven correct on this silicon by
 * dbg_boot_init muxing P2_2/P2_3 (UART0) at base+0x0C. func 0=GPIO, alt funcs 1..6
 * (SDK index_to_func_val); MCU_SPILCD = func value 4. */
static void iomux_set(int pin, int func)
{
    /* stock sub_2006FBA4: pins<32 -> base 0x40086004, pins>=32 -> base 0x4008607C */
    volatile uint32_t *r = (pin < 32)
                               ? (volatile uint32_t *)(0x40086004u + (uint32_t)(pin >> 3) * 4)
                               : (volatile uint32_t *)(0x4008607Cu + (uint32_t)((pin - 32) >> 3) * 4);
    int sh = (pin & 7) * 4;
    *r = (*r & ~(0xFu << sh)) | (((uint32_t)func & 0xF) << sh);
}

/* direct SoC GPIO (stock g_gpio_bank_base: bank0=0x40081000, bank1=0x40089000,
 * bank2=0x4008A000). reg_gpio_v2: DR_SET@0x00, DDR_SET@0x04, DR_CLR@0x0C. */
static const uint32_t g_gpio_bank[3] = {0x40081000u, 0x40089000u, 0x4008A000u};
static void gpio_dir_out(int pin) { *(volatile uint32_t *)(g_gpio_bank[pin >> 5] + 0x04) = 1u << (pin & 31); }
static void gpio_dir_in(int pin) { *(volatile uint32_t *)(g_gpio_bank[pin >> 5] + 0x10) = 1u << (pin & 31); }
static void gpio_set(int pin) { *(volatile uint32_t *)(g_gpio_bank[pin >> 5]) = 1u << (pin & 31); }
static void gpio_clr(int pin) { *(volatile uint32_t *)(g_gpio_bank[pin >> 5] + 0x0C) = 1u << (pin & 31); }
static int gpio_rd(int pin) { return (*(volatile uint32_t *)(g_gpio_bank[pin >> 5] + 0x50) >> (pin & 31)) & 1; }

#define LCD_CS_PIN 41
static void lcd_cs(int assert)
{
    if (assert)
        gpio_clr(LCD_CS_PIN);
    else
        gpio_set(LCD_CS_PIN);
}

/* stock screen_spi_transfer() for a 1-byte register read (screen_read_reg) */
static int lcdc_smpn_read_reg(uint8_t reg)
{
    cmu_smpn_xfer_clock(SMPN_XFER_MHZ); /* stock: clk->12MHz per transfer */
    lcd_cs(1);                          /* CS LOW (assert) */
    LCDC_R(LCDC_IRQ_ISR) &= ~0x40000u;
    LCDC_R(LCDC_LEN_27C) = 3; /* delay between Tx and Rx of bytes */
    LCDC_R(LCDC_SMPN_DATA) = ((uint32_t)reg << 8) | 0x03000000u;
    LCDC_R(LCDC_CFG_180) = (((uint32_t)7 << 16) & 0xFF0000u) | 0x2001F8B;
    LCDC_R(LCDC_LINE_RDY_230) |= 0x40u;
    LCDC_R(LCDC_SMPN_TRIG) = 1;
    g_last_done = (lcdc_wait_done() == 0); /* did the transfer actually complete? */
    LCDC_R(LCDC_SMPN_DATA) = 0;
    g_last_done = (lcdc_wait_done() == 0); /* did the transfer actually complete? */

    LCDC_R(LCDC_LINE_RDY_230) &= ~0x40u;
    LCDC_R(LCDC_LEN_27C) = 0;
    hal_sys_timer_delay(2);
    uint32_t rx = LCDC_R(LCDC_RX_FIFO);
    LCDC_R(LCDC_CFG_180) = 0x2000F89; /* restore (a1=2) */
    lcd_cs(0);                        /* CS HIGH (deassert) */
    cmu_disp_clk_96();                /* stock: clk->96MHz idle after xfer */
    g_last_rx = rx;                   /* expose full word for diag */
    return (int)(rx & 0xFF);
}

/* stock LCD_WriteCMD (0x2C564F10): smart-panel command WRITE (cmd FIFO 0x284 |0x2000000,
 * data FIFO 0x40200288).  Stock brackets it with the 12MHz clock + CS low (pin41). */
static void lcdc_smpn_write(uint8_t cmd, const uint8_t *data, int len)
{
    cmu_smpn_xfer_clock(SMPN_XFER_MHZ); /* stock: clk->12MHz per transfer */
    lcd_cs(1);                          /* CS LOW (stock j_lcdc_panel_cs_0(1)) */
    LCDC_R(LCDC_IRQ_ISR) &= ~0x40000u;
    LCDC_R(LCDC_SMPN_DATA) = ((uint32_t)cmd << 8) | 0x02000000u;
    if (len > 0 && data)
    {
        if (len >= 0x20)
            len = 0x20;
        volatile uint32_t *fifo = (volatile uint32_t *)0x40200288u;
        int v7 = 0, words = (len + 3) >> 2;
        for (int w = 0; w < words; w++)
        {
            uint32_t v = 0;
            for (int k = 0; k < 4; k++)
            {
                v <<= 8;
                if (len > v7)
                {
                    v |= data[v7];
                    v7++;
                }
            }
            *fifo++ = v;
        }
    }
    LCDC_R(LCDC_SMPN_TRIG) = 1;
    if (len > 0 && data)
        LCDC_R(LCDC_SMPN_TRIG) |= ((((uint32_t)len << 7) - 16) & 0x1FF0u) | 2u;
    lcdc_wait_done();
    lcd_cs(0);         /* CS HIGH */
    cmu_disp_clk_96(); /* stock: clk->96MHz idle after xfer */
}

/* Full-frame pixel push via the SMPN FIFO -- NO DMA, NO software bit-bang. The RAMWR uses
 * the SINGLE-write opcode 0x02 (the FIFO physically clocks 1 lane; the 0x02 opcode makes the
 * panel read the 1-lane data correctly. 0x32/quad would need the GRA-DMA, which never works
 * on this LCDC). CS stays LOW for the whole frame; pixels stream in 32-byte FIFO chunks.
 * SCLK = display(48MHz)/(REG_1A8+1) ~= 25MHz -> full 212x520 frame ~89ms. */
#define DISP_SCLK_MHZ 48
static int smpn_quad_frame(const uint8_t *fb, int npix)
{
    const uint8_t *p = fb;
    int total = npix; /* 8bpp RGB332 -> 1 byte/pixel */
    int first = 1;
    cmu_smpn_xfer_clock(DISP_SCLK_MHZ);
    lcd_cs(1); /* CS LOW for the whole frame */
    while (total > 0)
    {
        LCDC_R(LCDC_IRQ_ISR) &= ~0x40000u;
        if (first)
        {
            /* entry: RAMWR 0x2C with the SINGLE-write opcode 0x02 (correct 1-lane) */
            LCDC_R(LCDC_SMPN_DATA) = (0x2Cu << 8) | 0x02000000u;
        }
        else
        {
            /* continuation: put the next 4 PIXELS (4 bytes, 8bpp) into the cmd word -- it goes
             * out first per trigger, so the panel writes it as real colour.  These 4 pixels are
             * then skipped in the FIFO.  An all-ZERO cmd word (4 black pixels) stalls the SMPN
             * (done IRQ never fires), so force it to 1 in that case. */
            uint32_t hdr = 0;
            for (int k = 0; k < 4; k++)
            {
                hdr <<= 8;
                if (k < total)
                    hdr |= p[k];
            }
            if (hdr == 0)
                hdr = 1;
            LCDC_R(LCDC_SMPN_DATA) = hdr;
            p += 4;
            total -= 4;     /* the 4 pixels are now carried by the cmd word */
            if (total <= 0) /* frame ended exactly on the cmd word */
            {
                LCDC_R(LCDC_SMPN_TRIG) = 1;
                lcdc_wait_done();
                break;
            }
        }
        int len = total > 0x20 ? 0x20 : total;
        volatile uint32_t *fifo = (volatile uint32_t *)0x40200288u;
        int v7 = 0, words = (len + 3) >> 2;
        for (int wn = 0; wn < words; wn++)
        {
            uint32_t v = 0;
            for (int k = 0; k < 4; k++)
            {
                v <<= 8;
                if (len > v7)
                {
                    v |= p[v7];
                    v7++;
                }
            }
            *fifo++ = v;
        }
        /* bit0 = send cmd word (2 pixels, or RAMWR on first), bit1 + len = FIFO pixel data */
        LCDC_R(LCDC_SMPN_TRIG) = 1 | ((((uint32_t)len << 7) - 16) & 0x1FF0u) | 2u;
        if (lcdc_wait_done() != 0)
        {
            lcd_cs(0);
            cmu_disp_clk_96();
            return -1;
        }
        p += len;
        total -= len;
        first = 0;
    }
    lcd_cs(0); /* CS HIGH -> latch */
    cmu_disp_clk_96();
    return 0;
}

/* one init command: reg + up to 4 data bytes (len<0 = no data / command-only) */
struct rm690_cmd
{
    uint8_t reg;
    int8_t len;
    uint8_t d[4];
};
/* RM690C0 variant-0 init, extracted byte-for-byte from stock rm690c0_panel_init
 * (sub_2C10C254): page-select / CASET 24-235 / RASET 0-519 / pixfmt 0x55 / TE /
 * sleep-out / display-on / page-0x50 green-screen fix.  Stock sets brightness(0x51)=0;
 * we override to 0xFF at the end so the panel is actually visible. */
static const struct rm690_cmd g_rm690_init[] = {
    /* variant-3 (O66 TM, ID 0x460220) prelude — page 0x70 + 0x20 manufacturer setup
     * the earlier (variant-2) init was MISSING.  Extracted from stock
     * rm690c0_panel_init case 3 (sub_2C10C254 @ 0x2c10c7f4).  Configures the TM
     * panel's QSPI interface / pixel format (fixes the 8-bit read + offset). */
    {0xFE, 1, {0x70}},
    {0x24, 1, {0xC4}},
    {0x2F, 1, {0xC4}},
    {0x00, 1, {0xC4}},
    {0x09, 1, {0xC4}},
    {0xFE, 1, {0x20}},
    {0x63, 1, {0x11}},
    {0x78, 1, {0x57}},
    {0xFE, 1, {0x40}},
    {0x62, 1, {0x2A}},
    {0x44, 1, {0x00}},
    {0x4D, 1, {0x00}},
    {0x4E, 1, {0x0B}},
    {0x61, 1, {0x01}},
    {0x82, 1, {0x60}},
    {0xFE, 1, {0x00}},
    {0x53, 1, {0x20}},
    {0xC4, 1, {0x40}},
    {0x3A, 1, {0x55}},
    {0x2A, 4, {0x00, 0x00, 0x00, 0xD3}},
    {0x2B, 4, {0x00, 0x00, 0x02, 0x07}},
    {0x31, 4, {0x00, 0x01, 0x02, 0x06}},
    {0x30, 4, {0x00, 0x01, 0x00, 0xD2}},
    {0x36, 1, {0x60}},
    {0x35, 1, {0x02}},
    {0x67, 1, {0x02}},
    {0x68, 1, {0x20}},
    {0x63, 1, {0xFF}},
    {0x51, 1, {0x00}},
    {0x12, 0, {0}},
    {0x11, 0, {0}},
    {0xFF, -1, {0}}, /* sentinel: delay 60ms here */
    {0x29, 0, {0}},
    {0x51, 1, {0xFF}},
};
static void rm690_panel_init(void)
{
    for (unsigned i = 0; i < sizeof(g_rm690_init) / sizeof(g_rm690_init[0]); i++)
    {
        const struct rm690_cmd *c = &g_rm690_init[i];
        if (c->reg == 0xFF && c->len == -1)
        {
            hal_sys_timer_delay(MS_TO_TICKS(60));
            continue;
        }
        lcdc_smpn_write(c->reg, c->d, c->len);
    }
}

static const char *lcd_variant(uint32_t id)
{
    switch (id)
    {
    case 0x260120:
        return "RM690C0 VXN";
    case 0x260220:
        return "RM690C0 TM";
    case 0x460120:
        return "RM690C0 O66 VXN";
    case 0x460220:
        return "RM690C0 O66 TM";
    case 0x460420:
        return "RM690C0 O66 XL";
    default:
        return "unknown (->default rm690b0)";
    }
}

/* Power up the DISPLAY (DIS) power island via the AON PSC. hal_psc_display_enable()
 * is only declared (not built) for best1503, so replicate the proven hal_psc_bt_enable
 * manual-drive sequence on the DIS control register (AON_PSC 0x40085000 + 0x028).
 * Bits per reg_psc_best1503.h "reg_28": CLK_STOP/ISO_EN/RESETN_ASSERT/PSW_EN (_REG=value,
 * _DR=drive-override), MEM_PSW_EN. Power-up = PSW on -> reset deassert -> un-isolate ->
 * clock on, each via clearing the matching _REG bit while holding all _DR overrides. */
#define PSC_DIS_REG (*(volatile uint32_t *)(0x40085000u + 0x028))
#define PSC_WE 0xCAFE0000u
#define DIS_CLK_STOP_REG (1u << 0)
#define DIS_ISO_REG (1u << 1)
#define DIS_RSTN_REG (1u << 2)
#define DIS_PSW_REG (1u << 3)
#define DIS_CLK_STOP_DR (1u << 4)
#define DIS_ISO_DR (1u << 5)
#define DIS_RSTN_DR (1u << 6)
#define DIS_PSW_DR (1u << 7)
#define DIS_MEM_PSW_REG (1u << 8)
#define DIS_MEM_PSW_DR (1u << 9)
#define DIS_DR_ALL (DIS_MEM_PSW_DR | DIS_PSW_DR | DIS_RSTN_DR | DIS_ISO_DR | DIS_CLK_STOP_DR)
static void psc_display_powerup(void)
{
    /* EXACT stock sub_2C0A0FA2(DIS) sequence (writes to 0x40085028). Crucially the
     * stock NEVER sets PSW_DR(bit7) or MEM_PSW_DR(bit9): the power switch stays in
     * hardware-AUTO mode and only CLK_STOP/ISO/RSTN are software-driven (DR bits 4/5/6).
     * 0x7F=assert all REG + clk/iso/rstn DR -> 0x77=clear PSW_REG -> 0x70=clear
     * CLK_STOP/ISO/RSTN REG (clock on, un-isolate, un-reset). */
    PSC_DIS_REG = 0xCAFE02FF;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    PSC_DIS_REG = 0xCAFE02F7;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    PSC_DIS_REG = 0xCAFE02F0;
    hal_sys_timer_delay(MS_TO_TICKS(1));
}

/* ===== software (bit-bang) I2C — open-drain emulation via GPIO dir =====
 * release(input, pulled high) / drive-low(output 0). Relies on the bus pull-ups.
 * Touch = Zinitix ztw623, 7-bit addr 0x20, reset = pin 28 (P3_4). The 7 SoC I2C
 * controller pad pairs (from the stock iomux tables) are swept to find the bus. */
static int swi2c_scl, swi2c_sda;
static void i2c_dly(void)
{
    for (volatile int i = 0; i < 80; i++)
        __NOP();
}
/* release SCL, then honor clock-stretching: wait (with timeout) until the slave
 * lets SCL actually rise before proceeding. */
static void scl_hi(void)
{
    gpio_dir_in(swi2c_scl);
    for (volatile int i = 0; i < 20000; i++)
    {
        if (gpio_rd(swi2c_scl))
            break;
    }
}
static void scl_lo(void)
{
    gpio_clr(swi2c_scl);
    gpio_dir_out(swi2c_scl);
}
static void sda_hi(void) { gpio_dir_in(swi2c_sda); }
static void sda_lo(void)
{
    gpio_clr(swi2c_sda);
    gpio_dir_out(swi2c_sda);
}
/* enable internal pull-up (best1503 iomux: PU pins0-31@0x4008602C, 32+@0x40086094;
 * PD @0x40086030 / 0x40086098) so a released open-drain line idles HIGH. */
static void iomux_pullup(int p)
{
    if (p < 32)
    {
        *(volatile uint32_t *)0x4008602Cu |= (1u << p);
        *(volatile uint32_t *)0x40086030u &= ~(1u << p);
    }
    else
    {
        *(volatile uint32_t *)0x40086094u |= (1u << (p - 32));
        *(volatile uint32_t *)0x40086098u &= ~(1u << (p - 32));
    }
}
static void i2c_pins_init(int scl, int sda)
{
    swi2c_scl = scl;
    swi2c_sda = sda;
    iomux_set(scl, 0);
    iomux_set(sda, 0); /* GPIO func */
    iomux_pullup(scl);
    iomux_pullup(sda); /* internal pull-ups */
    gpio_clr(scl);
    gpio_clr(sda); /* preset output-low value */
    sda_hi();
    scl_hi(); /* both released */
}
static void i2c_start(void)
{
    sda_hi();
    scl_hi();
    i2c_dly();
    sda_lo();
    i2c_dly();
    scl_lo();
    i2c_dly();
}
static void i2c_stop(void)
{
    sda_lo();
    i2c_dly();
    scl_hi();
    i2c_dly();
    sda_hi();
    i2c_dly();
}
static int i2c_wbyte(unsigned b)
{ /* returns 0 if ACKed */
    for (int i = 0; i < 8; i++)
    {
        if (b & 0x80)
            sda_hi();
        else
            sda_lo();
        b <<= 1;
        i2c_dly();
        scl_hi();
        i2c_dly();
        scl_lo();
        i2c_dly();
    }
    sda_hi();
    i2c_dly();
    scl_hi();
    i2c_dly();
    int ack = gpio_rd(swi2c_sda);
    scl_lo();
    i2c_dly();
    return ack;
}
static unsigned i2c_rbyte(int ack)
{
    unsigned v = 0;
    sda_hi();
    for (int i = 0; i < 8; i++)
    {
        i2c_dly();
        scl_hi();
        i2c_dly();
        v = (v << 1) | gpio_rd(swi2c_sda);
        scl_lo();
    }
    if (ack)
        sda_lo();
    else
        sda_hi();
    i2c_dly();
    scl_hi();
    i2c_dly();
    scl_lo();
    sda_hi();
    i2c_dly();
    return v;
}
static int i2c_probe(int addr7)
{ /* 0 = device ACKed */
    i2c_start();
    int ack = i2c_wbyte((addr7 << 1) | 0);
    i2c_stop();
    return ack;
}

/* register read/write on top of the bit-bang primitives (8-bit reg, 8-bit data) */
static int swi2c_rdreg(int addr7, uint8_t reg)   /* -1 on NAK */
{
    i2c_start();
    if (i2c_wbyte((addr7 << 1) | 0)) { i2c_stop(); return -1; }
    i2c_wbyte(reg);
    i2c_start();                                   /* repeated start */
    if (i2c_wbyte((addr7 << 1) | 1)) { i2c_stop(); return -1; }
    unsigned v = i2c_rbyte(0);                      /* master-NAK the last byte */
    i2c_stop();
    return (int)(v & 0xFF);
}
static void swi2c_wrreg(int addr7, uint8_t reg, uint8_t val) __attribute__((unused));
static void swi2c_wrreg(int addr7, uint8_t reg, uint8_t val)
{
    i2c_start();
    i2c_wbyte((addr7 << 1) | 0);
    i2c_wbyte(reg);
    i2c_wbyte(val);
    i2c_stop();
}

/* ===== EXTERNAL charger watchdog kill (the ~2.5 min global reset) ============
 * Reversed from stock vela_ap: the charger sits on I2C *master 0*, whose pads are
 * P0_0 (SCL) / P0_1 (SDA) = GPIO pins 0/1. Chip family hp4560x/cps6103/sc7061;
 * chip-id is reg 0x0A (hp4560x = 0xE0). The WDT lives in reg 5 bits 0x60 and stock
 * cps6103_set_wdt_en DISABLES it by clearing them (reg5 &= 0x9F).
 *
 * The SDK's hal_i2c can't reach it (NuttX skips pad-mux + master routing -> 15 s
 * hang -> crash), so we bit-bang it as GPIO: every loop here is bounded, so it can
 * never trip the 15 s hung-task watchdog. We SCAN the bus and read the chip-id so we
 * only ever write reg 5 of the *verified* charger, never a wrong device. */
static void charger_wdt_kill(void)
{
    ST("---- CHARGER WDT KILL (SW-I2C bit-bang P0_0=SCL / P0_1=SDA) ----");
    i2c_pins_init(0, 1);                 /* GPIO0=SCL, GPIO1=SDA, internal pull-ups */

    int chg = -1, id = -1;
    for (int a = 0x03; a <= 0x77; a++) {
        if (i2c_probe(a) != 0) continue;
        int v = swi2c_rdreg(a, 0x02);            /* hp4560x chip-id lives in reg 0x02 (=0xE0) */
        ST("  [i2c0] @0x%02X ACK  id(reg0x02)=0x%02X", a, v);
        if (v == 0xE0 && chg < 0) { chg = a; id = v; }
    }
    if (chg < 0) { ST("  hp4560x charger (id 0xE0) NOT found on i2c0 -- WDT untouched"); return; }

    /* WDT enable = reg 0x1B bit 0x80 (verified: reads 0xAB, bit set). Stock cps6103_set_wdt_en
     * clears it to disable; some variants instead use reg 5 bits 0x60. Clear both, preserving
     * every other bit, so the charger stops resetting the SoC (~2.5 min). */
    int r1b = swi2c_rdreg(chg, 0x1B);
    int r05 = swi2c_rdreg(chg, 0x05);
    if (r1b >= 0) swi2c_wrreg(chg, 0x1B, (uint8_t)(r1b & ~0x80));
    if (r05 >= 0) swi2c_wrreg(chg, 0x05, (uint8_t)(r05 & ~0x60));
    int n1b = swi2c_rdreg(chg, 0x1B), n05 = swi2c_rdreg(chg, 0x05);
    ST("  hp4560x @0x%02X id=0x%02X  WDT OFF: reg0x1B 0x%02X->0x%02X  reg0x05 0x%02X->0x%02X",
       chg, id, r1b, n1b, r05, n05);
}

static void gpio_out_level(int pin, int level)
{
    iomux_set(pin, 0);
    if (level) gpio_set(pin); else gpio_clr(pin);
    gpio_dir_out(pin);
}
/* Power the auxiliary sensor/actuator rails so their bit-bang I2C buses come alive (reversed from
 * stock persist_init_board):
 *  - IMU/PPG on bus5 (P5_6/P5_7): rail enable = P3_6 (pin 30) HIGH.
 *  - motor aw86225 on bus2 (P0_4/P0_5): enable = P1_2 (pin 10) HIGH, reset = P1_3 (pin 11) HIGH->LOW->HIGH. */
static void sensor_power_on(void)
{
    gpio_out_level(30, 1);                                       /* IMU/PPG rail (P3_6) */
    gpio_out_level(10, 1);                                       /* motor enable (P1_2) */
    gpio_out_level(11, 1); hal_sys_timer_delay(MS_TO_TICKS(2));
    gpio_clr(11);          hal_sys_timer_delay(MS_TO_TICKS(5));  /* motor reset assert  */
    gpio_set(11);          hal_sys_timer_delay(MS_TO_TICKS(25)); /* release + settle    */
    ST("  sensor rails ON: IMU P3_6 high, motor P1_2 high + P1_3 reset");
}

#define DISPLAY_WIDTH 212
#define DISPLAY_HEIGHT 520
/* === SINGLE shared full-screen framebuffer (stock-faithful) ==================
 * Stock FW: `frameBuffer` @ 0x20000000, size 0x6BA80 (per IDA). In THIS firmware
 * 0x20000000 holds the live vector table + .data + .bss (up to ~0x2003a1c4), so the
 * FB cannot sit there without overwriting running data. We place it just above the
 * SDK-mapped 512KB SRAM window (0x20000000..0x20080000), in the extra on-chip SRAM.
 * It is DMA-reachable (same on-chip SRAM controller / AXI) and feeds BOTH the
 * software bit-bang QUAD and the hardware LCDC QUAD path -- ONE buffer, one truth.
 * NOTE: if a DMA-reach test shows only the low bank works, move code/data up and set
 * FB_FULL_ADDR back to 0x20000000 -- single knob below. */
#define FB_FULL_ADDR 0x20080000u
#define FB_FULL_SIZE 0x6BA80u /* reserved region size; an 8bpp frame uses W*H = 0x1AE60 bytes */
#define FB_ADDR FB_FULL_ADDR
#define FB_PTR ((uint8_t *)FB_ADDR) /* 8bpp RGB332: ONE byte per pixel (the panel reads 8bpp) */

/* RGB565 (our colour literals) -> RGB332 (RRRGGGBB), the format the panel actually reads */
static inline uint8_t rgb565_to_rgb332(uint16_t c)
{
    uint8_t r = (uint8_t)((c >> 13) & 0x7); /* top 3 of R5 */
    uint8_t g = (uint8_t)((c >> 8) & 0x7);  /* top 3 of G6 */
    uint8_t b = (uint8_t)((c >> 3) & 0x3);  /* top 2 of B5 */
    return (uint8_t)((r << 5) | (g << 2) | b);
}
static void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t col_data[] = {(uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF), (uint8_t)(x2 >> 8), (uint8_t)(x2 & 0xFF)};
    uint8_t row_data[] = {(uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF), (uint8_t)(y2 >> 8), (uint8_t)(y2 & 0xFF)};

    lcdc_smpn_write(0x2A, col_data, 4); // Column Address Set
    lcdc_smpn_write(0x2B, row_data, 4); // Row Address Set
}

static const unsigned char font57[] = {

    0x00, 0x00, 0x00, 0x00, 0x00, // 0x0
    0x3E, 0x5B, 0x4F, 0x5B, 0x3E, // 0x1
    0x3E, 0x6B, 0x4F, 0x6B, 0x3E, // 0x2
    0x1C, 0x3E, 0x7C, 0x3E, 0x1C, // 0x3
    0x18, 0x3C, 0x7E, 0x3C, 0x18, // 0x4
    0x1C, 0x57, 0x7D, 0x57, 0x1C, // 0x5
    0x1C, 0x5E, 0x7F, 0x5E, 0x1C, // 0x6
    0x00, 0x18, 0x3C, 0x18, 0x00, // 0x7
    0xFF, 0xE7, 0xC3, 0xE7, 0xFF, // 0x8
    0x00, 0x18, 0x24, 0x18, 0x00, // 0x9
    0xFF, 0xE7, 0xDB, 0xE7, 0xFF, // 0xA
    0x30, 0x48, 0x3A, 0x06, 0x0E, // 0xB
    0x26, 0x29, 0x79, 0x29, 0x26, // 0xC
    0x40, 0x7F, 0x05, 0x05, 0x07, // 0xD
    0x40, 0x7F, 0x05, 0x25, 0x3F, // 0xE
    0x5A, 0x3C, 0xE7, 0x3C, 0x5A, // 0xF
    0x7F, 0x3E, 0x1C, 0x1C, 0x08, // 0x10
    0x08, 0x1C, 0x1C, 0x3E, 0x7F, // 0x11
    0x14, 0x22, 0x7F, 0x22, 0x14, // 0x12
    0x5F, 0x5F, 0x00, 0x5F, 0x5F, // 0x13
    0x06, 0x09, 0x7F, 0x01, 0x7F, // 0x14
    0x00, 0x66, 0x89, 0x95, 0x6A, // 0x15
    0x60, 0x60, 0x60, 0x60, 0x60, // 0x16
    0x94, 0xA2, 0xFF, 0xA2, 0x94, // 0x17
    0x08, 0x04, 0x7E, 0x04, 0x08, // 0x18
    0x10, 0x20, 0x7E, 0x20, 0x10, // 0x19
    0x08, 0x08, 0x2A, 0x1C, 0x08, // 0x1A
    0x08, 0x1C, 0x2A, 0x08, 0x08, // 0x1B
    0x1E, 0x10, 0x10, 0x10, 0x10, // 0x1C
    0x0C, 0x1E, 0x0C, 0x1E, 0x0C, // 0x1D
    0x30, 0x38, 0x3E, 0x38, 0x30, // 0x1E
    0x06, 0x0E, 0x3E, 0x0E, 0x06, // 0x1F
    0x00, 0x00, 0x00, 0x00, 0x00, // 0x20
    0x00, 0x00, 0x5F, 0x00, 0x00, // 0x21
    0x00, 0x07, 0x00, 0x07, 0x00, // 0x22
    0x14, 0x7F, 0x14, 0x7F, 0x14, // 0x23
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // 0x24
    0x23, 0x13, 0x08, 0x64, 0x62, // 0x25
    0x36, 0x49, 0x56, 0x20, 0x50, // 0x26
    0x00, 0x08, 0x07, 0x03, 0x00, // 0x27
    0x00, 0x1C, 0x22, 0x41, 0x00, // 0x28
    0x00, 0x41, 0x22, 0x1C, 0x00, // 0x29
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A, // 0x2A
    0x08, 0x08, 0x3E, 0x08, 0x08, // 0x2B
    0x00, 0x80, 0x70, 0x30, 0x00, // 0x2C
    0x08, 0x08, 0x08, 0x08, 0x08, // 0x2D
    0x00, 0x00, 0x60, 0x60, 0x00, // 0x2E
    0x20, 0x10, 0x08, 0x04, 0x02, // 0x2F
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0x30
    0x00, 0x42, 0x7F, 0x40, 0x00, // 0x31
    0x72, 0x49, 0x49, 0x49, 0x46, // 0x32
    0x21, 0x41, 0x49, 0x4D, 0x33, // 0x33
    0x18, 0x14, 0x12, 0x7F, 0x10, // 0x34
    0x27, 0x45, 0x45, 0x45, 0x39, // 0x35
    0x3C, 0x4A, 0x49, 0x49, 0x31, // 0x36
    0x41, 0x21, 0x11, 0x09, 0x07, // 0x37
    0x36, 0x49, 0x49, 0x49, 0x36, // 0x38
    0x46, 0x49, 0x49, 0x29, 0x1E, // 0x39
    0x00, 0x00, 0x14, 0x00, 0x00, // 0x3A
    0x00, 0x40, 0x34, 0x00, 0x00, // 0x3B
    0x00, 0x08, 0x14, 0x22, 0x41, // 0x3C
    0x14, 0x14, 0x14, 0x14, 0x14, // 0x3D
    0x00, 0x41, 0x22, 0x14, 0x08, // 0x3E
    0x02, 0x01, 0x59, 0x09, 0x06, // 0x3F
    0x3E, 0x41, 0x5D, 0x59, 0x4E, // 0x40
    0x7C, 0x12, 0x11, 0x12, 0x7C, // 0x41
    0x7F, 0x49, 0x49, 0x49, 0x36, // 0x42
    0x3E, 0x41, 0x41, 0x41, 0x22, // 0x43
    0x7F, 0x41, 0x41, 0x41, 0x3E, // 0x44
    0x7F, 0x49, 0x49, 0x49, 0x41, // 0x45
    0x7F, 0x09, 0x09, 0x09, 0x01, // 0x46
    0x3E, 0x41, 0x41, 0x51, 0x73, // 0x47
    0x7F, 0x08, 0x08, 0x08, 0x7F, // 0x48
    0x00, 0x41, 0x7F, 0x41, 0x00, // 0x49
    0x20, 0x40, 0x41, 0x3F, 0x01, // 0x4A
    0x7F, 0x08, 0x14, 0x22, 0x41, // 0x4B
    0x7F, 0x40, 0x40, 0x40, 0x40, // 0x4C
    0x7F, 0x02, 0x1C, 0x02, 0x7F, // 0x4D
    0x7F, 0x04, 0x08, 0x10, 0x7F, // 0x4E
    0x3E, 0x41, 0x41, 0x41, 0x3E, // 0x4F
    0x7F, 0x09, 0x09, 0x09, 0x06, // 0x50
    0x3E, 0x41, 0x51, 0x21, 0x5E, // 0x51
    0x7F, 0x09, 0x19, 0x29, 0x46, // 0x52
    0x26, 0x49, 0x49, 0x49, 0x32, // 0x53
    0x03, 0x01, 0x7F, 0x01, 0x03, // 0x54
    0x3F, 0x40, 0x40, 0x40, 0x3F, // 0x55
    0x1F, 0x20, 0x40, 0x20, 0x1F, // 0x56
    0x3F, 0x40, 0x38, 0x40, 0x3F, // 0x57
    0x63, 0x14, 0x08, 0x14, 0x63, // 0x58
    0x03, 0x04, 0x78, 0x04, 0x03, // 0x59
    0x61, 0x59, 0x49, 0x4D, 0x43, // 0x5A
    0x00, 0x7F, 0x41, 0x41, 0x41, // 0x5B
    0x02, 0x04, 0x08, 0x10, 0x20, // 0x5C
    0x00, 0x41, 0x41, 0x41, 0x7F, // 0x5D
    0x04, 0x02, 0x01, 0x02, 0x04, // 0x5E
    0x40, 0x40, 0x40, 0x40, 0x40, // 0x5F
    0x00, 0x03, 0x07, 0x08, 0x00, // 0x60
    0x20, 0x54, 0x54, 0x78, 0x40, // 0x61
    0x7F, 0x28, 0x44, 0x44, 0x38, // 0x62
    0x38, 0x44, 0x44, 0x44, 0x28, // 0x63
    0x38, 0x44, 0x44, 0x28, 0x7F, // 0x64
    0x38, 0x54, 0x54, 0x54, 0x18, // 0x65
    0x00, 0x08, 0x7E, 0x09, 0x02, // 0x66
    0x18, 0xA4, 0xA4, 0x9C, 0x78, // 0x67
    0x7F, 0x08, 0x04, 0x04, 0x78, // 0x68
    0x00, 0x44, 0x7D, 0x40, 0x00, // 0x69
    0x20, 0x40, 0x40, 0x3D, 0x00, // 0x6A
    0x7F, 0x10, 0x28, 0x44, 0x00, // 0x6B
    0x00, 0x41, 0x7F, 0x40, 0x00, // 0x6C
    0x7C, 0x04, 0x78, 0x04, 0x78, // 0x6D
    0x7C, 0x08, 0x04, 0x04, 0x78, // 0x6E
    0x38, 0x44, 0x44, 0x44, 0x38, // 0x6F
    0xFC, 0x18, 0x24, 0x24, 0x18, // 0x70
    0x18, 0x24, 0x24, 0x18, 0xFC, // 0x71
    0x7C, 0x08, 0x04, 0x04, 0x08, // 0x72
    0x48, 0x54, 0x54, 0x54, 0x24, // 0x73
    0x04, 0x04, 0x3F, 0x44, 0x24, // 0x74
    0x3C, 0x40, 0x40, 0x20, 0x7C, // 0x75
    0x1C, 0x20, 0x40, 0x20, 0x1C, // 0x76
    0x3C, 0x40, 0x30, 0x40, 0x3C, // 0x77
    0x44, 0x28, 0x10, 0x28, 0x44, // 0x78
    0x4C, 0x90, 0x90, 0x90, 0x7C, // 0x79
    0x44, 0x64, 0x54, 0x4C, 0x44, // 0x7A
    0x00, 0x08, 0x36, 0x41, 0x00, // 0x7B
    0x00, 0x00, 0x77, 0x00, 0x00, // 0x7C
    0x00, 0x41, 0x36, 0x08, 0x00, // 0x7D
    0x02, 0x01, 0x02, 0x04, 0x02, // 0x7E
    0x3C, 0x26, 0x23, 0x26, 0x3C, // 0x7F
    0x1E, 0xA1, 0xA1, 0x61, 0x12, // 0x80
    0x3A, 0x40, 0x40, 0x20, 0x7A, // 0x81
    0x38, 0x54, 0x54, 0x55, 0x59, // 0x82
    0x21, 0x55, 0x55, 0x79, 0x41, // 0x83
    0x21, 0x54, 0x54, 0x78, 0x41, // 0x84
    0x21, 0x55, 0x54, 0x78, 0x40, // 0x85
    0x20, 0x54, 0x55, 0x79, 0x40, // 0x86
    0x0C, 0x1E, 0x52, 0x72, 0x12, // 0x87
    0x39, 0x55, 0x55, 0x55, 0x59, // 0x88
    0x39, 0x54, 0x54, 0x54, 0x59, // 0x89
    0x39, 0x55, 0x54, 0x54, 0x58, // 0x8A
    0x00, 0x00, 0x45, 0x7C, 0x41, // 0x8B
    0x00, 0x02, 0x45, 0x7D, 0x42, // 0x8C
    0x00, 0x01, 0x45, 0x7C, 0x40, // 0x8D
    0xF0, 0x29, 0x24, 0x29, 0xF0, // 0x8E  Ä
    0xF0, 0x28, 0x25, 0x28, 0xF0, // 0x8F
    0x7C, 0x54, 0x55, 0x45, 0x00, // 0x90
    0x20, 0x54, 0x54, 0x7C, 0x54, // 0x91
    0x7C, 0x0A, 0x09, 0x7F, 0x49, // 0x92
    0x32, 0x49, 0x49, 0x49, 0x32, // 0x93
    0x32, 0x48, 0x48, 0x48, 0x32, // 0x94
    0x32, 0x4A, 0x48, 0x48, 0x30, // 0x95
    0x3A, 0x41, 0x41, 0x21, 0x7A, // 0x96
    0x3A, 0x42, 0x40, 0x20, 0x78, // 0x97
    0x7f, 0x25, 0x25, 0x25, 0x1a, // 0x00, 0x9D, 0xA0, 0xA0, 0x7D,//0x98++++++++++++++++++++++++ß
    0x39, 0x44, 0x44, 0x44, 0x39, // 0x99
    0x3D, 0x40, 0x40, 0x40, 0x3D, // 0x9A
    0x3C, 0x24, 0xFF, 0x24, 0x24, // 0x9B
    0x48, 0x7E, 0x49, 0x43, 0x66, // 0x9C
    0x2B, 0x2F, 0xFC, 0x2F, 0x2B, // 0x9D
    0xFF, 0x09, 0x29, 0xF6, 0x20, // 0x9E
    0xC0, 0x88, 0x7E, 0x09, 0x03, // 0x9F
    0x20, 0x54, 0x54, 0x79, 0x41, // 0xA0
    0x00, 0x00, 0x44, 0x7D, 0x41, // 0xA1
    0x30, 0x48, 0x48, 0x4A, 0x32, // 0xA2
    0x38, 0x40, 0x40, 0x22, 0x7A, // 0xA3
    0x00, 0x7A, 0x0A, 0x0A, 0x72, // 0xA4
    0x7D, 0x0D, 0x19, 0x31, 0x7D, // 0xA5
    0x26, 0x29, 0x29, 0x2F, 0x28, // 0xA6
    0x26, 0x29, 0x29, 0x29, 0x26, // 0xA7
    0x30, 0x48, 0x4D, 0x40, 0x20, // 0xA8
    0x38, 0x08, 0x08, 0x08, 0x08, // 0xA9
    0x08, 0x08, 0x08, 0x08, 0x38, // 0xAA
    0x2F, 0x10, 0xC8, 0xAC, 0xBA, // 0xAB
    0x2F, 0x10, 0x28, 0x34, 0xFA, // 0xAC
    0x00, 0x00, 0x7B, 0x00, 0x00, // 0xAD
    0x08, 0x14, 0x2A, 0x14, 0x22, // 0xAE
    0x22, 0x14, 0x2A, 0x14, 0x08, // 0xAF
    0xAA, 0x00, 0x55, 0x00, 0xAA, // 0xB0
    0xAA, 0x55, 0xAA, 0x55, 0xAA, // 0xB1
    0x00, 0x00, 0x00, 0xFF, 0x00, // 0xB2
    0x10, 0x10, 0x10, 0xFF, 0x00, // 0xB3
    0x14, 0x14, 0x14, 0xFF, 0x00, // 0xB4
    0x10, 0x10, 0xFF, 0x00, 0xFF, // 0xB5
    0x10, 0x10, 0xF0, 0x10, 0xF0, // 0xB6
    0x14, 0x14, 0x14, 0xFC, 0x00, // 0xB7
    0x14, 0x14, 0xF7, 0x00, 0xFF, // 0xB8
    0x00, 0x00, 0xFF, 0x00, 0xFF, // 0xB9
    0x14, 0x14, 0xF4, 0x04, 0xFC, // 0xBA
    0x14, 0x14, 0x17, 0x10, 0x1F, // 0xBB
    0x10, 0x10, 0x1F, 0x10, 0x1F, // 0xBC
    0x14, 0x14, 0x14, 0x1F, 0x00, // 0xBD
    0x10, 0x10, 0x10, 0xF0, 0x00, // 0xBE
    0x00, 0x00, 0x00, 0x1F, 0x10, // 0xBF
    0x10, 0x10, 0x10, 0x1F, 0x10, // 0xC0
    0x10, 0x10, 0x10, 0xF0, 0x10, // 0xC1
    0x00, 0x00, 0x00, 0xFF, 0x10, // 0xC2
    0x10, 0x10, 0x10, 0x10, 0x10, // 0xC3
    0x10, 0x10, 0x10, 0xFF, 0x10, // 0xC4
    0x00, 0x00, 0x00, 0xFF, 0x14, // 0xC5
    0x00, 0x00, 0xFF, 0x00, 0xFF, // 0xC6
    0x00, 0x00, 0x1F, 0x10, 0x17, // 0xC7
    0x00, 0x00, 0xFC, 0x04, 0xF4, // 0xC8
    0x14, 0x14, 0x17, 0x10, 0x17, // 0xC9
    0x14, 0x14, 0xF4, 0x04, 0xF4, // 0xCA
    0x00, 0x00, 0xFF, 0x00, 0xF7, // 0xCB
    0x14, 0x14, 0x14, 0x14, 0x14, // 0xCC
    0x14, 0x14, 0xF7, 0x00, 0xF7, // 0xCD
    0x14, 0x14, 0x14, 0x17, 0x14, // 0xCE
    0x10, 0x10, 0x1F, 0x10, 0x1F, // 0xCF
    0x14, 0x14, 0x14, 0xF4, 0x14, // 0xD0
    0x10, 0x10, 0xF0, 0x10, 0xF0, // 0xD1
    0x00, 0x00, 0x1F, 0x10, 0x1F, // 0xD2
    0x00, 0x00, 0x00, 0x1F, 0x14, // 0xD3
    0x00, 0x00, 0x00, 0xFC, 0x14, // 0xD4
    0x00, 0x00, 0xF0, 0x10, 0xF0, // 0xD5
    0x10, 0x10, 0xFF, 0x10, 0xFF, // 0xD6
    0x14, 0x14, 0x14, 0xFF, 0x14, // 0xD7
    0x10, 0x10, 0x10, 0x1F, 0x00, // 0xD8
    0x00, 0x00, 0x00, 0xF0, 0x10, // 0xD9
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xDA
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, // 0xDB
    0xFF, 0xFF, 0xFF, 0x00, 0x00, // 0xDC
    0x00, 0x00, 0x00, 0xFF, 0xFF, // 0xDD
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, // 0xDE
    0x38, 0x44, 0x44, 0x38, 0x44, // 0xDF
    0x7C, 0x2A, 0x2A, 0x3E, 0x14, // 0xE0
    0x7E, 0x02, 0x02, 0x06, 0x06, // 0xE1
    0x02, 0x7E, 0x02, 0x7E, 0x02, // 0xE2
    0x63, 0x55, 0x49, 0x41, 0x63, // 0xE3
    0x38, 0x44, 0x44, 0x3C, 0x04, // 0xE4
    0x40, 0x7E, 0x20, 0x1E, 0x20, // 0xE5
    0x06, 0x02, 0x7E, 0x02, 0x02, // 0xE6
    0x99, 0xA5, 0xE7, 0xA5, 0x99, // 0xE7
    0x1C, 0x2A, 0x49, 0x2A, 0x1C, // 0xE8
    0x4C, 0x72, 0x01, 0x72, 0x4C, // 0xE9
    0x30, 0x4A, 0x4D, 0x4D, 0x30, // 0xEA
    0x30, 0x48, 0x78, 0x48, 0x30, // 0xEB
    0xBC, 0x62, 0x5A, 0x46, 0x3D, // 0xEC
    0x3E, 0x49, 0x49, 0x49, 0x00, // 0xED
    0x7E, 0x01, 0x01, 0x01, 0x7E, // 0xEE
    0x2A, 0x2A, 0x2A, 0x2A, 0x2A, // 0xEF
    0x44, 0x44, 0x5F, 0x44, 0x44, // 0xF0
    0x40, 0x51, 0x4A, 0x44, 0x40, // 0xF1
    0x40, 0x44, 0x4A, 0x51, 0x40, // 0xF2
    0x00, 0x00, 0xFF, 0x01, 0x03, // 0xF3
    0xE0, 0x80, 0xFF, 0x00, 0x00, // 0xF4
    0x08, 0x08, 0x6B, 0x6B, 0x08, // 0xF5
    0x36, 0x12, 0x36, 0x24, 0x36, // 0xF6
    0x06, 0x0F, 0x09, 0x0F, 0x06, // 0xF7
    0x00, 0x00, 0x18, 0x18, 0x00, // 0xF8
    0x00, 0x00, 0x10, 0x10, 0x00, // 0xF9
    0x30, 0x40, 0xFF, 0x01, 0x01, // 0xFA
    0x00, 0x1F, 0x01, 0x01, 0x1E, // 0xFB
    0x00, 0x19, 0x1D, 0x17, 0x12, // 0xFC
    0x00, 0x3C, 0x3C, 0x3C, 0x3C, // 0xFD
    0x00, 0x00, 0x00, 0x00, 0x00  // 0xFE
};

void fb_put_pixel(int x, int y, uint16_t color)
{
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT)
    {
        // MiBand 9 Layout ist oft Hochkant, Index-Berechnung:
        FB_PTR[y * DISPLAY_WIDTH + x] = rgb565_to_rgb332(color); /* 8bpp */
    }
}

// Den gesamten PSRAM Framebuffer löschen
void fb_clear(uint16_t color)
{
    uint8_t c8 = rgb565_to_rgb332(color); /* 8bpp */
    for (uint32_t i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); i++)
    {
        FB_PTR[i] = c8;
    }
}

/* 0 = real framebuffer (FB_PTR), 1 = solid red, 2 = 8-band colour-map test.
 * Tests 1/2 generate the colour inline (no framebuffer read) to validate the panel
 * format independently of the (dead) PSRAM framebuffer. */
#define FB_SOLID_TEST 2

/* colour for the inline test at a given pixel index (4 horizontal bands) */
static inline uint16_t fb_test_color(uint32_t idx, uint32_t total)
{
    /* 8 horizontal bands, 64 rows each, each exercising different RGB565 bits so we
     * can see exactly which channels survive / collapse:
     *  0 red(R) 1 green(G) 2 blue(B) 3 yellow(R+G) 4 cyan(G+B) 5 magenta(R+B)
     *  6 white(all) 7 black(none) */
    static const uint16_t pal[8] = {
        0xF800, 0x07E0, 0x001F, 0xFFE0, 0x07FF, 0xF81F, 0xFFFF, 0x0000};
    (void)total;
    uint32_t row = idx / (uint32_t)DISPLAY_WIDTH;
    return pal[(row / 64u) % 8u];
}

static inline uint16_t fb_get_px(uint32_t idx, uint32_t total)
{
#if FB_SOLID_TEST == 1
    (void)idx;
    (void)total;
    return 0xF800;
#elif FB_SOLID_TEST == 2
    return fb_test_color(idx, total);
#else
    return (idx < total) ? FB_PTR[idx] : 0;
#endif
}

/* (Former g_lcd_strip removed: the single shared full-screen framebuffer FB_PTR
 * (FB_FULL_ADDR) is now the one buffer used by BOTH the SW and HW quad paths.) */

/* QUAD frame via the LCDC compositor (0x32 auto-head, 4 data lanes — what the TM
 * panel actually needs; the manual SMPN path is single-line only).  Rendered as
 * horizontal strips from working SRAM (PSRAM 0x34000000 is dead).  Per strip: fill
 * SRAM, set the panel window, then compositor quad-DMA (REG_0F4 = strip, STARTCR). */
/* Inline UI test renderer: 8px red border + scaled 5x7 text, computed per-pixel.
 * No framebuffer is used (FB_PTR/PSRAM is not brought up yet) -- this both shows
 * real text and, via the border, makes any frame offset against the panel edges
 * immediately visible. */
#define UI_SCALE 2 /* 5x7 font *2 = 12px/char, fits 212w */
/* live touch state: updated by st_touch_display_loop(), drawn by ui_test_px(). */
static volatile int g_touch_x = 106, g_touch_y = 260, g_touch_down = 0;
static char g_touch_str[28] = "TOUCH THE SCREEN";
/* returns 1 if pixel (x,y) lands on a glyph of string s drawn at (sx,sy), scale*5x7 */
static int text_hit(int x, int y, int sx, int sy, int scale, const char *s)
{
    if (y < sy || y >= sy + 7 * scale)
        return 0;
    int row = (y - sy) / scale;
    for (int ci = 0; s[ci]; ci++)
    {
        int cx = sx + ci * (6 * scale);
        if (x < cx || x >= cx + 5 * scale)
            continue;
        int col = (x - cx) / scale;
        unsigned idx = (unsigned char)s[ci];
        if (idx > 0xFE)
            idx = 0x20;
        if (font57[idx * 5 + col] & (1 << row))
            return 1;
    }
    return 0;
}
static uint16_t ui_test_px(int x, int y)
{
    if (x < 8 || x >= DISPLAY_WIDTH - 8 || y < 8 || y >= DISPLAY_HEIGHT - 8)
        return 0xF800; /* static red 8px border */
    /* crosshair + dot at the live touch point */
    if (g_touch_down)
    {
        int dx = x - g_touch_x, dy = y - g_touch_y;
        if ((dx > -16 && dx < 16 && dy >= -1 && dy <= 1) ||
            (dy > -16 && dy < 16 && dx >= -1 && dx <= 1))
            return 0x07FF; /* cyan cross */
        if (dx * dx + dy * dy <= 16)
            return 0xFFE0; /* yellow dot  */
    }
    /* title + live coordinate readout (scale 2) */
    if (text_hit(x, y, 16, 18, UI_SCALE, "MIBAND"))
        return 0xFFFF;
    if (text_hit(x, y, 16, 40, UI_SCALE, g_touch_str))
        return 0xFFE0;
    return 0x0000; /* black background */
}
/* Render the procedural touch UI into the shared framebuffer FB_PTR. */
static void fb_render_ui(void)
{
    for (int y = 0; y < DISPLAY_HEIGHT; y++)
        for (int x = 0; x < DISPLAY_WIDTH; x++)
            FB_PTR[y * DISPLAY_WIDTH + x] = rgb565_to_rgb332(ui_test_px(x, y)); /* 8bpp */
}

/* set the panel write window (CASET/RASET) via the single-line FIFO command path */
static void disp_set_window(int x0, int y0, int x1, int y1)
{
    uint8_t ca[4] = {(uint8_t)(x0 >> 8), (uint8_t)x0, (uint8_t)(x1 >> 8), (uint8_t)x1};
    uint8_t ra[4] = {(uint8_t)(y0 >> 8), (uint8_t)y0, (uint8_t)(y1 >> 8), (uint8_t)y1};
    lcdc_smpn_write(0x2A, ca, 4);   /* CASET */
    lcdc_smpn_write(0x2B, ra, 4);   /* RASET */
}

/* THE display send: full-screen window + 1-lane HW QSPI frame from FB_PTR. */
static void disp_send_frame(const uint8_t *fb)
{
    disp_set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    smpn_quad_frame(fb, DISPLAY_WIDTH * DISPLAY_HEIGHT);
}
void fb_refresh(void)
{ /* full-frame repaint of the shared FB via HW QSPI (1-lane single-write). NO software bit-bang.
   * Call ONCE after a frame is fully rendered -- NOT from lcd_printf per character/line. */
    (void)fb_get_px;
    disp_send_frame(FB_PTR);
}

#ifdef DOOM
/* DOOM partial refresh: push only the given panel window. buf is the contiguous
 * RGB332 region (window-row-major, (x1-x0+1)*(y1-y0+1) bytes). Updating just the
 * game band instead of the whole 212x520 panel refreshes much faster. */
void disp_send_window(const uint8_t *buf, int x0, int y0, int x1, int y1)
{
    disp_set_window(x0, y0, x1, y1);
    smpn_quad_frame(buf, (x1 - x0 + 1) * (y1 - y0 + 1));
}
#endif

void fb_draw_char_57(int x, int y, char c, uint16_t color, uint16_t bg)
{
    // Da das Array bis 0xFE geht, stellen wir sicher, dass wir im Bereich bleiben
    unsigned int index = (unsigned char)c;
    if (index > 0xFE)
        index = 0x20; // Fallback auf Space

    const unsigned char *bitmap = &font57[index * 5];

    for (int col = 0; col < 5; col++)
    { // 5 Spalten
        unsigned char data = bitmap[col];
        for (int row = 0; row < 8; row++)
        { // 8 Bits pro Spalte
            // Prüfen, ob das Bit an dieser Stelle gesetzt ist
            uint16_t pixel_color = (data & (1 << row)) ? color : bg;
            fb_put_pixel(x + col, y + row, pixel_color);
        }
    }

    // Eine leere Spalte nach dem Buchstaben für den Abstand (Kerning)
    for (int row = 0; row < 8; row++)
    {
        fb_put_pixel(x + 5, y + row, bg);
    }
}

#include <stdarg.h>
#include <stdio.h>

static int g_cursor_x = 0;
static int g_cursor_y = 0;

void lcd_printf(uint16_t color, uint16_t bg, const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (char *p = buf; *p; p++)
    {
        if (*p == '\n')
        {
            g_cursor_x = 0;
            g_cursor_y += 10; // Zeilenhöhe
        }
        else if (*p == '\r')
        {
            g_cursor_x = 0;
        }
        else
        {
            fb_draw_char_57(g_cursor_x, g_cursor_y, *p, color, bg);
            g_cursor_x += 6; // 5 Pixel Font + 1 Pixel Abstand

            // Automatischer Zeilenumbruch
            if (g_cursor_x > (DISPLAY_WIDTH - 6))
            {
                g_cursor_x = 0;
                g_cursor_y += 10;
            }
        }

        // Am Ende des Bildschirms oben wieder anfangen (Scroll-Ersatz)
        if (g_cursor_y > (DISPLAY_HEIGHT - 10))
        {
            g_cursor_y = 0;
            // Optional: Bildschirm löschen beim Wrap-around
            // fb_clear(bg);
        }
    }
    /* NOTE: no fb_refresh() here -- lcd_printf only RENDERS into the framebuffer.
     * The caller pushes the finished image to the panel once via fb_refresh(). */
}

#ifdef DOOM
/* DOOM control overlay: title + on-screen legend for the touch zones, drawn ONCE
 * into the panel border (the doom frame only repaints the centred 120x160 image
 * at x46-165 y180-339, so these border labels persist).
 * Tap zones: very top = START/menu; above/below image = forward/back;
 * left/right of image = turn left/right; image centre = FIRE.
 * Text is kept away from the top edge (the panel has rounded corners). */
/* draw a string with the 5x7 font scaled by 'sc' (foreground only; panel is black). */
static void doom_text_sc(int x, int y, const char *s, uint16_t col, int sc)
{
    while (*s)
    {
        unsigned int idx = (unsigned char)*s++;
        if (idx > 0xFE) idx = 0x20;
        const unsigned char *bm = &font57[idx * 5];
        for (int c = 0; c < 5; c++)
        {
            unsigned char d = bm[c];
            for (int r = 0; r < 8; r++)
                if (d & (1 << r))
                    for (int yy = 0; yy < sc; yy++)
                        for (int xx = 0; xx < sc; xx++)
                            fb_put_pixel(x + c * sc + xx, y + r * sc + yy, col);
        }
        x += 6 * sc;
    }
}
/* outline rectangle for the on-screen control buttons */
static void doom_hline(int x0, int x1, int y, uint16_t c) { for (int x = x0; x <= x1; x++) fb_put_pixel(x, y, c); }
static void doom_vline(int x, int y0, int y1, uint16_t c) { for (int y = y0; y <= y1; y++) fb_put_pixel(x, y, c); }
static void doom_rect(int x0, int y0, int x1, int y1, uint16_t c)
{
    doom_hline(x0, x1, y0, c); doom_hline(x0, x1, y1, c);
    doom_vline(x0, y0, y1, c); doom_vline(x1, y0, y1, c);
}
void doom_draw_ui(void)
{
    fb_clear(0x0000);                                            /* black panel */

    /* top row: two buttons side by side (match zones y<60: left=START, right=USE) */
    doom_rect(6,  22, 102, 58, 0xFFE0); doom_text_sc(22,  32, "START", 0xFFE0, 2); /* left  */
    doom_rect(110,22, 206, 58, 0x07E0); doom_text_sc(141, 32, "USE",   0x07E0, 2); /* right (start game) */

    /* title */
    doom_text_sc(55,  68, "DOOM on MiBand 10", 0xFFFF, 1);
    doom_text_sc(46,  82, "by ATC1441",        0xFFFF, 2);      /* bigger, new line */

    /* direction keys all around the centred game image */
    doom_rect(40, 104, 171, 176, 0x07FF); doom_text_sc(79, 134, "FORWARD ^", 0x07FF, 1); /* above */
    doom_rect(2,  182,  44, 337, 0x07FF); doom_text_sc(18, 252, "<", 0x07FF, 1);          /* left  */
    doom_rect(167,182, 209, 337, 0x07FF); doom_text_sc(185,252, ">", 0x07FF, 1);          /* right */
    doom_rect(40, 346, 171, 430, 0x07FF); doom_text_sc(82, 382, "v BACK", 0x07FF, 1);     /* below */

    doom_text_sc(67, 450, "CENTRE = FIRE", 0xF800, 1);          /* image centre = fire */
    fb_refresh();                                               /* push the static screen ONCE */
}
#endif

extern volatile int g_probe_active, g_probe_faulted;
static int st_display_id(void)
{
    ST("---- DISPLAY: LCDC smart-panel RM690C0/B0 ID read ----");
    hal_trace_flush_buffer();

    /* OPTION-1 rail test: bring up every candidate PMU rail before the panel.  VSENSOR
     * + raise VIO (the only public LDO knobs) in case the display VCI/IOVCC is gated. */
    {
        extern void pmu_ldo_vsensor_enable(int user);
        extern void pmu_viorise_req(int user, int rise);
        pmu_ldo_vsensor_enable(1); /* PMU_VSENSOR_USER_SINGLE_WIRE_UART */
        pmu_viorise_req(0, 1);     /* VIORISE PWL0 -> raise VIO */
        pmu_viorise_req(2, 1);     /* VIORISE FLASH */
        ST("  PMU: VSENSOR enabled + VIO raised");
        hal_trace_flush_buffer();
    }

    /* 0a) DISPLAY power island: the LCDC/GPU/DSI sit in a PSC-gated power domain
     * that is OFF at boot -> register block reads 0 ("DARK"). Power it up first. */
    ST("  powering DISPLAY island (PSC DIS @0x40085028)...");
    hal_trace_flush_buffer();
    psc_display_powerup();
    ST("  DISPLAY island enabled");
    hal_trace_flush_buffer();

    /* 0) functional display clock: PLLDSI -> DSI/display clock source. The LCDC
     * register block stays dark (reads 0) on just the AHB gate; it needs this pixel
     * clock. CMU DSI_CLK_ENABLE(0x100): SEL_PLL_DSI(1<<3)|RSTN_DIV_DSI(1<<0). */
    ST("  enabling ALL PLLs (BB/DSI/USB) — SMPN source PLL must run...");
    hal_trace_flush_buffer();
    hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_ALL);
    hal_cmu_pll_enable(HAL_CMU_PLL_DSI, HAL_CMU_PLL_USER_ALL);
    hal_cmu_pll_enable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_ALL);
    ST("  PLL status: BB=%d DSI=%d USB=%d",
       hal_cmu_get_pll_status(HAL_CMU_PLL_BB),
       hal_cmu_get_pll_status(HAL_CMU_PLL_DSI),
       hal_cmu_get_pll_status(HAL_CMU_PLL_USB));
    hal_trace_flush_buffer();

    /* 1) clocks: the LCDC sits on the AHB2 / multimedia subsystem (X group:
     * PSRAM/SDMMC0/GPU/L2CC/VMMU/GA2D/MISC/LCDC/AHB2). The whole subsystem bus
     * (X_AHB2) must be clocked or no X-peripheral register responds. Bring up the
     * display pipeline + AHB2, then DISP pclk. */
    static const enum HAL_CMU_MOD_ID_T xmods[] = {
        HAL_CMU_MOD_X_AHB2,
        HAL_CMU_MOD_X_L2CC,
        HAL_CMU_MOD_X_VMMU,
        HAL_CMU_MOD_X_GA2D,
        HAL_CMU_MOD_X_MISC,
        HAL_CMU_MOD_X_LCDC,
    };
    for (unsigned i = 0; i < sizeof(xmods) / sizeof(xmods[0]); i++)
    {
        hal_cmu_clock_enable(xmods[i]);
        hal_cmu_reset_clear(xmods[i]);
    }
    hal_cmu_clock_enable(HAL_CMU_MOD_X_DISP);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISP);
    /* keep LCDC/DISP clocks always-on (MANUAL) — the stock brackets each panel
     * transfer with sub_2C4DD868(1/0), most likely an LCDC clock vote; auto-gating
     * would stop the SMPN SCLK mid-transfer. */
    hal_cmu_clock_set_mode(HAL_CMU_MOD_X_LCDC, HAL_CMU_CLK_MANUAL);
    hal_cmu_clock_set_mode(HAL_CMU_MOD_X_DISP, HAL_CMU_CLK_MANUAL);
    hal_cmu_clock_set_mode(HAL_CMU_MOD_X_AHB2, HAL_CMU_CLK_MANUAL);
    ST("  clocks enabled (X_AHB2..LCDC + X_DISP), LCDC/DISP clock = MANUAL");
    /* EXACT stock CMU display-clock prologue (sub_26F6B8(48)+sub_26F7BA(96)) — the
     * real reason the LCDC block read DARK: the SDK clocks hit the wrong registers. */
    stock_cmu_display_clock();
    ST("  stock CMU display clock applied (0x40000000/10/34/38/44/48/150/154/168/178)");
    hal_trace_flush_buffer();

    /* The transfer "ROM bracket" (sub_26FF90 -> off_2757A4) routes through a runtime-
     * initialised ROM-context function pointer that is INVALID in this image (it
     * resolves into a math routine) -> calling it hard-faults.  We replaced the clock
     * setup with the exact CMU register writes above, so the bracket is unnecessary.
     * Keep g_rom_ok = 0 so lcdc_smpn_read_reg never calls the ROM helpers. */
    ST("  ROM bracket disabled (CMU clock done directly)");
    hal_trace_flush_buffer();

    /* 2) panel power + reset — EXACT stock sub_2C10C138 sequence, decoded from IDA:
     *   pin 12 = P1_4  : power-enable, drive HIGH
     *   pin 24 = P3_0  : RESX (active-low), LOW -> (delay) -> HIGH
     *   pin 23 = P2_7  : 2nd enable / IOVCC, drive HIGH
     * These are real SoC AON-GPIO (bank0 @0x40081000). NOTE: the SDK's
     * hal_iomux_set_spilcd() muxes P3_0 as SPILCD_CLK — WRONG for this board (P3_0 is
     * the panel reset), so it is intentionally NOT called. LCDC drives its panel pads
     * directly; if a data-pad mux is still needed it must come from the stock iomux. */
    /* CORRECTED pins (decoded from stock sub_2C10C138 in IDA 0x0C view):
     *   84044=0x1484C -> pin 76 = GPIO bank2 (0x4008A000) bit12 : POWER-enable HIGH
     *   83992=0x14818 -> pin 24 = P3_0 (bank0) : RESX low->high
     *   83991=0x14817 -> pin 23 = P2_7 (bank0) : enable HIGH
     * (earlier I drove pin 12/P1_4 by a hex slip -> panel never got power.) */
    /* make sure the AON GPIO0/1 controllers (banks) are clocked before driving pin76(bank2) */
    hal_cmu_clock_enable(HAL_CMU_AON_A_GPIO0);
    hal_cmu_clock_enable(HAL_CMU_AON_A_GPIO1);
    /* EXACT stock GPIOs_LCD_inits_reset (sub_2C10C138) — verified: the ONLY display
     * GPIOs are 76/24/23 (pins 75/72/31/26 my old board_pre_lcd_gpios drove are NOT
     * touched by the stock — phantom pins that could interfere, now removed). */
    iomux_set(73, 0);
    gpio_dir_out(73);
    gpio_set(73); /* pin76 POWER -> HIGH */
    iomux_set(74, 0);
    gpio_dir_out(74);
    gpio_set(74); /* pin76 POWER -> HIGH */
    iomux_set(76, 0);
    gpio_dir_out(76);
    gpio_set(76); /* pin76 POWER -> HIGH */
    iomux_set(24, 0);
    gpio_dir_out(24);
    gpio_clr(24);                         /* pin24 RESX  -> LOW  */
    hal_sys_timer_delay(MS_TO_TICKS(10)); /* stock: 10ms */
    iomux_set(23, 0);
    gpio_dir_out(23);
    gpio_set(23);                         /* pin23 enable -> HIGH */
    hal_sys_timer_delay(MS_TO_TICKS(15)); /* stock: 15ms */
    gpio_set(24);                         /* pin24 RESX  -> HIGH (release) */
    hal_sys_timer_delay(MS_TO_TICKS(20)); /* stock: 20ms */
    /* read live pin state (GPIO_EXT_PORT @ +0x50): GPIO0 bit23(P2_7)/bit24(P3_0),
     * GPIO2 bit12 (pin76 power). Confirms which pins actually drive. */
    ST("  panel pwr done. GPIO0 ext=0x%08x (b23=%d b24=%d)  GPIO2 ext=0x%08x (b12=%d)",
       (unsigned)*(volatile uint32_t *)0x40081050u,
       (int)((*(volatile uint32_t *)0x40081050u >> 23) & 1),
       (int)((*(volatile uint32_t *)0x40081050u >> 24) & 1),
       (unsigned)*(volatile uint32_t *)0x4008A050u,
       (int)((*(volatile uint32_t *)0x4008A050u >> 12) & 1));
    hal_trace_flush_buffer();

    /* 4) MUX the real LCDC smart-panel pads.  Decoded from the stock pad table that
     * sub_26FFA0/sub_26FC94 program (RAM 0x20076031): {pin,func} =
     * {40,0x9f}{41,0x01}{42,0xa0}{43,0xa1}{44,0xa2}{45,0xa3}; the func-code -> 4-bit
     * map (tables @0x20076060/67) resolves to: P5_0/2/3/4/5 -> iomux func 2 (SMPN
     * data/clk), P5_1 -> func 0 (GPIO).  My earlier sweep tried pins 25-31 = wrong. */
    iomux_set(40, 2);
    iomux_set(42, 2);
    iomux_set(43, 2);
    iomux_set(44, 2);
    iomux_set(45, 2);
    iomux_set(41, 0);
    gpio_dir_out(41);
    gpio_set(41); /* P5_1 = panel CS, GPIO, idle HIGH */
    *(volatile uint32_t *)0x40080024 = 1;
    while (*(volatile uint32_t *)0x40080024 == 0)
    {
    }
    /* Stock lcdc_panel_pad_init extras the manual func-mux was missing:
     * (a) *((_DWORD*)0x40086000 + 0x2B) |= 0x200  -> reg 0x400860AC bit9 (LCDC pad
     *     direction/enable — the earlier commented line had the WRONG addr 0x4008602B);
     * (b) drive strength 2 for pads 40-45 (iomux_set_drive_strength) -> 0x400860A0,
     *     2 bits/pad at (pad-32)*2.  These give the pads the full bidirectional/driven
     *     config the stock uses (needed for the read pin-direction switch + frame). */
    *(volatile uint32_t *)0x400860ACu |= 0x200u;
    *(volatile uint32_t *)0x400860A0u =
        (*(volatile uint32_t *)0x400860A0u & ~0x0FFF0000u) | 0x0AAA0000u; /* pads40-45 = drive 2 */
    ST("  LCDC pads muxed: P5_0/2/3/4/5 -> func2 + drive2 + 0x400860AC|0x200, P5_1 -> CS(GPIO)");
    hal_trace_flush_buffer();

    /* 5) configure smart-panel pipeline (stock sub_2C564E0C(2,4)) */
    lcdc_smpn_config();
    /* 6b) switch to the 12MHz SMPN transfer clock (stock brackets every transfer with
     * this) — without it the SMPN engine has no SCLK and the transfer times out. */
    cmu_smpn_xfer_clock(SMPN_XFER_MHZ);
    ST("  SMPN transfer clock = 12MHz (sub_26F578(12))");
    hal_trace_flush_buffer();

    /* 7) RM690 RDID: registers 0xDA/0xDB/0xDC -> 24-bit manufacturer/version/driver.
     * raw return -1 = SMPN transfer TIMED OUT (clock/bracket problem); 0xFF with a
     * completing transfer = panel not driving the data line. */
    {
        int a = lcdc_smpn_read_reg(0xDA);
        uint32_t wa = g_last_rx;
        int b = lcdc_smpn_read_reg(0xDB);
        uint32_t wb = g_last_rx;
        int c = lcdc_smpn_read_reg(0xDC);
        uint32_t wc = g_last_rx;
        uint32_t id = ((uint32_t)(a & 0xFF) << 16) | ((uint32_t)(b & 0xFF) << 8) | (uint32_t)(c & 0xFF);
        ST("  TestRDID FIFO words: DA=0x%08lx DB=0x%08lx DC=0x%08lx",
           (unsigned long)wa, (unsigned long)wb, (unsigned long)wc);
        ST("  RDID(0xDA/DB/DC)=0x%06lX  %s", (unsigned long)id, lcd_variant(id));
        hal_trace_flush_buffer();
        /* read back panel state: 0x0A=power-mode, 0x0B=MADCTL, 0x0C=COLMOD(pixel fmt) */
        int pm = lcdc_smpn_read_reg(0x0A);
        int madctl = lcdc_smpn_read_reg(0x0B);
        int colmod = lcdc_smpn_read_reg(0x0C);
        ST("  panel regs: 0x0A(PM)=0x%02x 0x0B(MADCTL)=0x%02x 0x0C(COLMOD)=0x%02x (want 0x55=16bpp)",
           pm & 0xFF, madctl & 0xFF, colmod & 0xFF);
        hal_trace_flush_buffer();
    }
    /* confirm the pad mux landed: read back IOMUX func regs for pins 40-47 (0x40086080)
     * and 32-39 (0x4008607C). pin40 func nibble = 0x40086080[3:0]. */
    ST("  iomux readback: 0x4008607C=0x%08x 0x40086080=0x%08x (pins40-45 want func2)",
       (unsigned)*(volatile uint32_t *)0x4008607Cu, (unsigned)*(volatile uint32_t *)0x40086080u);
    hal_trace_flush_buffer();

    /* Run the REAL stock RM690C0 init sequence (rm690c0_panel_init), then re-read the
     * ID — the panel needs page-select/config before it behaves.  WATCH THE SCREEN. */
    ST("  >>> running RM690 panel init (33 cmds) + display-on + brightness... WATCH SCREEN <<<");
    hal_trace_flush_buffer();
    rm690_panel_init();
    ST("  panel init sent.");
    { /* POST-init panel state: did 0x3A=0x55 (16bpp) take? */
        int pm = lcdc_smpn_read_reg(0x0A);
        int madctl = lcdc_smpn_read_reg(0x0B);
        int colmod = lcdc_smpn_read_reg(0x0C);
        ST("  POST-init regs: 0x0A(PM)=0x%02x 0x0B(MADCTL)=0x%02x 0x0C(COLMOD)=0x%02x (want 0x55)",
           pm & 0xFF, madctl & 0xFF, colmod & 0xFF);
        hal_trace_flush_buffer();
    }
    hal_trace_flush_buffer();

    lcd_set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    // 2. Framebuffer im PSRAM vorbereiten (Hintergrund Schwarz)
    fb_clear(0x1234);
    g_cursor_x = 10;
    g_cursor_y = 20;
    fb_refresh(); /* draw the initial frame once */
    return 0;     /* hand off to the live touch -> display loop */
}

/* Software-I2C touch probe: power rails + reset the Zinitix ztw623 (pin28), then
 * bit-bang every SoC I2C pad pair looking for an ACK at addr 0x20. */
/* Stock board_init proves the touch hangs off I2C controller 3.  dword_2C63568C[3]
 * is the port-3 i2c struct; its pin-config (struct+4) resolves to SCL=pin6 (func
 * 0x3e), SDA=pin7 (func 0x3f) -> the touch bus is P0_6/P0_7, *not* 8/9.  ztw623
 * regs are 16-bit little-endian: write = [addr|W][reg_lo][reg_hi]; read is a
 * SEPARATE transaction [addr|R][lo][hi] (no repeated-start).  check-id wants 0xE623. */
static int touch_addr = 0x5A; /* ztw623 ACKs here on the real bus (scan-confirmed) */
/* ztw623 16-bit LE register access (matches stock ztw623_i2c_write/_read). */
static void tw_w16(int reg)
{
    i2c_start();
    i2c_wbyte((touch_addr << 1) | 0);
    i2c_wbyte(reg & 0xff);
    i2c_wbyte((reg >> 8) & 0xff);
    i2c_stop();
}
/* set register pointer (reg), then block-read n bytes (auto-increment) into buf */
static void tw_block(int reg, unsigned char *buf, int n)
{
    tw_w16(reg);
    i2c_start();
    i2c_wbyte((touch_addr << 1) | 1);
    for (int i = 0; i < n; i++)
        buf[i] = i2c_rbyte(i < n - 1); /* ACK all but last */
    i2c_stop();
}

/* The touch on this board is a Hynitron CST92xx (hynitron_core.c / "CST92xx_MCU_
 * Driver"), I2C addr 0x5A -- NOT the ztw623.  Confirm presence + read its touch
 * report (normal work mode: read N bytes starting at reg 0x0000). */
static int touch_try_bus(int scl, int sda)
{
    unsigned char buf[16];
    i2c_pins_init(scl, sda);
    hal_sys_timer_delay(MS_TO_TICKS(2));
    int aw = i2c_probe(touch_addr);
    for (int i = 0; i < 16; i++)
        buf[i] = 0;
    tw_block(0x0000, buf, 12); /* Hynitron normal-mode report block */
    ST("  CST92xx @0x%02x ack=%d  report: %02x %02x %02x %02x %02x %02x %02x %02x",
       touch_addr, aw, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    /* a present, talking IC -> ACK + at least one non-0xFF byte in the report */
    int real = 0;
    for (int i = 0; i < 12; i++)
        if (buf[i] != 0xFF)
            real = 1;
    hal_trace_flush_buffer();
    gpio_dir_in(scl);
    gpio_dir_in(sda);
    return (aw == 0) && real;
}

/* check if the touch bus (pins 6/7) is un-clamped: enable internal pull-up and
 * read.  Returns 1 if BOTH read high (touch powered, bus idle). */
static int touch_bus_alive(void)
{
    iomux_set(6, 0);
    gpio_dir_in(6);
    iomux_pullup(6);
    iomux_set(7, 0);
    gpio_dir_in(7);
    iomux_pullup(7);
    hal_sys_timer_delay(MS_TO_TICKS(5));
    return gpio_rd(6) && gpio_rd(7);
}

/* Hynitron CST92xx register access: registers are BIG-ENDIAN on the wire (write
 * [hi,lo]), slave 0x5A (decoded from stock sub_2C101138: cmd 0xA010 -> bytes A0 10). */
#define TOUCH_INT_PIN 27 /* INT (P3_3), active-low; reset=28, power=73/74 */
static void hyn_rd(int reg, unsigned char *buf, int n)
{
    i2c_start();
    i2c_wbyte((touch_addr << 1) | 0);
    i2c_wbyte((reg >> 8) & 0xff);
    i2c_wbyte(reg & 0xff);
    i2c_stop();
    i2c_start();
    i2c_wbyte((touch_addr << 1) | 1);
    for (int i = 0; i < n; i++)
        buf[i] = i2c_rbyte(i < n - 1);
    i2c_stop();
}
static void hyn_wr1(int reg, int d)
{
    i2c_start();
    i2c_wbyte((touch_addr << 1) | 0);
    i2c_wbyte((reg >> 8) & 0xff);
    i2c_wbyte(reg & 0xff);
    i2c_wbyte(d & 0xff);
    i2c_stop();
}

/* Read one CST92xx touch frame from the 0xD000 buffer.  Hynitron 12-bit packed
 * layout (verified on HW against 3 swipes): byte6 = 0xAB frame marker, byte0 =
 * event (0x06 down / 0x07 move / 0x00 up), X = (b1<<4)|(b3>>4), Y = (b2<<4)|(b3&0xF).
 * Returns 1 if a finger is present, filling x,y,ev. */
static int touch_read(int *x, int *y, int *ev)
{
    unsigned char b[8];
    hyn_rd(0xD000, b, 7);
    hyn_wr1(0xD000, 0xAB); /* ack the frame so the chip refreshes */
    if (b[6] != 0xAB)
        return 0; /* no valid frame -> no touch */
    *ev = b[0];
    *x = (b[1] << 4) | (b[3] >> 4);
    *y = (b[2] << 4) | (b[3] & 0x0F);
    return 1;
}

#ifdef DOOM
/* DOOM input hook (apps/doom/i_system.c read_touchscreen): one-time pin/bus init,
 * then poll the CST92xx. Returns 1 if a finger is down (x<212, y<520, ev as above). */
int doom_touch_poll(int *x, int *y, int *ev)
{
    /* The CST92xx only emits a fresh frame (b[6]==0xAB) on press/move; holding a
     * finger STILL produces no new frame, so touch_read() alone reads "no touch"
     * and movement stops. Keep the press latched while the finger is still on the
     * panel: trust the INT pin (active-low while touched) and, as a fallback for
     * pulsed-INT, a short grace timer refreshed by any frame/INT activity. */
    static int inited = 0, ldown = 0, lx = 106, ly = 260, hold = 0, int_idle = 1;
    if (!inited)
    {
        iomux_set(TOUCH_INT_PIN, 0); gpio_dir_in(TOUCH_INT_PIN); iomux_pullup(TOUCH_INT_PIN);
        i2c_pins_init(6, 7);
        int_idle = gpio_rd(TOUCH_INT_PIN);   /* learn the no-touch level (no finger at boot) */
        inited = 1;
    }

    int nx = 0, ny = 0, nev = 0;
    int frame   = touch_read(&nx, &ny, &nev);
    int present = (gpio_rd(TOUCH_INT_PIN) != int_idle);   /* finger on panel = INT differs from idle */

    if (frame)
    {
        if (nev == 0x00) { ldown = 0; hold = 0; }                   /* explicit lift */
        else             { ldown = 1; lx = nx; ly = ny; hold = 10; } /* press/move */
    }
    else if (present)    { ldown = 1; hold = 10; }                  /* held still, INT active */
    else if (hold > 0)   { if (--hold == 0) ldown = 0; }           /* grace for pulsed INT */
    else                 { ldown = 0; }

    *x = lx; *y = ly; *ev = ldown ? 0x06 : 0x00;
    return ldown;
}
#endif

/* Live touch poll: print decoded finger events while the user swipes the panel. */
static void st_touch_poll(void)
{
    ST("---- TOUCH POLL: swipe/tap the display now (~20s, X<212 Y<520) ----");
    iomux_set(TOUCH_INT_PIN, 0);
    gpio_dir_in(TOUCH_INT_PIN);
    iomux_pullup(TOUCH_INT_PIN);
    i2c_pins_init(6, 7);
    hal_trace_flush_buffer();
    int prev = 0;
    for (int it = 0; it < 650; it++)
    {
        int x = 0, y = 0, ev = 0;
        int down = touch_read(&x, &y, &ev);
        if (down)
        {
            const char *es = ev == 0x06 ? "DOWN" : ev == 0x07 ? "MOVE"
                                               : ev == 0x00   ? "UP  "
                                                              : "?   ";
            ST("  TOUCH %s  X=%3d  Y=%3d   INT=%d", es, x, y, gpio_rd(TOUCH_INT_PIN));
            hal_trace_flush_buffer();
        }
        else if (prev)
        {
            ST("  (lift)");
            hal_trace_flush_buffer();
        }
        prev = down;
        hal_sys_timer_delay(MS_TO_TICKS(30));
    }
    gpio_dir_in(6);
    gpio_dir_in(7);
    ST("---- TOUCH POLL done ----");
    hal_trace_flush_buffer();
}

/* === HARDWARE QUAD frame push experiment (goal: replace the slow CPU bit-bang).
 * LCDC compositor + SMPN quad auto-head (0x22C=0x86002C00), triggered SW-TE one-shot
 * exactly like stock hal_lcdc_start_dsi: gen_frame OFF, SMPN enable, TECR USE_SW,
 * write TE-trigger (0x258).  Pushes a solid 130-row strip from SRAM and reports
 * whether TXC-framedone (ISR 0x1C4 bit21) fires.  This was the long-standing blocker
 * (compositor ran VSYNC but never completed) -- retrying now everything else works. */
static void lcdc_pads_hw(void)
{ /* pads 40,42-45 -> LCDC func2, CS=GPIO */
    iomux_set(40, 2);
    iomux_set(42, 2);
    iomux_set(43, 2);
    iomux_set(44, 2);
    iomux_set(45, 2);
    iomux_set(41, 0);
    gpio_dir_out(41);
    gpio_set(41);
    *(volatile uint32_t *)0x400860ACu |= 0x200u;
    *(volatile uint32_t *)0x400860A0u =
        (*(volatile uint32_t *)0x400860A0u & ~0x0FFF0000u) | 0x0AAA0000u;
}
/* HW QSPI display init: clocks + SMPN config + panel init + pads + SCLK divider.
 * (Re-asserts the full working state; the panel was already brought up in st_display_id.) */
static void disp_init(void)
{
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_208M);
    cmu_disp_clk_full_seq();            /* full display clock-domain enable */
    lcdc_smpn_config();                 /* pads + SCLK_DIV + REG_2A8 + SMPN base regs */
    cmu_smpn_xfer_clock(SMPN_XFER_MHZ); /* command clock for the panel init */
    lcdc_pads_hw();                     /* 40,42-45 -> func2, pin41 = CS gpio */
    rm690_panel_init();                 /* RM690 init (CASET/RASET/COLMOD/DISPON) */
    LCDC_R(LCDC_SCLK_DIV) = 1;          /* SCLK = display(48MHz)/2 ~= 25MHz */
}

#ifdef DOOM
/* ====================================================================== *
 *  HW 4-LANE QUAD-SPI path for DOOM (driver: platform/main/disp_hwquad.c).
 *
 *  st_display_id() already powered the panel + PSC island + PLLs + the X-subsystem
 *  clocks. Here we add what the GEN_FRAME DMA path needs that the 1-lane FIFO path
 *  did not: the boot module HCLKs (disp_boot_clocks) + the LCDC sysfreq vote that
 *  gives the DMA master the AXI bandwidth to keep the 96 MHz serializer fed, then run
 *  the quad init (RDID -> variant init -> window). After this, lcd_render_fb() (lcd.c)
 *  ping-pongs two RGB565 buffers through disp_hwquad_start()/disp_hwquad_wait().
 * ====================================================================== */
extern void disp_boot_clocks(void);
extern void disp_hwquad_init(void);

void doom_quad_bringup(void)
{
    static int done = 0;            /* the launcher page brings this up first; DOOM's re-call is a no-op */
    if (done) return;
    done = 1;
    disp_boot_clocks();                                 /* module clocks for the GEN_FRAME DMA
                                                         * (leaves the UART clocks intact) */
    /* re-assert the X-subsystem clocks (disp_boot_clocks rewrites CMU) + raise AXI bandwidth */
    static const enum HAL_CMU_MOD_ID_T xmods[] = {
        HAL_CMU_MOD_X_AHB2, HAL_CMU_MOD_X_L2CC, HAL_CMU_MOD_X_VMMU,
        HAL_CMU_MOD_X_GA2D, HAL_CMU_MOD_X_MISC, HAL_CMU_MOD_X_LCDC,
    };
    for (unsigned i = 0; i < sizeof(xmods) / sizeof(xmods[0]); i++) {
        hal_cmu_clock_enable(xmods[i]); hal_cmu_reset_clear(xmods[i]);
    }
    hal_cmu_clock_enable(HAL_CMU_MOD_X_DISP); hal_cmu_reset_clear(HAL_CMU_MOD_X_DISP);
    hal_cmu_clock_set_mode(HAL_CMU_MOD_X_LCDC, HAL_CMU_CLK_MANUAL);
    hal_cmu_clock_set_mode(HAL_CMU_MOD_X_DISP, HAL_CMU_CLK_MANUAL);
    hal_cmu_clock_set_mode(HAL_CMU_MOD_X_AHB2, HAL_CMU_CLK_MANUAL);
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_208M);

    disp_hwquad_init();                                 /* PSC + 4-lane LCDC config + panel init */
}

/* Clean the framebuffer out of the D-cache so the LCDC DMA master sees the CPU's writes
 * (the panel buffers live in SRAM/RAM4-5). Called by lcd.c before each quad send. */
void doom_fb_flush(const void *p, unsigned bytes)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanDCache_by_Addr((uint32_t *)(uintptr_t)p, (int32_t)bytes);
#endif
    __DSB();
}

/* RGB565 control overlay, rendered ONCE into each ping-pong buffer (212x520). Mirrors the
 * 8bpp doom_draw_ui() layout but writes 16bpp into the given buffer; the centred 120x160 game
 * image (x46..165 y180..339) is left black and repainted every frame by lcd_render_fb(). */
/* SIDEWAYS overlay helpers: draw in SCREEN coords (sx = screen horizontal = panel py 0..519,
 * sy = screen vertical = panel px 0..211) so text reads upright when the watch is held rotated
 * 90 deg. Maps to fb[sx*DISPLAY_WIDTH + sy]. The DOOM image band (sx 119..400) is left black and
 * repainted by lcd.c; the LEFT strip (sx<119) and RIGHT strip (sx>=401) hold the controls. */
static inline void q_spx(uint16_t *b, int sx, int sy, uint16_t c)
{   /* sx -> panel py (long axis, MIRRORED), sy -> panel px. The py mirror turns the col->sx/row->sy
     * transpose into a proper rotation (upright text) and matches the un-mirrored DOOM + touch. */
    if ((unsigned)sx < DISPLAY_HEIGHT && (unsigned)sy < DISPLAY_WIDTH)
        b[(DISPLAY_HEIGHT - 1 - sx) * DISPLAY_WIDTH + sy] = c;
}
static void q_stext(uint16_t *b, int sx, int sy, const char *s, uint16_t col, int sc)
{
    while (*s) {
        unsigned int idx = (unsigned char)*s++; if (idx > 0xFE) idx = 0x20;
        const unsigned char *bm = &font57[idx * 5];
        for (int c = 0; c < 5; c++) {
            unsigned char d = bm[c];
            for (int r = 0; r < 8; r++) if (d & (1 << r))
                for (int yy = 0; yy < sc; yy++) for (int xx = 0; xx < sc; xx++)
                    q_spx(b, sx + c * sc + yy, sy + r * sc + xx, col);   /* q_spx mirrors sx (py) */
        }
        sx += 6 * sc;
    }
}
static void q_shline(uint16_t *b, int sx0, int sx1, int sy, uint16_t c) { for (int s = sx0; s <= sx1; s++) q_spx(b, s, sy, c); }
static void q_svline(uint16_t *b, int sx, int sy0, int sy1, uint16_t c) { for (int s = sy0; s <= sy1; s++) q_spx(b, sx, s, c); }
static void q_srect(uint16_t *b, int sx0, int sy0, int sx1, int sy1, uint16_t c)
{
    q_shline(b, sx0, sx1, sy0, c); q_shline(b, sx0, sx1, sy1, c);
    q_svline(b, sx0, sy0, sy1, c); q_svline(b, sx1, sy0, sy1, c);
}
void doom_ui_render565(uint16_t *b)
{
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) b[i] = 0x0000;   /* black panel */

    /* LEFT strip (sx 0..118) = arrow D-pad. screen top = sy 0, bottom = sy 211. */
    q_srect(b,  2,   2, 116,  68, 0x07FFu); q_stext(b, 41,  28, "FWD",  0x07FFu, 2); /* up    */
    q_srect(b,  2, 144, 116, 209, 0x07FFu); q_stext(b, 35, 170, "BACK", 0x07FFu, 2); /* down  */
    q_srect(b,  2,  72,  56, 140, 0x07FFu); q_stext(b, 21,  99, "<",    0x07FFu, 2); /* left  */
    q_srect(b, 62,  72, 116, 140, 0x07FFu); q_stext(b, 81,  99, ">",    0x07FFu, 2); /* right */

    /* RIGHT strip (sx 401..519) = function keys. */
    q_srect(b, 403,   2, 517, 118, 0xF800u); q_stext(b, 436,  53, "FIRE", 0xF800u, 2); /* fire */
    q_srect(b, 403, 122, 517, 174, 0x07E0u); q_stext(b, 442, 141, "USE",  0x07E0u, 2); /* use  */
    q_srect(b, 403, 178, 517, 209, 0xFFE0u); q_stext(b, 436, 187, "MENU", 0xFFE0u, 2); /* menu */

    /* "loading..." centred in the game band (sx 119..400). Shown while DOOM inits (1-2 s); the
     * first rendered frame (lcd_render_fb) repaints the band and overwrites it automatically. */
    q_stext(b, 175, 96, "loading...", 0xFFFFu, 3);
}

/* Title over the top of the DOOM image (sy small = screen top). Drawn every frame by lcd.c
 * AFTER the game blit so it stays on top (the game band is repainted each frame). */
void doom_draw_title(uint16_t *b)
{
    q_stext(b, 158,  6, "DOOM on MiBand 10", 0xFFFFu, 2);
    q_stext(b, 200, 26, "by ATC1441",        0xFFFFu, 2);
}

/* Called once per rendered frame from lcd.c; prints the frame rate to the trace UART each second.
 * Once DOOM owns the CPU the SDK trace never flushes (its DMA/thread is starved) and an explicit
 * hal_trace_flush_buffer() blocks -> hang. So write the line DIRECTLY to the trace UART with the
 * polled blocking putc (UART1 = DEBUG_PORT 2 = COM24): synchronous, no DMA/thread, ~0.2 ms/line. */
void doom_frame_tick(void)
{
    static uint32_t frames = 0, t0 = 0; static int inited = 0;
    if (!inited) { t0 = hal_sys_timer_get(); inited = 1; }
    frames++;
    uint32_t now = hal_sys_timer_get();
    if ((now - t0) >= MS_TO_TICKS(1000)) {
        extern void am_util_stdio_printf(const char *fmt, ...);
        am_util_stdio_printf("DOOM FPS: %lu\r\n", (unsigned long)frames);
        frames = 0; t0 = now;
    }
}
#endif /* DOOM */

/* Live touch -> on-screen, ENTIRELY via HW QSPI (1-lane, FIFO single-write). No SW bit-bang. */
static void st_touch_display_loop(void)
{
    ST("---- TOUCH -> DISPLAY (HW QSPI 1-lane): tap / swipe the screen ----");
    hal_trace_flush_buffer();
    iomux_set(TOUCH_INT_PIN, 0);
    gpio_dir_in(TOUCH_INT_PIN);
    iomux_pullup(TOUCH_INT_PIN);
    i2c_pins_init(6, 7);

    disp_init();             /* bring up the HW QSPI display */
    fb_render_ui();          /* render the touch UI into FB_PTR ... */
    disp_send_frame(FB_PTR); /* ... and push it via HW QSPI */

    int last_x = -1, last_y = -1, last_down = -1, dbg = 0;
    while (1)
    {
        int x = 0, y = 0, ev = 0;
        int got = touch_read(&x, &y, &ev);
        int pressed = got && (ev == 0x06 || ev == 0x07); /* 0x06 down, 0x07 move */
        if ((dbg++ % 120) == 0)
        {
            ST("  [poll] got=%d ev=0x%02x x=%d y=%d", got, ev, x, y);
            hal_trace_flush_buffer();
        }
        if (pressed)
        {
            g_touch_x = x;
            g_touch_y = y;
        }
        g_touch_down = pressed;
        if (pressed != last_down || (pressed && (x != last_x || y != last_y)))
        {
            if (pressed)
                snprintf(g_touch_str, sizeof(g_touch_str), "X=%d Y=%d", x, y);
            else
                snprintf(g_touch_str, sizeof(g_touch_str), "TOUCH THE SCREEN");
            fb_render_ui();
            disp_send_frame(FB_PTR); /* repaint via HW QSPI on change */
            last_x = x;
            last_y = y;
            last_down = pressed;
        }
        hal_sys_timer_delay(MS_TO_TICKS(8));
    }
}

static void st_swi2c_touch(void)
{
    ST("---- SW-I2C: Hynitron CST92xx touch (I2C ctrl 3 = pins 6/7, addr 0x5A) ----");
    hal_cmu_clock_enable(HAL_CMU_AON_A_GPIO0);
    hal_cmu_clock_enable(HAL_CMU_AON_A_GPIO1);
    /* pin73 = the touch/sensor VDD rail enable (found empirically: driving it HIGH
     * un-clamps pins 6/7 -> the ztw623 powers up & external I2C pull-ups energize). */
    iomux_set(73, 0);
    gpio_dir_out(73);
    gpio_set(73); /* TOUCH VDD RAIL ON (pin73) */
    iomux_set(74, 0);
    gpio_dir_out(74);
    gpio_set(74); /* touch enable (pin74) */
    hal_sys_timer_delay(MS_TO_TICKS(20));
    ST("  pin73 VDD on -> bus 6/7 idle = %d", touch_bus_alive());
    /* RESX (pin28): assert LOW 100ms, release HIGH, then give the ztw623 time to
     * boot its internal firmware (~200ms) before it answers on I2C. */
    iomux_set(28, 0);
    gpio_dir_out(28);
    gpio_clr(28);
    hal_sys_timer_delay(MS_TO_TICKS(100));
    gpio_set(28);
    hal_sys_timer_delay(MS_TO_TICKS(200));
    ST("  reset(pin28) done; scanning bus 6/7 for any device...");
    hal_trace_flush_buffer();

    /* full 7-bit address scan on the now-healthy bus (pins 6/7). The CST92xx can be
     * marginally slow to answer right after reset, so settle first. */
    i2c_pins_init(6, 7);
    hal_sys_timer_delay(MS_TO_TICKS(20));
    int n = 0;
    for (int a = 0x08; a <= 0x77; a++)
    {
        if (i2c_probe(a) == 0)
        {
            ST("    ACK @ 0x%02x", a);
            n++;
        }
    }
    ST("  bus scan: %d device(s) found", n);
    gpio_dir_in(6);
    gpio_dir_in(7);
    hal_trace_flush_buffer();

    int found = touch_try_bus(6, 7);
    ST("  SW-I2C touch: Hynitron CST92xx @0x5A %s",
       found ? "ALIVE (ACK + valid report) <<<<<" : "no response");
    hal_trace_flush_buffer();
    (void)st_touch_poll; /* touch input deferred — focus on display bring-up */
}

#ifdef DOOM
/* ===== boot LAUNCHER page: live sensor dashboard + touch buttons =============
 * A full-screen RGB565 page rendered (reusing the DOOM quad path + sideways text
 * helpers) BEFORE handing off to DOOM. Tap PLAY DOOM to start the game; tap VIBRATE
 * to pulse the aw86225 motor. Live data: charger hp4560x (bus0) + IMU @0x0E (bus5).
 * Every I2C access is the bounded bit-bang and the loop osDelay()s each frame, so it
 * can never trip the 15 s hung-task watchdog. */
extern void disp_hwquad_start(const uint16_t *fb);
extern void disp_hwquad_wait(void);

__attribute__((unused)) static char *put_hex(char *p, unsigned v, int digits)
{
    static const char H[] = "0123456789ABCDEF";
    for (int i = digits - 1; i >= 0; i--) { p[i] = H[v & 0xF]; v >>= 4; }
    return p + digits;
}
__attribute__((unused)) static void rd_regs(int addr, uint8_t start, uint8_t *out, int n)
{
    for (int i = 0; i < n; i++) { int v = swi2c_rdreg(addr, (uint8_t)(start + i)); out[i] = (v < 0) ? 0xFF : (uint8_t)v; }
}
__attribute__((unused)) static char *put_int(char *p, int v)        /* signed decimal */
{
    if (v < 0) { *p++ = '-'; v = -v; }
    char t[7]; int n = 0;
    do { t[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (n) *p++ = t[--n];
    return p;
}
/* touch reports panel px (tx 0..211) / py (ty 0..519); sideways screen is sx=519-ty, sy=tx. */
static int in_rect(int tx, int ty, int sx0, int sy0, int sx1, int sy1)
{
    int sx = 519 - ty, sy = tx;
    return sx >= sx0 && sx <= sx1 && sy >= sy0 && sy <= sy1;
}
/* best-effort aw86225 buzz: chip is already powered/reset by sensor_power_on(). Probe the
 * 0x58..0x5B address block on bus2 and kick the RTP GO bit (reg 0x09). Crude -- refine later. */
__attribute__((unused)) static int motor_buzz(void)
{
    i2c_pins_init(4, 5);                         /* bus2 = P0_4/P0_5 */
    int addr = -1;
    for (int a = 0x58; a <= 0x5B; a++) if (i2c_probe(a) == 0) { addr = a; break; }
    if (addr < 0) { ST("  motor: aw86225 no ACK on bus2 (P0_4/P0_5)"); return -1; }
    int id = swi2c_rdreg(addr, 0x00);
    swi2c_wrreg(addr, 0x09, 0x01);               /* GO */
    ST("  motor: aw86225 @0x%02X id=0x%02X kicked (GO)", addr, id);
    return addr;
}

static void launcher_page(void)
{
    ST("==> LAUNCHER page (tap START DOOM)");
    hal_trace_flush_buffer();
    doom_quad_bringup();                          /* quad streaming up (guarded; DOOM re-call no-op) */
    iomux_set(TOUCH_INT_PIN, 0); gpio_dir_in(TOUCH_INT_PIN); iomux_pullup(TOUCH_INT_PIN);

    uint16_t *b = (uint16_t *)0x20080000u;        /* DOOM FB-A; DOOM repaints it after we exit */
    int prev_down = 0;
    for (;;) {
        /* ---- render the info page (sideways: sx 0..519 horizontal, sy 0..211 vertical) ---- */
        for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) b[i] = 0x0000;
        q_stext(b, 104, 18, "BES2700iMP",          0x07FFu, 3);   /* cyan title (clear of rounded corner) */
        q_stext(b, 24,  56, "Reverse engineering", 0xFFFFu, 2);
        q_stext(b, 24,  78, "by ATC1441",          0xFFE0u, 2);   /* yellow              */
        q_stext(b, 24, 118, "full Quad SPI AMOLED", 0xFFFFu, 2);
        q_stext(b, 24, 142, "Driving DOOM Player",  0x07E0u, 2);  /* green               */

        /* single START DOOM button (right side, thick green border) */
        q_srect(b, 312,  66, 506, 150, 0x07E0u);
        q_srect(b, 313,  67, 505, 149, 0x07E0u);
        q_srect(b, 314,  68, 504, 148, 0x07E0u);
        q_stext(b, 348, 101, "START DOOM", 0x07E0u, 2);

        doom_fb_flush(b, DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
        disp_hwquad_start(b);
        disp_hwquad_wait();

        /* ---- touch ---- */
        i2c_pins_init(6, 7);
        int tx = 0, ty = 0, ev = 0;
        int down = touch_read(&tx, &ty, &ev);
        if (down && !prev_down) {
            ST("  TAP px=%d py=%d", tx, ty);
            if (in_rect(tx, ty, 312, 66, 506, 150)) { ST("  -> START DOOM"); break; }
        }
        prev_down = down;
        osDelay(30);                                /* yield to scheduler (feeds the WDT) */
    }
    ST("==> launcher exit -> launching DOOM");
    hal_trace_flush_buffer();
}
#endif /* DOOM */

#define test_current_gpio_pin 46
void hw_selftest(void)
{
    ST("========== HW SELFTEST START ==========");
    hal_trace_flush_buffer(); /* flush the banner immediately so we can see how far boot gets */
    /* --- FB sanity: is the high-SRAM framebuffer window real, mapped RAM? Write a test
     *     pattern to the first and last word of the 0x6BA80 region and read it back. If
     *     the write bus-faults, we see "writing..." but never the readback line. --- */
    {
        volatile uint32_t *fb_lo = (volatile uint32_t *)FB_FULL_ADDR;
        volatile uint32_t *fb_hi = (volatile uint32_t *)(FB_FULL_ADDR + FB_FULL_SIZE - 4u);
        ST("  FB probe: writing 0x%08x .. 0x%08x", (unsigned)FB_FULL_ADDR, (unsigned)(FB_FULL_ADDR + FB_FULL_SIZE - 4u));
        hal_trace_flush_buffer();
        *fb_lo = 0xA5A5A5A5u;
        *fb_hi = 0x5A5A5A5Au;
        ST("  FB probe readback: lo=0x%08x hi=0x%08x (expect A5A5A5A5 / 5A5A5A5A)",
           (unsigned)*fb_lo, (unsigned)*fb_hi);
        hal_trace_flush_buffer();
    }
    /*hal_cmu_clock_enable(HAL_CMU_AON_A_GPIO0);
    hal_cmu_clock_enable(HAL_CMU_AON_A_GPIO1);
    iomux_set(test_current_gpio_pin, 0);
    gpio_dir_out(test_current_gpio_pin);
    gpio_set(test_current_gpio_pin);
    while (1)
    {
        gpio_clr(test_current_gpio_pin);
        hal_sys_timer_delay(MS_TO_TICKS(300));
        gpio_set(test_current_gpio_pin);
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }*/
    st_swi2c_touch();        /* power + reset the CST92xx touch chip */
    st_display_id();         /* bring up the RM690C0 panel (returns after init) */
#ifdef DOOM
    /* GBADoom port (apps/doom/): panel is up, hand off to the game (never returns). */
    charger_wdt_kill();      /* disable the external charger WDT (stops the ~2.5 min reset) */
    sensor_power_on();       /* enable IMU/PPG (P3_6) + motor (P1_2/P1_3) rails */
    launcher_page();         /* dashboard + touch buttons; returns when PLAY DOOM is tapped */
    { extern void doom_launch(void); ST("==> launching DOOM"); hal_trace_flush_buffer(); doom_launch(); }
#endif
    st_touch_display_loop(); /* live touch -> on-screen crosshair + coordinates */
    st_ram_buffer();
    st_timer();
    (void)st_sleep_loop;
    (void)st_sram_full_test;
    (void)st_flash_test;
    (void)sw_i2c_power_sweep;
    (void)st_drive_safety_scan;
    ST("========== HW SELFTEST DONE ==========");
}

#endif /* HW_SELFTEST */
