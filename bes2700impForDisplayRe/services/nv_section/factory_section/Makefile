
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iplatform/cmsis/inc \
	-Iplatform/hal \
	-Iplatform/drivers/ana \
	-Iutils/heap \
	-Iutils/crc32 \
	-Iservices/norflash_api

ifeq ($(BT_ADDR_PRINT_CONFIG),1)
CFLAGS_factory_section.o += -DBT_ADDR_OUTPUT_PRINT_NUM=2
else
CFLAGS_factory_section.o += -DBT_ADDR_OUTPUT_PRINT_NUM=6
endif
