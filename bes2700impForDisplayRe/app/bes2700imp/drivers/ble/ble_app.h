/* ble_app.h — BLE GATT application (MiBand9). [SCAFFOLD]
 * The best1503 BT-host ROM is CLASSIC-only (no BLE host stack) — so BLE runs on the SDK's
 * own BLE host (services/ble_stack in 2700-src) on the controller. This module sets up the
 * GATT server: device-info + a band service (battery, HR notify, steps, control point).
 * Bring-up target: advertise + a few readable/notifiable characteristics first. */
#ifndef BES2700IMP_BLE_APP_H
#define BES2700IMP_BLE_APP_H
#include <stdint.h>
#include <stdbool.h>

void ble_app_init(void);                       /* init host, register GATT, start adv */
void ble_app_set_battery(uint8_t pct);
void ble_app_notify_hr(uint8_t bpm);           /* HR measurement notify (0x2A37)      */
void ble_app_notify_steps(uint32_t steps);
bool ble_app_is_connected(void);

#endif /* BES2700IMP_BLE_APP_H */
