/* board_init.h — top-level MiBand9 board bring-up. Call board_init() from the app thread
 * once main()/the RTOS is up (after the 2700-src low-level init has run). */
#ifndef BES2700IMP_BOARD_INIT_H
#define BES2700IMP_BOARD_INIT_H

typedef struct {
    unsigned sram_ok    : 1;   /* full 1.4 MB SRAM march PASS */
    unsigned display_ok : 1;
    unsigned touch_ok   : 1;
    unsigned ble_ok     : 1;
    unsigned imu_ok     : 1;
    unsigned als_ok     : 1;
    unsigned hr_ok      : 1;
} board_status_t;

void board_init(board_status_t *st);   /* brings up every subsystem, fills the status flags */

#endif /* BES2700IMP_BOARD_INIT_H */
