cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := -Iplatform/hal

ifeq ($(SECURE_BOOT),1)
ifneq ($(CHIP_HAS_SECURE_BOOT),1)
$(error CHIP=$(CHIP) does NOT support secure boot!)
endif
ccflags-y += -DSECURE_BOOT
endif

ifneq ($(SECURE_BOOT_CODE_VER),)
ccflags-y += -DSECURE_BOOT_CODE_VER=$(SECURE_BOOT_CODE_VER)
endif
