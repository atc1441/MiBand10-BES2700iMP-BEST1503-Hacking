cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

HSM_LIB_NAME := libhsm

ifeq ($(USE_TRACE_ID), 1)
HSM_LIB_NAME := $(HSM_LIB_NAME)_log_compressed
endif

$(HSM_LIB_NAME)-y := $(src_obj)
obj-y += $(HSM_LIB_NAME).a