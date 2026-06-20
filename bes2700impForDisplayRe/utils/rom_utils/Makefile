cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

rulib-y :=

rulib-y += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)$(CHIP)/*.c $(cur_dir)$(CHIP)/*.cpp $(cur_dir)$(CHIP)/*.S))

rulib-y := $(rulib-y:.c=.o)
rulib-y := $(rulib-y:.cpp=.o)
rulib-y := $(rulib-y:.S=.o)

-include $(cur_dir)$(CHIP)/Makefile
$(cur_dir)$(CHIP)/Makefile: ;

ifeq ($(rulib-y),)
rulib-y := rom_utils_dummy.o
endif

ROM_UTILS_LIB := $(CHIP)_librom_utils

$(ROM_UTILS_LIB)-y := $(rulib-y)

obj-y := $(ROM_UTILS_LIB).a

subdir-asflags-y += -Iutils/rom_utils

export AAC_IN_ROM ?= 0
ifeq ($(AAC_IN_ROM),1)
asflags-y += -DAAC_IN_ROM
endif

export CVSD_IN_ROM ?= 0
ifeq ($(CVSD_IN_ROM),1)
asflags-y += -DCVSD_IN_ROM
endif

export LC3_IN_ROM_V2 ?= 0
ifeq ($(LC3_IN_ROM_V2),1)
asflags-y += -DLC3_IN_ROM_V2
endif
