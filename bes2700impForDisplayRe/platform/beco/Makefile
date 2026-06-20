obj-y := libbeco/
ifeq ($(A2DP_VIRTUAL_SURROUND),0)
obj-y += libdsp/
obj-y += beco_nn/
endif
obj-y += beco_dsp/

subdir-ccflags-y := -Iinclude/beco/
subdir-ccflags-y += -Iinclude/beco_nn/
subdir-ccflags-y += -Iinclude/beco_dsp/
subdir-ccflags-y += -Iinclude/cmsis_nn/
subdir-ccflags-y += -Iinclude/cmsis_dsp/