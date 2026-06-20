
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y += -Iapps/common -Iutils/cqueue

ifeq ($(CP_ACCEL_DEBUG),1)
ccflags-y += -DCP_ACCEL_DEBUG
endif

ifeq ($(ASSERT_ON_CP_CRASH),1)
ccflags-y += -DASSERT_ON_CP_CRASH
endif

ifneq ($(CP_CPU_USAGE_INTVL_MS),)
CFLAGS_cp_accel.o += -DCP_CPU_USAGE_INTVL_MS=$(CP_CPU_USAGE_INTVL_MS)
endif

ifneq ($(CP_CPU_USAGE_TRACE_INTVL_MS),)
CFLAGS_cp_accel.o += -DCP_CPU_USAGE_TRACE_INTVL_MS=$(CP_CPU_USAGE_TRACE_INTVL_MS)
endif
