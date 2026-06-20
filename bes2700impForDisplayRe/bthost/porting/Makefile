cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))


obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o)

ccflags-y += \
	-Iinclude/cmsis_dsp \
    -Iutils/heap
