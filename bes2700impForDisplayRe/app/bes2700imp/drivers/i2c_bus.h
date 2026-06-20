/* i2c_bus.h — board sensor/touch I2C bus + register helpers. [SCAFFOLD]
 * Wraps hal_i2c (the controller + pads come from board_config; both are TODO(probe) — the
 * MiBand9 sensors are on board-specific pads and gated by a power rail the stock FW enables,
 * which a blind sweep can't reach, so fill BRD_SENSOR_I2C_ID/PWR_PIN from a logic-analyzer
 * trace). Once those are known every sensor below works through this. */
#ifndef BES2700IMP_I2C_BUS_H
#define BES2700IMP_I2C_BUS_H
#include <stdint.h>
#include <stdbool.h>

void    i2c_bus_init(int i2c_id, int pwr_pin);     /* power the rail + open the controller */
bool    i2c_present(int i2c_id, uint8_t addr);     /* address probe (ACK?)                 */
int     i2c_rd_reg(int i2c_id, uint8_t addr, uint8_t reg, uint8_t *buf, uint32_t len);
int     i2c_wr_reg(int i2c_id, uint8_t addr, uint8_t reg, uint8_t val);

#endif /* BES2700IMP_I2C_BUS_H */
