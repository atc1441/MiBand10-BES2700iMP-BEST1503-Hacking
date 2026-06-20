/* bes_compat.h - AmbiqSuite (Apollo) SDK shims for the BES2700/best1503 port.
 *
 * The GBADoom engine was copied from the MiBand8 Apollo4 example. A handful of
 * source files (d_main.c, i_system.c, lprintf.c) call Apollo SDK helpers. This
 * header replaces the former  #include "am_mcu_apollo.h"/"am_bsp.h"/"am_util.h"
 * so the engine is fully independent of the AmbiqSuite tree. The actual
 * implementations live in bes_glue.c.
 */
#ifndef BES_DOOM_COMPAT_H
#define BES_DOOM_COMPAT_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* am_util_stdio_printf -> BES trace (see bes_glue.c). */
void am_util_stdio_printf(const char *fmt, ...);

/* Watchdog kick. The BES firmware owns its own watchdog; doom's periodic
 * "feed the dog" calls become a no-op here (override in bes_glue.c if needed). */
#define AM_HAL_WDT_MCU            0
#define am_hal_wdt_restart(unit)  ((void)(unit))

/* Board input helpers used by i_system.c's touch reader (stubbed in bes_glue.c
 * until the best1503 touch controller is wired in). */
int  digitalRead(int pin);
void touch_transceive(unsigned int reg, unsigned int *buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* BES_DOOM_COMPAT_H */
