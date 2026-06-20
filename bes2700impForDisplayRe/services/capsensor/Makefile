cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y += \
    touch_wear_core.o \
    capsensor_debug_server.o \
    capsensor_factory_cal.o \
    capsensor_algorithm/ \
    ../../platform/drivers/capsensor/

ccflags-y +=  \
    -Iplatform/drivers/capsensor/ \
    -Iplatform/drivers/capsensor/best1502p/ \
    -Iservices/capsensor/capsensor_algorithm/ \
    -Iconfig/sensor_hub/ \
    -Iapps/common/

ifeq ($(CAPSENSOR_SPP_SERVER), 1)
ccflags-y += \
    -Iapps/app_spp
endif

ifeq ($(CHIP_SUBSYS),sensor_hub)
ccflags-y +=  \
    -Iapps/sensorhub
endif

ifeq ($(CAPSENSOR_TOUCH),1)
KBUILD_CFLAGS += -DCAPSENSOR_TOUCH
endif

ifeq ($(CAPSENSOR_WEAR),1)
KBUILD_CFLAGS += -DCAPSENSOR_WEAR
endif

ifeq ($(CAPSENSOR_MEDIAN_FILTERING),1)
KBUILD_CFLAGS += -DCAPSENSOR_MEDIAN_FILTERING
endif

ifeq ($(CAPSENSOR_ALPHA_FILTERING),1)
KBUILD_CFLAGS += -DCAPSENSOR_ALPHA_FILTERING
endif

ifeq ($(CAPSENSOR_FAC_CALCULATE),1)
KBUILD_CPPFLAGS += -DCAPSENSOR_FAC_CALCULATE
endif

ifeq ($(CAPSENSOR_READ_DATA_POLLING),1)
KBUILD_CFLAGS += -DCAPSENSOR_READ_DATA_POLLING
endif

ifeq ($(CAPSENSOR_WEAR_USE_ONE_PAD),1)
KBUILD_CFLAGS += -DCAPSENSOR_WEAR_USE_ONE_PAD
endif

