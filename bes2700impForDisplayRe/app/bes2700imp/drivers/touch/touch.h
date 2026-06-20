/* touch.h — capacitive touchscreen (MiBand9). [SCAFFOLD]
 * I2C touch controller + an INT line (data-ready). Address/INT pin TODO(probe). The panel
 * is the RM690B0 AMOLED; touch is a separate I2C controller IC (gesture + coordinates). */
#ifndef BES2700IMP_TOUCH_H
#define BES2700IMP_TOUCH_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { uint16_t x, y; uint8_t event; uint8_t gesture; } touch_point_t;
int  touch_init(void);                    /* power, reset, probe, hook the INT GPIO */
bool touch_read(touch_point_t *p);        /* latest point/gesture (call on INT)      */
void touch_set_callback(void (*cb)(const touch_point_t *));
#endif
