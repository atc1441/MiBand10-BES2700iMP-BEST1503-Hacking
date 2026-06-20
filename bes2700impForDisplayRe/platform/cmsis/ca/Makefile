cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S $(cur_dir)*.cpp))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)
obj-y := $(obj-y:.cpp=.o)

ccflags-y := -I$(obj)/../inc/ca

ifeq ($(CHIP_HAS_PSRAMUHS),1)
CFLAGS_system_ARMCA.o += -DCHIP_HAS_PSRAMUHS
endif
