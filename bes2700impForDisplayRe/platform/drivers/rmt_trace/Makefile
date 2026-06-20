
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

CFLAGS_rmt_trace_client.o += -Iutils/hwtimer_list

ifneq ($(RMT_TRC_BUF_SIZE),)
CFLAGS_rmt_trace_client.o += -DRMT_TRC_BUF_SIZE=$(RMT_TRC_BUF_SIZE)
endif

ifneq ($(RMT_TRC_ENTRY_CNT),)
CFLAGS_rmt_trace_client.o += -DRMT_TRC_ENTRY_CNT=$(RMT_TRC_ENTRY_CNT)
endif

ifneq ($(RMT_TRC_INTVL_MS),)
CFLAGS_rmt_trace_client.o += -DRMT_TRC_INTVL_MS=$(RMT_TRC_INTVL_MS)
endif

ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
ccflags-y += -DRMT_TRC_IN_MSG_CHAN
endif
