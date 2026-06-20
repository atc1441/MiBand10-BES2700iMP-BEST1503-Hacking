cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
    -Iservices/capsensor/ \
    -Iplatform/drivers/capsensor/ \
    -Iapps/sensorhub/


ifeq ($(CAPSENSOR_AT_SENS),1)
KBUILD_CFLAGS += -DCAPSENSOR_AT_SENS
endif

ifeq ($(CAPSENSOR_AT_MCU),1)
KBUILD_CFLAGS += -DCAPSENSOR_AT_MCU
endif
