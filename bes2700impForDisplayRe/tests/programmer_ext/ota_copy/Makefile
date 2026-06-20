cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ifeq ($(OTA_BIN_COMPRESSED),1)
PROGRAMMER_EXT_LIB_NAME := libprogrammer_ext_compressed
else
PROGRAMMER_EXT_LIB_NAME := libprogrammer_ext
endif

ifeq ($(SECURE_BOOT),1)
PROGRAMMER_EXT_LIB_NAME := $(PROGRAMMER_EXT_LIB_NAME)_scboot
endif

$(PROGRAMMER_EXT_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
obj-y += $(PROGRAMMER_EXT_LIB_NAME).a

ccflags-y += \
    -Iplatform/drivers/ana \
    -Itests/programmer/inc \
    -Iutils/boot_struct \
	-Iutils/crc32 \
	-Iutils/crc16 \
    -Iutils/rom_utils

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
ccflags-y += -DSINGLE_WIRE_DOWNLOAD
endif

ifeq ($(PROGRAMMER_WATCHDOG),1)
ccflags-y += -DPROGRAMMER_WATCHDOG
endif

ifeq ($(FORCE_COPY_NEW_IMAGE),1)
ccflags-y += -DFORCE_COPY_NEW_IMAGE
endif
