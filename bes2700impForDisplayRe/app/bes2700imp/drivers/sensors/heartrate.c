/* heartrate.c — PPG/HR AFE. [SCAFFOLD] */
#include "heartrate.h"
#include "i2c_bus.h"
#include "board_config.h"
#define BUS   BRD_SENSOR_I2C_ID
#define ADDR  ((uint8_t)BRD_HR_I2C_ADDR)

int hr_init(void)
{
    if (BUS < 0 || BRD_HR_I2C_ADDR < 0) return -1;          /* TODO(probe) */
    if (!i2c_present(BUS, ADDR)) return -1;
    /* TODO(probe): reset, LED current, sample rate, FIFO config */
    return 0;
}
void hr_start(void) { /* TODO(probe): enable LEDs + start sampling */ }
void hr_stop(void)  { /* TODO(probe): LEDs off */ }
bool hr_read_sample(uint32_t *ppg)
{
    uint8_t r[3] = {0,0,0};
    if (BUS < 0 || BRD_HR_I2C_ADDR < 0) return false;
    if (i2c_rd_reg(BUS, ADDR, 0x00 /*TODO: FIFO data*/, r, 3)) return false;
    *ppg = ((uint32_t)r[0] << 16) | ((uint32_t)r[1] << 8) | r[2];
    return true;
}
