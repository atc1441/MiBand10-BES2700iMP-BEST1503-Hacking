cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

obj-y += $(CHIP)/

ccflags-y +=  \
    -Iconfig/sensor_hub/

ifeq ($(CAPSENSOR_HAL_SPI),1)
KBUILD_CFLAGS  += -DCAPSENSOR_HAL_SPI
endif
ifeq ($(CAPSENSOR_GPIO_SPI),1)
KBUILD_CFLAGS  += -DCAPSENSOR_GPIO_SPI
endif
