cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

CAP_CALCULATE_LIB_NAME := libcapsensor_algorithm

ifeq ($(USE_TRACE_ID), 1)
CAP_CALCULATE_LIB_NAME := $(CAP_CALCULATE_LIB_NAME)_log_compressed
endif

$(CAP_CALCULATE_LIB_NAME)-y := $(src_obj)
obj-y := $(CAP_CALCULATE_LIB_NAME).a

ccflags-y +=  \
    -Iconfig/sensor_hub/ \
    -Iplatform/drivers/capsensor/

ifeq ($(CAPSENSOR_TRACE_DEBUG),1)
KBUILD_CFLAGS += -DCAPSENSOR_TRACE_DEBUG
endif
