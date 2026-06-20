
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

FONTS_BIN_FILE := platform/drivers/graphic/gpu/testcase/tst/glyphs/evo.bin

ifeq ($(wildcard $(cur_dir)$(notdir $(FONTS_BIN_FILE))),)
$(error Please copy the golden binary file to $(FONTS_BIN_FILE))
endif

$(obj)/evo_bin.o : $(FONTS_BIN_FILE)
AFLAGS_evo_bin.o += -Iplatform/drivers/graphic/gpu/testcase/tst/glyphs/
ccflags-y := \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/ \
           -Iplatform/drivers/graphic/gpu/testcase/tst/Elementary/inc/ \
           -Iplatform/drivers/graphic/gpu/testcase/tst/Elementary/src/ \


