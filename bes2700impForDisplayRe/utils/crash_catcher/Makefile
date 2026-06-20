
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(TRACE_CRLF),1)
CFLAGS_HexDump.o += -DTRACE_CRLF
endif

ccflags-y := \
	-Iutils/crash_catcher/include \
	-Iplatform/cmsis/inc/ \
	-Iservices/nv_section/log_section \
	-Irtos/rtx/TARGET_CORTEX_M/ \
	-Iutils/xyzmodem/ \
	-Iinclude/

