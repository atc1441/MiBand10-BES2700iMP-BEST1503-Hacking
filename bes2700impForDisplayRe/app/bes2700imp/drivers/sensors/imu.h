/* imu.h — accelerometer / IMU (MiBand9). [SCAFFOLD]
 * I2C accel (step/gesture/wear-detect). Part + address TODO(probe) — typical wrist IMUs
 * answer at 0x18/0x19 (Bosch BMA) or 0x68/0x6A (ICM/LSM). */
#ifndef BES2700IMP_IMU_H
#define BES2700IMP_IMU_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { int16_t x, y, z; } imu_xyz_t;
int  imu_init(void);                 /* power, probe WHO_AM_I, configure ODR/range */
bool imu_read_accel(imu_xyz_t *a);   /* one sample                                 */
uint8_t imu_whoami(void);
#endif
