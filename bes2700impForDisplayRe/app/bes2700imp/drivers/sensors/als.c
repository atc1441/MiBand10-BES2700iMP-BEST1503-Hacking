/* als.c — ambient light sensor. [SCAFFOLD] */
#include "als.h"
#include "i2c_bus.h"
#include "board_config.h"
#define BUS   BRD_SENSOR_I2C_ID
#define ADDR  ((uint8_t)BRD_ALS_I2C_ADDR)

int als_init(void)
{
    if (BUS < 0 || BRD_ALS_I2C_ADDR < 0) return -1;        /* TODO(probe) */
    if (!i2c_present(BUS, ADDR)) return -1;
    /* TODO(probe): enable + integration-time/gain config */
    return 0;
}
uint32_t als_read_lux(void)
{
    uint8_t r[2] = {0,0};
    if (BUS >= 0 && BRD_ALS_I2C_ADDR >= 0) i2c_rd_reg(BUS, ADDR, 0x00 /*TODO*/, r, 2);
    return (uint32_t)(r[0] | (r[1] << 8));                  /* TODO(probe): lux scaling */
}
