/* ble_app.c — BLE GATT app. [SCAFFOLD]
 * Wires into the SDK BLE host (gatt server table). The exact host API differs across the
 * SDK's ble_stack build; the structure here is the bring-up plan: standard 16-bit services
 * so a phone/nRF-Connect can see the band immediately. TODO(integrate): bind these to the
 * 2700-src ble_stack gatt-server registration + advertising calls. */
#include "ble_app.h"
#include "board_config.h"

/* --- GATT layout (bring-up) ---
 * 0x1800 Generic Access      : Device Name, Appearance(Wrist=0x0040)
 * 0x180A Device Information   : Manufacturer, Model="MiBand9", FW rev
 * 0x180F Battery Service      : Battery Level (read+notify)         -> ble_app_set_battery
 * 0x180D Heart Rate Service   : HR Measurement (notify, 0x2A37)     -> ble_app_notify_hr
 * 0xFEE0 (Xiaomi band)        : steps / control-point / auth        -> ble_app_notify_steps
 */
static bool s_connected;
static uint8_t s_batt;

void ble_app_init(void)
{
    /* TODO(integrate): ble_host_init(); register the GATT table above; set adv data
     * (flags + 0xFEE0 + name "MiBand9"); ble_start_advertising(). */
}
void ble_app_set_battery(uint8_t pct)      { s_batt = pct; /* TODO: gatt notify 0x2A19 */ }
void ble_app_notify_hr(uint8_t bpm)        { (void)bpm;    /* TODO: gatt notify 0x2A37 */ }
void ble_app_notify_steps(uint32_t steps)  { (void)steps;  /* TODO: 0xFEE0 char notify  */ }
bool ble_app_is_connected(void)            { return s_connected; }
