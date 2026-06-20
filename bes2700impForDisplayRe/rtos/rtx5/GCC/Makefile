cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifneq ($(filter m33 m55,$(CPU)),)
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)irq_armv8mml.S))
else ifeq ($(CPU),m4)
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)irq_cm4f.S))
else ifeq ($(CPU),a7)
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)irq_ca.S))
else
$(error Invalid CPU: $(CPU))
endif

obj-y := $(obj_s:.S=.o)

subdir-asflags-y += -Irtos/rtx5/GCC
