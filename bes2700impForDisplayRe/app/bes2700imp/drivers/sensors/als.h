/* als.h — ambient light sensor (MiBand9). [SCAFFOLD]
 * Drives auto-brightness. I2C ALS, address TODO(probe). The stock FW logs an
 * "als_lcd_coef" — ALS reading feeds the display brightness curve. */
#ifndef BES2700IMP_ALS_H
#define BES2700IMP_ALS_H
#include <stdint.h>
int      als_init(void);
uint32_t als_read_lux(void);   /* raw->lux (TODO(probe): part-specific scaling) */
#endif
