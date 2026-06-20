cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_s += ../../platform/main/startup_main.S
obj_c += ../../platform/main/dbg_boot_cp.c

PROGRAMMER_INFLASH_LIB_NAME := libprogrammer_inflash

ifeq ($(SECURE_BOOT),1)
PROGRAMMER_INFLASH_LIB_NAME := $(PROGRAMMER_INFLASH_LIB_NAME)_scboot
endif

$(PROGRAMMER_INFLASH_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_cpp:.cpp=.o)

obj-y += $(obj_s:.S=.o) $(PROGRAMMER_INFLASH_LIB_NAME).a

BOOT_LOADER_PRE_INIT_HOOK := 1
NO_NVIC_INIT := 1
# Keep the stock minimal startup (SystemInit/BootInit OFF). Running the full SDK
# init in this stripped programmer context hangs before MAIN_ENTRY (fragile
# section-copy / pmu / analogif / lpu steps). We instead bring up only what the
# HW-UART needs from inside MAIN_ENTRY, with SW-UART markers to trace it.
NO_BOOT_INIT := 1
NO_SYSTEM_INIT := 1

ifeq ($(BOOT_LOADER_ENTRY_HOOK),1)
asflags-y += -DBOOT_LOADER_ENTRY_HOOK
endif
ifeq ($(BOOT_LOADER_PRE_INIT_HOOK),1)
asflags-y += -DBOOT_LOADER_PRE_INIT_HOOK
endif
ifeq ($(BOOT_LOADER_POST_INIT_HOOK),1)
asflags-y += -DBOOT_LOADER_POST_INIT_HOOK
endif
ifeq ($(NO_NVIC_INIT),1)
asflags-y += -DNO_NVIC_INIT
endif
ifeq ($(NO_BOOT_INIT),1)
asflags-y += -DNO_BOOT_INIT
endif
ifeq ($(NO_SYSTEM_INIT),1)
asflags-y += -DNO_SYSTEM_INIT
endif

ccflags-y := \
	-Iutils/boot_struct

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
CFLAGS_programmer_inflash_main.o += -DSINGLE_WIRE_DOWNLOAD
endif
ifeq ($(UNCONDITIONAL_ENTER_SINGLE_WIRE_DOWNLOAD),1)
CFLAGS_programmer_inflash_main.o += -DUNCONDITIONAL_ENTER_SINGLE_WIRE_DOWNLOAD
endif
