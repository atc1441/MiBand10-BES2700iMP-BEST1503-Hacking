/* i2c_bus.c — board I2C helpers over hal_i2c. [SCAFFOLD] */
#include "i2c_bus.h"
#include "board_config.h"
#include "hal_i2c.h"
#include "hal_gpio.h"

void i2c_bus_init(int i2c_id, int pwr_pin)
{
    if (pwr_pin >= 0) {                       /* enable the sensor power rail first */
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pwr_pin, HAL_GPIO_DIR_OUT, 1);
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)pwr_pin);
    }
    if (i2c_id < 0) return;                   /* TODO(probe): real bus id */
    struct HAL_I2C_CONFIG_T cfg = {0};
    cfg.mode = HAL_I2C_API_MODE_SIMPLE; cfg.use_sync = 1; cfg.as_master = 1; cfg.speed = 400000;
    hal_i2c_open((enum HAL_I2C_ID_T)i2c_id, &cfg);   /* auto-muxes the controller's pads */
}

bool i2c_present(int i2c_id, uint8_t addr)
{
    if (i2c_id < 0) return false;
    uint8_t b; uint32_t act;
    return hal_i2c_mst_read((enum HAL_I2C_ID_T)i2c_id, addr, &b, 1, &act, 0, 1, 1) == 0;
}

int i2c_rd_reg(int i2c_id, uint8_t addr, uint8_t reg, uint8_t *buf, uint32_t len)
{
    if (i2c_id < 0) return -1;
    buf[0] = reg;   /* reg goes in buf[0]; recv writes `len` bytes back into buf */
    return (int)hal_i2c_recv((enum HAL_I2C_ID_T)i2c_id, addr, buf, 1, len,
                             1 /*restart_after_write*/, 0 /*transfer_id*/, 0 /*handler*/);
}

int i2c_wr_reg(int i2c_id, uint8_t addr, uint8_t reg, uint8_t val)
{
    if (i2c_id < 0) return -1;
    uint8_t b[2] = { reg, val };   /* reg_len=1, value_len=1 */
    return (int)hal_i2c_send((enum HAL_I2C_ID_T)i2c_id, addr, b, 1, 1,
                             0 /*transfer_id*/, 0 /*handler*/);
}
