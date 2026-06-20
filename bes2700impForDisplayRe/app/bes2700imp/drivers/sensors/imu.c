/* imu.c — IMU/accel over the sensor I2C bus. [SCAFFOLD] */
#include "imu.h"
#include "i2c_bus.h"
#include "board_config.h"

#define BUS   BRD_SENSOR_I2C_ID
#define ADDR  ((uint8_t)BRD_IMU_I2C_ADDR)
/* common WHO_AM_I register candidates (TODO(probe): pin to the real part) */
#define REG_WHOAMI   0x00

int imu_init(void)
{
    if (BUS < 0 || BRD_IMU_I2C_ADDR < 0) return -1;   /* TODO(probe): bus+addr */
    if (!i2c_present(BUS, ADDR)) return -1;
    /* TODO(probe): write CTRL regs (ODR, range, enable) for the identified part */
    return 0;
}
uint8_t imu_whoami(void)
{
    uint8_t v = 0;
    if (BUS >= 0 && BRD_IMU_I2C_ADDR >= 0) i2c_rd_reg(BUS, ADDR, REG_WHOAMI, &v, 1);
    return v;
}
bool imu_read_accel(imu_xyz_t *a)
{
    uint8_t r[6];
    if (BUS < 0 || BRD_IMU_I2C_ADDR < 0) return false;
    if (i2c_rd_reg(BUS, ADDR, 0x02 /*TODO: data reg*/, r, 6)) return false;
    a->x = (int16_t)(r[0] | (r[1] << 8));
    a->y = (int16_t)(r[2] | (r[3] << 8));
    a->z = (int16_t)(r[4] | (r[5] << 8));
    return true;
}
