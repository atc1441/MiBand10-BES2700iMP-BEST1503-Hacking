
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

FONTS_BIN_FILE := platform/drivers/graphic/gpu/testcase/tst/perf_test/landscape.bin

ifeq ($(wildcard $(cur_dir)$(notdir $(FONTS_BIN_FILE))),)
$(error Please copy the golden binary file to $(FONTS_BIN_FILE))
endif

$(obj)/landscape_bin.o : $(FONTS_BIN_FILE)
AFLAGS_landscape_bin.o += -Iplatform/drivers/graphic/gpu/testcase/tst/perf_test/
ccflags-y := \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/
