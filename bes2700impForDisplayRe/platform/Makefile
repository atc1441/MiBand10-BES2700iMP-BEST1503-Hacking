obj-y := cmsis/ drivers/ hal/

ifneq ($(DUAL_BOOT),1)
obj-y += main/
endif

ifeq ($(BECO),1)
obj-y += beco/
endif

subdir-ccflags-y += -Iplatform/cmsis/inc -Iplatform/hal -Iutils/hwtimer_list
subdir-ccflags-y += -Iinclude/cmsis_nn -Iinclude/cmsis_dsp