# Build & flash — bes2700imp

bes2700imp is the clean board/driver layer; the low-level HAL/RTOS/BT stack + the working
build/flash toolchain is the bring-up SDK in `../BES2700IHC/2700-src` (which already boots
and runs on the device). You build one flashable `best1503.bin` that includes this layer.

## 1. Carry the 4 verified fixes (already in 2700-src)
- `config/best1503/target.mk`: `LIBC_ROM ?= 0`
- skip best1306 CMU teardown in `hal_cmu_module_init_state` (best1503)
- `ANA_VAL_CHIP_ID = 0x153` in `hal_analogif_best1503.c`
- native-Windows build via MSYS2 make 4.4.1 (response files)

## 2. Pull this layer into the build
In `2700-src/platform/main/Makefile` add near the other `obj-y`:
```make
BES2700IMP_DIR := $(realpath ../../../bes2700imp)
include $(BES2700IMP_DIR)/build/bes2700imp.mk
```
and call the board init from `main()` (after the trace UART + RTOS are up), e.g. next to the
existing self-test hook:
```c
#if defined(BES2700IMP_BOARD)
    extern void board_init(void *); board_init(0);
#endif
```
Enable the display HAL backing once DSI_BASE/LCDC_BASE are filled:
add `DISPLAY_RM690B0=1` (sets `CONFIG_DISPLAY_DSI_BEST1503`, see 2700-src DISPLAY_BRINGUP.md).

## 3. Build
```bash
export PATH=/c/gcc-arm-none-eabi/bin:/c/MinGW/bin:/c/msys64/usr/bin:$PATH
cd ../BES2700IHC/2700-src
/c/msys64/usr/bin/make -f Makefile T=best1503 CHIP=best1503 DEBUG=1 MAGIC_NUM=1 \
    LIBC_ROM=0 SHELL=/usr/bin/sh -j4
```
Output: `out/best1503/best1503.bin` (ready to flash).

## 4. Flash + log
```bash
python flash_and_log.py            # boot checkpoints @921600
python flash_and_selftest.py       # SDK trace @1,152,000 (board init + self-test)
```

## What lights up today vs needs probing
- **OK now:** boots+runs, full 1.4 MB SRAM, in-system flash/OTA, trace, BLE scaffold.
- **Needs the per-board values (logic-analyzer on the stock FW / schematic):** I2C bus id +
  pads + sensor power rail (touch/IMU/ALS/HR addresses), the best1503 DSI_BASE/LCDC_BASE +
  CMU display clocks, and the PSRAM controller base/calib. All marked `TODO(probe)` in
  `board/board_config.h` — fill those and the matching subsystem comes up with no code change.
