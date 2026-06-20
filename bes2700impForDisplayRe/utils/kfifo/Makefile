
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-I$(obj)/../../cmsis/inc/

ccflags-y += -DTARGET_LPC1768 -DWORDS_STACK_SIZE=1024 -DOS_TIMERSTKSZ=1024 -D__CORTEX_M4
