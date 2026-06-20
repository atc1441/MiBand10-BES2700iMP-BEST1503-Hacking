/* touch.c — capacitive touch over I2C. [SCAFFOLD] */
#include "touch.h"
#include "i2c_bus.h"
#include "board_config.h"
#include "hal_gpio.h"

#define BUS   BRD_TOUCH_I2C_ID
#define ADDR  ((uint8_t)BRD_TOUCH_I2C_ADDR)
static void (*s_cb)(const touch_point_t *);

int touch_init(void)
{
    if (BUS < 0 || BRD_TOUCH_I2C_ADDR < 0) return -1;       /* TODO(probe): bus+addr */
    if (!i2c_present(BUS, ADDR)) return -1;
#if (BRD_TOUCH_INT_PIN >= 0)
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)BRD_TOUCH_INT_PIN, HAL_GPIO_DIR_IN, 0);
    /* TODO(probe): attach a falling-edge GPIO IRQ -> touch_read -> s_cb */
#endif
    return 0;
}
bool touch_read(touch_point_t *p)
{
    uint8_t r[6];
    if (BUS < 0 || BRD_TOUCH_I2C_ADDR < 0) return false;
    if (i2c_rd_reg(BUS, ADDR, 0x00 /*TODO: status/coord regs*/, r, 6)) return false;
    p->event   = r[0] >> 6;
    p->x       = ((uint16_t)(r[0] & 0x0F) << 8) | r[1];
    p->y       = ((uint16_t)(r[2] & 0x0F) << 8) | r[3];
    p->gesture = r[4];
    return true;
}
void touch_set_callback(void (*cb)(const touch_point_t *)) { s_cb = cb; }
