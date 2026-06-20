cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj-y := $(obj-y:.c=.o)

ccflags-y := -Iplatform/hal \
	-Iplatform/cmsis \
	-Iservices/audioflinger \
	-Iplatform/drivers/ana \

ifeq ($(SENSOR_HUB_MINIMA),1)
ccflags-y += -DSENSOR_HUB_MINIMA \
	-Iplatform/drivers/minima/Dmip \
	-Iplatform/drivers/minima/minima_avs \
	-Iplatform/drivers/minima/minima_dvfs \
	-Iplatform/drivers/minima/initClockTop \
	-Iplatform/drivers/minima/minima_driver_adaptation \

ifeq ($(MINIMA_TEST),1)
ccflags-y += -DMINIMA_TEST -Itests/minima_test
endif
endif
