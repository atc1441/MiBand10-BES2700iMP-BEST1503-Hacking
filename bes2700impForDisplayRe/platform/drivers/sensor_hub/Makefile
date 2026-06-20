
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(NO_SENS_IPC_WHEN_OPEN),1)
CFLAGS_sensor_hub.o += -DNO_SENS_IPC_WHEN_OPEN
endif

ifeq ($(SENS_TRC_TO_MCU),1)
CFLAGS_rx_sens_trc.o += -Iplatform/drivers/rmt_trace -DSENS_TRC_TO_MCU
CFLAGS_sensor_hub.o += -DSENS_TRC_TO_MCU
endif

ifeq ($(filter 1,$(SENS_FLASH_BOOT) $(SENSOR_HUB_LOAD_FROM_FLASH_ADDR)),)

ifeq ($(__NuttX__),1)
SENSOR_HUB_TGT ?= vela_sensor
$(obj)/sensor_hub_bin.o : $(NUTTX_ROOT)/$(SENSOR_HUB_TGT).bin
AFLAGS_sensor_hub_bin.o += -DSENSOR_HUB_BIN_NAME=$(SENSOR_HUB_TGT).bin -Wa,-I$(NUTTX_ROOT)
else
SENSOR_HUB_TGT ?= sensor_hub
$(obj)/sensor_hub_bin.o : out/$(SENSOR_HUB_TGT)/$(SENSOR_HUB_TGT).bin
AFLAGS_sensor_hub_bin.o += -DSENSOR_HUB_BIN_NAME=$(SENSOR_HUB_TGT).bin -Wa,-Iout/$(SENSOR_HUB_TGT)
endif

else # SENS_FLASH_BOOT || SENSOR_HUB_LOAD_FROM_FLASH_ADDR

ifeq ($(SENS_FLASH_BOOT),1)
CFLAGS_sensor_hub.o += -DSENS_FLASH_BOOT
endif

EXCLUDE_OBJS += sensor_hub_bin.o
obj-y := $(filter-out $(EXCLUDE_OBJS),$(obj-y))

endif # SENS_FLASH_BOOT || SENSOR_HUB_LOAD_FROM_FLASH_ADDR

ifeq ($(CAPSENSOR_ENABLE),1)
ccflags-y += \
-Iplatform/drivers/ana/ \
-Iplatform/drivers/bt
CFLAGS_sensor_hub.o += -DCAPSENSOR_ENABLE
endif
