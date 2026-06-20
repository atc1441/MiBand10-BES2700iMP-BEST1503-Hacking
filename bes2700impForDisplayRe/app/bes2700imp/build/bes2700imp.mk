# bes2700imp.mk — build fragment. The bes2700imp app/board/driver layer compiles on top of
# the proven 2700-src HAL/RTOS/BT build. Point BES2700IMP_DIR at this folder and `include`
# this from the 2700-src build (e.g. add to platform/main/Makefile's obj-y), then call
# board_init() from main()/the app thread. Produces the same flashable best1503.bin.

BES2700IMP_DIR ?= $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)

BES2700IMP_SRC := \
    $(BES2700IMP_DIR)/board/board_init.c \
    $(BES2700IMP_DIR)/platform/sram.c \
    $(BES2700IMP_DIR)/drivers/i2c_bus.c \
    $(BES2700IMP_DIR)/drivers/ota/ota.c \
    $(BES2700IMP_DIR)/drivers/display/rm690b0.c \
    $(BES2700IMP_DIR)/drivers/touch/touch.c \
    $(BES2700IMP_DIR)/drivers/ble/ble_app.c \
    $(BES2700IMP_DIR)/drivers/sensors/imu.c \
    $(BES2700IMP_DIR)/drivers/sensors/als.c \
    $(BES2700IMP_DIR)/drivers/sensors/heartrate.c

BES2700IMP_INC := \
    -I$(BES2700IMP_DIR) \
    -I$(BES2700IMP_DIR)/board \
    -I$(BES2700IMP_DIR)/drivers \
    -I$(BES2700IMP_DIR)/platform

# carry the verified config into the compile
BES2700IMP_FLAGS := -DBES2700IMP_BOARD -DDISPLAY_RM690B0

obj-y    += $(BES2700IMP_SRC:.c=.o)
ccflags-y += $(BES2700IMP_INC) $(BES2700IMP_FLAGS)
