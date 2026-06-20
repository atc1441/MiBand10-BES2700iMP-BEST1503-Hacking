/* board_init.c — MiBand9 top-level bring-up. Wires every subsystem in a safe order and
 * reports what came up. Each driver fails gracefully (returns !=0) when its TODO(probe)
 * board values aren't filled yet, so this runs cleanly today and lights up incrementally
 * as the per-board pins/addresses are pinned down. */
#include "board_init.h"
#include "board_config.h"
#include "platform/sram.h"
#include "drivers/i2c_bus.h"
#include "drivers/display/rm690b0.h"
#include "drivers/touch/touch.h"
#include "drivers/ble/ble_app.h"
#include "drivers/ota/ota.h"
#include "drivers/sensors/imu.h"
#include "drivers/sensors/als.h"
#include "drivers/sensors/heartrate.h"
#include "hal_trace.h"

#define BI(...)  TRACE(0, "[board] " __VA_ARGS__)

void board_init(board_status_t *st)
{
    board_status_t s = {0};
    BI("BES2700IMP / MiBand9 board init");

    /* 1) claim + verify the full 1.4 MB SRAM (extra ~1.1 MB for fb/heap) */
    s.sram_ok = (sram_full_test() == 0);
    BI("SRAM full-test: %s (free pool %u KB @0x%08x)", s.sram_ok?"PASS":"FAIL",
       (unsigned)(SRAM_FREE_SIZE/1024), SRAM_FREE_BASE);
    sram_pool_reset();

    /* 2) sensor I2C bus + power rail (shared by imu/als/hr); touch is its own bus */
    i2c_bus_init(BRD_SENSOR_I2C_ID, BRD_SENSOR_PWR_PIN);

    /* 3) sensors */
    s.imu_ok = (imu_init() == 0);   BI("IMU  : %s", s.imu_ok?"ok":"todo(probe pins/addr)");
    s.als_ok = (als_init() == 0);   BI("ALS  : %s", s.als_ok?"ok":"todo(probe pins/addr)");
    s.hr_ok  = (hr_init()  == 0);   BI("HR   : %s", s.hr_ok ?"ok":"todo(probe pins/addr)");

    /* 4) touch */
    s.touch_ok = (touch_init() == 0); BI("Touch: %s", s.touch_ok?"ok":"todo(probe pins/addr)");

    /* 5) display (needs best1503 DSI_BASE/LCDC + CMU display clocks — todo(probe)) */
    s.display_ok = (rm690b0_init() == 0);
    if (s.display_ok) rm690b0_fill(0x0000);
    BI("Display RM690B0: %s", s.display_ok?"ok":"todo(probe DSI base/clocks)");

    /* 6) BLE app (advertise + GATT) */
    ble_app_init(); s.ble_ok = 1;   BI("BLE  : adv/gatt scaffold up");

    /* OTA is event-driven (ota_program / ota_begin..finish) — no init needed. */
    BI("init done: sram=%d disp=%d touch=%d ble=%d imu=%d als=%d hr=%d",
       s.sram_ok,s.display_ok,s.touch_ok,s.ble_ok,s.imu_ok,s.als_ok,s.hr_ok);
    if (st) *st = s;
}
