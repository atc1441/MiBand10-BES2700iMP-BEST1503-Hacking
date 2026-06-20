# bes2700imp — MiBand 9 firmware base (BES2700IMP / best1503)

A clean board + driver scaffolding for the **BES2700IMP** SoC in the MiBand 9, built on
the proven best1503 bring-up. It bakes in every verified-on-silicon finding and provides
the framework (HAL hooks + clean APIs + TODOs) for the full firmware:
**display, touchscreen, BLE, OTA, light sensor (ALS), IMU, heart-rate (PPG).**

This folder is the *base layer*. The low-level HAL/RTOS/BT stack is the bring-up SDK in
`../BES2700IHC/2700-src` (which already builds, flashes, boots and runs). bes2700imp is the
clean app/board/driver layer on top — see `build/build_guide.md` for how it links into a
flashable image exactly like the current one.

## Verified hardware facts (measured on real silicon, not from the obfuscated stock FW)
| thing | value | how |
|---|---|---|
| SoC | BES2700IMP, ARMv8-M (star-mc1 / Cortex-M33-like), 170-ball BGA | — |
| Flash (XIP) | `0x2C000000` cached, `0x28000000` non-cached, magic `0xBE57EC1C` @ off 0 | boot + probe |
| **On-chip SRAM** | **~1.4 MB** `0x20000000..0x20160000` (datasheet-confirmed; SDK declares only 512 KB) | full march-test PASS |
| RAM alias | `0x24000000` mirrors `0x20000000` | probe |
| **External PSRAM** | window `~0x34000000` (uninitialised/flaky — needs controller clock+ZQ/DDR calib) | probe |
| UART0 trace | func-2 on **P2_2/P2_3** (=GPIO18/19), **1,152,000 baud** | calibrated |
| Extra GPIOs | the 170-BGA has **P4_4/P4_5** (idx 36/37) beyond the SDK's P0_0..P4_3 | drive-safety scan |
| Critical pads (never re-mux/drive) | P1_2/3, P2_2/3(UART), P3_0..5(QSPI flash), P4_0..3, P4_6+ | drive-safety scan |
| Display panel | **Raydium RM690B0** AMOLED, MIPI-DSI command mode, ~192×490 | Vela strings |

## The 4 chip-specific fixes (vs the stock best1306 SDK) — see board_config.h
1. **CMU teardown skipped** in hal_cmu_module_init_state (best1306 bits reset best1503 flash).
2. **PMU chip-id 0x153** (`ANA_VAL_CHIP_ID`) not 0x136.
3. **`LIBC_ROM=0`** — the best1306 ROM libc jump-table is incompatible (memset thunk faulted).
4. Native-Windows build (MSYS2 make 4.4.1 + response files) — see build guide.

## Layout
```
bes2700imp/
  board/      board_config.h (all findings/map), board_init.[ch] (top-level bring-up order)
  drivers/
    display/  rm690b0.[ch]      AMOLED panel (RM690B0) + DSI/LCDC hooks
    touch/    touch.[ch]        capacitive touch over I2C
    ble/      ble_app.[ch]      BLE GATT app (characteristics) on the BT host
    ota/      ota.[ch]          in-system flash update (hal_norflash, WP-aware) — WORKS
    sensors/  als.[ch] imu.[ch] heartrate.[ch]  I2C sensors
  platform/   sram.[ch] (use the full 1.4 MB), selftest.[ch] (on-SoC HW tests)
  build/      build_guide.md, bes2700imp.mk
```

## Status legend in the code
`[OK]` verified working · `[SCAFFOLD]` API + structure done, hardware values are `TODO(probe)`
(the per-board I2C pads/addresses + sensor power rail + PSRAM/DSI controller bases still
need a logic-analyzer trace of the stock FW or the schematic — everything else is measured).
