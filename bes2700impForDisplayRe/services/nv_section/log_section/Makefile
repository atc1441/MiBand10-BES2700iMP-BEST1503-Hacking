
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ifeq ($(TOTA_CRASH_DUMP_TOOL_ENABLE),1)
filtout_c := log_section.c crash_dump_section.c
obj_c := $(filter-out $(filtout_c),$(obj_c))
else
filtout_c := tota_crash_dump_tool.c
obj_c := $(filter-out $(filtout_c),$(obj_c))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iutils/crc32 \
	-Iutils/heap \
	-Iplatform/drivers/ana \
	-Iservices/norflash_api \
	-Iinclude/watchdog	

ifeq ($(TRACE_DUMP2FLASH),1)
ccflags-y += -DTRACE_DUMP2FLASH
endif

ifeq ($(TRACE_CRLF),1)
ccflags-y += -DTRACE_CRLF
endif

