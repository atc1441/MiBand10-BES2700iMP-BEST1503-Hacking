
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GOLDEN_BIN_FILE := platform/drivers/graphic/gpu/testcase/tst/tiger/tiger_golden.bin

ifeq ($(wildcard $(cur_dir)$(notdir $(GOLDEN_BIN_FILE))),)
$(error Please copy the golden binary file to $(GOLDEN_BIN_FILE))
endif

$(obj)/tiger_golden_bin.o : $(GOLDEN_BIN_FILE)
AFLAGS_tiger_golden_bin.o += -Wa,-Iplatform/drivers/graphic/gpu/testcase/tst/tiger

ccflags-y := \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/ \
           -Iplatform/drivers/ana/